#include "GhostEnemy.h"

#include "WCGhostAIController.h"
#include "WCCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "EnemyHealthBarWidget.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"

AGhostEnemy::AGhostEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCapsule"));
	CollisionCapsule->InitCapsuleSize(50.0f, 100.0f);
	CollisionCapsule->SetCollisionProfileName(TEXT("Pawn"));
	CollisionCapsule->SetCanEverAffectNavigation(false);
	RootComponent = CollisionCapsule;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SceneRoot->SetupAttachment(CollisionCapsule);
	// The legacy actor origin was at its feet. Keep the visuals in that space while
	// the Pawn's actor origin correctly represents the center of its collision capsule.
	SceneRoot->SetRelativeLocation(FVector(0.0f, 0.0f, -103.0f));

	EnemyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EnemyMesh"));
	EnemyMesh->SetupAttachment(SceneRoot);
	EnemyMesh->SetCanEverAffectNavigation(false);

	HealthWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidgetComponent"));
	HealthWidgetComponent->SetupAttachment(SceneRoot);
	HealthWidgetComponent->SetCanEverAffectNavigation(false);
	HealthWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
	HealthWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	HealthWidgetComponent->SetDrawSize(FVector2D(220.0f, 18.0f));
	HealthWidgetComponent->SetRelativeScale3D(FVector(0.25f));

	FloatingMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));
	FloatingMovement->UpdatedComponent = CollisionCapsule;
	FloatingMovement->Acceleration = 800.0f;
	FloatingMovement->Deceleration = 1200.0f;
	FloatingMovement->TurningBoost = 8.0f;

	AIControllerClass = AWCGhostAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw = true;

	MaxHealth = 100.0f;
	Health = MaxHealth;
}

void AGhostEnemy::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	// Existing Ghost Blueprints can retain the old SceneRoot transform after the
	// native root changed from a scene component to a capsule. Enforce the intended
	// feet-at-ground relationship for those existing assets at runtime.
	if (SceneRoot && CollisionCapsule)
	{
		SceneRoot->SetRelativeLocation(FVector(0.0f, 0.0f,
			-CollisionCapsule->GetUnscaledCapsuleHalfHeight() + VisualGroundOffset));
	}
	SnapToGround();
	HomeLocation = GetActorLocation();
	HomeRotation = GetActorRotation();
	SetAIState(EGhostAIState::Idle);

	if (FloatingMovement)
	{
		FloatingMovement->MaxSpeed = MoveSpeed;
	}

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
		if (UEnemyHealthBarWidget* HealthWidget =
			Cast<UEnemyHealthBarWidget>(HealthWidgetComponent->GetUserWidgetObject()))
		{
			HealthWidget->SetEnemyOwner(this);
		}
	}
}

void AGhostEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateEnemyBehavior();
	UpdateHealthWidgetFacingCamera();
}

UPawnMovementComponent* AGhostEnemy::GetMovementComponent() const
{
	return FloatingMovement;
}

void AGhostEnemy::TakeHit(float DamageAmount, FVector HitDirection, float KnockbackStrength)
{
	if (bIsDead)
	{
		return;
	}

	Health -= DamageAmount;

	// Damage is explicit hostility: a back attack must wake an otherwise idle enemy.
	if (AWCGhostAIController* GhostController = GetGhostAIController())
	{
		GhostController->HandleDamageAggro(GetPlayerCharacter());
	}

	if (Health <= 0.0f)
	{
		Die();
		return;
	}

	PlayEvilGhostHurtSound();
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
	SetAIState(EGhostAIState::Dead);

	if (AWCGhostAIController* GhostController = GetGhostAIController())
	{
		GhostController->HandlePawnDeath();
	}

	if (EvilGhostDeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EvilGhostDeathSound, GetActorLocation());
	}

	ClearCombatTimers();

	if (HealthWidgetComponent)
	{
		HealthWidgetComponent->SetVisibility(false);
	}

	if (EnemyMesh)
	{
		EnemyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (CollisionCapsule)
	{
		CollisionCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	OnEnemyDefeated.Broadcast(this);
	GetWorldTimerManager().SetTimer(DeathTimerHandle, this, &AGhostEnemy::FinishDeath,
		DeathDestroyDelay, false);
}

void AGhostEnemy::FinishDeath()
{
	GetWorldTimerManager().ClearTimer(DeathTimerHandle);
	Destroy();
}

void AGhostEnemy::StartHitReaction()
{
	if (bIsDead)
	{
		return;
	}

	bIsHitReacting = true;
	bIsMoving = false;
	bIsAttacking = false;

	if (AWCGhostAIController* GhostController = GetGhostAIController())
	{
		GhostController->PauseForHitReaction();
	}

	GetWorldTimerManager().ClearTimer(HitReactionTimerHandle);
	GetWorldTimerManager().ClearTimer(EnemyAttackDurationTimerHandle);
	GetWorldTimerManager().SetTimer(HitReactionTimerHandle, this,
		&AGhostEnemy::EndHitReaction, HitReactionDuration, false);
}

void AGhostEnemy::EndHitReaction()
{
	if (bIsDead)
	{
		return;
	}

	bIsHitReacting = false;
	if (AWCGhostAIController* GhostController = GetGhostAIController())
	{
		GhostController->ResumeAfterAttack();
	}
}

void AGhostEnemy::ApplyKnockback(FVector KnockbackDirection, float KnockbackStrength)
{
	if (bIsDead)
	{
		return;
	}

	KnockbackDirection.Z = 0.0f;
	if (!KnockbackDirection.Normalize())
	{
		return;
	}

	SetActorLocation(GetActorLocation() + KnockbackDirection * KnockbackStrength, true);
}

AWCCharacter* AGhostEnemy::GetPlayerCharacter() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const APlayerController* PlayerController = World->GetFirstPlayerController();
	return PlayerController ? Cast<AWCCharacter>(PlayerController->GetPawn()) : nullptr;
}

bool AGhostEnemy::IsPlayerValidAndAlive() const
{
	const AWCCharacter* PlayerCharacter = GetPlayerCharacter();
	return PlayerCharacter && !PlayerCharacter->GetIsDead();
}

bool AGhostEnemy::CanUpdateBehavior() const
{
	return !bIsDead && !bIsHitReacting && !bIsAttacking;
}

bool AGhostEnemy::CanStartAttack() const
{
	return !bIsDead && !bIsHitReacting && !bIsAttacking && bCanAttackPlayer;
}

void AGhostEnemy::ApplyPersistentDefeatedState()
{
	ClearCombatTimers();
	GetWorldTimerManager().ClearTimer(DeathTimerHandle);
	Health = 0.0f;
	bIsDead = true;
	bIsMoving = false;
	bIsAttacking = false;
	bIsHitReacting = false;
	bCanAttackPlayer = false;
	SetAIState(EGhostAIState::Dead);

	if (AWCGhostAIController* GhostController = GetGhostAIController())
	{
		GhostController->HandlePawnDeath();
	}

	if (HealthWidgetComponent)
	{
		HealthWidgetComponent->SetVisibility(false);
	}
	if (EnemyMesh)
	{
		EnemyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (CollisionCapsule)
	{
		CollisionCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
	SetActorHiddenInGame(true);

	UE_LOG(LogTemp, Log, TEXT("Ghost Enemy [%s] silently restored to persistent defeated state."),
		*GetName());
}

void AGhostEnemy::UpdateEnemyBehavior()
{
	AWCGhostAIController* GhostController = GetGhostAIController();
	AWCCharacter* PlayerCharacter = GhostController ? GhostController->GetTargetPlayer() : nullptr;

	if (!CanUpdateBehavior())
	{
		bIsMoving = false;
		return;
	}

	if (AIState != EGhostAIState::Chasing)
	{
		bIsMoving = AIState == EGhostAIState::Investigating ||
			AIState == EGhostAIState::ReturningHome;
		return;
	}

	if (!IsValid(PlayerCharacter) || PlayerCharacter->GetIsDead())
	{
		bIsMoving = false;
		if (GhostController)
		{
			GhostController->BeginReturnHome();
		}
		return;
	}

	const float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation());
	if (DistanceToPlayer <= AttackRange)
	{
		bIsMoving = false;
		GhostController->StopMovement();
		TryAttackPlayer();
		return;
	}

	bIsMoving = true;
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
	SetAIState(EGhostAIState::Attacking);

	GetWorldTimerManager().SetTimer(EnemyAttackDurationTimerHandle, this,
		&AGhostEnemy::EndEnemyAttack, EnemyAttackDuration, false);
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
	if (AWCGhostAIController* GhostController = GetGhostAIController())
	{
		GhostController->ResumeAfterAttack();
	}
}

void AGhostEnemy::ResetEnemyAttack()
{
	if (!bIsDead)
	{
		bCanAttackPlayer = true;
	}
}

void AGhostEnemy::DealDamageToPlayer()
{
	AWCCharacter* PlayerCharacter = GetPlayerCharacter();
	if (PlayerCharacter && !PlayerCharacter->GetIsDead())
	{
		PlayerCharacter->ReceiveDamage(EnemyAttackDamage);
	}
}

void AGhostEnemy::OnEnemyAttackHit()
{
	if (bIsDead || bIsHitReacting || !bIsAttacking)
	{
		return;
	}

	AWCCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || PlayerCharacter->GetIsDead())
	{
		return;
	}

	if (FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation()) > AttackRange)
	{
		PlayEvilGhostAttackWhiffSound();
		return;
	}

	PlayEvilGhostAttackHitSound();
	DealDamageToPlayer();
}

bool AGhostEnemy::GetIsMoving() const { return bIsMoving; }
bool AGhostEnemy::GetIsDead() const { return bIsDead; }
bool AGhostEnemy::GetIsAttacking() const { return bIsAttacking; }
bool AGhostEnemy::GetIsHitReacting() const { return bIsHitReacting; }
float AGhostEnemy::GetHealth() const { return Health; }
EGhostAIState AGhostEnemy::GetAIState() const { return AIState; }
FVector AGhostEnemy::GetHomeLocation() const { return HomeLocation; }

float AGhostEnemy::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f) : 0.0f;
}

void AGhostEnemy::UpdateHealthWidgetFacingCamera()
{
	if (!HealthWidgetComponent)
	{
		return;
	}

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
	const FVector DirectionToCamera = CameraLocation - HealthWidgetComponent->GetComponentLocation();
	if (!DirectionToCamera.IsNearlyZero())
	{
		HealthWidgetComponent->SetWorldRotation(
			FRotator(0.0f, DirectionToCamera.Rotation().Yaw, 0.0f));
	}
}

void AGhostEnemy::PlayEvilGhostAttackHitSound()
{
	USoundBase* SelectedSound = FMath::RandBool() ? EvilGhostAttackHitSound01 : EvilGhostAttackHitSound02;
	if (SelectedSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SelectedSound, GetActorLocation());
	}
}

void AGhostEnemy::PlayEvilGhostAttackWhiffSound()
{
	if (EvilGhostAttackWhiffSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EvilGhostAttackWhiffSound, GetActorLocation());
	}
}

void AGhostEnemy::PlayEvilGhostHurtSound()
{
	if (EvilGhostHurtSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EvilGhostHurtSound, GetActorLocation());
	}
}

void AGhostEnemy::SnapToGround()
{
	const FVector ActorLocation = GetActorLocation();
	const FVector TraceStart = ActorLocation + FVector(0.0f, 0.0f, GroundTraceStartHeight);
	const FVector TraceEnd = ActorLocation - FVector(0.0f, 0.0f, GroundTraceEndDepth);
	TArray<AActor*> ActorsToIgnore{this};
	FHitResult HitResult;

	const bool bHitGround = UKismetSystemLibrary::LineTraceSingle(
		this, TraceStart, TraceEnd, UEngineTypes::ConvertToTraceType(ECC_Visibility), false,
		ActorsToIgnore, EDrawDebugTrace::None, HitResult, true);
	if (bHitGround)
	{
		// A Pawn's location is the capsule center, not its feet. Placing the center
		// directly on the floor leaves half the capsule embedded; movement collision
		// then depenetrates it upward and makes the mesh appear to float.
		const float CapsuleHalfHeight = CollisionCapsule
			? CollisionCapsule->GetScaledCapsuleHalfHeight()
			: 0.0f;
		SetActorLocation(FVector(ActorLocation.X, ActorLocation.Y,
			HitResult.ImpactPoint.Z + CapsuleHalfHeight + GroundOffset), true);
	}
}

void AGhostEnemy::SetAIState(EGhostAIState NewState)
{
	if (AIState == NewState)
	{
		return;
	}

	if (bShowAIDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("Ghost [%s] state %d -> %d"), *GetName(),
			static_cast<int32>(AIState), static_cast<int32>(NewState));
	}

	AIState = NewState;
	bIsMoving = NewState == EGhostAIState::Chasing ||
		NewState == EGhostAIState::Investigating ||
		NewState == EGhostAIState::ReturningHome;
}

AWCGhostAIController* AGhostEnemy::GetGhostAIController() const
{
	return Cast<AWCGhostAIController>(GetController());
}
