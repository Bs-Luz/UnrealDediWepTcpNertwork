// Fill out your copyright notice in the Description page of Project Settings.


#include "SocketThread.h"

SocketThread::SocketThread()
{
}

SocketThread::~SocketThread()
{
	//스레드 삭제
	if (thread)
	{
		UE_LOG(LogTemp, Warning, TEXT("스레드 종료."));
		delete thread;
	}
}

bool SocketThread::Init()
{
	bRunThread = true;

	//TCP소켓 생성 연결
	//현재 플렛폼에 맞춰서 소켓을 가져오는 시스템 (새로운 소켓 생성, 소켓 이름, 비차단모드로 설정할지?)
	DediServerSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("DefaultSocket"), false);
	DediServerAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	FString IP = TEXT("127.0.0.1");	//tcp ip
	int32 port = 10101;	//tcp port
	FIPv4Address TemporaryAddr;		//임시 저장소
	FIPv4Address::Parse(IP, TemporaryAddr); //ip를 temporaryAddr에 변환해서 넣고
	DediServerAddress->SetPort(port);	//port넣음
	DediServerAddress->SetIp(TemporaryAddr.Value);	//ip넣음

	if (DediServerSocket->Connect(*DediServerAddress))
	{
		UE_LOG(LogTemp, Warning, TEXT("TCP서버 접속 성공"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TCP서버 접속 실패"));
	}

	return true;
}

uint32 SocketThread::Run()
{
	UE_LOG(LogTemp, Warning, TEXT("스레드 보내기."));

	while (bRunThread)
	{
		//소켓과 연결이 끊어지면 종료
		if (DediServerSocket->GetConnectionState() != ESocketConnectionState::SCS_Connected) break;

		////받아온 메세지를 uint8 데이터로 변환
		//FString MessageToSend = FString::FromInt(UUSingleton::GetInstance()->GetData().PlayerNum);	//int > fstring변환
		//MessageToSend += FString::FromInt(UUSingleton::GetInstance()->GetData().ServerPort);
		//MessageToSend += UUSingleton::GetInstance()->GetData().IP;

		//uint8* Data = (uint8*)TCHAR_TO_UTF8(*MessageToSend);
		//int32 BytesSent = 0;

		//if (DediServerSocket->GetConnectionState() == ESocketConnectionState::SCS_Connected)
		//{
		//	DediServerSocket->Send(Data, MessageToSend.Len(), BytesSent);
		//	//DediServerSocket->Send(Data, sizeof(Data), BytesSent);	//데이터전송
		//	bRunThread = false;
		//}

		bOneDediTCPInfo = USingletonData::GetInstance()->GetBool();

		if (bOneDediTCPInfo)
		{
			//20230424 받아온 메세지를 Mydata 구조체에 저장하고 구조체를 전송
			MyData data;
			data.MyServer = (int)1;
			data.PlayerNum = USingletonData::GetInstance()->GetData().PlayerCount;
			data.ServerPort = USingletonData::GetInstance()->GetData().ServerPort;

			UE_LOG(LogTemp, Warning, TEXT("Thread Port : %d"), USingletonData::GetInstance()->GetData().ServerPort);
			UE_LOG(LogTemp, Warning, TEXT("Thread IP : %s"), *USingletonData::GetInstance()->GetData().IP);

			//FString 변수에 저장된 IP 를 char 로 변환하고 strncpy_s 함수를 사용해 char 배열에 복사
			strncpy_s(data.IP, sizeof(data.IP), TCHAR_TO_ANSI(*USingletonData::GetInstance()->GetData().IP), _TRUNCATE);
			//패킹된 데이터를 보내기 위해 uint8 배열에 복사
			uint8_t buffer[sizeof(MyData)];
			memcpy(buffer, &data, sizeof(MyData));
			//데이터 전송
			int32 bytesSent = 0;
			if (DediServerSocket->GetConnectionState() == ESocketConnectionState::SCS_Connected)
			{
				DediServerSocket->Send(buffer, sizeof(MyData), bytesSent);

				USingletonData::GetInstance()->SerBool(false);
				bOneDediTCPInfo = false;
			}

			FPlatformProcess::Sleep(0.1f);	//스레드를 잠시 멈춘다
			//정보를 보냈으니 스레드 종료
			//if (!bRunThread)
			//{
			//	this->Stop();
			//	break;
			//}
		}
	}

	return 0;
}

void SocketThread::Stop()
{
	bRunThread = false;

	UE_LOG(LogTemp, Warning, TEXT("스레드 종료."));
}
