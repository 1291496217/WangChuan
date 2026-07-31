#pragma once

#include "CoreMinimal.h"
#include "StoryTypes.generated.h"

// ******************** Story NPC ********************

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

// ******************** Dialogue ********************

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

// ******************** Memory Echo ********************

/*
* Echo Relic 当前状态。
*/
UENUM(BlueprintType)
enum class EEchoRelicState : uint8
{
	/*
	* 尚未满足激活条件。
	*/
	locked UMETA(DisplayName = "Locked"),

	/*
	* 条件已满足，可以被玩家调查。
	*/
	Available UMETA(DisplayName = "Available"),

	/*
	* 玩家已阅读并确认该 Memory Echo。
	*/
	Activated UMETA(DisplayName = "Activated")
};

/*
* 一条可记录的 Memory Echo 数据。
*/
USTRUCT(BlueprintType)
struct FMemoryEchoData
{
	GENERATED_BODY()

	/*
	* 用于识别与去重。
	*
	* 例如：QuietChild.BellEcho01
	*/
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Memory Echo"
	)
	FName EchoID = NAME_None;

	/*
	* Journal 与 Echo UI 中显示的标题。
	*/
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Memory Echo"
	)
	FText Title;

	/*
	* 遗物自身释放出的记忆残响。
	*/
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Memory Echo",
		meta = (MultiLine = "true")
	)
	FText EchoText;

	/*
	* 残响引发的主角个人共鸣。
	*/
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Memory Echo",
		meta = (MultiLine = "true")
	)
	FText PlayerReasonanceText;
};
