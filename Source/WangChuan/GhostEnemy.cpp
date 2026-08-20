#include "GhostEnemy.h"

#include "WCGhostAIController.h"
#include "WCCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "EnemyHealthBarWidget.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NavigationSystem.h"

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
	// Compatibility entry point for the existing player attack trace. Week10
	// enemy attacks use TakeDamage below so the instigator is preserved.
	AActor* DamageCauser = nullptr;
	if (const UWorld* World = GetWorld())
	{
		if (const APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			DamageCauser = PlayerController->GetPawn();
		}
	}
	ApplyCombatHit(DamageAmount, HitDirection, KnockbackStrength, DamageCauser);
}

float AGhostEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	const float AppliedDamage = Super::TakeDamage(
		DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (AppliedDamage <= 0.0f || bIsDead)
	{
		return 0.0f;
	}

	FVector HitDirection = DamageCauser
		? GetActorLocation() - DamageCauser->GetActorLocation()
		: FVector::ZeroVector;
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		HitDirection = static_cast<const FPointDamageEvent&>(DamageEvent).ShotDirection;
	}

	ApplyCombatHit(AppliedDamage, HitDirection, 0.0f, DamageCauser);
	return AppliedDamage;
}

bool AGhostEnemy::IsCombatantAlive_Implementation() const
{
	return !bIsDead;
}

EWCCombatFaction AGhostEnemy::GetCombatFaction_Implementation() const
{
	return CombatFaction;
}

bool AGhostEnemy::CanBeCombatTargeted_Implementation() const
{
	return bCanBeCombatTargeted && !bIsDead;
}

void AGhostEnemy::ApplyCombatHit(float DamageAmount, FVector HitDirection,
	float KnockbackStrength, AActor* DamageCauser)
{
	if (bIsDead || DamageAmount <= 0.0f)
	{
		return;
	}

	Health = FMath::Max(0.0f, Health - DamageAmount);
	UE_LOG(LogTemp, Display,
		TEXT("[EnemyEcology] %s combatTeam=%d took %.1f damage from %s; health=%.1f"),
		*GetName(), static_cast<int32>(CombatFaction), DamageAmount,
		DamageCauser ? *DamageCauser->GetName() : TEXT("None"), Health);

	// Damage is explicit hostility: a hostile back attack must wake an idle enemy.
	if (AWCGhostAIController* GhostController = GetGhostAIController())
	{
		GhostController->HandleDamageAggro(DamageCauser);
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
	GetWorldTimerManager().ClearTimer(EnemyAttackHitTimerHandle);
}

void AGhostEnemy::Die()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	UE_LOG(LogTemp, Display, TEXT("[EnemyEcology] %s combatTeam=%d died"),
		*GetName(), static_cast<int32>(CombatFaction));
	bIsMoving = false;
	bIsAttacking = false;
	bCanAttackTarget = false;
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
	UE_LOG(LogTemp, Display, TEXT("[EnemyEcology] %s hit reaction started"), *GetName());

	if (AWCGhostAIController* GhostController = GetGhostAIController())
	{
		GhostController->PauseForHitReaction();
	}

	GetWorldTimerManager().ClearTimer(HitReactionTimerHandle);
	GetWorldTimerManager().ClearTimer(EnemyAttackDurationTimerHandle);
	GetWorldTimerManager().ClearTimer(EnemyAttackHitTimerHandle);
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
	UE_LOG(LogTemp, Display, TEXT("[EnemyEcology] %s hit reaction recovered"), *GetName());
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

	const FVector CurrentLocation = GetActorLocation();
	FVector SafeDestination = CurrentLocation + KnockbackDirection * KnockbackStrength;

	if (UNavigationSystemV1* NavigationSystem =
		UNavigationSystemV1::GetCurrent(GetWorld()))
	{
		FNavLocation ProjectedDestination;
		if (!NavigationSystem->ProjectPointToNavigation(
			SafeDestination, ProjectedDestination, FVector(200.0f, 200.0f, 300.0f)))
		{
			if (bShowAIDebug)
			{
				UE_LOG(LogTemp, Warning,
					TEXT("Ghost [%s] rejected knockback destination outside NavMesh: %s"),
					*GetName(), *SafeDestination.ToCompactString());
			}
			return;
		}

		SafeDestination.X = ProjectedDestination.Location.X;
		SafeDestination.Y = ProjectedDestination.Location.Y;
	}

	SetActorLocation(SafeDestination, true);
}

bool AGhostEnemy::CanUpdateBehavior() const
{
	return !bIsDead && !bIsHitReacting && !bIsAttacking;
}

bool AGhostEnemy::CanStartAttack() const
{
	return !bIsDead && !bIsHitReacting && !bIsAttacking && bCanAttackTarget;
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
	bCanAttackTarget = false;
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
	AActor* TargetActor = GhostController ? GhostController->GetTargetActor() : nullptr;

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

	if (!WCCombatant::AreHostile(this, TargetActor))
	{
		bIsMoving = false;
		if (GhostController)
		{
			GhostController->BeginReturnHome();
		}
		return;
	}

	const float DistanceToTarget = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
	if (DistanceToTarget <= AttackRange)
	{
		bIsMoving = false;
		GhostController->StopMovement();
		TryAttackTarget();
		return;
	}

	bIsMoving = true;
}

void AGhostEnemy::TryAttackTarget()
{
	if (!CanStartAttack())
	{
		return;
	}

	bCanAttackTarget = false;
	bIsAttacking = true;
	bAttackHitProcessed = false;
	bIsMoving = false;
	SetAIState(EGhostAIState::Attacking);

	GetWorldTimerManager().SetTimer(EnemyAttackDurationTimerHandle, this,
		&AGhostEnemy::EndEnemyAttack, EnemyAttackDuration, false);
	// Existing Blueprint animation notifies can call OnEnemyAttackHit first. This
	// one-shot native fallback keeps placeholder combatants functional even when
	// an inherited test AnimBP does not dispatch the notify after reparenting.
	GetWorldTimerManager().SetTimer(EnemyAttackHitTimerHandle, this,
		&AGhostEnemy::OnEnemyAttackHit,
		FMath::Max(0.01f, EnemyAttackDuration * 0.5f), false);
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
		bCanAttackTarget = true;
	}
}

void AGhostEnemy::DealDamageToTarget()
{
	AWCGhostAIController* GhostController = GetGhostAIController();
	AActor* TargetActor = GhostController ? GhostController->GetTargetActor() : nullptr;
	if (WCCombatant::AreHostile(this, TargetActor))
	{
		UGameplayStatics::ApplyDamage(TargetActor, EnemyAttackDamage,
			GetController(), this, UDamageType::StaticClass());
	}
}

void AGhostEnemy::OnEnemyAttackHit()
{
	if (bIsDead || bIsHitReacting || !bIsAttacking || bAttackHitProcessed)
	{
		return;
	}
	bAttackHitProcessed = true;
	GetWorldTimerManager().ClearTimer(EnemyAttackHitTimerHandle);

	AWCGhostAIController* GhostController = GetGhostAIController();
	AActor* TargetActor = GhostController ? GhostController->GetTargetActor() : nullptr;
	if (!WCCombatant::AreHostile(this, TargetActor))
	{
		return;
	}

	if (FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation()) > AttackRange)
	{
		PlayEvilGhostAttackWhiffSound();
		return;
	}

	PlayEvilGhostAttackHitSound();
	DealDamageToTarget();
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
