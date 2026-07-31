#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "StoryTypes.h"
#include "EchoRelic.generated.h"

class AEchoRelic;
class AWCCharacter;
class UPrimitiveComponent;
class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;

/*
* 当玩家完整阅读并确认 Echo Relic 时广播。
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnEchoRelicActivatedSignature,
	AEchoRelic*,
	ActivatedRelic
);

/*
* 可交互的轻量 Memory Echo 遗物。
*
* 当前状态：
* Locked
* -> Available
* -> Activated
*/
UCLASS()
class WANGCHUAN_API AEchoRelic : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	// ******************** Construction ********************

	AEchoRelic();

	// ******************** Interaction ********************

	virtual void Interact() override;

	virtual FString GetInteractionPrompt() override;

	/*
	* Encounter 在 Required Enemy 死亡后调用。
	*/
	UFUNCTION(BlueprintCallable, Category = "Echo Relic")
	bool UnlockRelic();

	/*
	* 玩家读完 Memory Echo 后由 AWCCharacter 调用。
	*/
	UFUNCTION(BlueprintCallable, Category = "Echo Relic")
	void ConfirmEchoRead();

	/*
	* 玩家死亡或 Echo UI 被异常终止时调用。
	*/
	UFUNCTION(BlueprintCallable, Category = "Echo Relic")
	void CancelEchoRead();

	// ******************** Getters ********************

	UFUNCTION(BlueprintCallable, Category = "Echo Relic")
	EEchoRelicState GetRelicState() const;

	UFUNCTION(BlueprintCallable, Category = "Echo Relic")
	FMemoryEchoData GetMemoryEchoData() const;

	/*
	* 返回此 Echo Relic 的稳定 Echo ID。
	*/
	UFUNCTION(BlueprintPure, Category = "Echo Relic|Identity")
	FName GetEchoID() const;

	// ******************** Events ********************

	/*
	* 只有在玩家完整读完残响后才广播。
	*/
	UPROPERTY(BlueprintAssignable, Category = "Echoo Relic|Events")
	FOnEchoRelicActivatedSignature OnEchoActivated;

	/*
	* 静默恢复 Relic 的稳定状态。
	*
	* 不打开 Memory Echo UI，
	* 不广播 OnEchoActivated，
	* 不记录 Journal。
	*/
	UFUNCTION(BlueprintCallable, Category = "Echo Relic|Persistence")
	void ApplySavedRelicState(EEchoRelicState SavedState);

protected:
	// ******************** Components ********************

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Echo Relic|Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Echo Relic|Components")
	UStaticMeshComponent* RelicMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Echo Relic|Components")
	USphereComponent* InteractionSphere;

	// ******************** Configuration ********************

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Echo Relic|State")
	EEchoRelicState RelicState = EEchoRelicState::locked;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Echo Relic|Interaction")
	FString LockedInteractionPrompt = TEXT("[E] Examine");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Echo Relic|Interaction")
	FString AvailableInteractionPrompt = TEXT("[E] Listen");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Echo Relic|Interaction",
		meta = (MultiLine = "true"))
	FText LockedInteractionText = FText::FromString(TEXT("The bell is silent."));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Echo Relic|Memory")
	FMemoryEchoData MemoryEchoData;

	// ******************** Runtime State ********************

	/*
	* Memory Echo UI 已经打开，
	* 但玩家尚未完成阅读。
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Echo Relic|State")
	bool bActivationInProgress = false;

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

	void DisableRelicInteraction();

	void RefreshPromptForOverlappingPlayer();

	void SetRelicInteractionEnabled(bool bEnabled);
};
