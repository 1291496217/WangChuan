#include "WCMemoryMazeEncounter.h"

#include "WCEncounterActionBase.h"
#include "WCEncounterConditionBase.h"

AWCMemoryMazeEncounter::AWCMemoryMazeEncounter()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWCMemoryMazeEncounter::BeginPlay()
{
	Super::BeginPlay();
	if (bAutoActivateOnBeginPlay)
	{
		ActivateEncounter();
	}
}

void AWCMemoryMazeEncounter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeactivateConditions();
	ReleaseConfiguredObjectOwnership();
	Super::EndPlay(EndPlayReason);
}

void AWCMemoryMazeEncounter::ActivateEncounter()
{
	if (EncounterState != EWCMemoryMazeEncounterState::Dormant)
	{
		return;
	}

	ValidateConfiguration();
	if (!TryClaimConfiguredObjectOwnership())
	{
		UE_LOG(LogTemp, Error,
			TEXT("Encounter '%s' remains Dormant because an ownership claim failed."),
			*GetActorNameOrLabel());
		return;
	}

	for (AWCEncounterConditionBase* Condition : CompletionConditions)
	{
		if (!IsValid(Condition))
		{
			continue;
		}

		Condition->OnConditionSatisfied.AddUniqueDynamic(
			this, &AWCMemoryMazeEncounter::HandleConditionSatisfied);
	}

	EncounterState = EWCMemoryMazeEncounterState::Active;

	for (AWCEncounterConditionBase* Condition : CompletionConditions)
	{
		if (IsValid(Condition))
		{
			Condition->ActivateCondition();
		}
	}

	OnEncounterActivated.Broadcast(this);
	EvaluateCompletion();
}

void AWCMemoryMazeEncounter::CompleteEncounter()
{
	if (EncounterState == EWCMemoryMazeEncounterState::Completed || bCompletionHandled)
	{
		return;
	}

	if (EncounterState != EWCMemoryMazeEncounterState::Active)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Encounter '%s' cannot complete while it is not Active."), *GetName());
		return;
	}

	bCompletionHandled = true;
	EncounterState = EWCMemoryMazeEncounterState::Completed;
	DeactivateConditions();

	TSet<AWCEncounterActionBase*> ExecutedActions;
	for (AWCEncounterActionBase* Action : CompletionActions)
	{
		if (!IsValid(Action))
		{
			UE_LOG(LogTemp, Error,
				TEXT("Encounter '%s' contains a null CompletionAction."), *GetName());
			continue;
		}

		if (ExecutedActions.Contains(Action))
		{
			continue;
		}

		ExecutedActions.Add(Action);
		Action->ExecuteAction(this);
	}

	OnEncounterCompleted.Broadcast(this);
}

EWCMemoryMazeEncounterState AWCMemoryMazeEncounter::GetEncounterState() const
{
	return EncounterState;
}

bool AWCMemoryMazeEncounter::IsEncounterCompleted() const
{
	return EncounterState == EWCMemoryMazeEncounterState::Completed;
}

void AWCMemoryMazeEncounter::HandleConditionSatisfied(
	AWCEncounterConditionBase* SatisfiedCondition)
{
	EvaluateCompletion();
}

void AWCMemoryMazeEncounter::EvaluateCompletion()
{
	if (EncounterState != EWCMemoryMazeEncounterState::Active ||
		CompletionConditions.IsEmpty())
	{
		if (EncounterState == EWCMemoryMazeEncounterState::Active &&
			CompletionConditions.IsEmpty())
		{
			UE_LOG(LogTemp, Warning,
				TEXT("Encounter '%s' has zero CompletionConditions and will remain Active."),
				*GetName());
		}
		return;
	}

	bool bAllSatisfied = true;
	bool bAnySatisfied = false;
	for (AWCEncounterConditionBase* Condition : CompletionConditions)
	{
		if (!IsValid(Condition))
		{
			bAllSatisfied = false;
			continue;
		}

		const bool bSatisfied = Condition->IsSatisfied();
		bAllSatisfied &= bSatisfied;
		bAnySatisfied |= bSatisfied;
	}

	if ((CompletionPolicy == EWCEncounterCompletionPolicy::All && bAllSatisfied) ||
		(CompletionPolicy == EWCEncounterCompletionPolicy::Any && bAnySatisfied))
	{
		CompleteEncounter();
	}
}

void AWCMemoryMazeEncounter::DeactivateConditions()
{
	for (AWCEncounterConditionBase* Condition : CompletionConditions)
	{
		if (!IsValid(Condition) || Condition->GetOwningEncounter() != this)
		{
			continue;
		}

		Condition->OnConditionSatisfied.RemoveDynamic(
			this, &AWCMemoryMazeEncounter::HandleConditionSatisfied);
		Condition->DeactivateCondition();
	}
}

bool AWCMemoryMazeEncounter::TryClaimConfiguredObjectOwnership()
{
	TArray<AWCEncounterConditionBase*> NewlyClaimedConditions;
	TArray<AWCEncounterActionBase*> NewlyClaimedActions;

	auto RollBackNewClaims = [this, &NewlyClaimedConditions, &NewlyClaimedActions]()
	{
		for (int32 Index = NewlyClaimedActions.Num() - 1; Index >= 0; --Index)
		{
			if (AWCEncounterActionBase* Action = NewlyClaimedActions[Index])
			{
				Action->ReleaseOwnership(this);
			}
		}

		for (int32 Index = NewlyClaimedConditions.Num() - 1; Index >= 0; --Index)
		{
			if (AWCEncounterConditionBase* Condition = NewlyClaimedConditions[Index])
			{
				Condition->ReleaseOwnership(this);
			}
		}
	};

	for (AWCEncounterConditionBase* Condition : CompletionConditions)
	{
		if (!IsValid(Condition))
		{
			continue;
		}

		const bool bWasUnowned = Condition->GetOwningEncounter() == nullptr;
		if (!Condition->TryClaimOwnership(this))
		{
			RollBackNewClaims();
			return false;
		}

		if (bWasUnowned)
		{
			NewlyClaimedConditions.AddUnique(Condition);
		}
	}

	for (AWCEncounterActionBase* Action : CompletionActions)
	{
		if (!IsValid(Action))
		{
			continue;
		}

		const bool bWasUnowned = Action->GetOwningEncounter() == nullptr;
		if (!Action->TryClaimOwnership(this))
		{
			RollBackNewClaims();
			return false;
		}

		if (bWasUnowned)
		{
			NewlyClaimedActions.AddUnique(Action);
		}
	}

	return true;
}

void AWCMemoryMazeEncounter::ReleaseConfiguredObjectOwnership()
{
	for (AWCEncounterConditionBase* Condition : CompletionConditions)
	{
		if (IsValid(Condition))
		{
			Condition->ReleaseOwnership(this);
		}
	}

	for (AWCEncounterActionBase* Action : CompletionActions)
	{
		if (IsValid(Action))
		{
			Action->ReleaseOwnership(this);
		}
	}
}

void AWCMemoryMazeEncounter::ValidateConfiguration() const
{
	TSet<const AWCEncounterConditionBase*> SeenConditions;
	for (const AWCEncounterConditionBase* Condition : CompletionConditions)
	{
		if (!IsValid(Condition))
		{
			UE_LOG(LogTemp, Error,
				TEXT("Encounter '%s' contains a null CompletionCondition."), *GetName());
			continue;
		}
		if (SeenConditions.Contains(Condition))
		{
			UE_LOG(LogTemp, Warning,
				TEXT("Encounter '%s' contains duplicate Condition '%s'."),
				*GetName(), *Condition->GetName());
		}
		SeenConditions.Add(Condition);
	}

	TSet<const AWCEncounterActionBase*> SeenActions;
	for (const AWCEncounterActionBase* Action : CompletionActions)
	{
		if (!IsValid(Action))
		{
			UE_LOG(LogTemp, Error,
				TEXT("Encounter '%s' contains a null CompletionAction."), *GetName());
			continue;
		}
		if (SeenActions.Contains(Action))
		{
			UE_LOG(LogTemp, Warning,
				TEXT("Encounter '%s' contains duplicate Action '%s'; it will execute once."),
				*GetName(), *Action->GetName());
		}
		SeenActions.Add(Action);
	}
}
