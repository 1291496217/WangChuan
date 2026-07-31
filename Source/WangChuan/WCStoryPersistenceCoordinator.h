#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WCStoryPersistenceCoordinator.generated.h"

class UWCGameInstance;

/*
* 当前关卡的轻量 Story Persistence 协调者。
*
* 负责：
* - 查找当前地图中的 Story Actors
* - 验证稳定 Persistence ID
* - 收集稳定 Story 状态
* - 将完整快照写入 UWCGameSaveGame
* - 请求 UWCGameInstance 写入磁盘
*
* 不负责：
* - 正常 Gameplay 推进
* - 恢复 Actor 状态
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
	UFUNCTION(
		BlueprintCallable,
		Category = "Story Persistence|Debug"
	)
	void PrintLoadedSaveSummary() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Story Persistence|Debug"
	)
	bool bShowOnScreenDebug = true;

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
};