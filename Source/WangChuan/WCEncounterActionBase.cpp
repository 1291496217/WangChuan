#include "WCEncounterActionBase.h"

#include "WCMemoryMazeEncounter.h"

AWCEncounterActionBase::AWCEncounterActionBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWCEncounterActionBase::ExecuteAction(AWCMemoryMazeEncounter* SourceEncounter)
{
	if (!IsValid(SourceEncounter) || GetOwningEncounter() != SourceEncounter)
	{
		UE_LOG(LogTemp, Error,
			TEXT("Action '%s' rejected execution from Encounter '%s'; owning Encounter is '%s'."),
			*GetActorNameOrLabel(), *GetNameSafe(SourceEncounter),
			*GetNameSafe(GetOwningEncounter()));
		return;
	}

	if (bExecuted)
	{
		return;
	}

	bExecuted = true;
	ExecuteActionInternal(SourceEncounter);
}

bool AWCEncounterActionBase::HasExecuted() const
{
	return bExecuted;
}

bool AWCEncounterActionBase::TryClaimOwnership(
	AWCMemoryMazeEncounter* RequestingEncounter)
{
	if (!IsValid(RequestingEncounter))
	{
		UE_LOG(LogTemp, Error,
			TEXT("Action '%s' rejected an ownership claim from a null Encounter."),
			*GetActorNameOrLabel());
		return false;
	}

	AWCMemoryMazeEncounter* ExistingOwner = OwningEncounter.Get();
	if (!IsValid(ExistingOwner))
	{
		ResetRuntimeStateForNewOwner();
		OwningEncounter = RequestingEncounter;
		return true;
	}

	if (ExistingOwner == RequestingEncounter)
	{
		return true;
	}

	UE_LOG(LogTemp, Error,
		TEXT("Encounter '%s' cannot claim Action '%s'; it is already owned by Encounter '%s'."),
		*RequestingEncounter->GetActorNameOrLabel(), *GetActorNameOrLabel(),
		*ExistingOwner->GetActorNameOrLabel());
	return false;
}

void AWCEncounterActionBase::ReleaseOwnership(
	AWCMemoryMazeEncounter* RequestingEncounter)
{
	if (RequestingEncounter != nullptr && OwningEncounter.Get() == RequestingEncounter)
	{
		OwningEncounter.Reset();
	}
}

AWCMemoryMazeEncounter* AWCEncounterActionBase::GetOwningEncounter() const
{
	return OwningEncounter.Get();
}

void AWCEncounterActionBase::ResetRuntimeStateForNewOwner()
{
	bExecuted = false;
}

void AWCEncounterActionBase::ExecuteActionInternal(AWCMemoryMazeEncounter* SourceEncounter)
{
}
