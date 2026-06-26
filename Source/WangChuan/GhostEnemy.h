// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GhostEnemy.generated.h"

UCLASS()
class WANGCHUAN_API AGhostEnemy : public AActor
{
	GENERATED_BODY()
	
public:
	AGhostEnemy();

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* EnemyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float Health = 100.0f;

public:	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TakeHit(float DamageAmount);

protected:
	void Die();
};
