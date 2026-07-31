#include "EnemyHealthBarWidget.h"
#include "GhostEnemy.h"

// ******************** Setters ********************

void UEnemyHealthBarWidget::SetEnemyOwner(AGhostEnemy* NewEnemyOwner)
{
	EnemyOwner = NewEnemyOwner;
}

// ******************** Getters ********************

float UEnemyHealthBarWidget::GetEnemyHealthPercent() const
{
	if (EnemyOwner == nullptr)
	{
		return 0.0f;
	}
	return EnemyOwner->GetHealthPercent();
}
