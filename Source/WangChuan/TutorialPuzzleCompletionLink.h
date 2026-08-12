#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialPuzzleCompletionLink.generated.h"

class ALanternSequencePuzzle;
class AStoryObjectiveBase;
class ATutorialMemoryFragment;
class AWCTutorialGate;

UCLASS()
class WANGCHUAN_API ATutorialPuzzleCompletionLink : public AActor
{
	GENERATED_BODY()

public:
	ATutorialPuzzleCompletionLink();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Tutorial Puzzle Link")
	TObjectPtr<ALanternSequencePuzzle> RequiredPuzzle;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Tutorial Puzzle Link")
	TObjectPtr<ATutorialMemoryFragment> RewardFragment;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Tutorial Puzzle Link")
	TObjectPtr<AWCTutorialGate> BossGate;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
		Category = "Tutorial Puzzle Link")
	bool bCompletionHandled = false;

	UFUNCTION()
	void HandlePuzzleCompleted(AStoryObjectiveBase* CompletedObjective);

private:
	void UnbindFromPuzzle();
};
