#pragma once

#include "CoreMinimal.h"
#include "WCEncounterActionBase.h"
#include "WCRevealTutorialFragmentEncounterAction.generated.h"

class ATutorialMemoryFragment;

UCLASS(Blueprintable)
class WANGCHUAN_API AWCRevealTutorialFragmentEncounterAction : public AWCEncounterActionBase
{
	GENERATED_BODY()

public:
	AWCRevealTutorialFragmentEncounterAction();

protected:
	virtual void ExecuteActionInternal(AWCMemoryMazeEncounter* SourceEncounter) override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly,
		Category = "Memory Maze Encounter|Action")
	TObjectPtr<ATutorialMemoryFragment> TargetFragment;
};
