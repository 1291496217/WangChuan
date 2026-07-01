// Fill out your copyright notice in the Description page of Project Settings.


#include "GhostEnemy.h"
#include "WCCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

// Sets default values
AGhostEnemy::AGhostEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject
		<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	EnemyMesh = CreateDefaultSubobject
		<USkeletalMeshComponent>(TEXT("EnemyMesh"));
	EnemyMesh->SetupAttachment(SceneRoot);

	MaxHealth = 100.0f;
	Health = MaxHealth;
}

void AGhostEnemy::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	if (EnemyMesh) {
		DynamicMaterial = EnemyMesh->CreateAndSetMaterialInstanceDynamic(0);

		if (DynamicMaterial) {
			DynamicMaterial->SetVectorParameterValue(
				TEXT("BaseColor"),
				NormalColor
			);
		}
	}
	
}

void AGhostEnemy::TakeHit(
	float DamageAmount,
	FVector HitDirection,
	float KnockbackStrength) {

	if (bIsDead) {
		return;
	}
	
	Health -= DamageAmount;

	if (GEngine) {
		FString HitMessage = FString::Printf(
			TEXT("Ghost Hit! Health: %.0f"),
			Health
		);
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Red,
			HitMessage
		);
	}

	if (Health <= 0.0f) { // If the last hit kill the enemy, no Feedback
		Die();
		return;
	}

	//ShowHitFeedback();
	StartHitReaction();
	ApplyKnockback(
		HitDirection,
		KnockbackStrength
	);
}

void AGhostEnemy::ClearCombatTimers() {
	GetWorldTimerManager().ClearTimer(HitFeedbackTimerHandle);
	GetWorldTimerManager().ClearTimer(HitReactionTimerHandle);
	GetWorldTimerManager().ClearTimer(EnemyAttackCooldownTimerHandle);
	GetWorldTimerManager().ClearTimer(EnemyAttackDurationTimerHandle);
}

void AGhostEnemy::Die() {
	if (bIsDead) {
		return;
	}
	bIsDead = true;
	bIsMoving = false;
	bIsAttacking = false;
	bCanAttackPlayer = false;
	bIsHitReacting = false;

	ClearCombatTimers();
	
	// Enemy can't block player after death;
	if (EnemyMesh) { 
		EnemyMesh->SetCollisionEnabled(
			ECollisionEnabled::NoCollision);
	}

	GetWorldTimerManager().SetTimer(
		DeathTimerHandle,
		this,
		&AGhostEnemy::FinishDeath,
		DeathDestroyDelay,
		false
	);
}

void AGhostEnemy::FinishDeath() {
	GetWorldTimerManager().ClearTimer(
		DeathTimerHandle
	);
	Destroy();
}

/* Legacy Color feedback
* 
void AGhostEnemy::ShowHitFeedback() {
	if (DynamicMaterial) {
		DynamicMaterial->SetVectorParameterValue(
			TEXT("BaseColor"),
			HitColor
		);
	}
	GetWorldTimerManager().ClearTimer(HitFeedbackTimerHandle);

	GetWorldTimerManager().SetTimer(
		HitFeedbackTimerHandle,
		this,
		&AGhostEnemy::ResetHitFeedback,
		HitFlashDuration,
		false
	);
}

void AGhostEnemy::ResetHitFeedback() {
	if (DynamicMaterial) {
		DynamicMaterial->SetVectorParameterValue(
			TEXT("BaseColor"),
			NormalColor
		);
	}
}
*/
void AGhostEnemy::StartHitReaction() {
	if (bIsDead) {
		return;
	}

	bIsHitReacting = true;
	bIsMoving = false;
	bIsAttacking = false;

	GetWorldTimerManager().ClearTimer(HitReactionTimerHandle);

	GetWorldTimerManager().ClearTimer(EnemyAttackDurationTimerHandle);

	GetWorldTimerManager().SetTimer(
		HitReactionTimerHandle,
		this,
		&AGhostEnemy::EndHitReaction,
		HitReactionDuration,
		false
	);
}

void AGhostEnemy::EndHitReaction() {
	if (bIsDead) {
		return;
	}
	bIsHitReacting = false;
}

void AGhostEnemy::ApplyKnockback(
	FVector KnockbackDirection,
	float KnockbackStrength) {

	if (bIsDead) {
		return;
	}

	KnockbackDirection.Z = 0.0f;

	if (KnockbackDirection.IsNearlyZero()) {
		return;
	}

	KnockbackDirection.Normalize();

	FVector NewLocation = GetActorLocation()
		+ KnockbackDirection
		* KnockbackStrength;

	SetActorLocation(NewLocation);
}

AWCCharacter* AGhostEnemy::GetPlayerCharacter() const {
	UWorld* World = GetWorld();

	if (World == nullptr) {
		return nullptr;
	}

	APlayerController* PlayerController =
		World->GetFirstPlayerController();

	if (PlayerController == nullptr) {
		return nullptr;
	}

	APawn* PlayerPawn = PlayerController->GetPawn();

	if (PlayerPawn == nullptr) {
		return nullptr;
	}

	return Cast<AWCCharacter>(PlayerPawn);
}

bool AGhostEnemy::IsPlayerValidAndAlive() const {
	AWCCharacter* PlayerCharacter = GetPlayerCharacter();

	if (PlayerCharacter == nullptr) {
		return false;
	}

	if (PlayerCharacter->GetIsDead()) {
		return false;
	}
	return true;
}

bool AGhostEnemy::CanUpdateBehavior() const {
	if (bIsDead) {
		return false;
	}
	if (bIsHitReacting) {
		return false;
	}
	if (bIsAttacking) {
		return false;
	}
	return true;
}

bool AGhostEnemy::CanStartAttack() const {
	if (bIsDead) {
		return false;
	}
	if (bIsHitReacting) {
		return false;
	}
	if (bIsAttacking) {
		return false;
	}
	if (!bCanAttackPlayer) {
		return false;
	}
	return true;
}
// **********Enemy Behavior**********

void AGhostEnemy::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	UpdateEnemyBehavior(DeltaTime);
}

// If player in attack range, attack, don't move
// Else if player in chase range, move toward player.
// Else, do nothing. 
void AGhostEnemy::UpdateEnemyBehavior(float DeltaTime) {
	// Dead > Hit Reaction > Attack > Move
	if (!CanUpdateBehavior()) {
		bIsMoving = false;
		return;
	}

	if (!IsPlayerValidAndAlive()) {
		bIsMoving = false;
		return;
	}

	AWCCharacter* PlayerCharacter = GetPlayerCharacter();

	if (PlayerCharacter == nullptr) {
		bIsMoving = false;
		return;
	}

	float DistanceToPlayer = FVector::Dist(
		GetActorLocation(),
		PlayerCharacter->GetActorLocation()
	);

	if (DistanceToPlayer <= AttackRange) {
		bIsMoving = false;
		TryAttackPlayer();
		return;
	}

	if (DistanceToPlayer <= ChaseRange) {
		bIsMoving = true;
		MoveTowardPlayer(PlayerCharacter, DeltaTime);
		return;
	}

	bIsMoving = false;
}

// move toward player's location, face toward player
void AGhostEnemy::MoveTowardPlayer(APawn* PlayerPawn, float DeltaTime) {
	if (PlayerPawn == nullptr) {
		return;
	}
	FVector Direction = PlayerPawn->
		GetActorLocation() - GetActorLocation();

	Direction.Z = 0.0f;

	if (Direction.IsNearlyZero()) {
		return;
	}

	Direction.Normalize();

	FVector NewLocation =
		GetActorLocation()
		+ Direction * MoveSpeed * DeltaTime;

	SetActorLocation(NewLocation);

	FRotator NewRotation = Direction.Rotation();

	SetActorRotation(
		FRotator(
			0.0f,
			NewRotation.Yaw,
			0.0f
		)
	);
}

void AGhostEnemy::TryAttackPlayer() {
	if (!CanStartAttack()) {
		return;
	}

	bCanAttackPlayer = false;
	bIsAttacking = true;
	bIsMoving = false;

	//DealDamageToPlayer();

	// attack anim finish
	GetWorldTimerManager().SetTimer(
		EnemyAttackDurationTimerHandle,
		this,
		&AGhostEnemy::EndEnemyAttack,
		EnemyAttackDuration,
		false
	);
	// attack cooldown finish
	GetWorldTimerManager().SetTimer(
		EnemyAttackCooldownTimerHandle,
		this,
		&AGhostEnemy::ResetEnemyAttack,
		AttackCooldown,
		false
	);
}

void AGhostEnemy::EndEnemyAttack() { // End attack anim 
	if (bIsDead) {
		return;
	}
	bIsAttacking = false;
}

void AGhostEnemy::ResetEnemyAttack() {
	if (bIsDead) {
		return;
	}
	bCanAttackPlayer = true;
}

void AGhostEnemy::DealDamageToPlayer() {
	AWCCharacter* PlayerCharacter = GetPlayerCharacter();

	if (PlayerCharacter == nullptr) {
		return;
	}

	if (PlayerCharacter->GetIsDead()) {
		return;
	}

	PlayerCharacter->ReceiveDamage(EnemyAttackDamage);
}

void AGhostEnemy::OnEnemyAttackHit() {
	if (bIsDead) {
		return;
	}

	if (bIsHitReacting) {
		return;
	}

	if (!bIsAttacking) {
		return;
	}

	AWCCharacter* PlayerCharacter = GetPlayerCharacter();

	if (PlayerCharacter == nullptr) {
		return;
	}

	if (PlayerCharacter->GetIsDead()) {
		return;
	}

	float DistanceToPlayer = FVector::Dist(
		GetActorLocation(), 
		PlayerCharacter->GetActorLocation()
	);

	if (DistanceToPlayer > AttackRange) {
		return;
	}

	DealDamageToPlayer();
}

bool AGhostEnemy::GetIsMoving() const {
	return bIsMoving;
}

bool AGhostEnemy::GetIsDead() const {
	return bIsDead;
}

bool AGhostEnemy::GetIsAttacking() const {
	return bIsAttacking;
}

bool AGhostEnemy::GetIsHitReacting() const {
	return bIsHitReacting;
}
