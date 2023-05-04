// Copyright Epic Games, Inc. All Rights Reserved.

#include "UDWTNGameMode.h"
#include "UDWTNCharacter.h"
#include "UObject/ConstructorHelpers.h"

struct DedicatedServerInfo
{
    FString DediIp[16];
    int32 DediPort;
    int32 DediPlayerNum;
};

AUDWTNGameMode::AUDWTNGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	/*ServerToInfoClient();
	SendServerInfoToTcpServer();*/
    SendDedicatedServerInfo(IP,Port,PlayerNum);
}

void AUDWTNGameMode::SendDedicatedServerInfo(const FString& ServerIP, int32 ServerPort, int32 Player)
{
    // 소켓 서브시스템 초기화
    const auto SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    check(SocketSubsystem != nullptr);

    // TCP 소켓 생성
    Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("default"), true);

    // TCP 서버에 연결
    TSharedRef<FInternetAddr> RemoteAddr = SocketSubsystem->CreateInternetAddr();
    bool bIsValid;
    RemoteAddr->SetIp(*ServerIP, bIsValid);
    check(bIsValid);
    RemoteAddr->SetPort(ServerPort);
    check(Socket->Connect(*RemoteAddr));

    // 데디케이티드 서버 정보를 전송
    DedicatedServerInfo ServerInfo;
    FMemory::Memzero(ServerInfo);
    FMemory::Memcpy(ServerInfo.DediIp, TCHAR_TO_UTF8(*ServerIP), FMath::Min(ServerIP.Len() + 1, 16));
    ServerInfo.DediPort = ServerPort;
    ServerInfo.DediPlayerNum = Player;
    int32 Sent = 0;
    int32 BytesToSend = sizeof(ServerInfo);
    bool bSuccessful = false;
    do
    {
        bSuccessful = Socket->Send((uint8*)&ServerInfo + Sent, BytesToSend - Sent, Sent);
        if (!bSuccessful)
        {
            break;
        }
        Sent += BytesToSend;
    } while (Sent < BytesToSend);

    Socket->Close();
}

//void AUDWTNGameMode::ServerToInfoClient()
//{
//	ClientSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("DefaultSocket"), false); //소켓 생성
//	ClientAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();//IPv4 주소체계를 사용하는 IP주소와 PORT 번호 저장
//
//	IP = TEXT("127.0.0.1"); //TCP 서버의 IP
//	Port = 10101; //TCP 서버의 PORT
//	FIPv4Address Temp; //직접 문자열로 전달해주는것보다 변환해서 파싱 하는게 안전하기 때문에 파싱한 IP값이 들어갈 저장소
//	FIPv4Address::Parse(IP, Temp); // IP값을 파싱해서 Temp에 저장
//	ClientAddress->SetIp(Temp.Value); //파싱된 IP 저장
//	ClientAddress->SetPort(Port); //PORT 저장 PORT는 정수임으로 파싱 노필요
//
//	if (ClientSocket->Connect(*ClientAddress)) //서버 연결 시도
//	{
//		UE_LOG(LogTemp, Warning, TEXT("클라이언트 접속 성공"));
//	}
//	else
//	{
//		UE_LOG(LogTemp, Error, TEXT("클라이언트 접속 실패"))
//	}
//
//	int PushData = 3;
//	uint8_t buffer[sizeof(int)];
//	memcpy(buffer, &PushData, sizeof(int));
//
//	int32 ByteSent = 0;
//	if (ClientSocket->GetConnectionState() == ESocketConnectionState::SCS_Connected)
//	{
//		ClientSocket->Send(buffer, sizeof(int), ByteSent);
//	}
//}
//
//void AUDWTNGameMode::SendServerInfoToTcpServer()
//{
//	// TCP 서버의 IP와 PORT
//	FString ServerIP = TEXT("127.0.0.1");
//	int32 ServerPort = 10101;
//
//	// TCP 서버와 연결을 수립
//	TSharedPtr<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
//	bool bIsValid;
//	RemoteAddress->SetIp(*ServerIP, bIsValid);
//	RemoteAddress->SetPort(ServerPort);
//
//	ClientAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
//	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), true);
//
//	bool bConnectSuccess = Socket->Connect(*RemoteAddress);
//
//	if (!bConnectSuccess)
//	{
//		UE_LOG(LogTemp, Error, TEXT("Failed to connect to TCP server."));
//		return;
//	}
//
//	// 데디케이티드 서버 정보를 버퍼에 담기
//	FString ServerName = TEXT("My Dedicated Server");
//	FString ServerAddress = TEXT("127.0.0.1");
//	int32 ServerPortNumber = 7777;
//
//	FString ServerInfoString = FString::Printf(TEXT("%s,%s,%d"), *ServerName, *ServerAddress, ServerPortNumber);
//	TArray<uint8> ServerInfoBytes;
//	FTCHARToUTF8 ConvertedString(*ServerInfoString);
//	ServerInfoBytes.Append((uint8*)ConvertedString.Get(), ConvertedString.Length());
//	ServerInfoBytes.Add('\0');
//
//	// TCP 서버에 데디케이티드 서버 정보 전송
//	int32 BytesSent = 0;
//	bool bSentSuccess = Socket->Send(ServerInfoBytes.GetData(), ServerInfoBytes.Num(), BytesSent);
//	if (!bSentSuccess || BytesSent != ServerInfoBytes.Num())
//	{
//		UE_LOG(LogTemp, Error, TEXT("Failed to send server info to TCP server."));
//	}
//	else
//	{
//		UE_LOG(LogTemp, Warning, TEXT("Server info has been sent to TCP server."));
//	}
//
//	// TCP 서버와 연결 종료
//	Socket->Close();
//}

