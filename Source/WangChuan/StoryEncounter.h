#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StoryEncounter.generated.h"

class AStoryAnchor;
class AWCStoryNPC;
class USceneComponent;
class AEchoRelic;
class AStoryObjectiveBase;

/*
* 轻量 Story Encounter 协调者。
* 
* 负责：
* - 监听统一 Story Objective 的完成事件。
* - 记录 Encounter 当前进度。
* - 保存 Story NPC， Echo Relic， 与 Next Anchor 等引用。
*/

UCLASS()
class WANGCHUAN_API AStoryEncounter : public AActor
{
	GENERATED_BODY()
	
public:	
	AStoryEncounter();

	/*
	* 返回此 Encounter 的稳定语义 ID。
	*/
	UFUNCTION(BlueprintPure, Category = "Story Encounter|Identity")
	FName GetEncounterID() const;

	/*
	* 整个 Encounter 是否已经完成。
	*/
	UFUNCTION(BlueprintPure, Category = "Story Encounter")
	bool GetIsEncounterCompleted() const;

	bool IsEncounterConditionResolved() const;

	void UnlockEchoRelicFromResolvedCondition();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(
		const EEndPlayReason::Type EndPlayReason
	) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, 
		Category = "Story Encounter|Components")
	USceneComponent* SceneRoot;

	/*
	* 用于在编辑器和 Debug 中识别当前 Eencounter。
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, 
		Category = "Story Encounter")
	FName EncounterID = NAME_None;

	/*
	* Required gameplay condition for this Encounter.
	* Condition-specific logic belongs to the configured Objective.
	*/
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly,
		Category = "StoryEncounter|Objective", meta = (AllowPrivateAccess = "true"))
	AStoryObjectiveBase* StoryObjective = nullptr;

	/*
	* 与本 Encounter 相关的 Story NPC。
	*/
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, 
		Category = "Story Encounter|References")
	AWCStoryNPC* StoryNPC = nullptr;

	/*
	* Encounter 完成后 NPC 前往目标 Anchor。
	*/
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, 
		Category = "Story Encounter|References")
	AStoryAnchor* NextStoryAnchor = nullptr;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly,
		Category = "Story Encounter|References")
	AEchoRelic* EchoRelic = nullptr;

	/*
	* 整条 Encounter 是否完成。
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, 
		Category = "Story Encounter|State")
	bool bEncounterCompleted = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Story Encounter|State", meta = (AllowPrivateAccess = "true"))
	bool bStoryObjectiveCompleted = false;

	/*
	* 是否显示 Encounter Debug Message。
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, 
		Category = "Story Encounter|Debug")
	bool bShowEncounterDebug = true;

	// Event Complete

	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		Category = "Story Encounter|Completion")
	FName CompletionStoryEventID =
		TEXT("QuietChild.BellEchoActivated");

	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		Category = "Story Encounter|Completion",
		meta = (ClampMin = "0"))
	int32 NextStoryStage = 1;

	void ShowEncounterDebugMessage(
		const FString& Message,
		const FColor& Color
	) const;

	UFUNCTION()
	void HandleEchoRelicActivated(
		AEchoRelic* ActivatedRelic
	);

	UFUNCTION()
	void HandleStoryObjectiveCompleted(
		AStoryObjectiveBase* CompletedObjective
	);
};
