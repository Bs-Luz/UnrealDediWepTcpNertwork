// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

//소켓통신을 위한 헤더
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "UDWTNGameMode.generated.h"


UCLASS(minimalapi)
class AUDWTNGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AUDWTNGameMode();

	void ServerToInfoClient();



private:
	TSharedPtr<FInternetAddr> ClientAddress; //FInternetAddr은 언리얼에서 ip주소와 port 번호를 저장하는 클래스
	class FSocket* ClientSocket;

	FString IP;
	int32 Port;

};



