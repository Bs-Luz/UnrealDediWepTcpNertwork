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
