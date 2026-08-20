#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WCCombatantInterface.generated.h"

UENUM(BlueprintType)
enum class EWCCombatFaction : uint8
{
	Player,
	// Legacy v0.1 values kept to preserve serialized Blueprint/map data.
	Skeleton,
	Ghost,
	Neutral,
	EnemyTeamA,
	EnemyTeamB
};

UENUM(BlueprintType)
enum class EWCFactionRelationship : uint8
{
	Friendly,
	Neutral,
	Hostile
};

UINTERFACE(BlueprintType)
class WANGCHUAN_API UWCCombatantInterface : public UInterface
{
	GENERATED_BODY()
};

class WANGCHUAN_API IWCCombatantInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combatant")
	bool IsCombatantAlive() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combatant")
	// Compatibility API name: the returned value represents Combat Team only,
	// never species, Blueprint class, or Skeleton archetype identity.
	EWCCombatFaction GetCombatFaction() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combatant")
	bool CanBeCombatTargeted() const;
};

namespace WCCombatant
{
	WANGCHUAN_API bool IsValidCombatant(const AActor* Actor);
	// Compatibility helper name. Semantically this returns the actor's Combat Team.
	WANGCHUAN_API EWCCombatFaction GetFaction(const AActor* Actor);
	WANGCHUAN_API EWCFactionRelationship GetRelationship(
		EWCCombatFaction SourceFaction, EWCCombatFaction TargetFaction);
	WANGCHUAN_API bool AreHostile(const AActor* SourceActor, const AActor* TargetActor);
}
