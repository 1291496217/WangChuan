#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "WCGameSaveGame.h"
#include "WCGameInstance.generated.h"

/*
* 《忘川河畔》的轻量 SaveGame 入口。
*
* 负责：
* - 固定 Save Slot
* - 创建内存中的 SaveGame Object
* - 检查、保存、加载和删除磁盘存档
* - 暂存当前已经创建或加载的 SaveGame Object
*
* 不负责：
* - 查找关卡 Story Actor
* - 收集当前世界状态
* - 恢复 NPC、Objective、Encounter 或 Journal
*/
UCLASS()
class WANGCHUAN_API UWCGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// ******************** Lifecycle ********************

	virtual void Init() override;

	// ******************** Save and Load ********************

	/*
	* 创建一个全新的、只存在于内存中的 SaveGame Object。
	*
	* 此函数不会自动写入磁盘，
	* 也不会自动删除已有 Slot。
	*/
	UFUNCTION(BlueprintCallable, Category = "Save Game")
	UWCGameSaveGame* CreateNewSave();

	/*
	* 检查固定 Slot 是否存在磁盘存档。
	*
	* 这与 LoadedSaveData 是否有效是两个不同问题。
	*/
	UFUNCTION(BlueprintPure, Category = "Save Game")
	bool HasSavedGame() const;

	/*
	* 将当前 LoadedSaveData 写入固定 Slot。
	*
	* 调用前必须已经 CreateNewSave() 或 LoadSavedGame()。
	*/
	UFUNCTION(BlueprintCallable, Category = "Save Game")
	bool SaveCurrentGame();

	/*
	* 从固定 Slot 读取并验证 UWCGameSaveGame。
	*
	* 成功后，LoadedSaveData 指向读取出的对象。
	*/
	UFUNCTION(BlueprintCallable, Category = "Save Game")
	bool LoadSavedGame();

	/*
	* 删除固定 Slot，并清理内存中的 LoadedSaveData。
	*
	* 若 Slot 本来就不存在，则最终目标已经满足，
	* 因此此函数仍返回 true。
	*/
	UFUNCTION(BlueprintCallable, Category = "Save Game")
	bool DeleteSavedGame();

	// ******************** Getters ********************

	/*
	* 返回当前在内存中创建或加载的 SaveGame Object。
	*
	* 后续 Persistence Coordinator 将通过此对象
	* 填充或读取世界状态。
	*/
	UFUNCTION(BlueprintPure, Category = "Save Game")
	UWCGameSaveGame* GetLoadedSaveData() const;

	UFUNCTION(BlueprintPure, Category = "Save Game")
	FString GetSaveSlotName() const;

	UFUNCTION(BlueprintPure, Category = "Save Game")
	int32 GetSaveUserIndex() const;

private:
	// ******************** Runtime State ********************

	/*
	* 当前在内存中的工作存档。
	*
	* Transient 表示 GameInstance 自己不会尝试将这个 Pointer
	* 作为另一层持久化数据保存。
	*/
	UPROPERTY(Transient)
	TObjectPtr<UWCGameSaveGame> LoadedSaveData;

	// ******************** Configuration ********************

	static const FString SaveSlotName;
	static constexpr int32 SaveUserIndex = 0;
};
