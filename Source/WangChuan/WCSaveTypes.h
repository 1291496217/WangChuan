#pragma once

#include "CoreMinimal.h"
#include "WCSaveTypes.generated.h"

/*
* Quiet Child 等 Story NPC 的稳定持久化结果。
*
* 只保存：
* - NPC 是谁
* - 已到达哪个 Story Stage
* - 当前稳定停留在哪个 Anchor
*
* 不保存 Relocating、Timer、Niagara 或 Dialogue 行数。
*/
USTRUCT(BlueprintType)
struct FWCSavedStoryNPCState
{
	GENERATED_BODY()

	/*
	* 与关卡中 AWCStoryNPC::StoryNPCID 匹配。
	*
	* 例如：
	* QuietChild
	*/
	UPROPERTY(
		BlueprintReadWrite,
		SaveGame,
		Category = "Save Data|Story NPC"
	)
	FName StoryNPCID = NAME_None;

	/*
	* NPC 已经稳定到达的 Story Stage。
	*/
	UPROPERTY(
		BlueprintReadWrite,
		SaveGame,
		Category = "Save Data|Story NPC"
	)
	int32 StoryStage = 0;

	/*
	* NPC 当前稳定停留的 Anchor ID。
	*
	* 例如：
	* QuietChild.Anchor03
	*/
	UPROPERTY(
		BlueprintReadWrite,
		SaveGame,
		Category = "Save Data|Story NPC"
	)
	FName AnchorID = NAME_None;
};

/*
* 一个 Story Objective 的最终持久化结果。
*
* 第一版只保存是否已完成。
* 不保存 Active、Preview、Reset 或部分输入进度。
*/
USTRUCT(BlueprintType)
struct FWCSavedObjectiveState
{
	GENERATED_BODY()

	/*
	* 与关卡中 AStoryObjectiveBase::ObjectiveID 匹配。
	*/
	UPROPERTY(
		BlueprintReadWrite,
		SaveGame,
		Category = "Save Data|Objective"
	)
	FName ObjectiveID = NAME_None;

	/*
	* 该 Objective 是否已经永久完成。
	*/
	UPROPERTY(
		BlueprintReadWrite,
		SaveGame,
		Category = "Save Data|Objective"
	)
	bool bCompleted = false;
};

/*
* 一个 Story Encounter 的最终持久化结果。
*
* Objective 完成与 Encounter 完成是两个不同事实，
* 因此需要分别保存。
*/
USTRUCT(BlueprintType)
struct FWCSavedEncounterState
{
	GENERATED_BODY()

	/*
	* 与关卡中 AStoryEncounter::EncounterID 匹配。
	*/
	UPROPERTY(
		BlueprintReadWrite,
		SaveGame,
		Category = "Save Data|Encounter"
	)
	FName EncounterID = NAME_None;

	/*
	* 玩家是否已经完成 Echo 阅读并正式完成 Encounter。
	*/
	UPROPERTY(
		BlueprintReadWrite,
		SaveGame,
		Category = "Save Data|Encounter"
	)
	bool bCompleted = false;
};