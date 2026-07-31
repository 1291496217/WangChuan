#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Components/WidgetComponent.h"
#include "Sound/SoundBase.h"
#include "GhostEnemy.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class UMaterialInstanceDynamic;
class AWCCharacter;
class AGhostEnemy;

/*
 * 当 Ghost Enemy 正式进入死亡状态时广播。
 *
 * 参数：
 * DefeatedEnemy = 被击败的敌人实例。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGhostEnemyDefeatedSignature, AGhostEnemy*,
											DefeatedEnemy);

UCLASS()
class WANGCHUAN_API AGhostEnemy : public AActor
{
	GENERATED_BODY()

public:
	// ******************** Construction ********************

	AGhostEnemy();

	// ******************** Events ********************

	/*
	 * 当敌人第一次正式进入死亡状态时广播。
	 *
	 * BlueprintAssignable 允许 Blueprint
	 * 或其他 Actor 监听该事件。
	 */
	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
	FOnGhostEnemyDefeatedSignature OnEnemyDefeated;

protected:
	// ******************** Lifecycle ********************

	virtual void BeginPlay() override;

public:
	// ******************** Lifecycle ********************

	virtual void Tick(float DeltaTime) override;

protected:
	// ******************** Components ********************

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* EnemyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* HealthWidgetComponent;

	// ******************** Configuration ********************

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GroundTraceStartHeight = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GroundTraceEndDepth = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GroundOffset = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsMoving = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsAttacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsHitReacting = false;

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

	// ******************** Runtime State ********************

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial; // 运行时颜色反馈使用的动态材质。

	FTimerHandle HitFeedbackTimerHandle;

	FTimerHandle HitReactionTimerHandle;

	FTimerHandle EnemyAttackCooldownTimerHandle;

	FTimerHandle EnemyAttackDurationTimerHandle;

	FTimerHandle DeathTimerHandle;

public:
	// ******************** Combat ********************

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TakeHit(float DamageAmount, FVector HitDirection, float KnockbackStrength);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnEnemyAttackHit();

	/*
	* 将 Enemy 静默恢复为“此前已经被击败”的稳定状态。
	*
	* 不调用 Die()，不播放音效，不广播 OnEnemyDefeated，
	* 不启动死亡 Timer，也不 Destroy Actor。
	*
	* Actor 保留在 World 中但隐藏和禁用，
	* 使重复 Restore 仍然安全。
	*/
	UFUNCTION(BlueprintCallable, Category = "Enemy|Persistence")
	void ApplyPersistentDefeatedState();

	// ******************** Getters ********************

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

protected:
	// ******************** Combat ********************

	void Die();

	void FinishDeath();

	void ClearCombatTimers();

	// ******************** State Queries ********************

	AWCCharacter* GetPlayerCharacter() const;

	bool IsPlayerValidAndAlive() const;

	bool CanUpdateBehavior() const;

	bool CanStartAttack() const;

	// ******************** Hit Reaction ********************

	void ApplyKnockback(FVector KnockbackDirection, float KnockbackStrength);

	void StartHitReaction();

	void EndHitReaction();

	// ******************** UI ********************

	void UpdateHealthWidgetFacingCamera();

	// ******************** Enemy Behavior ********************

	void UpdateEnemyBehavior(float DeltaTime);

	void MoveTowardPlayer(APawn* PlayerPawn, float DeltaTime);

	void TryAttackPlayer();

	void DealDamageToPlayer();

	void EndEnemyAttack();

	void ResetEnemyAttack();

	// ******************** Audio ********************

	void PlayEvilGhostAttackHitSound();
	void PlayEvilGhostAttackWhiffSound();
	void PlayEvilGhostHurtSound();

	// ******************** Movement ********************

	void SnapToGround();
};
