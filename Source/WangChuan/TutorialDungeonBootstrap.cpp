#include "TutorialDungeonBootstrap.h"

#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "TutorialFragmentWidget.h"
#include "TutorialHUDWidget.h"
#include "TutorialInstructionWidget.h"
#include "WCCharacter.h"

ATutorialDungeonBootstrap::ATutorialDungeonBootstrap()
{
	PrimaryActorTick.bCanEverTick = false;
	TutorialHUDWidgetClass = UTutorialHUDWidget::StaticClass();
	TutorialFragmentWidgetClass = UTutorialFragmentWidget::StaticClass();
	TutorialInstructionWidgetClass = UTutorialInstructionWidget::StaticClass();
}

void ATutorialDungeonBootstrap::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimerForNextTick(
		this, &ATutorialDungeonBootstrap::InitializeLocalTutorialPlayer);
}

void ATutorialDungeonBootstrap::InitializeLocalTutorialPlayer()
{
	APlayerController* PlayerController = GetWorld()
		? GetWorld()->GetFirstPlayerController()
		: nullptr;
	AWCCharacter* Player = PlayerController
		? Cast<AWCCharacter>(PlayerController->GetPawn())
		: nullptr;

	if (!Player)
	{
		UE_LOG(LogTemp, Error,
			TEXT("Tutorial Bootstrap could not find the local AWCCharacter."));
		return;
	}

	Player->InitializeTutorialSession(
		TutorialHUDWidgetClass,
		TutorialFragmentWidgetClass,
		TutorialInstructionWidgetClass,
		TotalFragmentCount
	);
	Player->ShowTutorialInstruction(
		TEXT("Tutorial.Instruction.MoveLook"),
		MoveLookInstructionTitle,
		MoveLookInstructionBody
	);
}
