#include "WCOpenGateEncounterAction.h"

#include "WCProgressionGate.h"

AWCOpenGateEncounterAction::AWCOpenGateEncounterAction()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWCOpenGateEncounterAction::ExecuteActionInternal(
	AWCMemoryMazeEncounter* SourceEncounter)
{
	if (!IsValid(TargetGate))
	{
		UE_LOG(LogTemp, Error,
			TEXT("Open Gate Action '%s' requires a valid TargetGate."), *GetName());
		return;
	}

	TargetGate->OpenGate();
}
