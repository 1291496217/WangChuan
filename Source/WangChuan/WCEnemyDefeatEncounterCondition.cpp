#include "WCEnemyDefeatEncounterCondition.h"

#include "GhostEnemy.h"

AWCEnemyDefeatEncounterCondition::AWCEnemyDefeatEncounterCondition()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWCEnemyDefeatEncounterCondition::BindToSource()
{
	if (!IsValid(RequiredEnemy))
	{
		UE_LOG(LogTemp, Error,
			TEXT("Enemy Defeat Condition '%s' requires a valid RequiredEnemy."), *GetName());
		return;
	}

	RequiredEnemy->OnEnemyDefeated.AddUniqueDynamic(
		this, &AWCEnemyDefeatEncounterCondition::HandleEnemyDefeated);
}

void AWCEnemyDefeatEncounterCondition::UnbindFromSource()
{
	if (IsValid(RequiredEnemy))
	{
		RequiredEnemy->OnEnemyDefeated.RemoveDynamic(
			this, &AWCEnemyDefeatEncounterCondition::HandleEnemyDefeated);
	}
}

void AWCEnemyDefeatEncounterCondition::CheckAlreadySatisfied()
{
	if (IsValid(RequiredEnemy) && RequiredEnemy->GetIsDead())
	{
		SetSatisfied();
	}
}

void AWCEnemyDefeatEncounterCondition::HandleEnemyDefeated(AGhostEnemy* DefeatedEnemy)
{
	if (DefeatedEnemy == RequiredEnemy)
	{
		SetSatisfied();
	}
}
