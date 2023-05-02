// Copyright Epic Games, Inc. All Rights Reserved.

#include "UDWTNGameMode.h"
#include "UDWTNCharacter.h"
#include "UObject/ConstructorHelpers.h"

AUDWTNGameMode::AUDWTNGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AUDWTNGameMode::ServerToInfoClient()
{
	ClientSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("DefaultSocket"), false); //소켓 생성
	ClientAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();//IPv4 주소체계를 사용하는 IP주소와 PORT 번호 저장

	IP = TEXT("127.0.0.1"); //TCP 서버의 IP
	Port = 10101; //TCP 서버의 PORT
	FIPv4Address Temp; //직접 문자열로 전달해주는것보다 변환해서 파싱 하는게 안전하기 때문에 파싱한 IP값이 들어갈 저장소
	FIPv4Address::Parse(IP, Temp); // IP값을 파싱해서 Temp에 저장
	ClientAddress->SetIp(Temp.Value); //파싱된 IP 저장
	ClientAddress->SetPort(Port); //PORT 저장 PORT는 정수임으로 파싱 노필요

	if (ClientSocket->Connect(*ClientAddress)) //서버 연결 시도
	{
		UE_LOG(LogTemp, Warning, TEXT("클라이언트 접속 성공"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("클라이언트 접속 실패"))
	}

	int PushData = 3;
	uint8_t buffer[sizeof(int)];
	memcpy(buffer, &PushData, sizeof(int));

	int32 ByteSent = 0;
	if (ClientSocket->GetConnectionState() == ESocketConnectionState::SCS_Connected)
	{
		ClientSocket->Send(buffer, sizeof(int), ByteSent);
	}
}