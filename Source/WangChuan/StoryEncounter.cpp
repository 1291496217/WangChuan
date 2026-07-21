// Fill out your copyright notice in the Description page of Project Settings.


#include "StoryEncounter.h"

#include "Components/SceneComponent.h"
#include "Engine/Engine.h"

#include "GhostEnemy.h"
#include "StoryAnchor.h"
#include "WCStoryNPC.h"
#include "EchoRelic.h"

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

	if (RequiredEnemy)
	{
		RequiredEnemy->OnEnemyDefeated
			.AddUniqueDynamic(
				this,
				&AStoryEncounter::HandleRequiredEnemyDefeated
			);

		ShowEncounterDebugMessage(
			FString::Printf(
				TEXT(
					"%s: Listening to "
					"RequiredEnemy: %s"
				),
				*EncounterID.ToString(),
				*GetNameSafe(RequiredEnemy)
			),
			FColor::Cyan
		);
	}
	else
	{
		ShowEncounterDebugMessage(
			FString::Printf(
				TEXT(
					"%s, RequiredEnemy "
					"is not assigned."
				),
				*EncounterID.ToString()
			),
			FColor::Red
		);
	}

	if (EchoRelic)
	{
		EchoRelic->OnEchoActivated
			.AddUniqueDynamic(
				this,
				&AStoryEncounter::HandleEchoRelicActivated
			);
	}
	else
	{
		ShowEncounterDebugMessage(
			FString::Printf(
				TEXT(
					"%s: EchoRelic "
					"is not assigned."
				),
				*EncounterID.ToString()
			),
			FColor::Red
		);
	}

	if (!StoryNPC)
	{
		ShowEncounterDebugMessage(
			FString::Printf(
				TEXT(
					"%s, StoryNPC "
					"is not assigned."
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

	if (IsValid(EchoRelic))
	{
		EchoRelic->OnEchoActivated
			.RemoveDynamic(
				this,
				&AStoryEncounter::HandleEchoRelicActivated
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

	if (EchoRelic)
	{
		const bool bUnlocked =
			EchoRelic->UnlockRelic();

		if (!bUnlocked)
		{
			ShowEncounterDebugMessage(
				FString::Printf(
					TEXT(
						"%s: Echo Relic could "
						"not be unlocked."
					),
					*EncounterID.ToString()
				),
				FColor::Yellow
			);
		}
	}

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

void AStoryEncounter::HandleEchoRelicActivated(
	AEchoRelic* ActivatedRelic)
{
	if (bEncounterCompleted)
	{
		return;
	}

	if (!bRequiredEnemyDefeated)
	{
		ShowEncounterDebugMessage(
			FString::Printf(
				TEXT(
					"%s: Echo activation "
					"rejected because Required "
					"Enemy is not defeated."
				),
				*EncounterID.ToString()
			),
			FColor::Red
		);

		return;
	}

	if (!ActivatedRelic ||
		ActivatedRelic != EchoRelic)
	{
		return;
	}

	bEncounterCompleted = true;

	if (StoryNPC)
	{
		StoryNPC->RecieveStoryEvent(
			CompletionStoryEventID
		);

		if (NextStoryAnchor)
		{
			const bool bRelocationStarted = StoryNPC
				->RelocateToStoryAnchor(
					NextStoryAnchor,
					NextStoryStage
				);

			if (!bRelocationStarted)
			{
				ShowEncounterDebugMessage(
					FString::Printf(
						TEXT(
							"%s: NPC relocation "
							"could not start."
						),
						*EncounterID.ToString()
					),
					FColor::Yellow
				);
			}
		}
	}

	ShowEncounterDebugMessage(
		FString::Printf(
			TEXT(
				"%s,\n"
				"Required Enemy Defeated: True\n"
				"Echo Activated: True\n"
				"Encounter Completed: True\n"
				"Next Story Stage: %d"
			),
			*EncounterID.ToString(),
			NextStoryStage
		),
		FColor::Green
	);

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Story Encounter [%s] completed "
			"through Echo Relic [%s]."
		),
		*EncounterID.ToString(),
		*GetNameSafe(ActivatedRelic)
	);
}

