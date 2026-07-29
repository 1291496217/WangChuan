#include "EnemyDefeatObjective.h"

#include "GhostEnemy.h"

AEnemyDefeatObjective::AEnemyDefeatObjective()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemyDefeatObjective::BeginPlay()
{
	Super::BeginPlay();

	bRequiredEnemyDefeated = false;
	bConfigurationValid = ValidateConfiguration();

	if (!bConfigurationValid)
	{
		return;
	}

	BindRequiredEnemy();
	RefreshRequiredEnemyState();

	/*
	* Enemy defeat is an immediately active world condition. The Encounter
	* observes the shared completion delegate but does not own activation.
	*/
	ActivateObjective();
}

void AEnemyDefeatObjective::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	UnbindRequiredEnemy();

	Super::EndPlay(EndPlayReason);
}

void AEnemyDefeatObjective::ActivateObjective()
{
	if (!bConfigurationValid)
	{
		bConfigurationValid = ValidateConfiguration();
	}

	if (!bConfigurationValid)
	{
		return;
	}

	BindRequiredEnemy();
	RefreshRequiredEnemyState();

	Super::ActivateObjective();

	CompleteIfReady();
}

bool AEnemyDefeatObjective::ValidateConfiguration() const
{
	if (IsValid(RequiredEnemy))
	{
		return true;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"Enemy Defeat Objective [%s] Actor [%s] has no "
			"RequiredEnemy assigned."
		),
		*ObjectiveID.ToString(),
		*GetName()
	);

	return false;
}

void AEnemyDefeatObjective::BindRequiredEnemy()
{
	if (bEnemyDelegateBound || !IsValid(RequiredEnemy))
	{
		return;
	}

	RequiredEnemy->OnEnemyDefeated.AddUniqueDynamic(
		this,
		&AEnemyDefeatObjective::HandleRequiredEnemyDefeated
	);

	bEnemyDelegateBound = true;

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Enemy Defeat Objective [%s] bound to Required Enemy [%s]."
		),
		*ObjectiveID.ToString(),
		*RequiredEnemy->GetName()
	);
}

void AEnemyDefeatObjective::UnbindRequiredEnemy()
{
	if (!bEnemyDelegateBound)
	{
		return;
	}

	if (IsValid(RequiredEnemy))
	{
		RequiredEnemy->OnEnemyDefeated.RemoveDynamic(
			this,
			&AEnemyDefeatObjective::HandleRequiredEnemyDefeated
		);
	}

	bEnemyDelegateBound = false;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Enemy Defeat Objective [%s] unbound."),
		*ObjectiveID.ToString()
	);
}

void AEnemyDefeatObjective::RefreshRequiredEnemyState()
{
	if (!IsValid(RequiredEnemy) || bRequiredEnemyDefeated)
	{
		return;
	}

	if (!RequiredEnemy->GetIsDead())
	{
		return;
	}

	bRequiredEnemyDefeated = true;

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Enemy Defeat Objective [%s] found Required Enemy "
			"[%s] already defeated."
		),
		*ObjectiveID.ToString(),
		*RequiredEnemy->GetName()
	);
}

void AEnemyDefeatObjective::CompleteIfReady()
{
	if (!bRequiredEnemyDefeated ||
		!GetIsObjectiveActive() ||
		GetIsObjectiveComplete())
	{
		return;
	}

	CompleteObjective();
}

void AEnemyDefeatObjective::HandleRequiredEnemyDefeated(
	AGhostEnemy* DefeatedEnemy)
{
	if (!bConfigurationValid ||
		bRequiredEnemyDefeated ||
		!IsValid(RequiredEnemy) ||
		!IsValid(DefeatedEnemy) ||
		DefeatedEnemy != RequiredEnemy)
	{
		return;
	}

	bRequiredEnemyDefeated = true;

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Enemy Defeat Objective [%s] received defeat from "
			"Required Enemy [%s]."
		),
		*ObjectiveID.ToString(),
		*DefeatedEnemy->GetName()
	);

	CompleteIfReady();
}
