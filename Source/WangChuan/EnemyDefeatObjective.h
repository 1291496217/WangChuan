#pragma once

#include "CoreMinimal.h"
#include "StoryObjectiveBase.h"
#include "EnemyDefeatObjective.generated.h"

class AGhostEnemy;

/*
* 将指定 Ghost Enemy 的死亡事件接入通用 Story Objective 完成接口。
*
* 此 Actor 不负责解锁遗物、完成 Encounter 或移动 NPC。
*/
UCLASS()
class WANGCHUAN_API AEnemyDefeatObjective : public AStoryObjectiveBase
{
	GENERATED_BODY()

public:
	// ******************** Construction ********************

	AEnemyDefeatObjective();

	// ******************** Public Interface ********************

	virtual void ActivateObjective() override;

protected:
	// ******************** Lifecycle ********************

	virtual void BeginPlay() override;

	virtual void EndPlay(
		const EEndPlayReason::Type EndPlayReason
	) override;

	// ******************** Configuration ********************

	// 完成该 Objective 所需击败的具体敌人 Actor。
	UPROPERTY(
		EditInstanceOnly,
		BlueprintReadOnly,
		Category = "Story Objective|Enemy"
	)
	TObjectPtr<AGhostEnemy> RequiredEnemy;

	// ******************** Runtime State ********************

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Story Objective|Enemy|State"
	)
	bool bRequiredEnemyDefeated = false;

private:
	// ******************** Runtime State ********************

	bool bConfigurationValid = false;
	bool bEnemyDelegateBound = false;

	// ******************** Helpers ********************

	bool ValidateConfiguration() const;
	void BindRequiredEnemy();
	void UnbindRequiredEnemy();
	void RefreshRequiredEnemyState();
	void CompleteIfReady();

	// ******************** Events ********************

	UFUNCTION()
	void HandleRequiredEnemyDefeated(
		AGhostEnemy* DefeatedEnemy
	);
};
