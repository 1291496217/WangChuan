#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "LanternPuzzlePiece.generated.h"

class ALaternPuzzlePiece;
class AWCCharacter;
class UPointLightComponent;
class UPrimitiveComponent;
class USceneComponent;
class USoundBase;
class USphereComponent;
class UStaticMeshComponent;

/*
* 当玩家成功与某一盏 Lantern Puzzle Piece 交互时广播。
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnLanternInteractedSignature,
	ALanternPuzzlePiece*,
	InteractedLantern
);

UCLASS()
class WANGCHUAN_API ALanternPuzzlePiece : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	// ******************** Construction ********************

	ALanternPuzzlePiece();

	// ******************** Interaction ********************

	virtual void Interact() override;

	virtual FString GetInteractionPrompt() override;

	/*
	* 开启或关闭玩家这盏灯的交互。
	*
	* 不会自动改变灯的亮灭状态。
	*/
	UFUNCTION(BlueprintCallable, Category = "Lantern Puzzle|Interaction")
	void SetInteractionEnabled(bool bEnabled);

	/*
	* 直接设置灯的持续亮灭状态。
	*/
	UFUNCTION(BlueprintCallable, Category = "Lantern Puzzle|Visual")
	void SetLanternLit(bool bNewLit);

	/*
	* 播放一次灯光与声音的反馈。
	*/
	UFUNCTION(BlueprintCallable, Category = "Lantern Puzzle|Feedback")
	void PlayLanternFeedback();

	/*
	* 使用指定持续时间播放一次光与声音反馈。
	*
	* Puzzle Controller 使用这个接口。
	* 保证 Preview 的灯光时间与 Controller 配置一致。
	*/
	UFUNCTION(BlueprintCallable, Category = "Lantern Puzzle|Feedback")
	void PlayLanternFeedbackForDuration(float Duration);

	// ******************** Getters ********************

	UFUNCTION(BlueprintPure, Category = "Lantern Puzzle")
	int32 GetPieceID() const;

	UFUNCTION(BlueprintPure, Category = "Lantern Puzzle|State")
	bool GetIsLanternLit() const;

	UFUNCTION(BlueprintPure, Category = "Lantern Puzzle|State")
	bool GetIsInteractionEnabled() const;

	// ******************** Events ********************

	/*
	* 只表示玩家触碰了哪一盏灯。
	*
	* 该事件本身不判断正确或错误。
	*/
	UPROPERTY(BlueprintAssignable, Category = "Lantern Puzzle|Events")
	FOnLanternInteractedSignature OnLanternInteracted;

protected:
	// ******************** Lifecycle ********************

	virtual void BeginPlay() override;

	virtual void EndPlay(
		const EEndPlayReason::Type EndPlayReason
	) override;

	// ******************** Components ********************

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Lantern Puzzle|Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Lantern Puzzle|Components")
	UStaticMeshComponent* LanternMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Lantern Puzzle|Components")
	USphereComponent* InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Lantern Puzzle|Components")
	UPointLightComponent* LanternLight;

	// ******************** Configuration ********************

	/*
	* 每盏灯的稳定身份。
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		Category = "Lantern Puzzle|Identity", meta = (ClampMin = "0"))
	int32 PieceID = 0;

	// ******************** Interaction ********************
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		Category = "Lantern Puzzle|Interaction")
	FString InteractionPrompt =
		TEXT("[E] Touch");

	/*
	* 用于 Day2 独立测试
	*
	* 正式接入 Puzzle Controller 后，
	* 谜题应通过 SetInteractionEnabled() 控制输入。
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		Category = "Lantern Puzzle|Interaction")
	bool bEnableInteractionOnBeginPlay = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Lantern Puzzle|State")
	bool bInteractionEnabled = false;

	// ******************** Feedback ********************
	UPROPERTY(EditAnywhere,BlueprintReadOnly,
		Category = "Lantern Puzzle|Feedback")
	USoundBase* LanternTone = nullptr;

	/*
	* 同一个音源可以通过 Volume 与 Pitch
	* 为不同 Lantern Piece 形成轻微差异。
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		Category = "Lantern Puzzle|Feedback", meta = (ClampMin = "0.0"))
	float ToneVolumeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		Category = "Lantern Puzzle|Feedback",
		meta = (ClampMin = "0.1", ClampMax = "4.0"))
	float TonePitchMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		Category = "Lantern Puzzle|Feedback", meta = (ClampMin = "0.05"))
	float FeedbackDuration = 0.6f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Lantern Puzzle|State")
	bool bIsLit = false;

	/*
	* Blueprint 视觉拓展。
	*
	* 可在 BP 中用它切换 Emissive Material。
	*/
	UFUNCTION(
		BlueprintImplementableEvent,
		Category = "Lantern Puzzle|Visual",
		meta = (DisplayName = "On Lantern Lit Changed"))
	void BP_OnLanternLitChanged(bool bNewLit);

	// ******************** Events ********************

	UFUNCTION()
	void OnPlayerEnter(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnPlayerExit(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	// ******************** Helpers ********************
	void ClearPlayerInteractionIfNeeded();

	void RefreshPromptForOverlappingPlayer();

	void PlayLanternTone();

	void FinishLanternTone();

	void FinishLanternFeedback();

	FTimerHandle LanternFeedbackTimerHandle;
};
