#include "TutorialFragmentWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "WCCharacter.h"

void UTutorialFragmentWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(
			this,
			&UTutorialFragmentWidget::HandleCloseClicked
		);
	}

	SetIsFocusable(true);
	RefreshText();
}

void UTutorialFragmentWidget::InitializeFragment(
	AWCCharacter* InPlayerOwner,
	const FText& InDisplayTitle,
	const FText& InDisplayText)
{
	PlayerOwner = InPlayerOwner;
	PendingTitle = InDisplayTitle;
	PendingBody = InDisplayText;
	RefreshText();
}

void UTutorialFragmentWidget::RefreshText()
{
	if (DisplayTitle)
	{
		DisplayTitle->SetText(PendingTitle);
	}

	if (DisplayText)
	{
		DisplayText->SetText(PendingBody);
	}
}

void UTutorialFragmentWidget::HandleCloseClicked()
{
	if (PlayerOwner)
	{
		PlayerOwner->EndTutorialFragmentView();
		return;
	}

	RemoveFromParent();
}
