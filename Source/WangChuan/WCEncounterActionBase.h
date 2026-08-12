#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WCEncounterActionBase.generated.h"

class AWCMemoryMazeEncounter;

UCLASS(Abstract, Blueprintable)
class WANGCHUAN_API AWCEncounterActionBase : public AActor
{
	GENERATED_BODY()

public:
	AWCEncounterActionBase();

	UFUNCTION(BlueprintCallable, Category = "Memory Maze Encounter|Action")
	void ExecuteAction(AWCMemoryMazeEncounter* SourceEncounter);

	UFUNCTION(BlueprintPure, Category = "Memory Maze Encounter|Action")
	bool HasExecuted() const;

	bool TryClaimOwnership(AWCMemoryMazeEncounter* RequestingEncounter);
	void ReleaseOwnership(AWCMemoryMazeEncounter* RequestingEncounter);

	UFUNCTION(BlueprintPure, Category = "Memory Maze Encounter|Action")
	AWCMemoryMazeEncounter* GetOwningEncounter() const;

protected:
	virtual void ExecuteActionInternal(AWCMemoryMazeEncounter* SourceEncounter);
	void ResetRuntimeStateForNewOwner();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
		Category = "Memory Maze Encounter|Action")
	bool bExecuted = false;

private:
	// Runtime-only weak ownership. This must never become serialized map state.
	TWeakObjectPtr<AWCMemoryMazeEncounter> OwningEncounter;
};
