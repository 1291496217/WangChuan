#include "TutorialHUDWidget.h"

#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "TimerManager.h"

void UTutorialHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshProgress();
	RefreshHint();
}

void UTutorialHUDWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HintTimerHandle);
	}

	Super::NativeDestruct();
}

void UTutorialHUDWidget::SetFragmentProgress(int32 CollectedCount, int32 TotalCount)
{
	PendingCollectedCount = FMath::Max(0, CollectedCount);
	PendingTotalCount = FMath::Max(1, TotalCount);
	RefreshProgress();
}

void UTutorialHUDWidget::ShowTimedHint(const FText& HintText, float Duration)
{
	// Day2: the HUD is progress-only. Kept as a no-op for Blueprint stability.
	PendingHintText = FText::GetEmpty();
	RefreshHint();
}

FText UTutorialHUDWidget::GetDisplayedProgressText() const
{
	return FText::Format(
		FText::FromString(TEXT("Fragments {0} / {1}")),
		FText::AsNumber(PendingCollectedCount),
		FText::AsNumber(PendingTotalCount)
	);
}

void UTutorialHUDWidget::RefreshProgress()
{
	if (FragmentProgressText)
	{
		FragmentProgressText->SetText(GetDisplayedProgressText());
	}
}

void UTutorialHUDWidget::RefreshHint()
{
	if (HintBackground)
	{
		HintBackground->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (!TutorialHintText)
	{
		return;
	}

	TutorialHintText->SetText(FText::GetEmpty());
	TutorialHintText->SetVisibility(ESlateVisibility::Collapsed);
}

void UTutorialHUDWidget::HideHint()
{
	PendingHintText = FText::GetEmpty();
	RefreshHint();
}
