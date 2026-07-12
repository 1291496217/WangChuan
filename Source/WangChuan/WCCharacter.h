// Fill out your copyright notice in the Description page of Project Settings.

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
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "Camera/CameraShakeBase.h"
#include "WCCharacter.generated.h"

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
	AWCCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(
		class UInputComponent* PlayerInputComponent
	) override;

	// Combat Public API
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

	// Interaction Public API
	IInteractable* CurrentInteractable;

	TSet<int32> CollectedFragments;

	FString CurrentPrompt;

	void ShowInteractionPrompt(const FString& Prompt);

	void HideInteractionPrompt();

protected:
	// Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Feedback")
	TSubclassOf<UCameraShakeBase> HitCameraShakeClass;

	// Input Assets
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

	// Combat Settings
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Combat")
	USoundBase* AttackHitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Combat")
	USoundBase* AttackWhiffSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Combat")
	USoundBase* PlayerDeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Combat")
	USoundBase* PlayerHurtSound;

	// VFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
	UParticleSystem* AttackHitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
	float AttackHitEffectScale = 0.4f;

	// UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI") 
	TSubclassOf<UUserWidget> PlayerHUDClass;

	UPROPERTY()
	UUserWidget* PlayerHUDWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPlayerHitFlashWidget> PlayerHitFlashWidgetClass;

	UPROPERTY()
	UPlayerHitFlashWidget* PlayerHitFlashWidget;

	// Input Functions
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Interact();

	void ShowMemoryJournal();

	void Attack();

	void HeavyAttack();

	// Combat Functions

	void PerformCurrentAttackTrace();

	void PlayLightAttackMontage(FName SectionName);

	void StartAttackTimer(float Duration);

	void EndAttack();

	void Die();

	void ShowAttackHitDebug(AActor* HitActor);

	void EnterCombatState();

	void ExitCombatState();

	// Helper Functions
	bool CanAct() const;

	void PlayAttackHitSound();

	void PlayAttackWhiffSound();

	void PlayAttackHitEffect(const FHitResult& HitResult);

	void PlayPlayerHitFeedback();

	void PlayComboFinisherFeedback(const FHitResult& HitResult);

	// Combo Helper
	void ResetLightCombo();

	FName GetLightComboSectionName(int32 ComboIndex) const;

	FPlayerAttackData GetLightComboAttackData(int32 ComboIndex) const;

	void AdvancedLightCombo();

	void OpenLightComboWindow();

	void CloseLightComboWindow();

	void ConsumeBufferedLightAttack();

	void ClearLightComboBuffer();
};

