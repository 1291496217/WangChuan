#include "TutorialEnemyDefeatFragmentLink.h"

#include "GhostEnemy.h"
#include "TutorialMemoryFragment.h"

ATutorialEnemyDefeatFragmentLink::ATutorialEnemyDefeatFragmentLink()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATutorialEnemyDefeatFragmentLink::BeginPlay()
{
	Super::BeginPlay();

	if (!IsValid(RequiredEnemy) || !IsValid(RewardFragment))
	{
		UE_LOG(LogTemp, Error,
			TEXT("Tutorial defeat link [%s] requires both enemy and fragment references."),
			*GetName());
		return;
	}

	if (RewardFragment->GetStartAvailable())
	{
		UE_LOG(LogTemp, Error,
			TEXT("Tutorial defeat link [%s] reward fragment must start unavailable."),
			*GetName());
		return;
	}

	RequiredEnemy->OnEnemyDefeated.AddUniqueDynamic(
		this, &ATutorialEnemyDefeatFragmentLink::HandleRequiredEnemyDefeated);
	bDelegateBound = true;
}

void ATutorialEnemyDefeatFragmentLink::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	UnbindRequiredEnemy();
	Super::EndPlay(EndPlayReason);
}

void ATutorialEnemyDefeatFragmentLink::HandleRequiredEnemyDefeated(
	AGhostEnemy* DefeatedEnemy)
{
	if (bRewardGranted || DefeatedEnemy != RequiredEnemy || !IsValid(RewardFragment))
	{
		return;
	}

	bRewardGranted = true;
	RewardFragment->SetFragmentAvailable(true);
	UnbindRequiredEnemy();
}

void ATutorialEnemyDefeatFragmentLink::UnbindRequiredEnemy()
{
	if (bDelegateBound && IsValid(RequiredEnemy))
	{
		RequiredEnemy->OnEnemyDefeated.RemoveDynamic(
			this, &ATutorialEnemyDefeatFragmentLink::HandleRequiredEnemyDefeated);
	}

	bDelegateBound = false;
}
