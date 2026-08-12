#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WCEncounterConditionBase.generated.h"

class AWCEncounterConditionBase;
class AWCMemoryMazeEncounter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FWCEncounterConditionSatisfiedSignature,
	AWCEncounterConditionBase*, SatisfiedCondition);

UCLASS(Abstract, Blueprintable)
class WANGCHUAN_API AWCEncounterConditionBase : public AActor
{
	GENERATED_BODY()

public:
	AWCEncounterConditionBase();

	UFUNCTION(BlueprintCallable, Category = "Memory Maze Encounter|Condition")
	void ActivateCondition();

	UFUNCTION(BlueprintCallable, Category = "Memory Maze Encounter|Condition")
	void DeactivateCondition();

	UFUNCTION(BlueprintPure, Category = "Memory Maze Encounter|Condition")
	bool IsSatisfied() const;

	bool TryClaimOwnership(AWCMemoryMazeEncounter* RequestingEncounter);
	void ReleaseOwnership(AWCMemoryMazeEncounter* RequestingEncounter);

	UFUNCTION(BlueprintPure, Category = "Memory Maze Encounter|Condition")
	AWCMemoryMazeEncounter* GetOwningEncounter() const;

	UPROPERTY(BlueprintAssignable, Category = "Memory Maze Encounter|Condition")
	FWCEncounterConditionSatisfiedSignature OnConditionSatisfied;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BindToSource();
	virtual void UnbindFromSource();
	virtual void CheckAlreadySatisfied();
	void ResetRuntimeStateForNewOwner();

	void SetSatisfied();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
		Category = "Memory Maze Encounter|Condition")
	bool bIsActive = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
		Category = "Memory Maze Encounter|Condition")
	bool bIsSatisfied = false;

private:
	// Runtime-only weak ownership. This must never become serialized map state.
	TWeakObjectPtr<AWCMemoryMazeEncounter> OwningEncounter;
};
