#include "TutorialBossEncounter.h"

#include "GhostEnemy.h"
#include "WCTutorialGate.h"

ATutorialBossEncounter::ATutorialBossEncounter()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATutorialBossEncounter::BeginPlay()
{
	Super::BeginPlay();

	bHandled = false;
	if (!IsValid(RequiredBoss) || !IsValid(ExitGate))
	{
		UE_LOG(LogTemp, Error,
			TEXT("TutorialBossEncounter '%s' requires valid RequiredBoss and ExitGate references."),
			*GetName());
		return;
	}

	RequiredBoss->OnEnemyDefeated.AddUniqueDynamic(
		this, &ATutorialBossEncounter::HandleBossDefeated);
}

void ATutorialBossEncounter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindBoss();
	Super::EndPlay(EndPlayReason);
}

void ATutorialBossEncounter::HandleBossDefeated(AGhostEnemy* DefeatedEnemy)
{
	if (bHandled || DefeatedEnemy != RequiredBoss)
	{
		return;
	}

	bHandled = true;
	if (IsValid(ExitGate))
	{
		ExitGate->OpenGate();
	}

	UnbindBoss();
}

void ATutorialBossEncounter::UnbindBoss()
{
	if (IsValid(RequiredBoss))
	{
		RequiredBoss->OnEnemyDefeated.RemoveDynamic(
			this, &ATutorialBossEncounter::HandleBossDefeated);
	}
}
