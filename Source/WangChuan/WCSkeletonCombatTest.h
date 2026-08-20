#pragma once

#include "CoreMinimal.h"
#include "GhostEnemy.h"
#include "WCSkeletonCombatTest.generated.h"

/**
 * Thin Skeleton identity/base used by the isolated Week10 arena. All movement,
 * perception, combat, health and death behavior remains inherited from Ghost.
 * Blueprint children provide archetype identity; CombatFaction is team-only.
 */
UCLASS(Blueprintable)
class WANGCHUAN_API AWCSkeletonCombatTest : public AGhostEnemy
{
	GENERATED_BODY()

public:
	AWCSkeletonCombatTest();
};
