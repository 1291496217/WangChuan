#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Components/WidgetComponent.h"
#include "Sound/SoundBase.h"
#include "GhostEnemy.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class UMaterialInstanceDynamic;
class UCapsuleComponent;
class UFloatingPawnMovement;
class AWCCharacter;
class AWCGhostAIController;
class AGhostEnemy;

UENUM(BlueprintType)
enum class EGhostAIState : uint8
{
	Idle,
	Chasing,
	Investigating,
	ReturningHome,
	Attacking,
	Dead
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGhostEnemyDefeatedSignature, AGhostEnemy*,
	DefeatedEnemy);

UCLASS()
class WANGCHUAN_API AGhostEnemy : public APawn
{
	GENERATED_BODY()

public:
	AGhostEnemy();

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
	FOnGhostEnemyDefeatedSignature OnEnemyDefeated;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCapsuleComponent* CollisionCapsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* EnemyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* HealthWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UFloatingPawnMovement* FloatingMovement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float DeathDestroyDelay = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	FLinearColor NormalColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	FLinearColor HitColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	float HitFlashDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	float HitReactionDuration = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Behavior")
	float AttackRange = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Behavior")
	float AttackCooldown = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Behavior")
	float EnemyAttackDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Behavior")
	float EnemyAttackDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Behavior")
	float MoveSpeed = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Sight", meta = (ClampMin = "0.0"))
	float SightRadius = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Sight", meta = (ClampMin = "0.0"))
	float LoseSightRadius = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Sight", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float PeripheralVisionHalfAngle = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Sight", meta = (ClampMin = "0.0"))
	float SightMaxAge = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Investigation", meta = (ClampMin = "0.0"))
	float LostSightGraceTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Investigation", meta = (ClampMin = "0.0"))
	float InvestigateWaitTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Leash", meta = (ClampMin = "0.0"))
	float MaxChaseDistanceFromHome = 2200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Debug")
	bool bShowAIDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GroundTraceStartHeight = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GroundTraceEndDepth = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GroundOffset = 0.0f;

	// Visual-only adjustment for animation/mesh feet. This does not move the
	// collision capsule or change the NavMesh agent height.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float VisualGroundOffset = -3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsMoving = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsAttacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsHitReacting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Debug")
	EGhostAIState AIState = EGhostAIState::Idle;

	bool bCanAttackPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Combat")
	USoundBase* EvilGhostAttackHitSound01;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Combat")
	USoundBase* EvilGhostAttackHitSound02;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Combat")
	USoundBase* EvilGhostAttackWhiffSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Combat")
	USoundBase* EvilGhostHurtSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Combat")
	USoundBase* EvilGhostDeathSound;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;

	FTimerHandle HitFeedbackTimerHandle;
	FTimerHandle HitReactionTimerHandle;
	FTimerHandle EnemyAttackCooldownTimerHandle;
	FTimerHandle EnemyAttackDurationTimerHandle;
	FTimerHandle DeathTimerHandle;

	FVector HomeLocation = FVector::ZeroVector;
	FRotator HomeRotation = FRotator::ZeroRotator;

public:
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TakeHit(float DamageAmount, FVector HitDirection, float KnockbackStrength);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnEnemyAttackHit();

	UFUNCTION(BlueprintCallable, Category = "Enemy|Persistence")
	void ApplyPersistentDefeatedState();

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetHealth() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool GetIsMoving() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool GetIsDead() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool GetIsAttacking() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool GetIsHitReacting() const;

	UFUNCTION(BlueprintPure, Category = "AI|Debug")
	EGhostAIState GetAIState() const;

	UFUNCTION(BlueprintPure, Category = "AI|Debug")
	FVector GetHomeLocation() const;

protected:
	void Die();
	void FinishDeath();
	void ClearCombatTimers();
	AWCCharacter* GetPlayerCharacter() const;
	bool IsPlayerValidAndAlive() const;
	bool CanUpdateBehavior() const;
	bool CanStartAttack() const;
	void ApplyKnockback(FVector KnockbackDirection, float KnockbackStrength);
	void StartHitReaction();
	void EndHitReaction();
	void UpdateHealthWidgetFacingCamera();
	void UpdateEnemyBehavior();
	void TryAttackPlayer();
	void DealDamageToPlayer();
	void EndEnemyAttack();
	void ResetEnemyAttack();
	void PlayEvilGhostAttackHitSound();
	void PlayEvilGhostAttackWhiffSound();
	void PlayEvilGhostHurtSound();
	void SnapToGround();

private:
	friend class AWCGhostAIController;
	void SetAIState(EGhostAIState NewState);
	AWCGhostAIController* GetGhostAIController() const;
};
