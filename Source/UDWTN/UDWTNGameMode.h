// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UDWTN_ClientWidget.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerVoiceChatActor.h"

//소켓통신을 위한 헤더
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Net/UnrealNetwork.h"
#include "Networking/Public/Networking.h"
#include "SocketThread.h"
#include "SingletonData.h"
#include "UDWTNGameMode.generated.h"


UCLASS(minimalapi)
class AUDWTNGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	AUDWTNGameMode();

	void ServerToInfoClient();

	void SendServerInfoToTcpServer();

	virtual void InitGameState() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	void InfoThread();

private:
	TSharedPtr<FInternetAddr> ClientAddress; //FInternetAddr은 언리얼에서 ip주소와 port 번호를 저장하는 클래스
	class FSocket* ClientSocket;
	class FSocket* Socket;

	//FString IP;
	//int32 Port;
	//int32 PlayerNum;

};