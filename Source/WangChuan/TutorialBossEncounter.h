#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialBossEncounter.generated.h"

class AGhostEnemy;
class AWCTutorialGate;

UCLASS()
class WANGCHUAN_API ATutorialBossEncounter : public AActor
{
	GENERATED_BODY()

public:
	ATutorialBossEncounter();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Tutorial Boss Encounter")
	TObjectPtr<AGhostEnemy> RequiredBoss;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Tutorial Boss Encounter")
	TObjectPtr<AWCTutorialGate> ExitGate;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
		Category = "Tutorial Boss Encounter")
	bool bHandled = false;

	UFUNCTION()
	void HandleBossDefeated(AGhostEnemy* DefeatedEnemy);

private:
	void UnbindBoss();
};
