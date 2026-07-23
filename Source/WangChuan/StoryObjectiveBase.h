#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StoryObjectiveBase.generated.h"

class USceneComponent;
class AStoryObjectiveBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnStoryObjectiveCompletedSignature,
	AStoryObjectiveBase*,
	CompletedObjective
);

UCLASS()
class WANGCHUAN_API AStoryObjectiveBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AStoryObjectiveBase();

	/*
	* Activates this Objective.
	* 
	* A completed Objective cannot be activated again
	* unless ResetObjective() has been called first.
	*/
	UFUNCTION(BlueprintCallable, Category = "Story Objective")
	virtual void ActivateObjective();

	/*
	* Completes this Objective and broadcasts OnObjectiveCompleted once.
	* 
	* The Objective must already be active.
	*/
	UFUNCTION(BlueprintCallable, Category = "Story Objective")
	virtual void CompleteObjective();

	/*
	* Returns this Objective to its initial inactive and incomplete state.
	* 
	* This is a full Objective reset. It is not the same as resetting one
	* failed lantern-puzzle attempt.
	*/
	UFUNCTION(BlueprintCallable, Category = "Story Objective")
	virtual void ResetObjective();

	UFUNCTION(BlueprintCallable, Category = "Story Objective")
	bool GetIsObjectiveActive() const;

	UFUNCTION(BlueprintCallable, Category = "Story Objective")
	bool GetIsObjectiveComplete() const;

public:
	/*
	* Broadcast once when this Objective successfully completes.
	*/
	UPROPERTY(BlueprintAssignable, Category = "Story Objective|Events")
	FOnStoryObjectiveCompletedSignature OnObjectiveCompleted;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, 
		Category = "Story Objective|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/*
	* Semantic identity used by level configuration, debugging and logging.
	* 
	* Example: 
	* QuietChild.LanteernPuzzle01
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Objective")
	FName ObjectiveID;

	/*
	* Runtime state. 
	*/
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Story Objective|State")
	bool bIsActive;

	/*
	* Runtime state.
	*/
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Story Objective|State")
	bool bIsCompleted;
};
