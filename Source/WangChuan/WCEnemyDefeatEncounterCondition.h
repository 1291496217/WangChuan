#pragma once

#include "CoreMinimal.h"
#include "WCEncounterConditionBase.h"
#include "WCEnemyDefeatEncounterCondition.generated.h"

class AGhostEnemy;

UCLASS(Blueprintable)
class WANGCHUAN_API AWCEnemyDefeatEncounterCondition : public AWCEncounterConditionBase
{
	GENERATED_BODY()

public:
	AWCEnemyDefeatEncounterCondition();

protected:
	virtual void BindToSource() override;
	virtual void UnbindFromSource() override;
	virtual void CheckAlreadySatisfied() override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly,
		Category = "Memory Maze Encounter|Condition")
	TObjectPtr<AGhostEnemy> RequiredEnemy;

	UFUNCTION()
	void HandleEnemyDefeated(AGhostEnemy* DefeatedEnemy);
};
