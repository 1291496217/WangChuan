#pragma once

#include "CoreMinimal.h"
#include "WCEncounterConditionBase.h"
#include "WCObjectiveCompleteEncounterCondition.generated.h"

class AStoryObjectiveBase;

UCLASS(Blueprintable)
class WANGCHUAN_API AWCObjectiveCompleteEncounterCondition : public AWCEncounterConditionBase
{
	GENERATED_BODY()

public:
	AWCObjectiveCompleteEncounterCondition();

protected:
	virtual void BindToSource() override;
	virtual void UnbindFromSource() override;
	virtual void CheckAlreadySatisfied() override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly,
		Category = "Memory Maze Encounter|Condition")
	TObjectPtr<AStoryObjectiveBase> RequiredObjective;

	UFUNCTION()
	void HandleObjectiveCompleted(AStoryObjectiveBase* CompletedObjective);
};
