#include "StoryObjectiveBase.h"

#include "Components/SceneComponent.h"

// ******************** Construction ********************

AStoryObjectiveBase::AStoryObjectiveBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ObjectiveID = NAME_None;
	bIsActive = false;
	bIsCompleted = false;
}

// ******************** Objectives ********************

void AStoryObjectiveBase::ActivateObjective()
{
	if (bIsCompleted)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Story Object [%s] cannot activate because it is already complete."),
			*ObjectiveID.ToString()
		);

		return;
	}

	if (bIsActive)
	{
		// 重复激活不应重新启动 Objective。
		return;
	}

	bIsActive = true;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Story Objective [%s] activated."),
		*ObjectiveID.ToString()
	);
}

void AStoryObjectiveBase::CompleteObjective()
{
	if (bIsCompleted)
	{
		// 防止重复完成和重复广播事件。
		return;
	}

	if (!bIsActive)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Story Objective [%s] cannot complete because it is not active."),
			*ObjectiveID.ToString()
		);

		return;
	}

	bIsCompleted = true;
	bIsActive = false;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Story Objective [%s] completed."),
		*ObjectiveID.ToString()
	);

	OnObjectiveCompleted.Broadcast(this);
}

void AStoryObjectiveBase::ResetObjective()
{
	bIsActive = false;
	bIsCompleted = false;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Story Objective [%s] reset."),
		*ObjectiveID.ToString()
	);
}

void AStoryObjectiveBase::ApplySavedObjectiveState(bool bSavedCompleted)
{
	/*
	* Restore 直接应用稳定事实。
	*
	* 无论完成或未完成，都不尝试恢复一次运行到一半的
	* Active / Preview / Timer 状态。
	*/
	bIsCompleted = bSavedCompleted;
	bIsActive = false;

	OnSavedObjectiveStateApplied();

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Story Objective [%s] silently restored. "
			"Completed=%s."
		),
		*ObjectiveID.ToString(),
		bIsCompleted
		? TEXT("True")
		: TEXT("False")
	);
}

void AStoryObjectiveBase::OnSavedObjectiveStateApplied()
{
	/*
	* Base Objective 没有额外表现需要恢复。
	*
	* EnemyDefeatObjective 与 LanternSequencePuzzle
	* 会覆盖这个 Hook。
	*/
}

// ******************** Getters ********************

FName AStoryObjectiveBase::GetObjectiveID() const
{
	return ObjectiveID;
}

bool AStoryObjectiveBase::GetIsObjectiveActive() const
{
	return bIsActive;
}

bool AStoryObjectiveBase::GetIsObjectiveComplete() const
{
	return bIsCompleted;
}
