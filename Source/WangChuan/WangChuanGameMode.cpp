// Copyright Epic Games, Inc. All Rights Reserved.

#include "WangChuanGameMode.h"
#include "WangChuanCharacter.h"
#include "UObject/ConstructorHelpers.h"

AWangChuanGameMode::AWangChuanGameMode()
{
	// 使用项目的角色 Blueprint 作为默认 Pawn。
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
