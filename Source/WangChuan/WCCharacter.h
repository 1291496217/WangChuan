#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Animation/AnimMontage.h"
#include "InteractionStone.h"
#include "Interactable.h"
#include "Containers/Set.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHitFlashWidget.h"
#include "StoryTypes.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "Camera/CameraShakeBase.h"
#include "WCCharacter.generated.h"

class AGhostEnemy;
class AWCStoryNPC;
class UDialogueWidget;
class AEchoRelic;
class UMemoryEchoWidget;
class UMemoryJournalWidget;

USTRUCT(BlueprintType)
struct FPlayerAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float Damage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float Range = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	FVector BoxHalfSize = FVector(10.0f, 60.0f, 70.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float Duration = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float KnockbackStrength = 100.0f;
};

UCLASS()
class WANGCHUAN_API AWCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// ******************** Construction ********************

	AWCCharacter();

protected:
	// ******************** Lifecycle ********************

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode,
									   uint8 PreviousCustomMode = 0) override;

public:
	// ******************** Lifecycle ********************

	virtual void Tick(float DeltaTime) override;

	// ******************** Input ********************

	virtual void SetupPlayerInputComponent(
		class UInputComponent* PlayerInputComponent) override;

	// ******************** Combat ********************

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ReceiveDamage(float DamageAmount);

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetHealth() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool GetIsDead() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnPlayerAttackHitNotify();

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool GetIsInCombat() const;

	UFUNCTION(BlueprintPure, Category = "Combat|Lock On")
	bool GetIsLockedOn() const;

	// ******************** Interaction ********************

	IInteractable* CurrentInteractable;

	TSet<int32> CollectedFragments;

	FString CurrentPrompt;

	void ShowInteractionPrompt(const FString& Prompt);

	void HideInteractionPrompt();

	// ******************** Dialogue ********************

	/*
	 * 打开统一 Dialogue UI。
	 *
	 * 返回 true 表示成功进入对话。
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool StartDialogue(AWCStoryNPC* StoryNPC, const FDialogueSequence& DialogueSequence);

	/*
	 * 关闭当前 Dialogue UI 并恢复正常控制。
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue();

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool GetIsInDialogue() const;

	// ******************** Memory Echo ********************

	UFUNCTION(BlueprintCallable, Category = "Memory Echo")
	bool StartMemoryEcho(const FMemoryEchoData& EchoData, AEchoRelic* SourceRelic);

	/*
	 * bCompleted:
	 * true = 玩家完整读完。
	 * false = 死亡或异常终止。
	 */
	UFUNCTION(BlueprintCallable, Category = "Memory Echo")
	void EndMemoryEcho(bool bCompleted);

	UFUNCTION(BlueprintCallable, Category = "Memory Echo")
	bool GetIsViewingMemoryEcho() const;

	UFUNCTION(BlueprintCallable, Category = "Memory Echo")
	bool RecordMemoryEcho(const FMemoryEchoData& EchoData);

	UFUNCTION(BlueprintCallable, Category = "Memory Echo")
	bool HasRecordedMemoryEcho(FName EchoID) const;

	/*
	* 返回玩家当前已完整记录的 Memory Echo。
	*
	* 只提供只读引用，Persistence Coordinator 不应直接修改
	* 玩家拥有的 Runtime Journal 数据。
	*/
	const TArray<FMemoryEchoData>&
		GetRecordedMemoryEchoes() const;

	// ******************** Memory Journal ********************

	UFUNCTION(BlueprintCallable, Category = "Memory Journal")
	void CloseMemoryJournal();

	UFUNCTION(BlueprintPure, Category = "Memory Journal")
	bool GetIsMemoryJournalOpen() const;

	/*
	* 静默恢复已记录的 Memory Echo。
	*
	* 不调用 RecordMemoryEcho()，
	* 不打开 Journal 或 Echo UI，
	* 不触发 Relic 或 Encounter。
	*/
	UFUNCTION(BlueprintCallable, Category = "Memory Journal|Persistence")
	void ApplySavedMemoryEchoes(const TArray<FMemoryEchoData>&SavedEchoes);

protected:
	// ******************** Components ********************

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Feedback")
	TSubclassOf<UCameraShakeBase> HitCameraShakeClass;

	// ******************** Input ********************

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JournalAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* HeavyAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LockOnAction;

	// ******************** Combat ********************

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	UAnimMontage* LightAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float Health = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bHasProcessedAttackHit = false;

	void StopCurrentAttackMontage(float BlendOutTime = 0.05f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UAnimMontage* CurrentAttackMontage = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsInCombat = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float CombatIdleDuration = 3.0f;

	FTimerHandle CombatIdleTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsCurrentAttackHeavy = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Debug")
	bool bShowAttackDebug;

	bool bIsAttacking = false;

	FTimerHandle AttackTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Heavy Attack")
	UAnimMontage* HeavyAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack Data")
	FPlayerAttackData LightAttackData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack Data")
	FPlayerAttackData HeavyAttackData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack Data")
	FPlayerAttackData CurrentAttackData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Light Combo")
	int32 MaxLightComboIndex = 4;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Light Combo")
	int32 CurrentLightComboIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Light Combo")
	TArray<FPlayerAttackData> LightComboAttackData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Light Combo")
	float ComboResetTime = 1.2f;

	FTimerHandle ComboResetTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Light Combo")
	bool bCanBufferLightAttack = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat| Light Combo")
	bool bHasBufferedLightAttack = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Light Combo")
	float ComboWindowOpenRatio = 0.55f;

	FTimerHandle ComboWindowOpenTimerHandle;
	FTimerHandle ComboWindowCloseTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Light Combo")
	bool bIsCurrentAttackFinisher = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Finisher Feedback")
	USoundBase* ComboFinisherHitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Finisher Feedback")
	TSubclassOf<UCameraShakeBase> ComboFinisherCameraShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Finisher Feedback")
	UParticleSystem* ComboFinisherHitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Heavy Feedback")
	USoundBase* HeavyAttackHitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Heavy Feedback")
	TSubclassOf<UCameraShakeBase> HeavyAttackCameraShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Heavy Feedback")
	UParticleSystem* HeavyAttackHitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Hit Stop")
	bool bEnableHitStop = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Hit Stop")
	float LightHitStopDuration = 0.035f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Hit Stop")
	float FinisherHitStopDuration = 0.055f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Hit Stop")
	float HeavyHitStopDuration = 0.065f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Hit Stop")
	float HitStopTimeDilation = 0.05f;

	FTimerHandle HitStopTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Lock On")
	bool bIsLockedOn = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Lock On")
	AGhostEnemy* CurrentLockOnTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Lock On")
	float LockOnRadius = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Lock On")
	float LockOnBreakDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Lock On")
	float LockOnRotationInterpSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Lock On")
	float LockOnMinCameraDot = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Lock On")
	float NormalWalkSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Lock On")
	float LockOnWalkSpeed = 300.0f;

	UPROPERTY()
	TWeakObjectPtr<AActor> HitStopTargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Combat")
	USoundBase* AttackHitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Combat")
	USoundBase* AttackWhiffSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Combat")
	USoundBase* PlayerDeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Combat")
	USoundBase* PlayerHurtSound;

	// ******************** VFX ********************

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
	UParticleSystem* AttackHitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
	float AttackHitEffectScale = 0.4f;

	// ******************** UI ********************

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> PlayerHUDClass;

	UPROPERTY()
	UUserWidget* PlayerHUDWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPlayerHitFlashWidget> PlayerHitFlashWidgetClass;

	UPROPERTY()
	UPlayerHitFlashWidget* PlayerHitFlashWidget;

	// WBP_Dialogue 对应的 Blueprint 类。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Dialogue")
	TSubclassOf<UDialogueWidget> DialogueWidgetClass;

	/*
	 * 当前正在显示的 Dialogue Widget。
	 */
	UPROPERTY()
	UDialogueWidget* ActiveDialogueWidget = nullptr;

	/*
	 * 当前正在与玩家对话的 NPC。
	 */
	UPROPERTY()
	AWCStoryNPC* ActiveDialogueNPC = nullptr;

	/*
	 * 玩家是否正在 Conversation Mode。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	bool bIsInDialogue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Memory Echo")
	TSubclassOf<UMemoryEchoWidget> MemoryEchoWidgetClass;

	UPROPERTY()
	UMemoryEchoWidget* ActiveMemoryEchoWidget = nullptr;

	UPROPERTY()
	AEchoRelic* ActiveEchoRelic = nullptr;

	UPROPERTY()
	FMemoryEchoData ActiveMemoryEchoData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory Echo")
	bool bIsViewingMemoryEcho = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory Echo")
	TArray<FMemoryEchoData> RecordedMemoryEchoes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Memory Journal")
	TSubclassOf<UMemoryJournalWidget> MemoryJournalWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UMemoryJournalWidget> ActiveMemoryJournalWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory Journal")
	bool bIsMemoryJournalOpen = false;

	// ******************** Input ********************

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Interact();

	void ToggleMemoryJournal();

	bool OpenMemoryJournal();

	void Attack();

	void HeavyAttack();

	void HandleJumpStarted();

	void HandleJumpCompleted();

	// ******************** Combat ********************

	bool IsAnyAttackActive() const;

	bool CanStartGroundAttack() const;

	bool CanStartJump() const;

	void CancelActiveAttackForAirborneTransition();

	void PerformCurrentAttackTrace();

	void PlayLightAttackMontage(FName SectionName);

	void StartAttackTimer(float Duration);

	void EndAttack();

	void Die();

	void ShowAttackHitDebug(AActor* HitActor);

	void EnterCombatState();

	void ExitCombatState();

	// ******************** Helpers ********************

	bool CanAct() const;

	void PlayAttackHitSound();

	void PlayAttackWhiffSound();

	void PlayAttackHitEffect(const FHitResult& HitResult);

	void PlayPlayerHitFeedback();

	void PlayComboFinisherFeedback(const FHitResult& HitResult);

	void PlayHeavyAttackHitFeedback(const FHitResult& HitResult);

	// ******************** Combo ********************

	void ResetLightCombo();

	FName GetLightComboSectionName(int32 ComboIndex) const;

	FPlayerAttackData GetLightComboAttackData(int32 ComboIndex) const;

	void AdvancedLightCombo();

	void OpenLightComboWindow();

	void CloseLightComboWindow();

	void ConsumeBufferedLightAttack();

	void ClearLightComboBuffer();

	void ApplyHitStop(AActor* HitActor, float Duration);

	void EndHitStop();

	// ******************** Lock-On ********************

	void ToggleLockOn();

	void LockOnToTarget();

	void UnlockTarget();

	void UpdateLockOn(float DeltaTime);

	AGhostEnemy* FindBestLockOnTarget() const;

	bool IsLockOnTargetValid() const;

	void FaceLockOnTargetInstantly();
};
