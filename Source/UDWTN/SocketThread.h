// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "IPAddress.h"
#include "SingletonData.h"
#include "CoreMinimal.h"

#pragma pack(push, 1)
struct MyData
{
	int MyServer;
	int PlayerNum;
	uint16_t ServerPort;
	char IP[16];
};
#pragma pack(pop)

/**
 * 
 */
class UDWTN_API SocketThread : public FRunnable

{
public:
	SocketThread();
	~SocketThread();

	bool Init() override;
	virtual uint32 Run() override;	//데이터를 보내는
	virtual void Stop() override;	//스레드 중지


private:
	FString Message;
	TSharedPtr<FInternetAddr> DediServerAddress;
	class FSocket* DediServerSocket;
	class FRunnableThread* thread;

	bool bRunThread = false;
	bool bOneDediTCPInfo = false;
};