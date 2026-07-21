// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StoryTypes.h"
#include "MemoryEchoWidget.generated.h"

class AWCCharacter;
class UButton;
class UTextBlock;

/**
 * 最小 Memory Echo UI。
 * 
 * Page 0：
 * Echo Text
 * 
 * Page 1：
 * PlayerReasonanceText
 */
UCLASS()
class WANGCHUAN_API UMemoryEchoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Memory Echo")
	void StartMemoryEcho(
		const FMemoryEchoData& NewEchoData,
		AWCCharacter* NewPlayerOwner
	);

	UFUNCTION(BlueprintCallable, Category = "Memory Echo")
	void AdvanceMemoryEcho();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* EchoTitleText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* EchoBodyText;

	UPROPERTY(meta = (BindWidget))
	UButton* EchoContinueButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* EchoContinueButtonLabel;

	UPROPERTY()
	FMemoryEchoData ActiveEchoData;

	UPROPERTY()
	AWCCharacter* PlayerOwner = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory Echo")
	int32 CurrentPageIndex = 0;

	void DisplayCurrentPage();

	void RequestCloseEcho();

	UFUNCTION()
	void HandleContinueClicked();
};
