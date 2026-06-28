// Fill out your copyright notice in the Description page of Project Settings.


#include "GhostEnemy.h"
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

void AGhostEnemy::TakeHit(float DamageAmount) {
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

	if (Health <= 0) { // If the last hit kill the enemy, no Feedback
		Die();
		return;
	}

	ShowHitFeedback();
	ApplyKnockback();
}

void AGhostEnemy::Die() {
	if (bIsDead) {
		return;
	}
	bIsDead = true;
	bIsMoving = false;
	bCanAttackPlayer = false;

	GetWorldTimerManager().ClearTimer(HitFeedbackTimerHandle);
	GetWorldTimerManager().ClearTimer(EnemyAttackCooldownTimerHandle);
	
	// Enemy can't block player after death;
	if (EnemyMesh) { 
		EnemyMesh->SetCollisionEnabled(
			ECollisionEnabled::NoCollision
		);
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

void AGhostEnemy::ApplyKnockback() {
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	if (PlayerController == nullptr) {
		return;
	}

	APawn* PlayerPawn = PlayerController->GetPawn();

	if (PlayerPawn == nullptr) {
		return;
	}

	FVector KnockbackDirection =
		GetActorLocation() - PlayerPawn->GetActorLocation();
	KnockbackDirection.Z = 0.0f;

	if (KnockbackDirection.IsNearlyZero()) {
		return;
	}

	KnockbackDirection.Normalize();

	FVector NewLocation =
		GetActorLocation() + KnockbackDirection * KnockbackDistance;

	SetActorLocation(NewLocation);
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
	if (bIsDead) {
		bIsMoving = false;
		return;
	}

	APlayerController* PlayerController =
		GetWorld()->GetFirstPlayerController();

	if (PlayerController == nullptr) {
		bIsMoving = false;
		return;
	}

	APawn* PlayerPawn = PlayerController->GetPawn();

	if (PlayerPawn == nullptr) {
		bIsMoving = false;
		return;
	}

	float DistanceToPlayer = FVector::Dist(
		GetActorLocation(),
		PlayerPawn->GetActorLocation()
	);

	if (DistanceToPlayer <= AttackRange) {
		bIsMoving = false;
		TryAttackPlayer();
		return;
	}

	if (DistanceToPlayer <= ChaseRange) {
		bIsMoving = true;
		MoveTowardPlayer(PlayerPawn, DeltaTime);
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
	if (bIsDead) {
		return;
	}

	if (!bCanAttackPlayer) {
		return;
	}

	bCanAttackPlayer = false;

	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.5f,
			FColor::Orange,
			TEXT("Player Hit")
		);
	}

	GetWorldTimerManager().SetTimer(
		EnemyAttackCooldownTimerHandle,
		this,
		&AGhostEnemy::ResetEnemyAttack,
		AttackCooldown,
		false
	);
}

void AGhostEnemy::ResetEnemyAttack() {
	bCanAttackPlayer = true;
}

bool AGhostEnemy::GetIsMoving() const {
	return bIsMoving;
}

bool AGhostEnemy::GetIsDead() const {
	return bIsDead;
}

