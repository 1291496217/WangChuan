#include "StoryObjectiveBase.h"

#include "Components/SceneComponent.h"

AStoryObjectiveBase::AStoryObjectiveBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ObjectiveID = NAME_None;
	bIsActive = false;
	bIsCompleted = false;
}

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
		// Repeated activation and should not restart the Objective.
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
		// Important duplicate-completion guard.
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

bool AStoryObjectiveBase::GetIsObjectiveActive() const
{
	return bIsActive;
}

bool AStoryObjectiveBase::GetIsObjectiveComplete() const
{
	return bIsCompleted;
}
