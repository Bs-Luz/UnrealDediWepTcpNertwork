// Universal Cross-Platform Voice Chat MeoPlay Copyright (C) 2021 MeoPlay <contact@meoplay.com> All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Core.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "MicrophoneSpeakComponent.h"
#include "PlayerVoiceChatActor.generated.h"

enum EVoiceChat_Mode
{
	VC_GLOBAL_NORADIO,
	VC_GLOBAL_RADIO,
	VC_PROXIMITY_NORADIO,
	VC_PROXIMITY_RADIO
};


// frames allowed 400 200 100 50 25
UENUM(BlueprintType)
enum class EOpusFramePerSec : uint8 {
	OPUS_FPS_400       UMETA(DisplayName = "OPUS_FPS_400"),
	OPUS_FPS_200       UMETA(DisplayName = "OPUS_FPS_200"),
	OPUS_FPS_100       UMETA(DisplayName = "OPUS_FPS_100"),
	OPUS_FPS_50        UMETA(DisplayName = "OPUS_FPS_50"),
	OPUS_FPS_25		 UMETA(DisplayName = "OPUS_FPS_25")
};


// global delegates for monitoring Voice Chat actors creation / deletion
DECLARE_DYNAMIC_DELEGATE_OneParam(FDelegateNewVoiceChatActor, const APlayerVoiceChatActor*, VoiceChatActor);
DECLARE_DYNAMIC_DELEGATE_OneParam(FDelegateDeleteVoiceChatActor, const APlayerVoiceChatActor*, VoiceChatActor);


UCLASS()
class UNIVERSALVOICECHATPRO_API APlayerVoiceChatActor : public AActor
{
	GENERATED_BODY()

		DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerNameReceived, FString, name);
		DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerMicrophoneOnReceived, bool, IsMicrophoneOn);

public:
	

	static APlayerVoiceChatActor* myPlayerVoiceActor; // local player reference
	static FString selectedAudioInputDevice; // selected microphone hardware on Windows
	static float rawMicrophoneGain; // volume gain
	static bool muteAll; // mute all players
	static float tickRateUpdateActor; // mute all players

	// constructor
	APlayerVoiceChatActor();

	/* the root scene component*/
	UPROPERTY()
		USceneComponent *RootSceneComponent;
		
	/* the component used to speak and receive voice*/
	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing = RepNotifyMicComp, Category = "VoiceChatUniversal")
		UMicrophoneSpeakComponent* microphoneSpeakComponent;

	/* delegate to clean up this actor*/
	UFUNCTION(Category = "VoiceChatUniversal")
		void DelegateEndPlayOwner(AActor* act, EEndPlayReason::Type EndPlayReason);
	
	/* the owner of this actor, used for muting a player for example */
	UPROPERTY(Transient, replicated, BlueprintReadOnly, Category = "VoiceChatUniversal")
		APlayerState *ownerPlayerState;
	
	UPROPERTY(Transient, replicated, EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"), Category = "VoiceChatUniversal")
		int32 idVoiceChat;

	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing = RepNotifyPlayerName, Category = "VoiceChatUniversal")
		FString playerName = "Player";

	UPROPERTY(Transient, ReplicatedUsing = RepNotifyIsMicrophoneOn, EditAnywhere, BlueprintReadWrite, Category = "VoiceChatUniversal")
		bool isMicrophoneOn = false;

	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing = RepNotifyVoiceVolume, Category = "VoiceChatUniversal")
		float voiceVolume = 5.0f;

	UPROPERTY(BlueprintReadWrite, Transient, replicated, Category = "VoiceChatUniversal")
		TArray<int32> radioChannelSubscribed;

	UPROPERTY(Transient, ReplicatedUsing = RepNotifyAttenuationAsset, EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"), Category = "VoiceChatUniversal")
		FString pathToAttenuationAsset = "";

	UPROPERTY(Transient, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"), Category = "VoiceChatUniversal")
		bool ServerPerformAntiCheat = false;
	
	UPROPERTY(Transient, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"), Category = "VoiceChatUniversal")
		bool AntiCheatAllowUseProximity = true;

	UPROPERTY(Transient, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"), Category = "VoiceChatUniversal")
		bool AntiCheatAllowUseGlobal = true;

	UPROPERTY(Transient, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"), Category = "VoiceChatUniversal")
		float AntiCheatMaxProximityRange = 1000;

	UPROPERTY(BlueprintAssignable, Category = "VoiceChatUniversal")
		FPlayerNameReceived OnPlayerNameReceived;

	UPROPERTY(BlueprintAssignable, Category = "VoiceChatUniversal")
		FPlayerMicrophoneOnReceived OnIsMicrophoneOnReceived;

	UFUNCTION(Category = "VoiceChatUniversal")
		void RepNotifyMicComp();

	UFUNCTION(Category = "VoiceChatUniversal")
		void RepNotifyAttenuationAsset();

	UFUNCTION(Category = "VoiceChatUniversal")
		void RepNotifyVoiceVolume();

	UFUNCTION(Category = "VoiceChatUniversal")
		void RepNotifyPlayerName();
	
	UFUNCTION(Category = "VoiceChatUniversal")
		void RepNotifyIsMicrophoneOn();

	UFUNCTION(Category = "VoiceChatUniversal")
		void muteAudio(bool isMute);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		void ServerSetAllowUseGlobal(bool _allowUseGlobal);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		void ServerAddChannel(int32 channelToAdd);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		void ServerRemoveChannel(int32 channelToRemove);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		void ServerSetAttenuation(bool enableAttenuation, FString _pathToAttenuationAsset);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		void ServerSetAllowUseProximity(bool _allowUseRange);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		void ServerSetMaxProximityRange(float _maxProximityRange);

	// client ask radio/team channel add
	UFUNCTION(Server, Reliable, Category = "VoiceChatUniversal")
		void RPCClientSetPlayerName(const FString& name);

	// client replicate its microphone status ( on / off )
	UFUNCTION(Server, Reliable, Category = "VoiceChatUniversal")
		void RPCClientSetIsMicrophoneOn(bool _isMicrophoneOn);

	// client replicate its microphone volume
	UFUNCTION(Server, Reliable, Category = "VoiceChatUniversal")
		void RPCClientSetMicrophoneVolume(float volume);

	// server update audio pos
	UFUNCTION(NetMulticast, Unreliable, Category = "VoiceChatUniversal")
	void RPCServerUpdatePosAudioComp(FVector worldPos, FRotator worldRotation);
	   
	// client ask radio/team channel add
	UFUNCTION(Server, Reliable, Category = "VoiceChatUniversal")
		void RPCClientAskAddChannel(int32 channelToAdd);

	// client ask radio/team channel remove
	UFUNCTION(Server, Reliable, Category = "VoiceChatUniversal")
		void RPCClientAskRemoveChannel(int32 channelToRemove);


protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
	virtual void Tick(float DeltaTime) override;
	virtual void OnRep_Owner() override;
};


/* bp library */
UCLASS()
class UNIVERSALVOICECHATPRO_API UUniversalVoiceChat : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	
public:

	// reference to local actor client side
	static APlayerVoiceChatActor *GetMyPlayerVoiceActor();
	// delegate creation / end any voice chat actor
	static FDelegateNewVoiceChatActor delegateStaticNewVoiceChatActor;
	static FDelegateDeleteVoiceChatActor delegateStaticDeleteVoiceChatActor;

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static APlayerVoiceChatActor* VoiceChatGetMyLocalPlayerVoiceChat();

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static bool VoiceChatInitAudioVoiceChatQuality(int32 _sampleRate = 48000, int32 _numChannels = 1, EOpusFramePerSec _opusFramePerSec = EOpusFramePerSec::OPUS_FPS_200);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static bool VoiceChatWasInitAudioVoiceChatQuality();


	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static bool VoiceChatStartSpeak(bool _shouldHearMyOwnVoice = true, bool isGlobal = true, int radioChannel = 0, bool useProximity = false, float maxProximityRange = 0);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static bool VoiceChatStopSpeak();
	
	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static bool VoiceChatSetMicrophoneVolume(float volume);
	
	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal", meta = (WorldContext = "WorldContextObject"))
		static void VoiceChatLocalMuteSomeone(const UObject* WorldContextObject, APlayerState *playerToMute, bool shouldMute);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal", meta = (WorldContext = "WorldContextObject"))
		static bool VoiceChatLocalIsMutedSomeone(const UObject* WorldContextObject, APlayerState *playerToCheckMute);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static bool  VoiceChatCheckRegisteredToChannel(int32 channelToCheck);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static bool VoiceChatAddChannel(int32 channelToAdd);
	
	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static bool VoiceChatRemoveChannel(int32 channelToRemove);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static bool  VoiceChatHasMicrophonePermission();

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static void  VoiceChatAskMicrophonePermission();

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static void VoiceChatSetRawMicrophoneGain(float gain);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static bool VoiceChatIsSpeaking();

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static void VoiceChatGetAudioDevicesList(TArray<FString>& outDevices);

	// set audio device used
	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static void VoiceChatSetHardwareAudioInput(FString audioInputDeviceName);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal", meta = (WorldContext = "WorldContextObject"))
		static APlayerVoiceChatActor* VoiceChatGetActorFromPlayerState(const UObject* WorldContextObject, APlayerState* fromPlayerState);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal", meta = (WorldContext = "WorldContextObject"))
		static float VoiceChatGetMicrophoneRuntimeVolumeFromPlayerState(const UObject* WorldContextObject, APlayerState* fromPlayerState);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static void RegisterCallbackNewVoiceChatActor(const FDelegateNewVoiceChatActor& Delegate);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static void RegisterCallbackDeleteVoiceChatActor(const FDelegateDeleteVoiceChatActor& Delegate);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static bool VoiceChatSetPlayerName(FString name);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static bool VoiceChatGetMuteAllPlayers();

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static void VoiceChatSetMuteAllPlayers(bool _muteAll);

	UFUNCTION(BlueprintCallable, Category = "VoiceChatUniversal")
		static void VoiceChatSetDefaultTickRateUpdateLocation(float tickRate);
};
