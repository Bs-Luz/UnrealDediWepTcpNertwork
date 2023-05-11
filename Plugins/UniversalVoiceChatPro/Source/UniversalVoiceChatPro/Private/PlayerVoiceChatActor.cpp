// Universal Cross-Platform Voice Chat MeoPlay Copyright (C) 2021 MeoPlay <contact@meoplay.com> All Rights Reserved.
#include "PlayerVoiceChatActor.h"
#include "IMediaCaptureSupport.h"
#include "IMediaModule.h"
#include "Misc/MediaBlueprintFunctionLibrary.h"
#include "MediaCaptureSupport.h"

/* singleton local player */
APlayerVoiceChatActor *APlayerVoiceChatActor::myPlayerVoiceActor = NULL;
/* selected audio device */
FString APlayerVoiceChatActor::selectedAudioInputDevice = "";
/* microphone gain */
float APlayerVoiceChatActor::rawMicrophoneGain = 1.0f;
/* mute all player */
bool APlayerVoiceChatActor::muteAll = false;
/* update actor location and rotation tickrate */
float APlayerVoiceChatActor::tickRateUpdateActor = 0.2f;

FDelegateNewVoiceChatActor UUniversalVoiceChat::delegateStaticNewVoiceChatActor;
FDelegateDeleteVoiceChatActor UUniversalVoiceChat::delegateStaticDeleteVoiceChatActor;

APlayerVoiceChatActor *UUniversalVoiceChat::GetMyPlayerVoiceActor() {
	return APlayerVoiceChatActor::myPlayerVoiceActor;
}

APlayerVoiceChatActor::APlayerVoiceChatActor() : Super() {
	bReplicates = true;
	NetCullDistanceSquared = 50000000000.0f;
	bNetLoadOnClient = true;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;
	SetActorTickInterval(0.2f);
	//root component
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootSceneComponent;

}

// server add mic comp on start
void APlayerVoiceChatActor::BeginPlay() {
	Super::BeginPlay();

	if (APlayerVoiceChatActor::tickRateUpdateActor > 0.0f) {
		UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor BeginPlay set tickrate %f"), APlayerVoiceChatActor::tickRateUpdateActor);
		SetActorTickInterval(APlayerVoiceChatActor::tickRateUpdateActor);
	}

	UUniversalVoiceChat::delegateStaticNewVoiceChatActor.ExecuteIfBound(this);

	// spawn micro comp par server
	if (GetWorld()->GetNetMode() == ENetMode::NM_ListenServer || GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer || GetWorld()->GetNetMode() == ENetMode::NM_Standalone) {
		microphoneSpeakComponent = NewObject<UMicrophoneSpeakComponent>(this, UMicrophoneSpeakComponent::StaticClass());		
		microphoneSpeakComponent->RegisterComponent();		
		microphoneSpeakComponent->SetIsReplicated(true);
		if (GetWorld()->GetNetMode() == NM_Standalone) {
			SetOwner(UGameplayStatics::GetPlayerController(this, 0));
			OnRep_Owner(); // not called in standalone, so call it manually
		}
		
		if (GetWorld()->GetNetMode() == NM_ListenServer) {
			bool OwnedLocally = IsOwnedBy(UGameplayStatics::GetPlayerController(this, 0));
			if (OwnedLocally) {
				UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor OnRep_Owner OwnedLocally server %d"), GetWorld()->GetNetMode() == ENetMode::NM_ListenServer || GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer || GetWorld()->GetNetMode() == ENetMode::NM_Standalone);
				myPlayerVoiceActor = this;
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor OnRep_Owner not OwnedLocally server %d"), GetWorld()->GetNetMode() == ENetMode::NM_ListenServer || GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer || GetWorld()->GetNetMode() == ENetMode::NM_Standalone);
			}
		}

		
		if (this->microphoneSpeakComponent) {
			UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor BeginPlay try set attenuation"));
			if (!pathToAttenuationAsset.IsEmpty()) {
				this->microphoneSpeakComponent->setAttenuationAssetPath(true, pathToAttenuationAsset);
			}
			else {
				this->microphoneSpeakComponent->setAttenuationAssetPath(false, pathToAttenuationAsset);
			}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor BeginPlay try set attenuation his->microphoneSpeakComponent null"));
		}

		FScriptDelegate Delegate;
		Delegate.BindUFunction(this, "DelegateEndPlayOwner");
		GetOwner()->OnEndPlay.AddUnique(Delegate);

		
		if (GetOwner()->IsA(APlayerController::StaticClass())) {
			APlayerController* controllerOwner = ((APlayerController*)GetOwner());
			ownerPlayerState = controllerOwner->GetPlayerState<APlayerState>();
		}
	}	
}

// clean static if owned locally on destroy
void APlayerVoiceChatActor::EndPlay(const EEndPlayReason::Type EndPlayReason){

	Super::EndPlay(EndPlayReason);

	UUniversalVoiceChat::delegateStaticDeleteVoiceChatActor.ExecuteIfBound(this);

	//UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor OnDestroy"));
	bool OwnedLocally = myPlayerVoiceActor == this;
	if (OwnedLocally) {
		UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor EndPlay OwnedLocally server %d"), GetWorld()->GetNetMode() == NM_ListenServer || GetWorld()->GetNetMode() == NM_DedicatedServer);
		// make sure to cut audio
		UUniversalVoiceChat::VoiceChatStopSpeak();
		// NULL the singleton
		myPlayerVoiceActor = NULL;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor EndPlay not OwnedLocally server %d"), GetWorld()->GetNetMode() == NM_ListenServer || GetWorld()->GetNetMode() == NM_DedicatedServer);
	}


	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerVoiceChatActor::StaticClass(), FoundActors);
	UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor EndPlay total actodleft %d"), FoundActors.Num());
}

// replication UE4 interne
void APlayerVoiceChatActor::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APlayerVoiceChatActor, microphoneSpeakComponent);
	DOREPLIFETIME(APlayerVoiceChatActor, idVoiceChat);
	DOREPLIFETIME(APlayerVoiceChatActor, playerName);
	DOREPLIFETIME(APlayerVoiceChatActor, ownerPlayerState);
	DOREPLIFETIME(APlayerVoiceChatActor, radioChannelSubscribed);
	DOREPLIFETIME(APlayerVoiceChatActor, pathToAttenuationAsset);
	DOREPLIFETIME(APlayerVoiceChatActor, voiceVolume);
	DOREPLIFETIME(APlayerVoiceChatActor, isMicrophoneOn);
	
	UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor GetLifetimeReplicatedProps"));
}

void APlayerVoiceChatActor::RepNotifyMicComp() {

	UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor::RepNotifyMicComp"));
	if (this->microphoneSpeakComponent) {
		if (!pathToAttenuationAsset.IsEmpty()) {
			this->microphoneSpeakComponent->setAttenuationAssetPath(true, pathToAttenuationAsset);
		}
		else {
			this->microphoneSpeakComponent->setAttenuationAssetPath(false, pathToAttenuationAsset);
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor RepNotifyMicComp his->microphoneSpeakComponent null"));
	}
}

void APlayerVoiceChatActor::RepNotifyAttenuationAsset() {
	
	UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor::RepNotifyAttenuationAsset"));
	//if (!pathToAttenuationAsset.IsEmpty()) {
	//	UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor RepNotifyAttenuationAsset server ? %d %s"), GetWorld()->IsServer(), *pathToAttenuationAsset);
	//}
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("RepNotifyAttenuationAsset %s empty? %d"), *GetName(), pathToAttenuationAsset.IsEmpty()));

	if (this->microphoneSpeakComponent) {
		if (!pathToAttenuationAsset.IsEmpty()) {
			this->microphoneSpeakComponent->setAttenuationAssetPath(true, pathToAttenuationAsset);
		}
		else {
			this->microphoneSpeakComponent->setAttenuationAssetPath(false, pathToAttenuationAsset);
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor RepNotifyAttenuationAsset his->microphoneSpeakComponent null"));
	}	
}

void APlayerVoiceChatActor::RepNotifyVoiceVolume() {
	UE_LOG(LogTemp, Warning, TEXT("RepNotifyVoiceVolume"));
	if (this->microphoneSpeakComponent) {
		this->microphoneSpeakComponent->SetVoiceVolume(voiceVolume);
	}
}

void APlayerVoiceChatActor::RepNotifyPlayerName() {
	UE_LOG(LogTemp, Warning, TEXT("RepNotifyPlayerName %s"), *playerName);
	OnPlayerNameReceived.Broadcast(playerName);
}

void APlayerVoiceChatActor::RepNotifyIsMicrophoneOn() {
	UE_LOG(LogTemp, Warning, TEXT("RepNotifyIsMicrophoneOn %d"), isMicrophoneOn);
	OnIsMicrophoneOnReceived.Broadcast(isMicrophoneOn);
}

// automatically remove voice chat if owner is destroyed
void APlayerVoiceChatActor::DelegateEndPlayOwner(AActor* act, EEndPlayReason::Type EndPlayReason) {
	UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor DelegateEndPlayOwner"));
	if (IsValid(this)) {
		UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor DelegateEndPlayOwner start destroy this"));
		Destroy();
	}
}

// event called when server has set owner
void APlayerVoiceChatActor::OnRep_Owner() {
	Super::OnRep_Owner();
	//UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor OnRep_Owner"));
	
	bool OwnedLocally = IsOwnedBy(UGameplayStatics::GetPlayerController(this, 0)) && GetNetMode() != NM_DedicatedServer;
	if (OwnedLocally) {
		UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor OnRep_Owner OwnedLocally server %d"), GetWorld()->GetNetMode() == ENetMode::NM_ListenServer || GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer || GetWorld()->GetNetMode() == ENetMode::NM_Standalone);
		myPlayerVoiceActor = this;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor OnRep_Owner not OwnedLocally server %d"), GetWorld()->GetNetMode() == ENetMode::NM_ListenServer || GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer || GetWorld()->GetNetMode() == ENetMode::NM_Standalone);
	}
}


void APlayerVoiceChatActor::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	//UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor server ? %d ownerPlayerState NULL ? %d "), GetWorld()->IsServer(), ownerPlayerState == NULL);


	if (GetWorld() != NULL && (GetWorld()->GetNetMode() == ENetMode::NM_ListenServer || GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer || GetWorld()->GetNetMode() == ENetMode::NM_Standalone)) {
		APlayerController *ownerPC = (APlayerController *)GetOwner();
		if (ownerPC != NULL) {
			if (ownerPC->GetPawn() != NULL) {
				SetActorLocation(ownerPC->GetPawn()->GetActorLocation());
				SetActorRotation(ownerPC->GetPawn()->GetActorRotation());
				RPCServerUpdatePosAudioComp(GetActorLocation(), GetActorRotation());
				//UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor  GetActorLocation %s"), *(ownerPC->GetCharacter()->GetActorLocation().ToString()) );
			}
		}
	}
	
}


/* server set, pathToAttenuationAsset is automatically replicated */
void APlayerVoiceChatActor::ServerSetAttenuation(bool enableAttenuation, FString _pathToAttenuationAsset) {
	if (enableAttenuation) {
		pathToAttenuationAsset = _pathToAttenuationAsset;
	}
	else {
		pathToAttenuationAsset = "";
	}
}


void APlayerVoiceChatActor::ServerSetAllowUseProximity(bool _allowUseRange) {
	AntiCheatAllowUseProximity = _allowUseRange;
}



void APlayerVoiceChatActor::ServerSetAllowUseGlobal(bool _allowUseGlobal) {
	AntiCheatAllowUseGlobal = _allowUseGlobal;
}



void  APlayerVoiceChatActor::ServerSetMaxProximityRange(float _maxProximityRange) {
	AntiCheatMaxProximityRange = _maxProximityRange;
}



/* mute audio locally, no server involved in this */
void APlayerVoiceChatActor::muteAudio(bool isMute) {
	if (microphoneSpeakComponent != NULL) {
		microphoneSpeakComponent->muteAudio(isMute);
	}	
}

/* server set, radioChannelSubscribed is automatically replicated */
void APlayerVoiceChatActor::ServerAddChannel(int32 channelToAdd) {
	if (!radioChannelSubscribed.Contains(channelToAdd)) {
		radioChannelSubscribed.Add(channelToAdd);
	}
	//UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor::AddChannel total %d"), radioChannelSubscribed.Num());
}

/* server set, radioChannelSubscribed is automatically replicated */
void APlayerVoiceChatActor::ServerRemoveChannel(int32 channelToRemove) {
	radioChannelSubscribed.Remove(channelToRemove);
	//UE_LOG(LogTemp, Warning, TEXT("APlayerVoiceChatActor::RemoveChannel total %d"), radioChannelSubscribed.Num());
}

/* RPC Server => Client */
void APlayerVoiceChatActor::RPCServerUpdatePosAudioComp_Implementation(FVector worldPos, FRotator worldRotation) {
	SetActorLocation(worldPos);
	SetActorRotation(worldRotation);
}

/* RPC Client => Server */
void APlayerVoiceChatActor::RPCClientAskAddChannel_Implementation(int32 channelToAdd) {
	ServerAddChannel(channelToAdd);
}

/* RPC Client => Server */
void APlayerVoiceChatActor::RPCClientAskRemoveChannel_Implementation(int32 channelToRemove) {
	ServerRemoveChannel(channelToRemove);
}

void APlayerVoiceChatActor::RPCClientSetMicrophoneVolume_Implementation(float volume) {
	voiceVolume = volume;
	if (microphoneSpeakComponent != NULL) {
		microphoneSpeakComponent->SetVoiceVolume(volume);
	}
}

void APlayerVoiceChatActor::RPCClientSetPlayerName_Implementation(const FString& name) {
	playerName = name;
}

void APlayerVoiceChatActor::RPCClientSetIsMicrophoneOn_Implementation(bool _isMicrophoneOn) {
	isMicrophoneOn = _isMicrophoneOn;
}

/************** BLUEPRINT LIBRARY ***************************************/


// get your local player voice chat, you need this if you want to receive raw microphone data
APlayerVoiceChatActor* UUniversalVoiceChat::VoiceChatGetMyLocalPlayerVoiceChat() {
	if (APlayerVoiceChatActor::myPlayerVoiceActor == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("UUniversalVoiceChat::GetMyLocalPlayerVoiceChat is NULL"));
	}
	return APlayerVoiceChatActor::myPlayerVoiceActor;
}

bool UUniversalVoiceChat::VoiceChatInitAudioVoiceChatQuality(int32 _sampleRate, int32 _numChannels, EOpusFramePerSec opusFramePerSec) {
	if (APlayerVoiceChatActor::myPlayerVoiceActor == NULL || APlayerVoiceChatActor::myPlayerVoiceActor->microphoneSpeakComponent == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("UUniversalVoiceChat::initVoiceChatQuality myPlayerVoiceActor is NULL"));
		return false;
	}
	int32 opusfps = 200;
	switch (opusFramePerSec) {
		case EOpusFramePerSec::OPUS_FPS_400:
			opusfps = 400;
			break;
		case EOpusFramePerSec::OPUS_FPS_200:
			opusfps = 200;
			break;
		case EOpusFramePerSec::OPUS_FPS_100:
			opusfps = 100;
			break;
		case EOpusFramePerSec::OPUS_FPS_50:
			opusfps = 50;
			break;
		case EOpusFramePerSec::OPUS_FPS_25:
			opusfps = 25;
			break;
		default:
			break;
	};

	return APlayerVoiceChatActor::myPlayerVoiceActor->microphoneSpeakComponent->initAudioResources(_sampleRate, _numChannels, opusfps);
}

bool UUniversalVoiceChat::VoiceChatWasInitAudioVoiceChatQuality() {
	if (APlayerVoiceChatActor::myPlayerVoiceActor == NULL || APlayerVoiceChatActor::myPlayerVoiceActor->microphoneSpeakComponent == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("UUniversalVoiceChat::wasInitAudioVoiceChatQuality is NULL"));
		return false;
	}
	return APlayerVoiceChatActor::myPlayerVoiceActor->microphoneSpeakComponent->getWasInitAudioResources();
}

// use this to start speaking (local owned client side only)
bool UUniversalVoiceChat::VoiceChatStartSpeak(bool _shouldHearMyOwnVoice, bool isGlobal, int radioChannel, bool useRange, float maxRange) {

	bool res = false;

	if (APlayerVoiceChatActor::myPlayerVoiceActor != NULL && IsValid(APlayerVoiceChatActor::myPlayerVoiceActor) && APlayerVoiceChatActor::myPlayerVoiceActor->microphoneSpeakComponent) {
		UE_LOG(LogTemp, Warning, TEXT("VoiceChatStartSpeak ok _shouldHearMyOwnVoice %d  isGlobal %d radioChannel %d"), _shouldHearMyOwnVoice, isGlobal, radioChannel);
		res = APlayerVoiceChatActor::myPlayerVoiceActor->microphoneSpeakComponent->startSpeaking(_shouldHearMyOwnVoice, isGlobal, radioChannel, useRange, maxRange);
		if(res) APlayerVoiceChatActor::myPlayerVoiceActor->RPCClientSetIsMicrophoneOn(true);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("VoiceChatStartSpeak error"));
		res = false;
	}
	return res;
}

// use this to stop speaking (local owned client side only)
bool UUniversalVoiceChat::VoiceChatStopSpeak() {

	if (APlayerVoiceChatActor::myPlayerVoiceActor != NULL && IsValid(APlayerVoiceChatActor::myPlayerVoiceActor) && APlayerVoiceChatActor::myPlayerVoiceActor->microphoneSpeakComponent != nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("VoiceChatStopSpeak ok"));
		APlayerVoiceChatActor::myPlayerVoiceActor->microphoneSpeakComponent->endSpeaking();
		APlayerVoiceChatActor::myPlayerVoiceActor->RPCClientSetIsMicrophoneOn(false);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("VoiceChatStopSpeak error"));
		return false;
	}
	return true;
}

bool UUniversalVoiceChat::VoiceChatSetMicrophoneVolume(float volume) {

	if (APlayerVoiceChatActor::myPlayerVoiceActor != NULL && IsValid(APlayerVoiceChatActor::myPlayerVoiceActor)) {
		UE_LOG(LogTemp, Warning, TEXT("VoiceChatSetMicrophoneVolume ok"));
		APlayerVoiceChatActor::myPlayerVoiceActor->RPCClientSetMicrophoneVolume(volume);

		if (APlayerVoiceChatActor::myPlayerVoiceActor->microphoneSpeakComponent != NULL) {
			APlayerVoiceChatActor::myPlayerVoiceActor->microphoneSpeakComponent->SetVoiceVolume(volume);
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("VoiceChatSetMicrophoneVolume error"));
		return false;
	}
	return true;
}



// use this to mute someone (local owned client side only)
void UUniversalVoiceChat::VoiceChatLocalMuteSomeone(const UObject* WorldContextObject, APlayerState *playerToMute, bool shouldMute) {
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull), APlayerVoiceChatActor::StaticClass(), FoundActors);
	UE_LOG(LogTemp, Warning, TEXT(" VoiceChatMuteSomeone total actor found %d"), FoundActors.Num());
	for (int i = 0; i < FoundActors.Num(); i++) {
		if (((APlayerVoiceChatActor*)FoundActors[i])->ownerPlayerState == playerToMute) {
			((APlayerVoiceChatActor*)FoundActors[i])->muteAudio(shouldMute);
			break;
		}
	}
}

// check if we muted someone locally (local owned client side only)
bool UUniversalVoiceChat::VoiceChatLocalIsMutedSomeone(const UObject* WorldContextObject, APlayerState *playerToCheckMute) {
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull), APlayerVoiceChatActor::StaticClass(), FoundActors);
	UE_LOG(LogTemp, Warning, TEXT(" VoiceChatLocalIsMutedSomeone total actor found %d"), FoundActors.Num());
	for (int i = 0; i < FoundActors.Num(); i++) {
		if (((APlayerVoiceChatActor*)FoundActors[i])->ownerPlayerState == playerToCheckMute && ((APlayerVoiceChatActor*)FoundActors[i])->microphoneSpeakComponent) {
			return ((APlayerVoiceChatActor*)FoundActors[i])->microphoneSpeakComponent->isMutedLocalSetting;
		}
	}
	return false;
}


bool  UUniversalVoiceChat::VoiceChatCheckRegisteredToChannel(int32 channelToCheck) {
	if (APlayerVoiceChatActor::myPlayerVoiceActor != NULL && IsValid(APlayerVoiceChatActor::myPlayerVoiceActor)) {
		UE_LOG(LogTemp, Warning, TEXT("VoiceChatCheckRegisteredToChannel ok"));
		return APlayerVoiceChatActor::myPlayerVoiceActor->radioChannelSubscribed.Contains(channelToCheck);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("VoiceChatCheckRegisteredToChannel error"));
		return false;
	}
}


bool UUniversalVoiceChat::VoiceChatAddChannel(int32 channelToAdd) {
	if (APlayerVoiceChatActor::myPlayerVoiceActor != NULL && IsValid(APlayerVoiceChatActor::myPlayerVoiceActor)) {
		UE_LOG(LogTemp, Warning, TEXT("VoiceChatAddChannel ok"));
		APlayerVoiceChatActor::myPlayerVoiceActor->RPCClientAskAddChannel(channelToAdd);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("VoiceChatAddChannel error"));
		return false;
	}
	return true;
}

bool UUniversalVoiceChat::VoiceChatRemoveChannel(int32 channelToRemove) {
	if (APlayerVoiceChatActor::myPlayerVoiceActor != NULL && IsValid(APlayerVoiceChatActor::myPlayerVoiceActor)) {
		UE_LOG(LogTemp, Warning, TEXT("VoiceChatRemoveChannel ok"));
		APlayerVoiceChatActor::myPlayerVoiceActor->RPCClientAskRemoveChannel(channelToRemove);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("VoiceChatRemoveChannel error"));
		return false;
	}
	return true;
}

bool  UUniversalVoiceChat::VoiceChatHasMicrophonePermission() {

#if PLATFORM_IOS
	bool res = false;
	switch ([[AVAudioSession sharedInstance] recordPermission]) {
		case AVAudioSessionRecordPermissionGranted:
			res = true;
			break;
		case AVAudioSessionRecordPermissionDenied:
			res = false;
			break;
		case AVAudioSessionRecordPermissionUndetermined:
			res = false;
			break;
		default:
			res = false;
			break;
	}
	return res;
#endif
#if PLATFORM_ANDROID
	return UAudioCaptureAndroid::AndroidHasPermission();
#endif
#if PLATFORM_WINDOWS	
	return true;
#endif
	return true;
}

void  UUniversalVoiceChat::VoiceChatAskMicrophonePermission() {
#if PLATFORM_IOS
	[[AVAudioSession sharedInstance] requestRecordPermission:^ (BOOL granted) {
			if (granted)
			{

			}
			else
			{

			}

	}];

#endif
#if PLATFORM_ANDROID
	UAudioCaptureAndroid::AndroidAskPermission();
#endif
#if PLATFORM_WINDOWS	
	
#endif
}

void UUniversalVoiceChat::VoiceChatSetRawMicrophoneGain(float gain) {
	APlayerVoiceChatActor::rawMicrophoneGain = gain;
}

bool UUniversalVoiceChat::VoiceChatIsSpeaking() {
	bool res = false;

	if (APlayerVoiceChatActor::myPlayerVoiceActor != NULL && IsValid(APlayerVoiceChatActor::myPlayerVoiceActor)) {
		UE_LOG(LogTemp, Warning, TEXT("VoiceChatIsSpeaking ok"));
		res = APlayerVoiceChatActor::myPlayerVoiceActor->microphoneSpeakComponent->isSpeakingLocalSetting;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("VoiceChatIsSpeaking error"));
	}

	return res;
}

void UUniversalVoiceChat::VoiceChatGetAudioDevicesList(TArray<FString>& outDevices) {

#if PLATFORM_WINDOWS
	TArray<FMediaCaptureDeviceInfo> DeviceInfos;
	MediaCaptureSupport::EnumerateAudioCaptureDevices(DeviceInfos);
	for (const FMediaCaptureDeviceInfo& DeviceInfo : DeviceInfos)
	{
		outDevices.Add(DeviceInfo.DisplayName.ToString());
	}
#endif

}

void UUniversalVoiceChat::VoiceChatSetHardwareAudioInput(FString audioInputDeviceName) {
	APlayerVoiceChatActor::selectedAudioInputDevice = audioInputDeviceName;
}

APlayerVoiceChatActor* UUniversalVoiceChat::VoiceChatGetActorFromPlayerState(const UObject* WorldContextObject, APlayerState* fromPlayerState) {
	APlayerVoiceChatActor* res = nullptr;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull), APlayerVoiceChatActor::StaticClass(), FoundActors);

	for (int i = 0; i < FoundActors.Num(); i++) {
		if (((APlayerVoiceChatActor*)FoundActors[i])->ownerPlayerState == fromPlayerState) {
			res = (APlayerVoiceChatActor*)FoundActors[i];
			break;
		}
	}

	return res;
}

float UUniversalVoiceChat::VoiceChatGetMicrophoneRuntimeVolumeFromPlayerState(const UObject* WorldContextObject, APlayerState* fromPlayerState) {
	float res = 0.0f;

	// optimization poll actors every 2 sec
	static float lastVolumeRetrieveActors = 0.0f;
	static TArray<AActor*> FoundVolumeActors;

	UWorld* worldco = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	
	if (worldco) {
		if (worldco->TimeSeconds - lastVolumeRetrieveActors > 2.0f) {
			UGameplayStatics::GetAllActorsOfClass(worldco, APlayerVoiceChatActor::StaticClass(), FoundVolumeActors);
			lastVolumeRetrieveActors = worldco->TimeSeconds;
		}

		for (int i = 0; i < FoundVolumeActors.Num(); i++) {
			if (((APlayerVoiceChatActor*)FoundVolumeActors[i])->ownerPlayerState == fromPlayerState) {
				APlayerVoiceChatActor* actorFound = (APlayerVoiceChatActor*)FoundVolumeActors[i];
				res = (actorFound != nullptr && actorFound->microphoneSpeakComponent != nullptr) ? actorFound->microphoneSpeakComponent->latestVolume : 0.0f;
				break;
			}
		}
	}

	return res;
}

bool UUniversalVoiceChat::VoiceChatSetPlayerName(FString name) {

	if (APlayerVoiceChatActor::myPlayerVoiceActor != NULL && IsValid(APlayerVoiceChatActor::myPlayerVoiceActor)) {
		UE_LOG(LogTemp, Warning, TEXT("VoiceChatSetPlayerName ok"));
		APlayerVoiceChatActor::myPlayerVoiceActor->RPCClientSetPlayerName(name);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("VoiceChatSetPlayerName error"));
		return false;
	}
	return true;
}

bool UUniversalVoiceChat::VoiceChatGetMuteAllPlayers() {
	return APlayerVoiceChatActor::muteAll;
}

void UUniversalVoiceChat::VoiceChatSetMuteAllPlayers(bool _muteAll) {
	APlayerVoiceChatActor::muteAll = _muteAll;
}

void UUniversalVoiceChat::VoiceChatSetDefaultTickRateUpdateLocation(float tickRate) {
	APlayerVoiceChatActor::tickRateUpdateActor = tickRate;
}

void UUniversalVoiceChat::RegisterCallbackNewVoiceChatActor(const FDelegateNewVoiceChatActor& Delegate) {
	delegateStaticNewVoiceChatActor = Delegate;
}

void UUniversalVoiceChat::RegisterCallbackDeleteVoiceChatActor(const FDelegateDeleteVoiceChatActor& Delegate) {
	delegateStaticDeleteVoiceChatActor = Delegate;
}

