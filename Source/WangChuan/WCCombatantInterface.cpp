#include "WCCombatantInterface.h"

#include "GameFramework/Actor.h"

namespace WCCombatant
{
	bool IsValidCombatant(const AActor* Actor)
	{
		return IsValid(Actor) &&
			Actor->GetClass()->ImplementsInterface(UWCCombatantInterface::StaticClass()) &&
			IWCCombatantInterface::Execute_IsCombatantAlive(Actor) &&
			IWCCombatantInterface::Execute_CanBeCombatTargeted(Actor);
	}

	EWCCombatFaction GetFaction(const AActor* Actor)
	{
		if (!IsValid(Actor) ||
			!Actor->GetClass()->ImplementsInterface(UWCCombatantInterface::StaticClass()))
		{
			return EWCCombatFaction::Neutral;
		}

		return IWCCombatantInterface::Execute_GetCombatFaction(Actor);
	}

	EWCFactionRelationship GetRelationship(
		EWCCombatFaction SourceTeam, EWCCombatFaction TargetTeam)
	{
		if (SourceTeam == EWCCombatFaction::Neutral ||
			TargetTeam == EWCCombatFaction::Neutral)
		{
			return EWCFactionRelationship::Neutral;
		}

		return SourceTeam == TargetTeam
			? EWCFactionRelationship::Friendly
			: EWCFactionRelationship::Hostile;
	}

	bool AreHostile(const AActor* SourceActor, const AActor* TargetActor)
	{
		return IsValidCombatant(SourceActor) && IsValidCombatant(TargetActor) &&
			GetRelationship(GetFaction(SourceActor), GetFaction(TargetActor)) ==
				EWCFactionRelationship::Hostile;
	}
}
