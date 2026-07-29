#include "MemoryJournalEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

TSharedRef<SWidget> UMemoryJournalEntryWidget::RebuildWidget()
{
	BuildWidgetTree();
	return Super::RebuildWidget();
}

void UMemoryJournalEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildWidgetTree();

	if (EntryButton)
	{
		EntryButton->OnClicked.AddUniqueDynamic(
			this,
			&UMemoryJournalEntryWidget::HandleEntryClicked
		);
	}

	if (EntryTitleText)
	{
		EntryTitleText->SetText(EntryTitle);
	}

	ApplySelectionVisual();
}

void UMemoryJournalEntryWidget::InitializeEntry(
	int32 InEntryIndex,
	const FText& InTitle)
{
	EntryIndex = InEntryIndex;
	EntryTitle = InTitle;

	if (EntryTitleText)
	{
		EntryTitleText->SetText(EntryTitle);
	}
}

void UMemoryJournalEntryWidget::SetEntrySelected(bool bSelected)
{
	bIsSelected = bSelected;
	ApplySelectionVisual();
}

void UMemoryJournalEntryWidget::BuildWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (SelectionBorder &&
		EntryButton &&
		EntryTitleText)
	{
		return;
	}

	SelectionBorder =
		WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(),
			TEXT("SelectionBorder")
		);
	SelectionBorder->SetPadding(FMargin(4.0f));
	WidgetTree->RootWidget = SelectionBorder;

	EntryButton =
		WidgetTree->ConstructWidget<UButton>(
			UButton::StaticClass(),
			TEXT("EntryButton")
		);
	SelectionBorder->SetContent(EntryButton);

	EntryTitleText =
		WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(),
			TEXT("EntryTitleText")
		);
	EntryTitleText->SetColorAndOpacity(
		FSlateColor(FLinearColor(0.88f, 0.84f, 0.73f, 1.0f))
	);
	EntryTitleText->SetAutoWrapText(true);

	FSlateFontInfo EntryFont = EntryTitleText->GetFont();
	EntryFont.Size = 18;
	EntryTitleText->SetFont(EntryFont);

	EntryButton->SetContent(EntryTitleText);
}

void UMemoryJournalEntryWidget::ApplySelectionVisual()
{
	if (!SelectionBorder)
	{
		return;
	}

	SelectionBorder->SetBrushColor(
		bIsSelected
		? FLinearColor(0.30f, 0.24f, 0.14f, 0.95f)
		: FLinearColor(0.08f, 0.07f, 0.055f, 0.75f)
	);
}

void UMemoryJournalEntryWidget::HandleEntryClicked()
{
	if (EntryIndex == INDEX_NONE)
	{
		return;
	}

	OnEntrySelected.Broadcast(EntryIndex);
}
