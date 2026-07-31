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
	// ******************** Public Interface ********************

	UFUNCTION(BlueprintCallable, Category = "Memory Echo")
	void StartMemoryEcho(
		const FMemoryEchoData& NewEchoData,
		AWCCharacter* NewPlayerOwner
	);

	UFUNCTION(BlueprintCallable, Category = "Memory Echo")
	void AdvanceMemoryEcho();

protected:
	// ******************** Lifecycle ********************

	virtual void NativeConstruct() override;

	// ******************** UI ********************

	UPROPERTY(meta = (BindWidget))
	UTextBlock* EchoTitleText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* EchoBodyText;

	UPROPERTY(meta = (BindWidget))
	UButton* EchoContinueButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* EchoContinueButtonLabel;

	// ******************** Runtime State ********************

	UPROPERTY()
	FMemoryEchoData ActiveEchoData;

	UPROPERTY()
	AWCCharacter* PlayerOwner = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory Echo")
	int32 CurrentPageIndex = 0;

	// ******************** Helpers ********************

	void DisplayCurrentPage();

	void RequestCloseEcho();

	// ******************** Events ********************

	UFUNCTION()
	void HandleContinueClicked();
};
