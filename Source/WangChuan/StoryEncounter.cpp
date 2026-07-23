// Fill out your copyright notice in the Description page of Project Settings.


#include "StoryEncounter.h"

#include "Components/SceneComponent.h"
#include "Engine/Engine.h"

#include "GhostEnemy.h"
#include "StoryAnchor.h"
#include "WCStoryNPC.h"
#include "EchoRelic.h"
#include "StoryObjectiveBase.h"

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
	bStoryObjectiveCompleted = false;

	const bool bHasRequiredEnemy = IsValid(RequiredEnemy);
	const bool bHasStoryObjective = IsValid(StoryObjective);

	if (bHasRequiredEnemy && bHasStoryObjective)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Story Encounter [%s] has both RequiredEnemy and StoryObjective. "
				"Configure exactly one completion source."
			),
			*EncounterID.ToString()
		);
	}
	else if (bHasStoryObjective)
	{
		StoryObjective->OnObjectiveCompleted.AddUniqueDynamic(
			this,
			&AStoryEncounter::HandleStoryObjectiveCompleted
		);

		UE_LOG(
			LogTemp,
			Log,
			TEXT("Story Encounter [%s] bound to Story Objective [%s]."),
			*EncounterID.ToString(),
			*StoryObjective->GetName()
		);

		/*
		* This protects against BeginPlay ordering.
		*
		* If the Objective happened to complete before the Encounter bound to it,
		* the Encounter can still recover the resolved state.
		*/
		if (StoryObjective->GetIsObjectiveComplete())
		{
			HandleStoryObjectiveCompleted(StoryObjective);
		}
	}
	else if (bHasRequiredEnemy)
	{
		RequiredEnemy->OnEnemyDefeated.AddUniqueDynamic(
			this,
			&AStoryEncounter::HandleRequiredEnemyDefeated
		);

		UE_LOG(
			LogTemp,
			Log,
			TEXT("Story Encounter [%s] bound to Required Enemy [%s]."),
			*EncounterID.ToString(),
			*RequiredEnemy->GetName()
		);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"Story Encounter [%s] has no RequiredEnemy or StoryObjective."
			),
			*EncounterID.ToString()
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

	if (IsValid(StoryObjective))
	{
		StoryObjective->OnObjectiveCompleted
			.RemoveDynamic(
				this,
				&AStoryEncounter::HandleStoryObjectiveCompleted
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
	if (bEncounterCompleted)
	{
		return;
	}

	/*
	* 防止 Encounter 重复处理同一次结果。
	*/
	if (bRequiredEnemyDefeated)
	{
		return;
	}

	if (!IsValid(RequiredEnemy))
	{
		return;
	}

	if (!IsValid(DefeatedEnemy))
	{
		return;
	}

	/*
	* 只能由 RequiredEnemy 推进流程。
	*/
	if (DefeatedEnemy != RequiredEnemy)
	{
		return;
	}

	/*
	* This path should not be used when a Story Objective is configured.
	*/
	if (IsValid(StoryObjective))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Story Encounter [%s] cannot process RequiredEnemy "
				"because StoryObjective is also configured."
			),
			*EncounterID.ToString()
		);

		return;
	}
	bRequiredEnemyDefeated = true;

	UnlockEchoRelicFromResolvedCondition();
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

	if (!IsEncounterConditionResolved())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"Story Encounter [%s] ignored Echo activation because "
				"its configured condition has not been resolved."
			),
			*EncounterID.ToString()
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

void AStoryEncounter::HandleStoryObjectiveCompleted(
	AStoryObjectiveBase* CompletedObjective)
{
	if (bEncounterCompleted)
	{
		return;
	}

	if (bStoryObjectiveCompleted)
	{
		return;
	}

	if (!IsValid(StoryObjective))
	{
		return;
	}

	if (!IsValid(CompletedObjective))
	{
		return;
	}

	if (CompletedObjective != StoryObjective)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"Story Encounter [%s] ignored completion from an "
				"unconfigured Story Objective [%s]."
			),
			*EncounterID.ToString(),
			*CompletedObjective->GetName()
		);

		return;
	}

	/*
	* Exactly one condition source is allowed.
	*/
	if (IsValid(RequiredEnemy))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Story Encounter [%s] cannot process StoryObjective "
				"because RequiredEnemy is also configured."
			),
			*EncounterID.ToString()
		);

		return;
	}

	bStoryObjectiveCompleted = true;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Story Encounter [%s] Story Objective resolved."),
		*EncounterID.ToString()
	);

	UnlockEchoRelicFromResolvedCondition();
}

void AStoryEncounter::UnlockEchoRelicFromResolvedCondition()
{
	if (!IsValid(EchoRelic))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"Story Encounter [%s] resolved its condition, "
				"but no EchoRelic is configured."
			),
			*EncounterID.ToString()
		);

		return;
	}

	EchoRelic->UnlockRelic();

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Story Encounter [%s] unlocked Echo Relic [%s]."),
		*EncounterID.ToString(),
		*EchoRelic->GetName()
	);
}

bool AStoryEncounter::IsEncounterConditionResolved() const
{
	/*
	* Resolve from the recorded result, not from the current validity of the
	* configured Actor pointer.
	*
	* Required enemies are destroyed shortly after death. By the time the
	* player finishes reading the Echo, RequiredEnemy can therefore be invalid
	* even though this Encounter already received and recorded its defeat.
	*
	* BeginPlay rejects configurations with both or neither source, and the
	* source-specific handlers prevent both result flags from becoming true.
	*/
	if (bRequiredEnemyDefeated == bStoryObjectiveCompleted)
	{
		return false;
	}

	return bRequiredEnemyDefeated || bStoryObjectiveCompleted;
}
