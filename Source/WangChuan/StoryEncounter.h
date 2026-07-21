#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StoryEncounter.generated.h"

class AGhostEnemy;
class AStoryAnchor;
class AWCStoryNPC;
class USceneComponent;
class AEchoRelic;

/*
* 轻量 Story Encounter 协调者。
* 
* 负责：
* - 监听指定 RequiredEnemy 的死亡事件。
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
	* 指定敌人是否已经被击败。
	*/
	UFUNCTION(BlueprintPure, Category = "Story Encounter")
	bool GetIsRequiredEnemyDefeated() const;

	/*
	* 整个 Encounter 是否已经完成。
	*/
	UFUNCTION(BlueprintPure, Category = "Story Encounter")
	bool GetIsEncounterCompleted() const;

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Encounter")
	FName EncounterID = TEXT("StoryEncounter.None");

	/*
	* 玩家必须击败的特定敌人。
	* 
	* 使用 EditInstanceOnly， 
	* 因为它引用当前地图中的敌人实例。
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, 
		Category = "Story Encounter|References")
	AGhostEnemy* RequiredEnemy = nullptr;

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
	* 指定 RequiredEnemy 是否已死亡。
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, 
		Category = "Story Encounter|State")
	bool bRequiredEnemyDefeated = false;

	/*
	* 整条 Encounter 是否完成。
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, 
		Category = "Story Encounter|State")
	bool bEncounterCompleted = false;

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

	/*
	* RequiredEnemy 广播死亡时调用。
	* 
	* 参数签名必须与
	* FOnGhostEnemyDefeatedSignature 完全一致。
	*/
	UFUNCTION()
	void HandleRequiredEnemyDefeated(
		AGhostEnemy* DefeatedEnemy
	);

	void ShowEncounterDebugMessage(
		const FString& Message,
		const FColor& Color
	) const;

	UFUNCTION()
	void HandleEchoRelicActivated(
		AEchoRelic* ActivatedRelic
	);
};
