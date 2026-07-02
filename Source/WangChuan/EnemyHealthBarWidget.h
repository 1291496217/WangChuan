// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHealthBarWidget.generated.h"

class AGhostEnemy;

UCLASS()
class WANGCHUAN_API UEnemyHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public: 
	UFUNCTION(BlueprintCallable, Category = "Enemy UI")
	void SetEnemyOwner(AGhostEnemy* NewEnemyOwner);

	UFUNCTION(BlueprintPure, Category = "Enemy UI")
	float GetEnemyHealthPercent() const;

private: 
	UPROPERTY()
	AGhostEnemy* EnemyOwner;
};
