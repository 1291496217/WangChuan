#include "WCObjectiveCompleteEncounterCondition.h"

#include "StoryObjectiveBase.h"

AWCObjectiveCompleteEncounterCondition::AWCObjectiveCompleteEncounterCondition()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWCObjectiveCompleteEncounterCondition::BindToSource()
{
	if (!IsValid(RequiredObjective))
	{
		UE_LOG(LogTemp, Error,
			TEXT("Objective Condition '%s' requires a valid RequiredObjective."), *GetName());
		return;
	}

	RequiredObjective->OnObjectiveCompleted.AddUniqueDynamic(
		this, &AWCObjectiveCompleteEncounterCondition::HandleObjectiveCompleted);
}

void AWCObjectiveCompleteEncounterCondition::UnbindFromSource()
{
	if (IsValid(RequiredObjective))
	{
		RequiredObjective->OnObjectiveCompleted.RemoveDynamic(
			this, &AWCObjectiveCompleteEncounterCondition::HandleObjectiveCompleted);
	}
}

void AWCObjectiveCompleteEncounterCondition::CheckAlreadySatisfied()
{
	if (IsValid(RequiredObjective) && RequiredObjective->GetIsObjectiveComplete())
	{
		SetSatisfied();
	}
}

void AWCObjectiveCompleteEncounterCondition::HandleObjectiveCompleted(
	AStoryObjectiveBase* CompletedObjective)
{
	if (CompletedObjective == RequiredObjective)
	{
		SetSatisfied();
	}
}
