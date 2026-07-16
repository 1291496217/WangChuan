#pragma once

#include "CoreMinimal.h"
#include "StoryTypes.generated.h"

/**
* 通用Story NPC 状态
* 
* CurrentStoryStage 表示 NPC 目前处于第几段故事；
* EStoryNPCState 表示 NPC 在这一阶段中的当前行为状态。
*/
UENUM(BlueprintType)
enum class EStoryNPCState : uint8
{
	Dormant			UMETA(DisplayName = "Dormant"),
	Available		UMETA(DisplayName = "Available"),
	WaitingForEvent UMETA(DisplayName = "Waiting For Event"),
	EventResolved	UMETA(DisplayName = "Event Resolved"),
	Relocating		UMETA(DisplayName = "Relocating"),
	ChapterComplete UMETA(DisplayName = "Chapter Complete")
};

/**
* 单行对话数据。
*/
USTRUCT(BlueprintType)
struct FDialogueLine
{
	GENERATED_BODY()

	/**
	* 当前对话行的说话者。
	* 
	* 允许未来同一段 Sequence 中出现不同说话者，
	* 例如 NPC, 主角内心或无名残响。
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText SpeakerName;

	/*
	* 实际对话内容。
	* 
	* MultiLine 允许在 Blueprint Details 中编辑较长文本。
	*/
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Dialogue",
		meta = (MultiLine = true)
	)
	FText DialogueText;
};

/**
* 一组按顺序播放的对话。
*/
USTRUCT(BlueprintType)
struct FDialogueSequence
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TArray<FDialogueLine> Lines;
};