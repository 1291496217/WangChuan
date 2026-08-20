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
#include "WCCombatantInterface.h"
#include "WCCharacter.generated.h"

class AGhostEnemy;
class AWCStoryNPC;
class UDialogueWidget;
class AEchoRelic;
class UMemoryEchoWidget;
class UMemoryJournalWidget;
class UWCCheckpointMenuWidget;
class AWCPlayerCheckpoint;
class AWCStoryPersistenceCoordinator;
class UTutorialFragmentWidget;
class UTutorialHUDWidget;
class UTutorialInstructionWidget;

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
class WANGCHUAN_API AWCCharacter : public ACharacter, public IWCCombatantInterface
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
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;
	virtual bool IsCombatantAlive_Implementation() const override;
	virtual EWCCombatFaction GetCombatFaction_Implementation() const override;
	virtual bool CanBeCombatTargeted_Implementation() const override;

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

	// ******************** Tutorial Session ********************

	void InitializeTutorialSession(
		TSubclassOf<UTutorialHUDWidget> InHUDWidgetClass,
		TSubclassOf<UTutorialFragmentWidget> InFragmentWidgetClass,
		TSubclassOf<UTutorialInstructionWidget> InInstructionWidgetClass,
		int32 InTotalFragmentCount
	);

	UFUNCTION(BlueprintCallable, Category = "Tutorial Instruction")
	bool ShowTutorialInstruction(
		FName InstructionID,
		const FText& InstructionTitle,
		const FText& InstructionBody);

	UFUNCTION(BlueprintCallable, Category = "Tutorial Instruction")
	void EndTutorialInstruction();

	UFUNCTION(BlueprintPure, Category = "Tutorial Instruction")
	bool GetIsViewingTutorialInstruction() const;

	bool CollectTutorialFragment(
		FName FragmentID,
		const FText& DisplayTitle,
		const FText& DisplayText
	);

	UFUNCTION(BlueprintCallable, Category = "Tutorial Fragment|Testing")
	bool TryCollectTutorialFragmentForTest(FName FragmentID);

	UFUNCTION(BlueprintCallable, Category = "Tutorial Fragment")
	void EndTutorialFragmentView();

	UFUNCTION(BlueprintPure, Category = "Tutorial Fragment")
	bool GetIsViewingTutorialFragment() const;

	UFUNCTION(BlueprintPure, Category = "Tutorial Fragment")
	bool HasCollectedTutorialFragment(FName FragmentID) const;

	UFUNCTION(BlueprintPure, Category = "Tutorial Fragment")
	int32 GetCollectedTutorialFragmentCount() const;

	UFUNCTION(BlueprintPure, Category = "Tutorial Fragment")
	int32 GetTutorialFragmentTotal() const;

	bool ShowTutorialHintOnce(
		FName HintID,
		const FText& HintText,
		float Duration
	);

	bool CanReceiveTutorialInteraction() const;

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

	/*
	* 更新当前 Runtime Checkpoint。
	*
	* 只更新当前 Session 的状态，
	* 不会自动写入 SaveGame。
	*/
	UFUNCTION(
		BlueprintCallable,
		Category = "Persistence|Checkpoint"
	)
	void SetCurrentCheckpointID(
		FName NewCheckpointID
	);

	UFUNCTION(
		BlueprintPure,
		Category = "Persistence|Checkpoint"
	)
	FName GetCurrentCheckpointID() const;

	/*
	* 静默将玩家恢复到已验证的 Checkpoint Transform。
	*
	* 主要用于新 World 启动后的 Resume，
	* 不是战斗中的通用 Quick Load / 时间倒流功能。
	*/
	UFUNCTION(
		BlueprintCallable,
		Category = "Persistence|Checkpoint"
	)
	bool ApplySavedCheckpointState(
		FName SavedCheckpointID,
		const FTransform& ResumeTransform
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "Persistence|Checkpoint"
	)
	bool UnlockCheckpoint(
		FName CheckpointID
	);

	UFUNCTION(
		BlueprintPure,
		Category = "Persistence|Checkpoint"
	)
	bool HasUnlockedCheckpoint(
		FName CheckpointID
	) const;

	const TArray<FName>&
		GetUnlockedCheckpointIDs() const;

	/*
	* 直接设置 Runtime Checkpoint 进度。
	*
	* 用于：
	* - Save 失败时回滚
	* - Load Restore
	*
	* 不移动玩家，不写入磁盘。
	*/
	void ApplyRuntimeCheckpointProgress(
		FName NewCurrentCheckpointID,
		const TArray<FName>&
		NewUnlockedCheckpointIDs
	);

	UFUNCTION(
		BlueprintPure,
		Category = "Checkpoint"
	)
	bool CanUseCheckpoint() const;

	bool OpenCheckpointMenu(
		AWCPlayerCheckpoint* SourceCheckpoint,
		AWCStoryPersistenceCoordinator* Coordinator
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "Checkpoint|UI"
	)
	void CloseCheckpointMenu();

	UFUNCTION(
		BlueprintPure,
		Category = "Checkpoint|UI"
	)
	bool GetIsCheckpointMenuOpen() const;
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

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Checkpoint|UI"
	)
	TSubclassOf<UWCCheckpointMenuWidget>
		CheckpointMenuWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UWCCheckpointMenuWidget>
		ActiveCheckpointMenuWidget;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Transient,
		Category = "Checkpoint|UI"
	)
	bool bIsCheckpointMenuOpen = false;

	UPROPERTY(Transient)
	TObjectPtr<UTutorialHUDWidget> ActiveTutorialHUDWidget;

	UPROPERTY(Transient)
	TObjectPtr<UTutorialFragmentWidget> ActiveTutorialFragmentWidget;

	UPROPERTY(Transient)
	TObjectPtr<UTutorialInstructionWidget> ActiveTutorialInstructionWidget;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
		Category = "Tutorial Fragment")
	bool bIsTutorialSessionActive = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
		Category = "Tutorial Fragment")
	bool bIsViewingTutorialFragment = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
		Category = "Tutorial Instruction")
	bool bIsViewingTutorialInstruction = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
		Category = "Tutorial Fragment")
	TSet<FName> CollectedTutorialFragmentIDs;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
		Category = "Tutorial Fragment")
	TSet<FName> ShownTutorialHintIDs;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
		Category = "Tutorial Instruction")
	TSet<FName> ShownTutorialInstructionIDs;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
		Category = "Tutorial Fragment")
	int32 TutorialFragmentTotal = 3;

	UPROPERTY(Transient)
	TSubclassOf<UTutorialFragmentWidget> TutorialFragmentWidgetClass;

	UPROPERTY(Transient)
	TSubclassOf<UTutorialInstructionWidget> TutorialInstructionWidgetClass;

	bool bInstructionOwnsPause = false;
	bool bWorldWasPausedBeforeInstruction = false;

	/*
	* 当前 Gameplay Session 中，
	* 玩家最近激活的 Checkpoint。
	*
	* Transient：
	* 真正持久化数据位于 UWCGameSaveGame。
	*/
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
		Category = "Persistence|Checkpoint"
	)
	FName CurrentCheckpointID = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
		Category = "Persistence|Checkpoint"
	)
	TArray<FName> UnlockedCheckpointIDs;

	// ******************** Input ********************

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Interact();

	void ToggleMemoryJournal();

	bool OpenMemoryJournal();

	bool StartTutorialFragmentView(
		const FText& DisplayTitle,
		const FText& DisplayText
	);

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
