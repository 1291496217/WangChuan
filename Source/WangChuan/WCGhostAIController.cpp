#include "WCGhostAIController.h"

#include "GhostEnemy.h"
#include "WCCharacter.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "TimerManager.h"

AWCGhostAIController::AWCGhostAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	bSetControlRotationFromPawnOrientation = true;

	SightPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("SightPerception"));
	SetPerceptionComponent(*SightPerception);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1200.0f;
	SightConfig->LoseSightRadius = 1500.0f;
	SightConfig->PeripheralVisionAngleDegrees = 70.0f;
	SightConfig->SetMaxAge(2.0f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	SightPerception->ConfigureSense(*SightConfig);
	SightPerception->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AWCGhostAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	GhostPawn = Cast<AGhostEnemy>(InPawn);
	TargetPlayer = nullptr;
	bCurrentlySeesPlayer = false;
	bLeashReturnLocked = false;

	if (!GhostPawn)
	{
		return;
	}

	ConfigureSightFromPawn();
	SetControlRotation(GhostPawn->GetActorRotation());
	GhostPawn->SetAIState(EGhostAIState::Idle);
	SightPerception->OnTargetPerceptionUpdated.AddUniqueDynamic(
		this, &AWCGhostAIController::HandleTargetPerceptionUpdated);
}

void AWCGhostAIController::OnUnPossess()
{
	if (SightPerception)
	{
		SightPerception->OnTargetPerceptionUpdated.RemoveDynamic(
			this, &AWCGhostAIController::HandleTargetPerceptionUpdated);
	}

	ClearAITimers();
	ClearFocus(EAIFocusPriority::Gameplay);
	GhostPawn = nullptr;
	TargetPlayer = nullptr;
	Super::OnUnPossess();
}

void AWCGhostAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!GhostPawn || GhostPawn->GetIsDead() || bLeashReturnLocked)
	{
		return;
	}

	const EGhostAIState State = GhostPawn->GetAIState();
	if (State != EGhostAIState::Chasing && State != EGhostAIState::Investigating &&
		State != EGhostAIState::Attacking)
	{
		return;
	}

	const float DistanceFromHome = FVector::Dist2D(
		GhostPawn->GetActorLocation(), GhostPawn->HomeLocation);
	if (DistanceFromHome > GhostPawn->MaxChaseDistanceFromHome)
	{
		bLeashReturnLocked = true;
		BeginReturnHome();
		GetWorldTimerManager().SetTimer(LeashHysteresisTimerHandle, this,
			&AWCGhostAIController::FinishLeashHysteresis, 1.0f, false);
	}
}

void AWCGhostAIController::OnMoveCompleted(FAIRequestID RequestID,
	const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (!GhostPawn || GhostPawn->GetIsDead())
	{
		return;
	}

	if (GhostPawn->GetAIState() == EGhostAIState::ReturningHome)
	{
		if (FVector::Dist2D(GhostPawn->GetActorLocation(), GhostPawn->HomeLocation) <= 100.0f)
		{
			GhostPawn->SetActorRotation(GhostPawn->HomeRotation);
			SetControlRotation(GhostPawn->HomeRotation);
			GhostPawn->SetAIState(EGhostAIState::Idle);
			TargetPlayer = nullptr;
			bCurrentlySeesPlayer = false;
			if (SightPerception)
			{
				SightPerception->ForgetAll();
				SightPerception->RequestStimuliListenerUpdate();
			}
		}
		else
		{
			GetWorldTimerManager().SetTimer(MoveRetryTimerHandle, this,
				&AWCGhostAIController::RetryReturnHome, 0.5f, false);
		}
	}
}

AWCCharacter* AWCGhostAIController::GetTargetPlayer() const
{
	return TargetPlayer;
}

void AWCGhostAIController::HandleDamageAggro(AWCCharacter* DamageInstigator)
{
	if (!GhostPawn || GhostPawn->GetIsDead() || !IsValid(DamageInstigator))
	{
		return;
	}

	bCurrentlySeesPlayer = true;
	LastSeenPlayerLocation = DamageInstigator->GetActorLocation();
	EnterChase(DamageInstigator);
}

void AWCGhostAIController::PauseForHitReaction()
{
	StopMovement();
}

void AWCGhostAIController::ResumeAfterAttack()
{
	if (!GhostPawn || GhostPawn->GetIsDead())
	{
		return;
	}

	// If the player died during this attack, let the current attack timer finish
	// naturally, then disengage. Do not start a fixed corpse-attack sequence.
	if (IsValid(TargetPlayer) && TargetPlayer->GetIsDead())
	{
		BeginReturnHome();
		return;
	}

	if (bCurrentlySeesPlayer && IsTargetUsable())
	{
		GhostPawn->SetAIState(EGhostAIState::Chasing);
		IssueChaseMove();
		return;
	}

	BeginInvestigation(LastSeenPlayerLocation);
}

void AWCGhostAIController::HandlePawnDeath()
{
	ClearAITimers();
	StopMovement();
	ClearFocus(EAIFocusPriority::Gameplay);
	TargetPlayer = nullptr;
	bCurrentlySeesPlayer = false;
	if (SightPerception)
	{
		SightPerception->SetSenseEnabled(UAISense_Sight::StaticClass(), false);
	}
}

void AWCGhostAIController::BeginReturnHome()
{
	if (!GhostPawn || GhostPawn->GetIsDead())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(LostSightTimerHandle);
	GetWorldTimerManager().ClearTimer(InvestigationTimerHandle);
	GetWorldTimerManager().ClearTimer(MoveRetryTimerHandle);
	bCurrentlySeesPlayer = false;
	TargetPlayer = nullptr;
	ClearFocus(EAIFocusPriority::Gameplay);
	StopMovement();
	GhostPawn->SetAIState(EGhostAIState::ReturningHome);
	const EPathFollowingRequestResult::Type MoveResult = MoveToLocation(
		GhostPawn->HomeLocation, 75.0f, false, true, true, false, nullptr, true);
	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		GetWorldTimerManager().SetTimer(MoveRetryTimerHandle, this,
			&AWCGhostAIController::RetryReturnHome, 0.5f, false);
	}
}

void AWCGhostAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	AWCCharacter* PlayerCharacter = Cast<AWCCharacter>(Actor);
	if (!GhostPawn || GhostPawn->GetIsDead() || !PlayerCharacter || bLeashReturnLocked)
	{
		return;
	}

	// A dead player can remain visible to AI Perception. Never let that stimulus
	// restart Chase while the Ghost is finishing its attack or returning home.
	if (PlayerCharacter->GetIsDead())
	{
		bCurrentlySeesPlayer = false;
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		bCurrentlySeesPlayer = true;
		LastSeenPlayerLocation = PlayerCharacter->GetActorLocation();
		EnterChase(PlayerCharacter);
		return;
	}

	if (TargetPlayer == PlayerCharacter)
	{
		bCurrentlySeesPlayer = false;
		LastSeenPlayerLocation = Stimulus.StimulusLocation;
		if (LastSeenPlayerLocation.IsNearlyZero())
		{
			LastSeenPlayerLocation = PlayerCharacter->GetActorLocation();
		}
		BeginInvestigation(LastSeenPlayerLocation);
	}
}

void AWCGhostAIController::ConfigureSightFromPawn()
{
	SightConfig->SightRadius = GhostPawn->SightRadius;
	SightConfig->LoseSightRadius = FMath::Max(GhostPawn->LoseSightRadius, GhostPawn->SightRadius);
	SightConfig->PeripheralVisionAngleDegrees = GhostPawn->PeripheralVisionHalfAngle;
	SightConfig->SetMaxAge(GhostPawn->SightMaxAge);
	SightPerception->ConfigureSense(*SightConfig);
	SightPerception->RequestStimuliListenerUpdate();
}

void AWCGhostAIController::EnterChase(AWCCharacter* PlayerCharacter)
{
	if (!GhostPawn || !IsValid(PlayerCharacter))
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(LostSightTimerHandle);
	GetWorldTimerManager().ClearTimer(InvestigationTimerHandle);
	GetWorldTimerManager().ClearTimer(MoveRetryTimerHandle);
	TargetPlayer = PlayerCharacter;
	GhostPawn->SetAIState(EGhostAIState::Chasing);
	SetFocus(PlayerCharacter, EAIFocusPriority::Gameplay);
	IssueChaseMove();
}

void AWCGhostAIController::BeginInvestigation(const FVector& LastSeenLocation)
{
	if (!GhostPawn || GhostPawn->GetIsDead())
	{
		return;
	}

	StopMovement();
	ClearFocus(EAIFocusPriority::Gameplay);
	GhostPawn->SetAIState(EGhostAIState::Investigating);
	MoveToLocation(LastSeenLocation, 75.0f, true, true, true, false, nullptr, true);
	GetWorldTimerManager().ClearTimer(LostSightTimerHandle);
	GetWorldTimerManager().SetTimer(LostSightTimerHandle, this,
		&AWCGhostAIController::FinishLostSightGrace, GhostPawn->LostSightGraceTime, false);
}

void AWCGhostAIController::FinishLostSightGrace()
{
	if (!GhostPawn || GhostPawn->GetIsDead() || bCurrentlySeesPlayer)
	{
		return;
	}

	StopMovement();
	GetWorldTimerManager().SetTimer(InvestigationTimerHandle, this,
		&AWCGhostAIController::FinishInvestigation, GhostPawn->InvestigateWaitTime, false);
}

void AWCGhostAIController::FinishInvestigation()
{
	if (!bCurrentlySeesPlayer)
	{
		BeginReturnHome();
	}
}

void AWCGhostAIController::FinishLeashHysteresis()
{
	bLeashReturnLocked = false;
}

void AWCGhostAIController::IssueChaseMove()
{
	if (!GhostPawn || !IsTargetUsable())
	{
		return;
	}

	// MoveToActor tracks a moving target through PathFollowing; do not rebuild every Tick.
	const EPathFollowingRequestResult::Type MoveResult = MoveToActor(
		TargetPlayer, GhostPawn->AttackRange * 0.8f, false, true, true, nullptr, true);

	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		FNavLocation ProjectedStart;
		FNavLocation ProjectedGoal;
		UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld());
		const bool bStartOnNav = NavigationSystem && NavigationSystem->ProjectPointToNavigation(
			GhostPawn->GetActorLocation(), ProjectedStart, FVector(200.0f, 200.0f, 300.0f));
		const bool bGoalOnNav = NavigationSystem && NavigationSystem->ProjectPointToNavigation(
			TargetPlayer->GetActorLocation(), ProjectedGoal, FVector(200.0f, 200.0f, 300.0f));
		if (!bMoveFailureLogged)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("Ghost [%s] MoveTo failed. StartOnNav=%s GoalOnNav=%s Start=%s Goal=%s"),
				*GhostPawn->GetName(), bStartOnNav ? TEXT("true") : TEXT("false"),
				bGoalOnNav ? TEXT("true") : TEXT("false"),
				*GhostPawn->GetActorLocation().ToCompactString(),
				*TargetPlayer->GetActorLocation().ToCompactString());
			bMoveFailureLogged = true;
		}

		// Dynamic NavMesh can still be finishing its initial tiles when PIE begins.
		// Retry at a low frequency instead of rebuilding a path every Tick.
		GetWorldTimerManager().SetTimer(MoveRetryTimerHandle, this,
			&AWCGhostAIController::RetryChaseMove, 0.5f, false);
	}
	else
	{
		bMoveFailureLogged = false;
	}

	if (GhostPawn->bShowAIDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("Ghost [%s] MoveToActor result: %d"),
			*GhostPawn->GetName(), static_cast<int32>(MoveResult));
	}
}

void AWCGhostAIController::RetryChaseMove()
{
	if (GhostPawn && GhostPawn->GetAIState() == EGhostAIState::Chasing &&
		bCurrentlySeesPlayer && IsTargetUsable())
	{
		IssueChaseMove();
	}
}

void AWCGhostAIController::RetryReturnHome()
{
	if (GhostPawn && GhostPawn->GetAIState() == EGhostAIState::ReturningHome)
	{
		BeginReturnHome();
	}
}

void AWCGhostAIController::ClearAITimers()
{
	GetWorldTimerManager().ClearTimer(LostSightTimerHandle);
	GetWorldTimerManager().ClearTimer(InvestigationTimerHandle);
	GetWorldTimerManager().ClearTimer(LeashHysteresisTimerHandle);
	GetWorldTimerManager().ClearTimer(MoveRetryTimerHandle);
}

bool AWCGhostAIController::IsTargetUsable() const
{
	return IsValid(TargetPlayer) && !TargetPlayer->GetIsDead();
}
