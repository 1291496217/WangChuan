#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WCMemoryMazeEncounter.generated.h"

class AWCEncounterActionBase;
class AWCEncounterConditionBase;
class AWCMemoryMazeEncounter;

UENUM(BlueprintType)
enum class EWCMemoryMazeEncounterState : uint8
{
	Dormant,
	Active,
	Completed
};

UENUM(BlueprintType)
enum class EWCEncounterCompletionPolicy : uint8
{
	All,
	Any
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FWCMemoryMazeEncounterSignature,
	AWCMemoryMazeEncounter*, Encounter);

UCLASS(Blueprintable)
class WANGCHUAN_API AWCMemoryMazeEncounter : public AActor
{
	GENERATED_BODY()

public:
	AWCMemoryMazeEncounter();

	UFUNCTION(BlueprintCallable, Category = "Memory Maze Encounter")
	void ActivateEncounter();

	UFUNCTION(BlueprintCallable, Category = "Memory Maze Encounter")
	void CompleteEncounter();

	UFUNCTION(BlueprintPure, Category = "Memory Maze Encounter")
	EWCMemoryMazeEncounterState GetEncounterState() const;

	UFUNCTION(BlueprintPure, Category = "Memory Maze Encounter")
	bool IsEncounterCompleted() const;

	UPROPERTY(BlueprintAssignable, Category = "Memory Maze Encounter|Events")
	FWCMemoryMazeEncounterSignature OnEncounterActivated;

	UPROPERTY(BlueprintAssignable, Category = "Memory Maze Encounter|Events")
	FWCMemoryMazeEncounterSignature OnEncounterCompleted;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Memory Maze Encounter")
	FName EncounterID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory Maze Encounter")
	bool bAutoActivateOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory Maze Encounter")
	EWCEncounterCompletionPolicy CompletionPolicy = EWCEncounterCompletionPolicy::All;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Memory Maze Encounter")
	TArray<TObjectPtr<AWCEncounterConditionBase>> CompletionConditions;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Memory Maze Encounter")
	TArray<TObjectPtr<AWCEncounterActionBase>> CompletionActions;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
		Category = "Memory Maze Encounter")
	EWCMemoryMazeEncounterState EncounterState = EWCMemoryMazeEncounterState::Dormant;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
		Category = "Memory Maze Encounter")
	bool bCompletionHandled = false;

private:
	UFUNCTION()
	void HandleConditionSatisfied(AWCEncounterConditionBase* SatisfiedCondition);

	void EvaluateCompletion();
	void DeactivateConditions();
	void ValidateConfiguration() const;
	bool TryClaimConfiguredObjectOwnership();
	void ReleaseConfiguredObjectOwnership();
};
