#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WCPlayerCheckpoint.h"
#include "WCStoryPersistenceCoordinator.generated.h"

class AStoryAnchor;
class AStoryEncounter;
class AStoryObjectiveBase;
class AEchoRelic;
class AWCCharacter;
class AWCStoryNPC;

class UWCGameInstance;
class UWCGameSaveGame;

/*
* 当前关卡的轻量 Story Persistence 协调者。
*
* 负责：
* - 查找当前地图中的 Story Actors
* - 验证稳定 Persistence ID
* - 收集稳定 Story 状态
* - 将完整快照写入 UWCGameSaveGame
* - 请求 UWCGameInstance 写入磁盘
* - 按顺序静默恢复 Story World 与玩家 Checkpoint
*
* 不负责：
* - 正常 Gameplay 推进
* - 播放 Story / Relocation 表现
* - 自动保存
*/
UCLASS()
class WANGCHUAN_API AWCStoryPersistenceCoordinator :
	public AActor
{
	GENERATED_BODY()

public:
	AWCStoryPersistenceCoordinator();

	/*
	* 验证当前关卡所有持久化 Actor 的 ID。
	*
	* 检查：
	* - ID 不能为 None
	* - 同类型 ID 不能重复
	* - 必需 Actor 类型不能完全缺失
	*/
	UFUNCTION(
		BlueprintCallable,
		Category = "Story Persistence|Validation"
	)
	bool ValidateWorldPersistenceIDs() const;

	/*
	* 收集当前世界的稳定状态并保存到固定 Slot。
	*
	* 此函数只 Capture + Save，不进行 Load 或 Restore。
	*/
	UFUNCTION(
		BlueprintCallable,
		Category = "Story Persistence"
	)
	bool CaptureAndSaveWorldState();

	/*
	* 输出当前 LoadedSaveData 的详细内容。
	*
	* 用于在新 PIE Session 中验证磁盘数据，
	* 不会将数据应用到当前世界。
	*/
	UFUNCTION(BlueprintCallable, Category = "Story Persistence|Debug")
	void PrintLoadedSaveSummary() const;

	/*
	* 从固定 Slot 读取 SaveGame，并立即恢复当前 World。
	*
	* 没有存档时不会修改默认 World。
	*/
	UFUNCTION(BlueprintCallable, Category = "Story Persistence")
	bool LoadAndRestoreWorldState();

	/*
	* 将当前 UWCGameInstance::LoadedSaveData
	* 应用到本关卡 Actor。
	*
	* 本函数不会读取磁盘。
	*/
	UFUNCTION(BlueprintCallable, Category = "Story Persistence")
	bool RestoreLoadedWorldState();

	UFUNCTION(BlueprintPure, Category = "Story Persistence")
	bool GetHasRestoredLoadedWorld() const;

	/*
	* 可见休憩点的玩家主动保存入口。
	* 保存失败时恢复 Player 的 Runtime Checkpoint 状态。
	*/
	UFUNCTION(
		BlueprintCallable,
		Category = "Story Persistence|Checkpoint"
	)
	bool SaveAtCheckpoint(
		AWCPlayerCheckpoint* Checkpoint
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "Story Persistence|Checkpoint"
	)
	bool TravelPlayerToCheckpoint(
		FName TargetCheckpointID
	);

	UFUNCTION(
		BlueprintPure,
		Category = "Story Persistence|Checkpoint"
	)
	TArray<FWCCheckpointTravelOption>
		GetCheckpointTravelOptions() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Persistence|Debug")
	bool bShowOnScreenDebug = true;

	/*
	* 默认关闭旧 Level Blueprint 的 P / L 调试入口。
	* 需要专门回归旧接口时可在 Coordinator 实例上临时开启。
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Persistence|Debug")
	bool bAllowDirectDebugPersistenceActions = false;

	virtual void EndPlay(
		const EEndPlayReason::Type EndPlayReason
	) override;

	/*
	* PIE / Map 启动后自动检查固定 Save Slot。
	*
	* 没有 Save Slot 时保留默认新游戏世界。
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Persistence|Load")
	bool bAutoLoadAndRestoreOnBeginPlay = true;

private:
	UWCGameInstance* GetWCGameInstance() const;

	bool RegisterPersistenceID(
		FName ID,
		const AActor* OwnerActor,
		const TCHAR* TypeLabel,
		TSet<FName>& SeenIDs
	) const;

	void ShowPersistenceMessage(
		const FString& Message,
		const FColor& Color
	) const;

	void HandleDeferredAutoRestore();

	bool InitializeDefaultCheckpointForNewGame();

	AWCPlayerCheckpoint* FindCheckpointByID(
		FName CheckpointID
	) const;

	bool BuildWorldActorMaps(
		TMap<FName, AWCStoryNPC*>&
		OutStoryNPCs,
		TMap<FName, AStoryObjectiveBase*>&
		OutObjectives,
		TMap<FName, AStoryEncounter*>&
		OutEncounters,
		TMap<FName, AEchoRelic*>&
		OutEchoRelics,
		TMap<FName, AStoryAnchor*>&
		OutAnchors,
		TMap<FName, AWCPlayerCheckpoint*>&
		OutCheckpoints
	) const;

	bool ValidateLoadedSaveDataForWorld(
		const UWCGameSaveGame* SaveData,
		const TMap<FName, AWCStoryNPC*>&
		WorldStoryNPCs,
		const TMap<FName, AStoryObjectiveBase*>&
		WorldObjectives,
		const TMap<FName, AStoryEncounter*>&
		WorldEncounters,
		const TMap<FName, AEchoRelic*>&
		WorldEchoRelics,
		const TMap<FName, AStoryAnchor*>&
		WorldAnchors,
		const TMap<FName, AWCPlayerCheckpoint*>&
		WorldCheckpoints
	) const;

	FTimerHandle DeferredRestoreTimerHandle;

	bool bRestoreInProgress = false;
	bool bHasRestoredLoadedWorld = false;
	bool bCheckpointSaveRequestInProgress = false;
	bool bStartupRestoreRequestInProgress = false;

	int32 SuccessfulRestoreCount = 0;

	void RefreshCheckpointPresentations(
		const TArray<FName>&
		UnlockedCheckpointIDs
	) const;
};
