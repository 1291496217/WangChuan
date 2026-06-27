// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "GhostEnemy.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	FLinearColor NormalColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	FLinearColor HitColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	float HitFlashDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
	float KnockbackDistance = 40.0f;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial; // change enemy's color in runtime

	FTimerHandle HitFeedbackTimerHandle;

public:	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TakeHit(float DamageAmount);

protected:
	void Die();

	void ShowHitFeedback();

	void ResetHitFeedback();

	void ApplyKnockback();
};
