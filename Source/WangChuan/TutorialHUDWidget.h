#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialHUDWidget.generated.h"

class UTextBlock;
class UBorder;

UCLASS()
class WANGCHUAN_API UTutorialHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetFragmentProgress(int32 CollectedCount, int32 TotalCount);
	void ShowTimedHint(const FText& HintText, float Duration);
	FText GetDisplayedProgressText() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> FragmentProgressText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TutorialHintText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> HintBackground;

	int32 PendingCollectedCount = 0;
	int32 PendingTotalCount = 3;
	FText PendingHintText;
	FTimerHandle HintTimerHandle;

	void RefreshProgress();
	void RefreshHint();
	void HideHint();
};
