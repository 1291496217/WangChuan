#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialFragmentWidget.generated.h"

class AWCCharacter;
class UButton;
class UTextBlock;

UCLASS()
class WANGCHUAN_API UTutorialFragmentWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeFragment(
		AWCCharacter* InPlayerOwner,
		const FText& InDisplayTitle,
		const FText& InDisplayText
	);

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<AWCCharacter> PlayerOwner;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DisplayTitle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DisplayText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	FText PendingTitle;
	FText PendingBody;

	void RefreshText();

	UFUNCTION()
	void HandleCloseClicked();
};
