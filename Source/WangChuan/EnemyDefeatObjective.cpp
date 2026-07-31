#include "EnemyDefeatObjective.h"

#include "GhostEnemy.h"

// ******************** Construction ********************

AEnemyDefeatObjective::AEnemyDefeatObjective()
{
	PrimaryActorTick.bCanEverTick = false;
}

// ******************** Lifecycle ********************

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
	* 敌人死亡是立即生效的世界条件。
	* Encounter 监听通用完成事件，但不负责激活此 Objective。
	*/
	ActivateObjective();
}

void AEnemyDefeatObjective::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	UnbindRequiredEnemy();

	Super::EndPlay(EndPlayReason);
}

// ******************** Objectives ********************

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

// ******************** Helpers ********************

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

// ******************** Events ********************

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
