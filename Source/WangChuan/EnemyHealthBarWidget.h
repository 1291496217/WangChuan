#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHealthBarWidget.generated.h"

class AGhostEnemy;

UCLASS()
class WANGCHUAN_API UEnemyHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ******************** Setters ********************

	UFUNCTION(BlueprintCallable, Category = "Enemy UI")
	void SetEnemyOwner(AGhostEnemy* NewEnemyOwner);

	// ******************** Getters ********************

	UFUNCTION(BlueprintPure, Category = "Enemy UI")
	float GetEnemyHealthPercent() const;

private:
	// ******************** Runtime State ********************

	UPROPERTY()
	AGhostEnemy* EnemyOwner;
};
