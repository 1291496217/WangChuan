// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHitFlashWidget.generated.h"

/**
 * 
 */
UCLASS()
class WANGCHUAN_API UPlayerHitFlashWidget : public UUserWidget
{
	GENERATED_BODY()
	
public: 
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void PlayHitFlash();
};
