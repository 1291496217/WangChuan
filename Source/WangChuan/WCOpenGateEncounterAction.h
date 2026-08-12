#pragma once

#include "CoreMinimal.h"
#include "WCEncounterActionBase.h"
#include "WCOpenGateEncounterAction.generated.h"

class AWCProgressionGate;

UCLASS(Blueprintable)
class WANGCHUAN_API AWCOpenGateEncounterAction : public AWCEncounterActionBase
{
	GENERATED_BODY()

public:
	AWCOpenGateEncounterAction();

protected:
	virtual void ExecuteActionInternal(AWCMemoryMazeEncounter* SourceEncounter) override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly,
		Category = "Memory Maze Encounter|Action")
	TObjectPtr<AWCProgressionGate> TargetGate;
};
