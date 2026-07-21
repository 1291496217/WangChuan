#include "MemoryEchoWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

#include "WCCharacter.h"

void UMemoryEchoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (EchoContinueButton)
	{
		EchoContinueButton
			->OnClicked.AddUniqueDynamic(
				this,
				&UMemoryEchoWidget::HandleContinueClicked
			);
	}
}

void UMemoryEchoWidget::StartMemoryEcho(
	const FMemoryEchoData& NewEchoData,
	AWCCharacter* NewPlayerOwner)
{
	ActiveEchoData = NewEchoData;
	PlayerOwner = NewPlayerOwner;
	CurrentPageIndex = 0;

	DisplayCurrentPage();
}

void UMemoryEchoWidget::AdvanceMemoryEcho()
{
	/*
	* Page 0
	* -> Page 1
	*/
	if (CurrentPageIndex == 0)
	{
		CurrentPageIndex = 1;

		DisplayCurrentPage();
		return;
	}

	/*
	* Page 1 之后关闭。
	*/
	RequestCloseEcho();
}

void UMemoryEchoWidget::DisplayCurrentPage()
{
	if (EchoTitleText)
	{
		EchoTitleText->SetText(
			ActiveEchoData.Title
		);
	}

	if (EchoBodyText)
	{
		if (CurrentPageIndex == 0)
		{
			EchoBodyText->SetText(
				ActiveEchoData.EchoText
			);
		}
		else
		{
			EchoBodyText->SetText(
				ActiveEchoData.PlayerReasonanceText
			);
		}
	}

	if (EchoContinueButtonLabel)
	{
		EchoContinueButtonLabel->SetText(
			CurrentPageIndex == 0
			? FText::FromString(
				TEXT("Continue")
			)
			: FText::FromString(
				TEXT("Close")
			)
		);
	}
}

void UMemoryEchoWidget::RequestCloseEcho()
{
	if (!PlayerOwner)
	{
		RemoveFromParent();
		return;
	}

	/*
	* true 表示玩家完整阅读完两页。
	*/
	PlayerOwner->EndMemoryEcho(true);
}

void UMemoryEchoWidget::HandleContinueClicked()
{
	AdvanceMemoryEcho();
}

