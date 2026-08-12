#include "WCRevealTutorialFragmentEncounterAction.h"

#include "TutorialMemoryFragment.h"

AWCRevealTutorialFragmentEncounterAction::AWCRevealTutorialFragmentEncounterAction()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWCRevealTutorialFragmentEncounterAction::ExecuteActionInternal(
	AWCMemoryMazeEncounter* SourceEncounter)
{
	if (!IsValid(TargetFragment))
	{
		UE_LOG(LogTemp, Error,
			TEXT("Reveal Tutorial Fragment Action '%s' requires a valid TargetFragment."),
			*GetName());
		return;
	}

	TargetFragment->SetFragmentAvailable(true);
}
