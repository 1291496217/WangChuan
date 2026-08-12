#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialInstructionWidget.generated.h"

class AWCCharacter;
class UButton;
class UTextBlock;

UCLASS()
class WANGCHUAN_API UTutorialInstructionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeInstruction(
		AWCCharacter* InPlayerOwner,
		const FText& InTitle,
		const FText& InBody);

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<AWCCharacter> PlayerOwner;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> InstructionTitle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> InstructionBody;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	FText PendingTitle;
	FText PendingBody;

	void RefreshText();
	void RequestClose();

	UFUNCTION()
	void HandleCloseClicked();
};
