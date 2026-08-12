#include "TutorialPuzzleCompletionLink.h"

#include "LanternSequencePuzzle.h"
#include "StoryObjectiveBase.h"
#include "TutorialMemoryFragment.h"
#include "WCTutorialGate.h"

ATutorialPuzzleCompletionLink::ATutorialPuzzleCompletionLink()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATutorialPuzzleCompletionLink::BeginPlay()
{
	Super::BeginPlay();

	bCompletionHandled = false;
	if (!IsValid(RequiredPuzzle) || !IsValid(RewardFragment) || !IsValid(BossGate))
	{
		UE_LOG(LogTemp, Error,
			TEXT("Tutorial Puzzle Completion Link [%s] has invalid references."),
			*GetName());
		return;
	}

	RequiredPuzzle->OnObjectiveCompleted.AddUniqueDynamic(
		this, &ATutorialPuzzleCompletionLink::HandlePuzzleCompleted);

	if (RequiredPuzzle->GetIsObjectiveComplete())
	{
		HandlePuzzleCompleted(RequiredPuzzle);
	}
}

void ATutorialPuzzleCompletionLink::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindFromPuzzle();
	Super::EndPlay(EndPlayReason);
}

void ATutorialPuzzleCompletionLink::HandlePuzzleCompleted(
	AStoryObjectiveBase* CompletedObjective)
{
	if (bCompletionHandled || CompletedObjective != RequiredPuzzle)
	{
		return;
	}

	if (!IsValid(RewardFragment) || !IsValid(BossGate))
	{
		UE_LOG(LogTemp, Error,
			TEXT("Tutorial Puzzle Completion Link [%s] lost a reward reference."),
			*GetName());
		return;
	}

	bCompletionHandled = true;
	RewardFragment->SetFragmentAvailable(true);
	BossGate->OpenGate();
	UnbindFromPuzzle();
}

void ATutorialPuzzleCompletionLink::UnbindFromPuzzle()
{
	if (IsValid(RequiredPuzzle))
	{
		RequiredPuzzle->OnObjectiveCompleted.RemoveDynamic(
			this, &ATutorialPuzzleCompletionLink::HandlePuzzleCompleted);
	}
}
