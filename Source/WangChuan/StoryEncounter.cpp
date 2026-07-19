// Fill out your copyright notice in the Description page of Project Settings.


#include "StoryEncounter.h"

#include "Components/SceneComponent.h"
#include "Engine/Engine.h"

#include "GhostEnemy.h"
#include "StoryAnchor.h"
#include "WCStoryNPC.h"

AStoryEncounter::AStoryEncounter()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot =
		CreateDefaultSubobject<USceneComponent>(
			TEXT("SceneRoot")
		);

	SetRootComponent(SceneRoot);

	SetActorEnableCollision(false);
}

void AStoryEncounter::BeginPlay()
{
	Super::BeginPlay();

	bRequiredEnemyDefeated = false;
	bEncounterCompleted = false;

	if (!RequiredEnemy)
	{
		ShowEncounterDebugMessage(
			FString::Printf(
				TEXT(
					"%s, RequiredEnemy is not assigned."
				),
				*EncounterID.ToString()
			),
			FColor::Red
		);
		return;
	}

	/*
	* 只绑定当前 Encounter 指定的敌人。
	* 
	* 其他普通敌人死亡不会推进该 Encounter。
	*/
	RequiredEnemy->OnEnemyDefeated
		.AddUniqueDynamic(
			this,
			&AStoryEncounter::HandleRequiredEnemyDefeated
		);

	ShowEncounterDebugMessage(
		FString::Printf(
			TEXT(
				"% s: Listening to RequiredEnemy : % s"
			),
			*EncounterID.ToString(),
			*GetNameSafe(RequiredEnemy)
		),
		FColor::Cyan
	);

	if (!StoryNPC)
	{
		ShowEncounterDebugMessage(
			FString::Printf(
				TEXT(
					"%s, StoryNPC is not assigned."
				),
				*EncounterID.ToString()
			),
			FColor::Yellow
		);
	}
	
	if (!NextStoryAnchor)
	{
		ShowEncounterDebugMessage(
			FString::Printf(
				TEXT(
					"%s, Next Story Anchor "
					"is not assigned."
				),
				*EncounterID.ToString()
			),
			FColor::Yellow
		);
	}
}

void AStoryEncounter::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	/*
	* Encounter 被销毁或关卡结束时解除绑定。
	*/
	if (IsValid(RequiredEnemy))
	{
		RequiredEnemy->OnEnemyDefeated
			.RemoveDynamic(
				this,
				&AStoryEncounter::HandleRequiredEnemyDefeated
			);
	}

	Super::EndPlay(EndPlayReason);
}

bool AStoryEncounter::GetIsRequiredEnemyDefeated() const
{
	return bRequiredEnemyDefeated;
}

bool AStoryEncounter::GetIsEncounterCompleted() const
{
	return bEncounterCompleted;
}

void AStoryEncounter::HandleRequiredEnemyDefeated(
	AGhostEnemy* DefeatedEnemy)
{
	/*
	* 防止 Encounter 重复处理同一次结果。
	*/
	if (bRequiredEnemyDefeated)
	{
		return;
	}

	/*
	* 只能由 RequiredEnemy 推进流程。
	*/
	if (!DefeatedEnemy ||
		DefeatedEnemy != RequiredEnemy)
	{
		return;
	}

	bRequiredEnemyDefeated = true;

	ShowEncounterDebugMessage(
		FString::Printf(
			TEXT(
				"%s\n"
				"Required Enemy Defeated: True\n"
				"Encounter Compeleted: False\n"
				"Defeated Enemy: %s"
			),
			*EncounterID.ToString(),
			*GetNameSafe(DefeatedEnemy)
		),
		FColor::Green
	);

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Story Encounter [%s]: "
			"Required Enemy [%s] defeated. "
			"Encounter is waiting for Echo activation."
		),
		*EncounterID.ToString(),
		*GetNameSafe(DefeatedEnemy)
	);
}

void AStoryEncounter::ShowEncounterDebugMessage(
	const FString& Message,
	const FColor& Color) const
{
	if (!bShowEncounterDebug)
	{
		return;
	}
	if (!GEngine)
	{
		return;
	}
	GEngine->AddOnScreenDebugMessage(
		-1,
		5.0f,
		Color,
		Message
	);
}

