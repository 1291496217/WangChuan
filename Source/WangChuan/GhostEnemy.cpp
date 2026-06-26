// Fill out your copyright notice in the Description page of Project Settings.


#include "GhostEnemy.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

// Sets default values
AGhostEnemy::AGhostEnemy()
{
	PrimaryActorTick.bCanEverTick = false;

	EnemyMesh = CreateDefaultSubobject
		<UStaticMeshComponent>(TEXT("EnemyMesh"));
	RootComponent = EnemyMesh;

	MaxHealth = 100.0f;
	Health = MaxHealth;
}

void AGhostEnemy::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	
}

void AGhostEnemy::TakeHit(float DamageAmount) {
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
	if (Health <= 0.0f) {
		Die();
	}
}

void AGhostEnemy::Die() {
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Purple,
			TEXT("Ghost Defeated")
		);
	}
	Destroy();
}

