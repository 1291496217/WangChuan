#include "MemoryJournalWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

#include "MemoryJournalEntryWidget.h"
#include "WCCharacter.h"

namespace
{
	void SetTextSize(UTextBlock* TextBlock, int32 Size)
	{
		if (!TextBlock)
		{
			return;
		}

		FSlateFontInfo Font = TextBlock->GetFont();
		Font.Size = Size;
		TextBlock->SetFont(Font);
	}
}

TSharedRef<SWidget> UMemoryJournalWidget::RebuildWidget()
{
	BuildWidgetTree();
	return Super::RebuildWidget();
}

void UMemoryJournalWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildWidgetTree();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(
			this,
			&UMemoryJournalWidget::HandleCloseClicked
		);
	}

	SetIsFocusable(true);
	RebuildEntryList();
}

void UMemoryJournalWidget::InitializeJournal(
	AWCCharacter* InPlayerOwner,
	const TArray<FMemoryEchoData>& InEntries)
{
	PlayerOwner = InPlayerOwner;
	RefreshJournalEntries(InEntries);
}

void UMemoryJournalWidget::RefreshJournalEntries(
	const TArray<FMemoryEchoData>& InEntries)
{
	DisplayedEntries = InEntries;
	SelectedEntryIndex =
		DisplayedEntries.IsEmpty()
		? INDEX_NONE
		: 0;

	RebuildEntryList();
}

void UMemoryJournalWidget::BuildWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (CloseButton &&
		EntryListBox &&
		SelectedEntryTitle &&
		SelectedEntryBody &&
		EmptyStateText)
	{
		return;
	}

	UOverlay* RootOverlay =
		WidgetTree->ConstructWidget<UOverlay>(
			UOverlay::StaticClass(),
			TEXT("JournalRoot")
		);
	WidgetTree->RootWidget = RootOverlay;

	UBorder* ScreenShade =
		WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(),
			TEXT("ScreenShade")
		);
	ScreenShade->SetBrushColor(
		FLinearColor(0.015f, 0.012f, 0.01f, 0.78f)
	);
	if (UOverlaySlot* ShadeSlot =
		RootOverlay->AddChildToOverlay(ScreenShade))
	{
		ShadeSlot->SetHorizontalAlignment(HAlign_Fill);
		ShadeSlot->SetVerticalAlignment(VAlign_Fill);
	}

	USizeBox* JournalSize =
		WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(),
			TEXT("JournalSize")
		);
	JournalSize->SetWidthOverride(1040.0f);
	JournalSize->SetHeightOverride(680.0f);
	if (UOverlaySlot* JournalSlot =
		RootOverlay->AddChildToOverlay(JournalSize))
	{
		JournalSlot->SetHorizontalAlignment(HAlign_Center);
		JournalSlot->SetVerticalAlignment(VAlign_Center);
		JournalSlot->SetPadding(FMargin(30.0f));
	}

	UBorder* JournalBorder =
		WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(),
			TEXT("JournalBackground")
		);
	JournalBorder->SetBrushColor(
		FLinearColor(0.075f, 0.061f, 0.043f, 0.98f)
	);
	JournalBorder->SetPadding(FMargin(28.0f));
	JournalSize->SetContent(JournalBorder);

	UVerticalBox* JournalColumn =
		WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			TEXT("JournalColumn")
		);
	JournalBorder->SetContent(JournalColumn);

	UHorizontalBox* Header =
		WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(),
			TEXT("JournalHeader")
		);
	if (UVerticalBoxSlot* HeaderSlot =
		JournalColumn->AddChildToVerticalBox(Header))
	{
		HeaderSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 16.0f));
	}

	UTextBlock* JournalTitle =
		WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(),
			TEXT("JournalTitle")
		);
	JournalTitle->SetText(
		FText::FromString(TEXT("Memory Journal"))
	);
	JournalTitle->SetColorAndOpacity(
		FSlateColor(FLinearColor(0.92f, 0.87f, 0.74f, 1.0f))
	);
	SetTextSize(JournalTitle, 32);
	if (UHorizontalBoxSlot* TitleSlot =
		Header->AddChildToHorizontalBox(JournalTitle))
	{
		TitleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		TitleSlot->SetVerticalAlignment(VAlign_Center);
	}

	CloseButton =
		WidgetTree->ConstructWidget<UButton>(
			UButton::StaticClass(),
			TEXT("CloseButton")
		);
	UTextBlock* CloseLabel =
		WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(),
			TEXT("CloseButtonLabel")
		);
	CloseLabel->SetText(FText::FromString(TEXT("Close")));
	CloseLabel->SetColorAndOpacity(
		FSlateColor(FLinearColor(0.9f, 0.86f, 0.76f, 1.0f))
	);
	SetTextSize(CloseLabel, 18);
	CloseButton->SetContent(CloseLabel);
	if (UHorizontalBoxSlot* CloseSlot =
		Header->AddChildToHorizontalBox(CloseButton))
	{
		CloseSlot->SetPadding(FMargin(20.0f, 0.0f, 0.0f, 0.0f));
		CloseSlot->SetVerticalAlignment(VAlign_Center);
	}

	UBorder* HeaderSeparator =
		WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(),
			TEXT("HeaderSeparator")
		);
	HeaderSeparator->SetBrushColor(
		FLinearColor(0.38f, 0.31f, 0.20f, 0.9f)
	);
	if (UVerticalBoxSlot* SeparatorSlot =
		JournalColumn->AddChildToVerticalBox(HeaderSeparator))
	{
		SeparatorSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
		SeparatorSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}
	HeaderSeparator->SetDesiredSizeScale(FVector2D(1.0f, 0.04f));

	UHorizontalBox* Content =
		WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(),
			TEXT("JournalContent")
		);
	if (UVerticalBoxSlot* ContentSlot =
		JournalColumn->AddChildToVerticalBox(Content))
	{
		ContentSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	USizeBox* ListWidth =
		WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(),
			TEXT("EntryListWidth")
		);
	ListWidth->SetWidthOverride(300.0f);
	if (UHorizontalBoxSlot* ListSlot =
		Content->AddChildToHorizontalBox(ListWidth))
	{
		ListSlot->SetPadding(FMargin(0.0f, 0.0f, 24.0f, 0.0f));
	}

	UScrollBox* EntryListScroll =
		WidgetTree->ConstructWidget<UScrollBox>(
			UScrollBox::StaticClass(),
			TEXT("EntryListScrollBox")
		);
	ListWidth->SetContent(EntryListScroll);

	EntryListBox =
		WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			TEXT("EntryListBox")
		);
	EntryListScroll->AddChild(EntryListBox);

	UVerticalBox* DetailColumn =
		WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			TEXT("DetailColumn")
		);
	if (UHorizontalBoxSlot* DetailSlot =
		Content->AddChildToHorizontalBox(DetailColumn))
	{
		DetailSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	SelectedEntryTitle =
		WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(),
			TEXT("SelectedEntryTitle")
		);
	SelectedEntryTitle->SetColorAndOpacity(
		FSlateColor(FLinearColor(0.93f, 0.88f, 0.76f, 1.0f))
	);
	SelectedEntryTitle->SetAutoWrapText(true);
	SetTextSize(SelectedEntryTitle, 27);
	if (UVerticalBoxSlot* DetailTitleSlot =
		DetailColumn->AddChildToVerticalBox(SelectedEntryTitle))
	{
		DetailTitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 16.0f));
	}

	UScrollBox* BodyScroll =
		WidgetTree->ConstructWidget<UScrollBox>(
			UScrollBox::StaticClass(),
			TEXT("SelectedEntryBodyScrollBox")
		);
	if (UVerticalBoxSlot* BodyScrollSlot =
		DetailColumn->AddChildToVerticalBox(BodyScroll))
	{
		BodyScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	SelectedEntryBody =
		WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(),
			TEXT("SelectedEntryBody")
		);
	SelectedEntryBody->SetColorAndOpacity(
		FSlateColor(FLinearColor(0.82f, 0.80f, 0.73f, 1.0f))
	);
	SelectedEntryBody->SetAutoWrapText(true);
	SetTextSize(SelectedEntryBody, 20);
	BodyScroll->AddChild(SelectedEntryBody);

	EmptyStateText =
		WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(),
			TEXT("EmptyStateText")
		);
	EmptyStateText->SetText(
		FText::FromString(
			TEXT("No echoes have returned to you yet.")
		)
	);
	EmptyStateText->SetColorAndOpacity(
		FSlateColor(FLinearColor(0.58f, 0.55f, 0.49f, 1.0f))
	);
	EmptyStateText->SetAutoWrapText(true);
	SetTextSize(EmptyStateText, 22);
	if (UVerticalBoxSlot* EmptySlot =
		DetailColumn->AddChildToVerticalBox(EmptyStateText))
	{
		EmptySlot->SetPadding(FMargin(0.0f, 28.0f));
	}
}

void UMemoryJournalWidget::RebuildEntryList()
{
	if (!EntryListBox)
	{
		return;
	}

	EntryListBox->ClearChildren();
	EntryWidgets.Reset();

	TSubclassOf<UMemoryJournalEntryWidget> EffectiveEntryClass =
		EntryWidgetClass;
	if (!EffectiveEntryClass)
	{
		EffectiveEntryClass =
			UMemoryJournalEntryWidget::StaticClass();
	}

	for (int32 EntryIndex = 0;
		EntryIndex < DisplayedEntries.Num();
		++EntryIndex)
	{
		UMemoryJournalEntryWidget* EntryWidget =
			CreateWidget<UMemoryJournalEntryWidget>(
				GetOwningPlayer(),
				EffectiveEntryClass
			);
		if (!EntryWidget)
		{
			continue;
		}

		EntryWidget->InitializeEntry(
			EntryIndex,
			DisplayedEntries[EntryIndex].Title
		);
		EntryWidget->OnEntrySelected.AddUniqueDynamic(
			this,
			&UMemoryJournalWidget::HandleEntrySelected
		);
		EntryWidget->SetEntrySelected(
			EntryIndex == SelectedEntryIndex
		);

		if (UVerticalBoxSlot* EntrySlot =
			EntryListBox->AddChildToVerticalBox(EntryWidget))
		{
			EntrySlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
		}

		EntryWidgets.Add(EntryWidget);
	}

	RefreshDetailPanel();
}

void UMemoryJournalWidget::SelectEntry(int32 EntryIndex)
{
	if (!DisplayedEntries.IsValidIndex(EntryIndex))
	{
		return;
	}

	SelectedEntryIndex = EntryIndex;

	for (int32 WidgetIndex = 0;
		WidgetIndex < EntryWidgets.Num();
		++WidgetIndex)
	{
		if (EntryWidgets[WidgetIndex])
		{
			EntryWidgets[WidgetIndex]->SetEntrySelected(
				WidgetIndex == SelectedEntryIndex
			);
		}
	}

	RefreshDetailPanel();
}

void UMemoryJournalWidget::RefreshDetailPanel()
{
	const bool bHasSelection =
		DisplayedEntries.IsValidIndex(SelectedEntryIndex);

	if (EmptyStateText)
	{
		EmptyStateText->SetVisibility(
			bHasSelection
			? ESlateVisibility::Collapsed
			: ESlateVisibility::Visible
		);
	}

	if (SelectedEntryTitle)
	{
		SelectedEntryTitle->SetVisibility(
			bHasSelection
			? ESlateVisibility::Visible
			: ESlateVisibility::Collapsed
		);
	}

	if (SelectedEntryBody)
	{
		SelectedEntryBody->SetVisibility(
			bHasSelection
			? ESlateVisibility::Visible
			: ESlateVisibility::Collapsed
		);
	}

	if (!bHasSelection)
	{
		return;
	}

	const FMemoryEchoData& Entry =
		DisplayedEntries[SelectedEntryIndex];

	SelectedEntryTitle->SetText(Entry.Title);

	FString Body = Entry.EchoText.ToString();
	if (!Entry.PlayerReasonanceText.IsEmpty())
	{
		if (!Body.IsEmpty())
		{
			Body += TEXT("\n\n— — —\n\n");
		}
		Body += Entry.PlayerReasonanceText.ToString();
	}

	SelectedEntryBody->SetText(FText::FromString(Body));
}

void UMemoryJournalWidget::HandleCloseClicked()
{
	if (PlayerOwner)
	{
		PlayerOwner->CloseMemoryJournal();
		return;
	}

	RemoveFromParent();
}

void UMemoryJournalWidget::HandleEntrySelected(int32 EntryIndex)
{
	SelectEntry(EntryIndex);
}
