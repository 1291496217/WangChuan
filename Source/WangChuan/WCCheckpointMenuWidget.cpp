#include "WCCheckpointMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

#include "WCCharacter.h"
#include "WCStoryPersistenceCoordinator.h"

namespace
{
	void SetCheckpointMenuTextSize(
		UTextBlock* TextBlock,
		int32 Size)
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

TSharedRef<SWidget> UWCCheckpointMenuWidget::RebuildWidget()
{
	BuildFallbackWidgetTree();
	return Super::RebuildWidget();
}

void UWCCheckpointMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildFallbackWidgetTree();
	ResolveWidgetReferences();

	if (DestinationButtons.Num() == 3)
	{
		DestinationButtons[0]->OnClicked.AddUniqueDynamic(
			this,
			&UWCCheckpointMenuWidget::HandleDestination0Clicked
		);
		DestinationButtons[1]->OnClicked.AddUniqueDynamic(
			this,
			&UWCCheckpointMenuWidget::HandleDestination1Clicked
		);
		DestinationButtons[2]->OnClicked.AddUniqueDynamic(
			this,
			&UWCCheckpointMenuWidget::HandleDestination2Clicked
		);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(
			this,
			&UWCCheckpointMenuWidget::HandleCloseClicked
		);
	}

	SetIsFocusable(true);
	RefreshPresentation();
}

void UWCCheckpointMenuWidget::InitializeCheckpointMenu(
	AWCCharacter* InPlayer,
	AWCPlayerCheckpoint* InSourceCheckpoint,
	AWCStoryPersistenceCoordinator* InCoordinator)
{
	PlayerOwner = InPlayer;
	SourceCheckpoint = InSourceCheckpoint;
	Coordinator = InCoordinator;

	TravelOptions =
		IsValid(Coordinator)
		? Coordinator->GetCheckpointTravelOptions()
		: TArray<FWCCheckpointTravelOption>();

	RefreshPresentation();
	OnCheckpointMenuInitialized();
}

const TArray<FWCCheckpointTravelOption>&
UWCCheckpointMenuWidget::GetTravelOptions() const
{
	return TravelOptions;
}

void UWCCheckpointMenuWidget::RequestTravelToCheckpoint(
	FName TargetCheckpointID)
{
	if (!IsValid(PlayerOwner) ||
		!IsValid(Coordinator))
	{
		return;
	}

	/*
	* Coordinator 会在全部目的地验证通过后关闭菜单。
	* 无效请求因此不会意外把玩家留在无 UI 状态。
	*/
	Coordinator->TravelPlayerToCheckpoint(
		TargetCheckpointID
	);
}

void UWCCheckpointMenuWidget::RequestCloseMenu()
{
	if (IsValid(PlayerOwner))
	{
		PlayerOwner->CloseCheckpointMenu();
		return;
	}

	RemoveFromParent();
}

void UWCCheckpointMenuWidget::BuildFallbackWidgetTree()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UOverlay* RootOverlay =
		WidgetTree->ConstructWidget<UOverlay>(
			UOverlay::StaticClass(),
			TEXT("CheckpointMenuRoot")
		);
	WidgetTree->RootWidget = RootOverlay;

	UBorder* ScreenShade =
		WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(),
			TEXT("CheckpointMenuShade")
		);
	ScreenShade->SetBrushColor(
		FLinearColor(0.01f, 0.015f, 0.02f, 0.78f)
	);
	if (UOverlaySlot* ShadeSlot =
		RootOverlay->AddChildToOverlay(ScreenShade))
	{
		ShadeSlot->SetHorizontalAlignment(HAlign_Fill);
		ShadeSlot->SetVerticalAlignment(VAlign_Fill);
	}

	USizeBox* MenuSize =
		WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(),
			TEXT("CheckpointMenuSize")
		);
	MenuSize->SetWidthOverride(560.0f);
	MenuSize->SetHeightOverride(500.0f);
	if (UOverlaySlot* MenuSlot =
		RootOverlay->AddChildToOverlay(MenuSize))
	{
		MenuSlot->SetHorizontalAlignment(HAlign_Center);
		MenuSlot->SetVerticalAlignment(VAlign_Center);
	}

	UBorder* MenuBorder =
		WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(),
			TEXT("CheckpointMenuBackground")
		);
	MenuBorder->SetBrushColor(
		FLinearColor(0.035f, 0.055f, 0.065f, 0.98f)
	);
	MenuBorder->SetPadding(FMargin(30.0f));
	MenuSize->SetContent(MenuBorder);

	UVerticalBox* MenuColumn =
		WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			TEXT("CheckpointMenuColumn")
		);
	MenuBorder->SetContent(MenuColumn);

	UTextBlock* Title =
		WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(),
			TEXT("CheckpointMenuTitle")
		);
	Title->SetText(FText::FromString(TEXT("Rest Point")));
	Title->SetColorAndOpacity(
		FSlateColor(FLinearColor(0.72f, 0.92f, 0.95f, 1.0f))
	);
	SetCheckpointMenuTextSize(Title, 32);
	if (UVerticalBoxSlot* TitleSlot =
		MenuColumn->AddChildToVerticalBox(Title))
	{
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
		TitleSlot->SetHorizontalAlignment(HAlign_Center);
	}

	UTextBlock* Status =
		WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(),
			TEXT("CheckpointMenuStatus")
		);
	Status->SetText(FText::FromString(TEXT("Memory Anchored")));
	Status->SetColorAndOpacity(
		FSlateColor(FLinearColor(0.55f, 0.72f, 0.75f, 1.0f))
	);
	SetCheckpointMenuTextSize(Status, 18);
	if (UVerticalBoxSlot* StatusSlot =
		MenuColumn->AddChildToVerticalBox(Status))
	{
		StatusSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 24.0f));
		StatusSlot->SetHorizontalAlignment(HAlign_Center);
	}

	DestinationButtons.Reset();
	DestinationLabels.Reset();

	for (int32 OptionIndex = 0;
		OptionIndex < 3;
		++OptionIndex)
	{
		UButton* DestinationButton =
			WidgetTree->ConstructWidget<UButton>(
				UButton::StaticClass(),
				*FString::Printf(
					TEXT("CheckpointDestinationButton%d"),
					OptionIndex
				)
			);

		UTextBlock* DestinationLabel =
			WidgetTree->ConstructWidget<UTextBlock>(
				UTextBlock::StaticClass(),
				*FString::Printf(
					TEXT("CheckpointDestinationLabel%d"),
					OptionIndex
				)
			);
		DestinationLabel->SetColorAndOpacity(
			FSlateColor(FLinearColor(0.86f, 0.93f, 0.93f, 1.0f))
		);
		SetCheckpointMenuTextSize(DestinationLabel, 20);
		DestinationButton->SetContent(DestinationLabel);

		if (UVerticalBoxSlot* DestinationSlot =
			MenuColumn->AddChildToVerticalBox(DestinationButton))
		{
			DestinationSlot->SetPadding(
				FMargin(0.0f, 0.0f, 0.0f, 12.0f)
			);
		}

		DestinationButtons.Add(DestinationButton);
		DestinationLabels.Add(DestinationLabel);
	}

	CloseButton =
		WidgetTree->ConstructWidget<UButton>(
			UButton::StaticClass(),
			TEXT("CheckpointMenuCloseButton")
		);
	UTextBlock* CloseLabel =
		WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(),
			TEXT("CheckpointMenuCloseLabel")
		);
	CloseLabel->SetText(FText::FromString(TEXT("Close")));
	SetCheckpointMenuTextSize(CloseLabel, 18);
	CloseButton->SetContent(CloseLabel);
	if (UVerticalBoxSlot* CloseSlot =
		MenuColumn->AddChildToVerticalBox(CloseButton))
	{
		CloseSlot->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 0.0f));
	}
}

void UWCCheckpointMenuWidget::ResolveWidgetReferences()
{
	if (!WidgetTree || !WidgetTree->RootWidget)
	{
		return;
	}

	static const FName ButtonNames[] =
	{
		TEXT("StartTravelButton"),
		TEXT("Encounter01TravelButton"),
		TEXT("Encounter02TravelButton")
	};
	static const FName DestinationTextNames[] =
	{
		TEXT("StartDestinationText"),
		TEXT("Encounter01DestinationText"),
		TEXT("Encounter02DestinationText")
	};
	static const FName ActionTextNames[] =
	{
		TEXT("StartTravelButtonText"),
		TEXT("Encounter01TravelButtonText"),
		TEXT("Encounter02TravelButtonText")
	};

	TArray<TObjectPtr<UButton>> FormalButtons;
	TArray<TObjectPtr<UTextBlock>> FormalDestinationLabels;

	for (int32 OptionIndex = 0; OptionIndex < 3; ++OptionIndex)
	{
		UButton* Button = Cast<UButton>(
			WidgetTree->FindWidget(ButtonNames[OptionIndex])
		);
		UTextBlock* DestinationLabel = Cast<UTextBlock>(
			WidgetTree->FindWidget(DestinationTextNames[OptionIndex])
		);
		UTextBlock* ActionLabel = Cast<UTextBlock>(
			WidgetTree->FindWidget(ActionTextNames[OptionIndex])
		);

		if (!Button || !DestinationLabel || !ActionLabel)
		{
			return;
		}

		FormalButtons.Add(Button);
		FormalDestinationLabels.Add(DestinationLabel);
	}

	UButton* FormalCloseButton = Cast<UButton>(
		WidgetTree->FindWidget(TEXT("MenuCloseButton"))
	);
	if (!FormalCloseButton)
	{
		return;
	}

	DestinationButtons = MoveTemp(FormalButtons);
	DestinationLabels = MoveTemp(FormalDestinationLabels);
	CloseButton = FormalCloseButton;
}

void UWCCheckpointMenuWidget::RefreshPresentation()
{
	if (DestinationButtons.Num() != 3 ||
		DestinationLabels.Num() != 3)
	{
		return;
	}

	for (int32 OptionIndex = 0;
		OptionIndex < 3;
		++OptionIndex)
	{
		static const FName ActionTextNames[] =
		{
			TEXT("StartTravelButtonText"),
			TEXT("Encounter01TravelButtonText"),
			TEXT("Encounter02TravelButtonText")
		};
		UTextBlock* DestinationActionLabel =
			WidgetTree
				? Cast<UTextBlock>(WidgetTree->FindWidget(
					ActionTextNames[OptionIndex]
				))
				: nullptr;

		const bool bHasOption =
			TravelOptions.IsValidIndex(OptionIndex);

		FString DestinationLabel = TEXT("Undiscovered");
		FString ActionLabel = TEXT("Undiscovered");
		bool bEnabled = false;

		if (bHasOption)
		{
			const FWCCheckpointTravelOption& Option =
				TravelOptions[OptionIndex];

			DestinationLabel =
				Option.DisplayName.IsEmpty()
					? Option.CheckpointID.ToString()
					: Option.DisplayName.ToString();

			if (!Option.bUnlocked)
			{
				ActionLabel = TEXT("Undiscovered");
			}
			else if (Option.bCurrent)
			{
				ActionLabel = TEXT("Current");
			}
			else
			{
				ActionLabel = TEXT("Travel");
				bEnabled = true;
			}
		}

		if (DestinationActionLabel)
		{
			DestinationLabels[OptionIndex]->SetText(
				FText::FromString(DestinationLabel)
			);
			DestinationActionLabel->SetText(
				FText::FromString(ActionLabel)
			);
		}
		else
		{
			DestinationLabels[OptionIndex]->SetText(
				FText::FromString(FString::Printf(
					TEXT("%s  -  %s"),
					*DestinationLabel,
					*ActionLabel
				))
			);
		}
		DestinationButtons[OptionIndex]->SetIsEnabled(
			bEnabled
		);
	}
}

void UWCCheckpointMenuWidget::RequestTravelOptionByIndex(
	int32 OptionIndex)
{
	if (!TravelOptions.IsValidIndex(OptionIndex))
	{
		return;
	}

	const FWCCheckpointTravelOption& Option =
		TravelOptions[OptionIndex];

	if (!Option.bUnlocked || Option.bCurrent)
	{
		return;
	}

	RequestTravelToCheckpoint(Option.CheckpointID);
}

void UWCCheckpointMenuWidget::HandleDestination0Clicked()
{
	RequestTravelOptionByIndex(0);
}

void UWCCheckpointMenuWidget::HandleDestination1Clicked()
{
	RequestTravelOptionByIndex(1);
}

void UWCCheckpointMenuWidget::HandleDestination2Clicked()
{
	RequestTravelOptionByIndex(2);
}

void UWCCheckpointMenuWidget::HandleCloseClicked()
{
	RequestCloseMenu();
}
