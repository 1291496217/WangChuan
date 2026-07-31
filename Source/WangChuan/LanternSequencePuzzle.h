#pragma once

#include "CoreMinimal.h"
#include "StoryObjectiveBase.h"
#include "LanternSequencePuzzle.generated.h"

class ALanternPuzzlePiece;
class AWCCharacter;
class UBoxComponent;
class UPrimitiveComponent;
class USoundBase;

/*
* Lantern Sequence Puzzle 的运行时状态。
*
* Dormant:
* 尚未进入区域或启动谜题。
*
* Previewing:
* 正在播放正确光音顺序，玩家不可输入。
*
* AwaitingInput:
* 等待玩家复现顺序。
*
* Resetting:
* 玩家输入错误，正在等待自动重演。
*
* Completed:
* 谜题已完成，不再接受输入。
*/
UENUM(BlueprintType)
enum class ELanternPuzzleState : uint8
{
	Dormant
	UMETA(DisplayName = "Dormant"),

	Previewing
	UMETA(DisplayName = "Previewing"),

	AwaitingInput
	UMETA(DisplayName = "Awaiting Input"),

	Resetting
	UMETA(DisplayName = "Resetting"),

	Completed
	UMETA(DisplayName = "Completed")
};

UCLASS()
class WANGCHUAN_API ALanternSequencePuzzle
	: public AStoryObjectiveBase
{
	GENERATED_BODY()

public:
	// ******************** Construction ********************

	ALanternSequencePuzzle();

	// ******************** Objectives ********************

	/*
	* 正式启动谜题。
	*
	* 第一版由 ActivationBox 调用，
	* 但保留 BlueprintCallable，
	* 方便隔离测试与未来外部触发。
	*/
	UFUNCTION(
		BlueprintCallable,
		Category = "Lantern Puzzle"
	)
	void StartPuzzle();

	/*
	* 完整重置整个 Story Objective。
	*
	* 这不同于一次错误输入后的 Attempt Reset。
	*/
	virtual void ResetObjective() override;

	/*
	* 检查 Lantern、PieceID 与 CorrectSequence 配置。
	*/
	UFUNCTION(
		BlueprintCallable,
		Category = "Lantern Puzzle|Validation"
	)
	bool ValidatePuzzleConfiguration();

	// ******************** Getters ********************

	UFUNCTION(
		BlueprintPure,
		Category = "Lantern Puzzle|State"
	)
	ELanternPuzzleState GetPuzzleState() const;

	UFUNCTION(
		BlueprintPure,
		Category = "Lantern Puzzle|State"
	)
	int32 GetCurrentInputCount() const;

protected:
	// ******************** Lifecycle ********************

	virtual void BeginPlay() override;

	virtual void EndPlay(
		const EEndPlayReason::Type EndPlayReason
	) override;

	// ******************** Components ********************

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Lantern Puzzle|Components"
	)
	TObjectPtr<UBoxComponent> ActivationBox;

	// ******************** Configuration ********************

	/*
	* 关卡中实际存在的 Lantern Piece 实例。
	*/
	UPROPERTY(
		EditInstanceOnly,
		BlueprintReadOnly,
		Category = "Lantern Puzzle|Configuration"
	)
	TArray<TObjectPtr<ALanternPuzzlePiece>>
		LanternPieces;

	/*
	* 只保存 PieceID，不依赖数组位置。
	*
	* Generic default:
	* 0 -> 2 -> 1
	*/
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Lantern Puzzle|Configuration"
	)
	TArray<int32> CorrectSequence;

	/*
	* 当前这一次尝试中，
	* 玩家已经正确提交的 PieceID。
	*/
	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Lantern Puzzle|State"
	)
	TArray<int32> CurrentPlayerInput;

	// ******************** Timing ********************

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Lantern Puzzle|Timing",
		meta = (ClampMin = "0.0")
	)
	float InitialPreviewDelay = 0.6f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Lantern Puzzle|Timing",
		meta = (ClampMin = "0.05")
	)
	float PreviewLightDuration = 0.6f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Lantern Puzzle|Timing",
		meta = (ClampMin = "0.0")
	)
	float PreviewGapDuration = 0.3f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Lantern Puzzle|Timing",
		meta = (ClampMin = "0.0")
	)
	float ResetDelay = 0.8f;

	// ******************** Feedback ********************

	/*
	* 可选错误音效。
	*
	* 为空时不会影响 Reset 和 Replay。
	*/
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Lantern Puzzle|Feedback"
	)
	TObjectPtr<USoundBase> WrongInputSound;

	// ******************** Runtime State ********************

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Lantern Puzzle|State"
	)
	ELanternPuzzleState PuzzleState =
		ELanternPuzzleState::Dormant;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Lantern Puzzle|State"
	)
	bool bConfigurationValid = false;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Lantern Puzzle|State"
	)
	int32 CurrentPreviewIndex = 0;

	// ******************** Events ********************

	UFUNCTION()
	void OnPuzzleAreaEntered(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void HandleLanternInteracted(ALanternPuzzlePiece* InteractedLantern);

	// ******************** Preview ********************

	void PlaySequencePreview();

	void AdvanceSequencePreview();

	void FinishSequencePreview();

	// ******************** Input Evaluation ********************

	void HandleCorrectInput(ALanternPuzzlePiece* InteractedLantern);

	void HandleWrongInput(ALanternPuzzlePiece* InteractedLantern);

	/*
	* 只重置当前失败尝试。
	*
	* 不调用 AStoryObjectiveBase::ResetObjective()。
	*/
	void ResetPuzzleInput();

	void FinishPuzzle();

	// ******************** Helpers ********************

	void BindLanternDelegates();

	void UnbindLanternDelegates();

	void SetAllLanternsLit(bool bLit);

	void SetLanternInteractionEnabled(bool bEnabled);

	ALanternPuzzlePiece*FindLanternByPieceID(int32 PieceID) const;

	bool IsConfiguredLantern(const ALanternPuzzlePiece*Lantern) const;

	void ClearPuzzleTimers();

	void PlayWrongInputSound(const ALanternPuzzlePiece*WrongLantern) const;

	FTimerHandle PreviewTimerHandle;

	FTimerHandle ResetTimerHandle;
};
