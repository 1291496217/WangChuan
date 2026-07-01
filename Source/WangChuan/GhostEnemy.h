// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "GhostEnemy.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class UMaterialInstanceDynamic;
class AWCCharacter;

UCLASS()
class WANGCHUAN_API AGhostEnemy : public AActor
{
	GENERATED_BODY()
	
public:
	AGhostEnemy();

protected:
	virtual void BeginPlay() override;

public: 
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* EnemyMesh;

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
	float ChaseRange = 600.0f;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsMoving = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsAttacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsHitReacting = false;

	bool bCanAttackPlayer = true;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial; // change enemy's color in runtime

	FTimerHandle HitFeedbackTimerHandle;

	FTimerHandle HitReactionTimerHandle;

	FTimerHandle EnemyAttackCooldownTimerHandle;

	FTimerHandle EnemyAttackDurationTimerHandle;

	FTimerHandle DeathTimerHandle;

public:	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TakeHit(
		float DamageAmount,
		FVector HitDirection,
		float KnockbackStrength
	);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnEnemyAttackHit();

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool GetIsMoving() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool GetIsDead() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool GetIsAttacking() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool GetIsHitReacting() const;

protected:
	// Combat state
	void Die();

	void FinishDeath();

	void ClearCombatTimers();

	// Player helper
	AWCCharacter* GetPlayerCharacter() const;

	bool IsPlayerValidAndAlive() const;

	// Enemy behavior helper
	bool CanUpdateBehavior() const;

	bool CanStartAttack() const;

	// Reaction
	void ApplyKnockback(
		FVector KnockbackDirection, 
		float KnockbackStrength
	);

	void StartHitReaction();

	void EndHitReaction();

	// Legacy color feedback. currently replaced by hit reaction animation.
	//void ShowHitFeedback();
	//void ResetHitFeedback();

	// Enemy behavior 
	void UpdateEnemyBehavior(float DeltaTime);

	void MoveTowardPlayer(APawn* PlayerPawn, float DeltaTime);

	void TryAttackPlayer();

	void DealDamageToPlayer();

	void EndEnemyAttack();

	void ResetEnemyAttack();
};
