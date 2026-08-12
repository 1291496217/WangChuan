#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialEnemyDefeatFragmentLink.generated.h"

class AGhostEnemy;
class ATutorialMemoryFragment;

UCLASS()
class WANGCHUAN_API ATutorialEnemyDefeatFragmentLink : public AActor
{
	GENERATED_BODY()

public:
	ATutorialEnemyDefeatFragmentLink();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Tutorial Reward")
	TObjectPtr<AGhostEnemy> RequiredEnemy;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Tutorial Reward")
	TObjectPtr<ATutorialMemoryFragment> RewardFragment;

private:
	bool bDelegateBound = false;
	bool bRewardGranted = false;

	UFUNCTION()
	void HandleRequiredEnemyDefeated(AGhostEnemy* DefeatedEnemy);

	void UnbindRequiredEnemy();
};
