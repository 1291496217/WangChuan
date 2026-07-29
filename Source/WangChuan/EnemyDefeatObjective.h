#pragma once

#include "CoreMinimal.h"
#include "StoryObjectiveBase.h"
#include "EnemyDefeatObjective.generated.h"

class AGhostEnemy;

/*
* Adapts one configured Ghost Enemy death into the shared Story Objective
* completion interface.
*
* This Actor does not unlock relics, complete encounters, or move NPCs.
*/
UCLASS()
class WANGCHUAN_API AEnemyDefeatObjective : public AStoryObjectiveBase
{
	GENERATED_BODY()

public:
	AEnemyDefeatObjective();

	virtual void ActivateObjective() override;

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(
		const EEndPlayReason::Type EndPlayReason
	) override;

	/*
	* Concrete enemy Actor instance that resolves this Objective.
	*/
	UPROPERTY(
		EditInstanceOnly,
		BlueprintReadOnly,
		Category = "Story Objective|Enemy"
	)
	TObjectPtr<AGhostEnemy> RequiredEnemy;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Story Objective|Enemy|State"
	)
	bool bRequiredEnemyDefeated = false;

private:
	bool bConfigurationValid = false;
	bool bEnemyDelegateBound = false;

	bool ValidateConfiguration() const;
	void BindRequiredEnemy();
	void UnbindRequiredEnemy();
	void RefreshRequiredEnemyState();
	void CompleteIfReady();

	UFUNCTION()
	void HandleRequiredEnemyDefeated(
		AGhostEnemy* DefeatedEnemy
	);
};
