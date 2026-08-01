
#include "StoryEncounter.h"

#include "Components/SceneComponent.h"
#include "Engine/Engine.h"

#include "StoryAnchor.h"
#include "WCStoryNPC.h"
#include "EchoRelic.h"
#include "StoryObjectiveBase.h"

// ******************** Construction ********************

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

// ******************** Lifecycle ********************

void AStoryEncounter::BeginPlay()
{
	Super::BeginPlay();

	bEncounterCompleted = false;
	bStoryObjectiveCompleted = false;

	if (IsValid(StoryObjective))
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
		* 此检查用于规避 BeginPlay 顺序差异。
		*
		* 如果 Objective 在 Encounter 完成绑定前已经结束，
		* Encounter 仍可在此恢复已解决状态。
		*/
		if (StoryObjective->GetIsObjectiveComplete())
		{
			HandleStoryObjectiveCompleted(StoryObjective);
		}
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"Story Encounter [%s] has no StoryObjective assigned."
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

// ******************** Getters ********************

FName AStoryEncounter::GetEncounterID() const
{
	return EncounterID;
}

bool AStoryEncounter::GetIsEncounterCompleted() const
{
	return bEncounterCompleted;
}

AStoryObjectiveBase*
AStoryEncounter::GetStoryObjective() const
{
	return StoryObjective;
}

AEchoRelic*
AStoryEncounter::GetEchoRelic() const
{
	return EchoRelic;
}
// ******************** Helpers ********************

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

// ******************** Events ********************

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
				"Story Objective Completed: %s\n"
				"Echo Activated: True\n"
				"Encounter Completed: True\n"
				"Next Story Stage Config: %d"
			),
			*EncounterID.ToString(),
			bStoryObjectiveCompleted
			? TEXT("True")
			: TEXT("False"),
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

	bStoryObjectiveCompleted = true;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Story Encounter [%s] Story Objective resolved."),
		*EncounterID.ToString()
	);

	UnlockEchoRelicFromResolvedCondition();
}

// ******************** Public Interface ********************

void AStoryEncounter::UnlockEchoRelicFromResolvedCondition()
{
	if (!IsValid(EchoRelic))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"Story Encounter [%s] resolved its "
				"condition, but no EchoRelic is configured."
			),
			*EncounterID.ToString()
		);
		return;
	}

	const bool bRelicUnlocked = EchoRelic->UnlockRelic();

	if (bRelicUnlocked)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT(
				"Story Encounter [%s] unlocked "
				"Echo Relic [%s]."
			),
			*EncounterID.ToString(),
			*EchoRelic->GetName()
		);

		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"Story Encounter [%s] could not unlock "
			"Echo Relic [%s] because it was no longer Locked."
		),
		*EncounterID.ToString(),
		*EchoRelic->GetName()
	);
}

bool AStoryEncounter::IsEncounterConditionResolved() const
{
	return bStoryObjectiveCompleted;
}

void AStoryEncounter::ApplySavedEncounterState(bool bSavedCompleted)
{
	bEncounterCompleted = bSavedCompleted;

	/*
	* 这个值可以由当前已恢复的 Objective 推导。
	*
	* 因此 Day3 完整 Restore 时必须先恢复 Objective，
	* 再恢复 Encounter。
	*/
	bStoryObjectiveCompleted =
		IsValid(StoryObjective) &&
		StoryObjective
		->GetIsObjectiveComplete();

	if (bEncounterCompleted &&
		!bStoryObjectiveCompleted)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"Story Encounter [%s] is saved as "
				"completed, but its configured Objective "
				"is currently incomplete."
			),
			*EncounterID.ToString()
		);
	}

	/*
	* 不调用：
	*
	* HandleStoryObjectiveCompleted()
	* UnlockEchoRelicFromResolvedCondition()
	* HandleEchoRelicActivated()
	* RecieveStoryEvent()
	* RelocateToStoryAnchor()
	*/

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Story Encounter [%s] silently restored. "
			"ObjectiveResolved=%s, Completed=%s."
		),
		*EncounterID.ToString(),
		bStoryObjectiveCompleted
		? TEXT("True")
		: TEXT("False"),
		bEncounterCompleted
		? TEXT("True")
		: TEXT("False")
	);
}