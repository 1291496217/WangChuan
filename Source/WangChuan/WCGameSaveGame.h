#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "StoryTypes.h"
#include "WCSaveTypes.h"
#include "WCGameSaveGame.generated.h"

/*
* 《忘川河畔》当前单槽位故事存档数据容器。
*
* 此类只保存序列化数据。
* 它不读取关卡 Actor，也不执行任何 Gameplay 行为。
*/
UCLASS()
class WANGCHUAN_API UWCGameSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// ******************** Construction ********************

	UWCGameSaveGame();

	// ******************** Configuration ********************

	/*
	* 当前 SaveGame 数据格式版本。
	*
	* Week7 不制作版本迁移系统，
	* 但加载时仍需要识别不支持的格式。
	*/
	static constexpr int32 CurrentSaveVersion = 1;

	// ******************** Save Data ********************

	/*
	* 当前存档的数据格式版本。
	*/
	UPROPERTY(
		BlueprintReadWrite,
		SaveGame,
		Category = "Save Data|Meta"
	)
	int32 SaveVersion = 1;

	/*
	* 玩家应恢复到的稳定 Checkpoint。
	*
	* Day2 只创建和测试该字段，
	* 尚不创建 AWCPlayerCheckpoint 或移动玩家。
	*/
	UPROPERTY(
		BlueprintReadWrite,
		SaveGame,
		Category = "Save Data|Checkpoint"
	)
	FName CurrentCheckpointID = NAME_None;

	/*
	* Story NPC 的稳定持久化状态。
	*/
	UPROPERTY(
		BlueprintReadWrite,
		SaveGame,
		Category = "Save Data|Story"
	)
	TArray<FWCSavedStoryNPCState> StoryNPCStates;

	/*
	* Objective 的最终完成状态。
	*/
	UPROPERTY(
		BlueprintReadWrite,
		SaveGame,
		Category = "Save Data|Story"
	)
	TArray<FWCSavedObjectiveState> ObjectiveStates;

	/*
	* Encounter 的最终完成状态。
	*/
	UPROPERTY(
		BlueprintReadWrite,
		SaveGame,
		Category = "Save Data|Story"
	)
	TArray<FWCSavedEncounterState> EncounterStates;

	/*
	* 玩家已经完整阅读并记录的 Memory Echo。
	*
	* 当前直接保存完整 FMemoryEchoData，
	* 因为工程尚未建立 Echo Data Registry。
	*/
	UPROPERTY(
		BlueprintReadWrite,
		SaveGame,
		Category = "Save Data|Journal"
	)
	TArray<FMemoryEchoData> RecordedMemoryEchoes;
};
