// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHealthBarWidget.h"
#include "GhostEnemy.h"

void UEnemyHealthBarWidget::SetEnemyOwner(AGhostEnemy* NewEnemyOwner) {
	EnemyOwner = NewEnemyOwner;
}

float UEnemyHealthBarWidget::GetEnemyHealthPercent() const {
	if (EnemyOwner == nullptr) {
		return 0.0f;
	}
	return EnemyOwner->GetHealthPercent();
}
