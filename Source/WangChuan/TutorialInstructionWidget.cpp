#include "TutorialInstructionWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"
#include "WCCharacter.h"

void UTutorialInstructionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(
			this, &UTutorialInstructionWidget::HandleCloseClicked);
	}

	SetIsFocusable(true);
	RefreshText();
}

void UTutorialInstructionWidget::InitializeInstruction(
	AWCCharacter* InPlayerOwner,
	const FText& InTitle,
	const FText& InBody)
{
	PlayerOwner = InPlayerOwner;
	PendingTitle = InTitle;
	PendingBody = InBody;
	RefreshText();
}

FReply UTutorialInstructionWidget::NativeOnKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::E)
	{
		RequestClose();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UTutorialInstructionWidget::RefreshText()
{
	if (InstructionTitle)
	{
		InstructionTitle->SetText(PendingTitle);
	}

	if (InstructionBody)
	{
		InstructionBody->SetText(PendingBody);
	}
}

void UTutorialInstructionWidget::RequestClose()
{
	if (PlayerOwner)
	{
		PlayerOwner->EndTutorialInstruction();
		return;
	}

	RemoveFromParent();
}

void UTutorialInstructionWidget::HandleCloseClicked()
{
	RequestClose();
}
