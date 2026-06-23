// Copyright Epic Games, Inc. All Rights Reserved.

#include "WangChuanGameMode.h"
#include "WangChuanCharacter.h"
#include "UObject/ConstructorHelpers.h"

AWangChuanGameMode::AWangChuanGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
