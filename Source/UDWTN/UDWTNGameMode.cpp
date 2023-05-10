// Copyright Epic Games, Inc. All Rights Reserved.

#include "UDWTNGameMode.h"
#include "UDWTNCharacter.h"
#include "UObject/ConstructorHelpers.h"

struct DedicatedServerInfo
{
    char DediIp[16];
    int DediPort;
    int DediPlayerNum;
};

AUDWTNGameMode::AUDWTNGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	/*UMG_Login 위젯 생성하기*/
	static ConstructorHelpers::FClassFinder<UUDWTN_ClientWidget> LoginWidgetClass(TEXT("/Game/ThirdPerson/Blueprints/UDWTN_ClientLoginWidget"));
	if (LoginWidgetClass.Succeeded())
	{
		UUDWTN_ClientWidget* LoginWidget = CreateWidget<UUDWTN_ClientWidget>(GetWorld(), LoginWidgetClass.Class);

		// UMG_Login 위젯 화면에 추가하기
		if (LoginWidget != nullptr)
		{
			LoginWidget->AddToViewport();

			UE_LOG(LogTemp, Warning, TEXT("위젯 뜸"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("위젯 안뜸"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("그냥 다시 해"));
	}

	//스레드생성
	//FTestThread* Sender = new FTestThread(FString::Printf(TEXT("%i"), GetNumPlayers()));
	SocketThread* Sender = new SocketThread();
	FRunnableThread* Thread = FRunnableThread::Create(Sender, TEXT("SendThread"));

	/*ServerToInfoClient();*/
	/*SendServerInfoToTcpServer();*/
    
    
}

void AUDWTNGameMode::ServerToInfoClient()
{
	ClientSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("DefaultSocket"), false);
	ClientAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	FString IP = TEXT("192.168.0.53");	//tcp ip
	int32 port = 10101;	//tcp port
	FIPv4Address TemporaryAddr;		//임시 저장소
	FIPv4Address::Parse(IP, TemporaryAddr); //ip를 temporaryAddr에 변환해서 넣고
	ClientAddress->SetPort(port);	//port넣음
	ClientAddress->SetIp(TemporaryAddr.Value);	//ip넣음

	if (ClientSocket->Connect(*ClientAddress))
	{
		UE_LOG(LogTemp, Warning, TEXT("Client : TCP서버 접속 성공"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Client : TCP서버 접속 실패"));
	}

	int MyClientServer = 2;
	uint8_t buffer[sizeof(int)];
	memcpy(buffer, &MyClientServer, sizeof(int));

	int32 bytesSent = 0;
	if (ClientSocket->GetConnectionState() == ESocketConnectionState::SCS_Connected)
	{
		ClientSocket->Send(buffer, sizeof(int), bytesSent);
	}


	struct ServerData
	{
		uint16_t ServerPort;
		char IP[16];
	};
	ServerData SData;
	uint8_t DBBuffer[1024] = { 0, };
	int32 bytes = 0;
	int ClientServerRecvBytes = 0;

	UE_LOG(LogTemp, Warning, TEXT("TCP DB Data...."));

	if (ClientSocket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromSeconds(5)))
	{
		ClientServerRecvBytes = ClientSocket->Recv(DBBuffer, sizeof(DBBuffer), bytes);

		if (ClientServerRecvBytes < 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Error"));
		}
		else if (ClientSocket == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("No Data"));
		}
		else
		{
			//DBBuffer[ClientServerRecvBytes] = '\0';
			UE_LOG(LogTemp, Warning, TEXT("In Data"));
			memcpy(&SData, DBBuffer, sizeof(ServerData));

			UE_LOG(LogTemp, Warning, TEXT("%d"), SData.ServerPort);
			FString str(SData.IP);
			str += ":";
			str += FString::FromInt(SData.ServerPort);

			UE_LOG(LogTemp, Warning, TEXT("%s"), *str);

			GetWorld()->GetFirstPlayerController()->ClientTravel(str, TRAVEL_Absolute);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No"));
	}
}

void AUDWTNGameMode::SendServerInfoToTcpServer()
{
	// TCP 서버의 IP와 PORT
	FString ServerIP = TEXT("192.168.0.53");
	int32 ServerPort = 10101;

	// TCP 서버와 연결을 수립
	TSharedPtr<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	bool bIsValid;
	RemoteAddress->SetIp(*ServerIP, bIsValid);
	RemoteAddress->SetPort(ServerPort);

	ClientAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), true);

	bool bConnectSuccess = Socket->Connect(*RemoteAddress);

	if (!bConnectSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to connect to TCP server."));
		return;
	}

	// 데디케이티드 서버 정보를 버퍼에 담기
	FString ServerName = TEXT("My Dedicated Server");
	FString ServerAddress = TEXT("127.0.0.1");
	int32 ServerPortNumber = 7777;

	FString ServerInfoString = FString::Printf(TEXT("%s,%s,%d"), *ServerName, *ServerAddress, ServerPortNumber);
	TArray<uint8> ServerInfoBytes;
	FTCHARToUTF8 ConvertedString(*ServerInfoString);
	ServerInfoBytes.Append((uint8*)ConvertedString.Get(), ConvertedString.Length());
	ServerInfoBytes.Add('\0');

	// TCP 서버에 데디케이티드 서버 정보 전송
	int32 BytesSent = 0;
	bool bSentSuccess = Socket->Send(ServerInfoBytes.GetData(), ServerInfoBytes.Num(), BytesSent);
	if (!bSentSuccess || BytesSent != ServerInfoBytes.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to send server info to TCP server."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Server info has been sent to TCP server."));
	}

	// TCP 서버와 연결 종료
	Socket->Close();
}

void AUDWTNGameMode::InitGameState()
{
}

void AUDWTNGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UE_LOG(LogTemp, Warning, TEXT("클라이언트 접속함"));

	InfoThread();

	////voice
	//FActorSpawnParameters paramSpawn;
	//paramSpawn.Owner = NewPlayer;
	//APlayerVoiceChatActor* VoiceChatActor = GetWorld()->SpawnActor<APlayerVoiceChatActor>(FVector(0, 0, 0), FRotator(0, 0, 0), paramSpawn);

	//if (VoiceChatActor != NULL)
	//{
	//	VoiceChatActor->ServerSetAttenuation(true, FString("/Script/Engine.SoundAttenuation'/Game/NewSoundAttenuation.NewSoundAttenuation'"));
	//	VoiceChatActor->ServerPerformAntiCheat = false;
	//	VoiceChatActor->AntiCheatAllowUseProximity = true;
	//	VoiceChatActor->AntiCheatAllowUseGlobal = true;
	//	VoiceChatActor->AntiCheatMaxProximityRange = 1000.0f;
	//	VoiceChatActor->ServerSetAllowUseGlobal(true);
	//}
}

void AUDWTNGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	InfoThread();
}

void AUDWTNGameMode::InfoThread()
{
	//서버의 포트번호 / IP 불러오는 방법
	FString URLString = GetWorld()->URL.ToString();
	FURL URL(NULL, *URLString, ETravelType::TRAVEL_Absolute);
	int32 ServerPort;
	if (URL.Valid && URL.Port > 0)
	{
		ServerPort = URL.Port;
	}

	// Get IP Address
	FString IPAddress = "";
	bool canBind = false;
	TSharedRef<FInternetAddr>LocalIp =
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, canBind);

	if (LocalIp->IsValid())
	{
		IPAddress = LocalIp->ToString(false);
		UE_LOG(LogTemp, Error, TEXT("GameMode IP : %s"), *IPAddress);
	}

	//싱글톤으로 정보보내기
	//IP 인자값 = 로컬서버를 사용하지 않으면 URL.Host 를 쓰면 된다.
	UE_LOG(LogTemp, Error, TEXT("GameMode Port : %d"), ServerPort);
	USingletonData::GetInstance()->SetData(GetNumPlayers(), static_cast<int>(ServerPort), *IPAddress);
	USingletonData::GetInstance()->SerBool(true);
}
