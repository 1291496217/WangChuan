
#include "DialogueWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "WCCharacter.h"
#include "WCStoryNPC.h"

// ******************** Lifecycle ********************

void UDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ContinueButton)
	{
		ContinueButton->OnClicked.AddUniqueDynamic(
			this,
			&UDialogueWidget::HandleContinueClicked
		);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(
			this,
			&UDialogueWidget::HandleCloseClicked
		);
	}
}

// ******************** Dialogue ********************

void UDialogueWidget::StartDialogue(
	const FDialogueSequence& NewDialogueSequence,
	AWCStoryNPC* NewStoryNPC,
	AWCCharacter* NewPlayerOwner)
{
	ActiveDialogueSequence = NewDialogueSequence;
	StoryNPCOwner = NewStoryNPC;
	PlayerOwner = NewPlayerOwner;

	CurrentLineIndex = 0;

	if (ActiveDialogueSequence.Lines.Num() == 0)
	{
		RequestCloseDialogue();
		return;
	}

	DisplayCurrentLine();
}

void UDialogueWidget::AdvanceDialogue()
{
	if (ActiveDialogueSequence.Lines.Num() == 0)
	{
		RequestCloseDialogue();
		return;
	}

	++CurrentLineIndex;

	// 已经越过最后一行。
	if (!ActiveDialogueSequence.Lines.IsValidIndex(
		CurrentLineIndex))
	{
		RequestCloseDialogue();
		return;
	}

	DisplayCurrentLine();
}

// ******************** UI ********************

void UDialogueWidget::DisplayCurrentLine()
{
	if (!ActiveDialogueSequence.Lines.IsValidIndex(
		CurrentLineIndex))
	{
		RequestCloseDialogue();
		return;
	}

	const FDialogueLine& CurrentLine =
		ActiveDialogueSequence.Lines[
			CurrentLineIndex
		];

	FText DisplaySpeakerName =
		CurrentLine.SpeakerName;

	if (DisplaySpeakerName.IsEmpty() &&
		StoryNPCOwner)
	{
		DisplaySpeakerName =
			StoryNPCOwner->GetNPCDisplayName();
	}

	if (SpeakerNameText)
	{
		SpeakerNameText->SetText(
			DisplaySpeakerName);
	}

	if (DialogueText)
	{
		DialogueText->SetText(
			CurrentLine.DialogueText);
	}

	if (ContinueButtonLabel)
	{
		const bool bIsLastLine =
			CurrentLineIndex >=
			ActiveDialogueSequence.Lines.Num() - 1;

		ContinueButtonLabel->SetText(
			bIsLastLine
			? FText::FromString(TEXT("Close"))
			: FText::FromString(TEXT("Continue"))
		);
	}
}

void UDialogueWidget::RequestCloseDialogue()
{
	if (PlayerOwner)
	{
		PlayerOwner->EndDialogue();
		return;
	}

	// Player 引用无效时的安全保护。
	RemoveFromParent();
}

// ******************** Events ********************

void UDialogueWidget::HandleContinueClicked()
{
	AdvanceDialogue();
}

void UDialogueWidget::HandleCloseClicked()
{
	RequestCloseDialogue();
}
