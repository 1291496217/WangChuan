#include "WCEncounterConditionBase.h"

#include "WCMemoryMazeEncounter.h"

AWCEncounterConditionBase::AWCEncounterConditionBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWCEncounterConditionBase::ActivateCondition()
{
	if (bIsActive || bIsSatisfied)
	{
		return;
	}

	bIsActive = true;
	BindToSource();
	CheckAlreadySatisfied();
}

void AWCEncounterConditionBase::DeactivateCondition()
{
	if (!bIsActive)
	{
		return;
	}

	UnbindFromSource();
	bIsActive = false;
}

bool AWCEncounterConditionBase::IsSatisfied() const
{
	return bIsSatisfied;
}

bool AWCEncounterConditionBase::TryClaimOwnership(
	AWCMemoryMazeEncounter* RequestingEncounter)
{
	if (!IsValid(RequestingEncounter))
	{
		UE_LOG(LogTemp, Error,
			TEXT("Condition '%s' rejected an ownership claim from a null Encounter."),
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
		TEXT("Encounter '%s' cannot claim Condition '%s'; it is already owned by Encounter '%s'."),
		*RequestingEncounter->GetActorNameOrLabel(), *GetActorNameOrLabel(),
		*ExistingOwner->GetActorNameOrLabel());
	return false;
}

void AWCEncounterConditionBase::ReleaseOwnership(
	AWCMemoryMazeEncounter* RequestingEncounter)
{
	if (RequestingEncounter != nullptr && OwningEncounter.Get() == RequestingEncounter)
	{
		OwningEncounter.Reset();
	}
}

AWCMemoryMazeEncounter* AWCEncounterConditionBase::GetOwningEncounter() const
{
	return OwningEncounter.Get();
}

void AWCEncounterConditionBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeactivateCondition();
	Super::EndPlay(EndPlayReason);
}

void AWCEncounterConditionBase::ResetRuntimeStateForNewOwner()
{
	bIsActive = false;
	bIsSatisfied = false;
}

void AWCEncounterConditionBase::BindToSource()
{
}

void AWCEncounterConditionBase::UnbindFromSource()
{
}

void AWCEncounterConditionBase::CheckAlreadySatisfied()
{
}

void AWCEncounterConditionBase::SetSatisfied()
{
	if (!bIsActive || bIsSatisfied)
	{
		return;
	}

	bIsSatisfied = true;
	OnConditionSatisfied.Broadcast(this);
}
