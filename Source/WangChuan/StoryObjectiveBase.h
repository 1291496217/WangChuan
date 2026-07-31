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
	// ******************** Construction ********************

	AStoryObjectiveBase();

	// ******************** Objectives ********************

	/*
	* 激活此 Objective。
	*
	* 已完成的 Objective 只有在调用 ResetObjective() 后才能再次激活。
	*/
	UFUNCTION(BlueprintCallable, Category = "Story Objective")
	virtual void ActivateObjective();

	/*
	* 完成此 Objective，并广播一次 OnObjectiveCompleted。
	*
	* 调用时 Objective 必须已经激活。
	*/
	UFUNCTION(BlueprintCallable, Category = "Story Objective")
	virtual void CompleteObjective();

	/*
	* 将此 Objective 恢复为未激活且未完成的初始状态。
	*
	* 这是完整重置，不同于重置一次失败的 Lantern Puzzle 尝试。
	*/
	UFUNCTION(BlueprintCallable, Category = "Story Objective")
	virtual void ResetObjective();

	/*
	* 静默应用存档中的最终 Objective 状态。
	*
	* 与 CompleteObjective() 不同：
	* - 不要求 Objective 已经 Active
	* - 不广播 OnObjectiveCompleted
	* - 不触发 Encounter
	* - 不播放任何一次性反馈
	*/
	UFUNCTION(BlueprintCallable, Category = "Story Objective|Persistence")
	virtual void ApplySavedObjectiveState(bool bSavedCompleted);

	// ******************** Getters ********************

	/*
	* 返回此 Objective 的稳定语义 ID。
	*/
	UFUNCTION(BlueprintPure, Category = "Story Objective|Identity")
	FName GetObjectiveID() const;

	UFUNCTION(BlueprintPure, Category = "Story Objective")
	bool GetIsObjectiveActive() const;

	UFUNCTION(BlueprintPure, Category = "Story Objective")
	bool GetIsObjectiveComplete() const;

public:
	// ******************** Events ********************

	// Objective 成功完成时广播一次。
	UPROPERTY(BlueprintAssignable, Category = "Story Objective|Events")
	FOnStoryObjectiveCompletedSignature OnObjectiveCompleted;

protected:
	// ******************** Components ********************

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Story Objective|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	// ******************** Configuration ********************

	/*
	* 用于关卡配置、调试和日志的稳定语义标识。
	*
	* 示例：
	* QuietChild.LanteernPuzzle01
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Objective")
	FName ObjectiveID;

	// ******************** Runtime State ********************

	// Objective 当前是否已激活。
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Story Objective|State")
	bool bIsActive;

	// Objective 当前是否已完成。
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Story Objective|State")
	bool bIsCompleted;

	/*
	* 子类恢复自身表现与引用状态的 Hook。
	*
	* 调用时，Base 的：
	* bIsCompleted
	* bIsActive
	*
	* 已经设置完毕。
	*/
	virtual void OnSavedObjectiveStateApplied();
};
