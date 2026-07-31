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

void AEnemyDefeatObjective::OnSavedObjectiveStateApplied()
{
	Super::OnSavedObjectiveStateApplied();

	if (GetIsObjectiveComplete())
	{
		bRequiredEnemyDefeated = true;

		/*
		* 已完成 Objective 不再需要监听 Enemy。
		*/
		UnbindRequiredEnemy();

		if (IsValid(RequiredEnemy))
		{
			RequiredEnemy
				->ApplyPersistentDefeatedState();
		}

		UE_LOG(
			LogTemp,
			Log,
			TEXT(
				"Enemy Defeat Objective [%s] silently "
				"restored as completed."
			),
			*ObjectiveID.ToString()
		);

		return;
	}

	/*
	* 未完成存档不恢复中途战斗状态。
	* 在新的默认 World 中，Enemy 应处于正常存活状态。
	*/
	bRequiredEnemyDefeated = false;

	bConfigurationValid =
		ValidateConfiguration();

	if (!bConfigurationValid)
	{
		UnbindRequiredEnemy();
		bIsActive = false;
		return;
	}

	/*
	* 如果存档说未完成，但当前 Enemy 已经死亡，
	* 当前 World 与 Save Data 存在矛盾。
	*
	* 不通过 CompleteObjective() 自动纠正，
	* 因为那会广播 Gameplay Delegate。
	*/
	if (IsValid(RequiredEnemy) &&
		RequiredEnemy->GetIsDead())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"Enemy Defeat Objective [%s] restore "
				"found an already-dead Enemy while the "
				"saved Objective is incomplete."
			),
			*ObjectiveID.ToString()
		);

		UnbindRequiredEnemy();
		bIsActive = false;
		return;
	}

	BindRequiredEnemy();

	/*
	* Enemy Objective 是立即活动的世界条件。
	*
	* 直接设置 Active，不调用 ActivateObjective()，
	* 避免它重新检查 Enemy 并可能调用 CompleteObjective()。
	*/
	bIsActive = true;

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Enemy Defeat Objective [%s] silently "
			"restored as incomplete and active."
		),
		*ObjectiveID.ToString()
	);
}