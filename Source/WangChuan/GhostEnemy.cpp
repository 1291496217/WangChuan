#include "GhostEnemy.h"
#include "WCCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "EnemyHealthBarWidget.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Engine.h"

// ******************** Construction ********************

AGhostEnemy::AGhostEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	EnemyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EnemyMesh"));
	EnemyMesh->SetupAttachment(SceneRoot);

	HealthWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidgetComponent"));
	HealthWidgetComponent->SetupAttachment(SceneRoot);

	HealthWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));

	HealthWidgetComponent->SetWidgetSpace(EWidgetSpace::World);

	HealthWidgetComponent->SetDrawSize(FVector2D(220.0f, 18.0f));

	HealthWidgetComponent->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.25f));

	MaxHealth = 100.0f;
	Health = MaxHealth;
}

// ******************** Lifecycle ********************

void AGhostEnemy::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	if (EnemyMesh)
	{
		DynamicMaterial = EnemyMesh->CreateAndSetMaterialInstanceDynamic(0);

		if (DynamicMaterial)
		{
			DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), NormalColor);
		}
	}

	if (HealthWidgetComponent)
	{
		UEnemyHealthBarWidget* HealthWidget =
			Cast<UEnemyHealthBarWidget>(HealthWidgetComponent->GetUserWidgetObject());

		if (HealthWidget)
		{
			HealthWidget->SetEnemyOwner(this);
		}
	}
}

// ******************** Combat ********************

void AGhostEnemy::TakeHit(float DamageAmount, FVector HitDirection, float KnockbackStrength)
{

	if (bIsDead)
	{
		return;
	}

	Health -= DamageAmount;

	if (Health <= 0.0f)
	{
		// 致命一击直接进入死亡流程，不再播放普通受击反馈。
		Die();
		return;
	}

	PlayEvilGhostHurtSound();

	// ShowHitFeedback();
	StartHitReaction();
	ApplyKnockback(HitDirection, KnockbackStrength);
}

void AGhostEnemy::ClearCombatTimers()
{
	GetWorldTimerManager().ClearTimer(HitFeedbackTimerHandle);
	GetWorldTimerManager().ClearTimer(HitReactionTimerHandle);
	GetWorldTimerManager().ClearTimer(EnemyAttackCooldownTimerHandle);
	GetWorldTimerManager().ClearTimer(EnemyAttackDurationTimerHandle);
}

void AGhostEnemy::Die()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	bIsMoving = false;
	bIsAttacking = false;
	bCanAttackPlayer = false;
	bIsHitReacting = false;

	if (EvilGhostDeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EvilGhostDeathSound, GetActorLocation());
	}

	ClearCombatTimers();

	if (HealthWidgetComponent)
	{
		HealthWidgetComponent->SetVisibility(false);
	}

	// 死亡后立即关闭碰撞，避免继续阻挡玩家。
	if (EnemyMesh)
	{
		EnemyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	/*
	 * 此时敌人已经进入稳定死亡状态。
	 * Encounter 可以安全处理叙事事件。
	 */
	OnEnemyDefeated.Broadcast(this);

	GetWorldTimerManager().SetTimer(DeathTimerHandle, this, &AGhostEnemy::FinishDeath,
									DeathDestroyDelay, false);
}

void AGhostEnemy::FinishDeath()
{
	GetWorldTimerManager().ClearTimer(DeathTimerHandle);
	Destroy();
}

// ******************** Hit Reaction ********************

void AGhostEnemy::StartHitReaction()
{
	if (bIsDead)
	{
		return;
	}

	bIsHitReacting = true;
	bIsMoving = false;
	bIsAttacking = false;

	GetWorldTimerManager().ClearTimer(HitReactionTimerHandle);

	GetWorldTimerManager().ClearTimer(EnemyAttackDurationTimerHandle);

	GetWorldTimerManager().SetTimer(HitReactionTimerHandle, this, &AGhostEnemy::EndHitReaction,
									HitReactionDuration, false);
}

void AGhostEnemy::EndHitReaction()
{
	if (bIsDead)
	{
		return;
	}
	bIsHitReacting = false;
}

void AGhostEnemy::ApplyKnockback(FVector KnockbackDirection, float KnockbackStrength)
{

	if (bIsDead)
	{
		return;
	}

	KnockbackDirection.Z = 0.0f;

	if (KnockbackDirection.IsNearlyZero())
	{
		return;
	}

	KnockbackDirection.Normalize();

	FVector NewLocation = GetActorLocation() + KnockbackDirection * KnockbackStrength;

	SetActorLocation(NewLocation);
}

// ******************** State Queries ********************

AWCCharacter* AGhostEnemy::GetPlayerCharacter() const
{
	UWorld* World = GetWorld();

	if (World == nullptr)
	{
		return nullptr;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();

	if (PlayerController == nullptr)
	{
		return nullptr;
	}

	APawn* PlayerPawn = PlayerController->GetPawn();

	if (PlayerPawn == nullptr)
	{
		return nullptr;
	}

	return Cast<AWCCharacter>(PlayerPawn);
}

bool AGhostEnemy::IsPlayerValidAndAlive() const
{
	AWCCharacter* PlayerCharacter = GetPlayerCharacter();

	if (PlayerCharacter == nullptr)
	{
		return false;
	}

	if (PlayerCharacter->GetIsDead())
	{
		return false;
	}
	return true;
}

bool AGhostEnemy::CanUpdateBehavior() const
{
	if (bIsDead)
	{
		return false;
	}
	if (bIsHitReacting)
	{
		return false;
	}
	if (bIsAttacking)
	{
		return false;
	}
	return true;
}

bool AGhostEnemy::CanStartAttack() const
{
	if (bIsDead)
	{
		return false;
	}
	if (bIsHitReacting)
	{
		return false;
	}
	if (bIsAttacking)
	{
		return false;
	}
	if (!bCanAttackPlayer)
	{
		return false;
	}
	return true;
}
// ******************** Enemy Behavior ********************

void AGhostEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateEnemyBehavior(DeltaTime);

	UpdateHealthWidgetFacingCamera();
}

// 根据玩家距离在攻击、追逐和待机状态之间更新行为。
void AGhostEnemy::UpdateEnemyBehavior(float DeltaTime)
{
	// 状态优先级：死亡、受击、攻击、移动。
	if (!CanUpdateBehavior())
	{
		bIsMoving = false;
		return;
	}

	if (!IsPlayerValidAndAlive())
	{
		bIsMoving = false;
		return;
	}

	AWCCharacter* PlayerCharacter = GetPlayerCharacter();

	if (PlayerCharacter == nullptr)
	{
		bIsMoving = false;
		return;
	}

	float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation());

	if (DistanceToPlayer <= AttackRange)
	{
		bIsMoving = false;
		TryAttackPlayer();
		return;
	}

	if (DistanceToPlayer <= ChaseRange)
	{
		bIsMoving = true;
		MoveTowardPlayer(PlayerCharacter, DeltaTime);
		return;
	}

	bIsMoving = false;
}

// 向玩家位置移动，并保持朝向玩家。
void AGhostEnemy::MoveTowardPlayer(APawn* PlayerPawn, float DeltaTime)
{
	if (PlayerPawn == nullptr)
	{
		return;
	}
	FVector Direction = PlayerPawn->GetActorLocation() - GetActorLocation();

	Direction.Z = 0.0f;

	if (Direction.IsNearlyZero())
	{
		return;
	}

	Direction.Normalize();

	FVector NewLocation = GetActorLocation() + Direction * MoveSpeed * DeltaTime;

	SetActorLocation(NewLocation);

	SnapToGround();

	FRotator NewRotation = Direction.Rotation();

	SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));
}

void AGhostEnemy::TryAttackPlayer()
{
	if (!CanStartAttack())
	{
		return;
	}

	bCanAttackPlayer = false;
	bIsAttacking = true;
	bIsMoving = false;

	// DealDamageToPlayer();

	// 攻击动画结束后解除攻击状态。
	GetWorldTimerManager().SetTimer(EnemyAttackDurationTimerHandle, this,
									&AGhostEnemy::EndEnemyAttack, EnemyAttackDuration, false);
	// 冷却结束后允许下一次攻击。
	GetWorldTimerManager().SetTimer(EnemyAttackCooldownTimerHandle, this,
									&AGhostEnemy::ResetEnemyAttack, AttackCooldown, false);
}

void AGhostEnemy::EndEnemyAttack()
{
	if (bIsDead)
	{
		return;
	}
	bIsAttacking = false;
}

void AGhostEnemy::ResetEnemyAttack()
{
	if (bIsDead)
	{
		return;
	}
	bCanAttackPlayer = true;
}

void AGhostEnemy::DealDamageToPlayer()
{
	AWCCharacter* PlayerCharacter = GetPlayerCharacter();

	if (PlayerCharacter == nullptr)
	{
		return;
	}

	if (PlayerCharacter->GetIsDead())
	{
		return;
	}

	PlayerCharacter->ReceiveDamage(EnemyAttackDamage);
}

void AGhostEnemy::OnEnemyAttackHit()
{
	if (bIsDead)
	{
		return;
	}

	if (bIsHitReacting)
	{
		return;
	}

	if (!bIsAttacking)
	{
		return;
	}

	AWCCharacter* PlayerCharacter = GetPlayerCharacter();

	if (PlayerCharacter == nullptr)
	{
		return;
	}

	if (PlayerCharacter->GetIsDead())
	{
		return;
	}

	float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation());

	if (DistanceToPlayer > AttackRange)
	{
		PlayEvilGhostAttackWhiffSound();
		return;
	}

	PlayEvilGhostAttackHitSound();

	DealDamageToPlayer();
}

// ******************** Getters ********************

bool AGhostEnemy::GetIsMoving() const
{
	return bIsMoving;
}

bool AGhostEnemy::GetIsDead() const
{
	return bIsDead;
}

bool AGhostEnemy::GetIsAttacking() const
{
	return bIsAttacking;
}

bool AGhostEnemy::GetIsHitReacting() const
{
	return bIsHitReacting;
}

float AGhostEnemy::GetHealth() const
{
	return Health;
}

float AGhostEnemy::GetHealthPercent() const
{
	if (MaxHealth <= 0.0f)
	{
		return 0.0f;
	}
	return FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f);
	;
}

// ******************** UI ********************

void AGhostEnemy::UpdateHealthWidgetFacingCamera()
{
	if (HealthWidgetComponent == nullptr)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (World == nullptr)
	{
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();

	if (PlayerController == nullptr)
	{
		return;
	}

	FVector CameraLocation;
	FRotator CameraRotation;

	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector WidgetLocation = HealthWidgetComponent->GetComponentLocation();

	FVector DirectionToCamera = CameraLocation - WidgetLocation;

	if (DirectionToCamera.IsNearlyZero())
	{
		return;
	}

	FRotator LookAtRotation = DirectionToCamera.Rotation();

	HealthWidgetComponent->SetWorldRotation(FRotator(0.0f, LookAtRotation.Yaw, 0.0f));
}

// ******************** Audio ********************

void AGhostEnemy::PlayEvilGhostAttackHitSound()
{
	USoundBase* SelectedSound = nullptr;

	int32 RandomIndex = FMath::RandRange(0, 1);

	if (RandomIndex == 0)
	{
		SelectedSound = EvilGhostAttackHitSound01;
	}
	else
	{
		SelectedSound = EvilGhostAttackHitSound02;
	}

	if (SelectedSound == nullptr)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(this, SelectedSound, GetActorLocation());
}

void AGhostEnemy::PlayEvilGhostAttackWhiffSound()
{
	if (EvilGhostAttackWhiffSound == nullptr)
	{
		return;
	}
	UGameplayStatics::PlaySoundAtLocation(this, EvilGhostAttackWhiffSound, GetActorLocation());
}

void AGhostEnemy::PlayEvilGhostHurtSound()
{
	if (EvilGhostHurtSound == nullptr)
	{
		return;
	}
	UGameplayStatics::PlaySoundAtLocation(this, EvilGhostHurtSound, GetActorLocation());
}

// ******************** Movement ********************

void AGhostEnemy::SnapToGround()
{
	FVector ActorLocation = GetActorLocation();

	FVector TraceStart = ActorLocation + FVector(0.0f, 0.0f, GroundTraceStartHeight);

	FVector TraceEnd = ActorLocation - FVector(0.0f, 0.0f, GroundTraceEndDepth);

	TArray<AActor*> ActorToIgnore;
	ActorToIgnore.Add(this);

	FHitResult HitResult;

	bool bHitGround = UKismetSystemLibrary::LineTraceSingle(
		this, TraceStart, TraceEnd, UEngineTypes::ConvertToTraceType(ECC_Visibility), false,
		ActorToIgnore, EDrawDebugTrace::None, HitResult, true);

	if (!bHitGround)
	{
		return;
	}

	FVector NewLocation = ActorLocation;
	NewLocation.Z = HitResult.ImpactPoint.Z + GroundOffset;

	SetActorLocation(NewLocation);
}
