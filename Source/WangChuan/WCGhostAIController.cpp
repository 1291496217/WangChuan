#include "WCGhostAIController.h"

#include "GhostEnemy.h"
#include "WCCombatantInterface.h"
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
	CurrentTarget = nullptr;
	bCurrentlySeesTarget = false;
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
	ClearCurrentTarget();
	GhostPawn = nullptr;
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
		return;
	}
	if (!IsTargetUsable())
	{
		const FVector PreviousTargetLocation = LastSeenTargetLocation;
		ClearCurrentTarget();
		if (!SelectNearestPerceivedHostile())
		{
			BeginInvestigation(PreviousTargetLocation);
		}
		return;
	}
	RecoverInterruptedChaseMove();
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
			ClearCurrentTarget();
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
	else if (GhostPawn->GetAIState() == EGhostAIState::Chasing &&
		!GhostPawn->GetIsHitReacting() && !GhostPawn->GetIsAttacking() &&
		(Result.Code == EPathFollowingResult::Blocked ||
			Result.Code == EPathFollowingResult::OffPath ||
			Result.Code == EPathFollowingResult::Invalid))
	{
		if (!GetWorldTimerManager().IsTimerActive(MoveRetryTimerHandle))
		{
			GetWorldTimerManager().SetTimer(MoveRetryTimerHandle, this,
				&AWCGhostAIController::RetryChaseMove, 0.5f, false);
		}
	}
}

AActor* AWCGhostAIController::GetTargetActor() const
{
	return CurrentTarget;
}

void AWCGhostAIController::HandleDamageAggro(AActor* DamageInstigator)
{
	if (!GhostPawn || GhostPawn->GetIsDead() ||
		!WCCombatant::AreHostile(GhostPawn, DamageInstigator))
	{
		return;
	}
	if (IsTargetUsable())
	{
		return;
	}
	bCurrentlySeesTarget = true;
	LastSeenTargetLocation = DamageInstigator->GetActorLocation();
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
	if (!IsTargetUsable())
	{
		ClearCurrentTarget();
		if (!SelectNearestPerceivedHostile())
		{
			BeginReturnHome();
		}
		return;
	}
	if (bCurrentlySeesTarget)
	{
		GhostPawn->SetAIState(EGhostAIState::Chasing);
		IssueChaseMove();
		return;
	}
	BeginInvestigation(LastSeenTargetLocation);
}

void AWCGhostAIController::HandlePawnDeath()
{
	ClearAITimers();
	StopMovement();
	ClearCurrentTarget();
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
	ClearCurrentTarget();
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
	if (!GhostPawn || GhostPawn->GetIsDead() || bLeashReturnLocked ||
		!WCCombatant::AreHostile(GhostPawn, Actor))
	{
		return;
	}
	if (Stimulus.WasSuccessfullySensed())
	{
		if (CurrentTarget == Actor)
		{
			bCurrentlySeesTarget = true;
			LastSeenTargetLocation = Actor->GetActorLocation();
			return;
		}
		if (!IsTargetUsable())
		{
			SelectNearestPerceivedHostile();
		}
		return;
	}
	if (CurrentTarget == Actor)
	{
		LastSeenTargetLocation = Stimulus.StimulusLocation.IsNearlyZero()
			? Actor->GetActorLocation()
			: Stimulus.StimulusLocation;
		ClearCurrentTarget();
		if (!SelectNearestPerceivedHostile())
		{
			BeginInvestigation(LastSeenTargetLocation);
		}
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

void AWCGhostAIController::EnterChase(AActor* TargetActor)
{
	if (!GhostPawn || !WCCombatant::AreHostile(GhostPawn, TargetActor))
	{
		return;
	}
	GetWorldTimerManager().ClearTimer(LostSightTimerHandle);
	GetWorldTimerManager().ClearTimer(InvestigationTimerHandle);
	GetWorldTimerManager().ClearTimer(MoveRetryTimerHandle);
	CurrentTarget = TargetActor;
	LastSeenTargetLocation = TargetActor->GetActorLocation();
	bCurrentlySeesTarget = true;
	UE_LOG(LogTemp, Display, TEXT("[EnemyEcology] %s acquired target %s"),
		*GhostPawn->GetName(), *TargetActor->GetName());
	GhostPawn->SetAIState(EGhostAIState::Chasing);
	SetFocus(TargetActor, EAIFocusPriority::Gameplay);
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
	if (!GhostPawn || GhostPawn->GetIsDead() || bCurrentlySeesTarget)
	{
		return;
	}
	StopMovement();
	GetWorldTimerManager().SetTimer(InvestigationTimerHandle, this,
		&AWCGhostAIController::FinishInvestigation, GhostPawn->InvestigateWaitTime, false);
}

void AWCGhostAIController::FinishInvestigation()
{
	if (!bCurrentlySeesTarget)
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
	const EPathFollowingRequestResult::Type MoveResult = MoveToActor(
		CurrentTarget, GhostPawn->AttackRange * 0.8f, false, true, true, nullptr, true);
	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		FNavLocation ProjectedStart;
		FNavLocation ProjectedGoal;
		UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld());
		const bool bStartOnNav = NavigationSystem && NavigationSystem->ProjectPointToNavigation(
			GhostPawn->GetActorLocation(), ProjectedStart, FVector(200.0f, 200.0f, 300.0f));
		const bool bGoalOnNav = NavigationSystem && NavigationSystem->ProjectPointToNavigation(
			CurrentTarget->GetActorLocation(), ProjectedGoal, FVector(200.0f, 200.0f, 300.0f));
		if (!bMoveFailureLogged)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("Ghost [%s] MoveTo failed. StartOnNav=%s GoalOnNav=%s Start=%s Goal=%s"),
				*GhostPawn->GetName(), bStartOnNav ? TEXT("true") : TEXT("false"),
				bGoalOnNav ? TEXT("true") : TEXT("false"),
				*GhostPawn->GetActorLocation().ToCompactString(),
				*CurrentTarget->GetActorLocation().ToCompactString());
			bMoveFailureLogged = true;
		}
		GetWorldTimerManager().SetTimer(MoveRetryTimerHandle, this,
			&AWCGhostAIController::RetryChaseMove, 0.5f, false);
	}
	else
	{
		bMoveFailureLogged = false;
	}
	if (GhostPawn->bShowAIDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("Ghost [%s] MoveToActor result: %d target=%s"),
			*GhostPawn->GetName(), static_cast<int32>(MoveResult), *CurrentTarget->GetName());
	}
}

void AWCGhostAIController::RecoverInterruptedChaseMove()
{
	if (!GhostPawn || GhostPawn->GetAIState() != EGhostAIState::Chasing ||
		GhostPawn->GetIsHitReacting() || GhostPawn->GetIsAttacking() ||
		!bCurrentlySeesTarget || !IsTargetUsable() ||
		GetWorldTimerManager().IsTimerActive(MoveRetryTimerHandle))
	{
		return;
	}
	const UPathFollowingComponent* PathFollowing = GetPathFollowingComponent();
	if (!PathFollowing || PathFollowing->GetStatus() != EPathFollowingStatus::Idle ||
		FVector::Dist2D(GhostPawn->GetActorLocation(), CurrentTarget->GetActorLocation()) <=
			GhostPawn->AttackRange)
	{
		return;
	}
	// Preserve the Week9 recovery after hit reaction, knockback or a failed move.
	IssueChaseMove();
}

void AWCGhostAIController::RetryChaseMove()
{
	if (GhostPawn && GhostPawn->GetAIState() == EGhostAIState::Chasing &&
		!GhostPawn->GetIsHitReacting() && !GhostPawn->GetIsAttacking() &&
		bCurrentlySeesTarget && IsTargetUsable())
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

void AWCGhostAIController::ClearCurrentTarget()
{
	CurrentTarget = nullptr;
	bCurrentlySeesTarget = false;
	ClearFocus(EAIFocusPriority::Gameplay);
}

bool AWCGhostAIController::SelectNearestPerceivedHostile()
{
	if (!GhostPawn || !SightPerception)
	{
		return false;
	}
	if (IsTargetUsable())
	{
		return true;
	}
	TArray<AActor*> PerceivedActors;
	SightPerception->GetCurrentlyPerceivedActors(
		UAISense_Sight::StaticClass(), PerceivedActors);
	AActor* NearestHostile = nullptr;
	float NearestDistanceSquared = TNumericLimits<float>::Max();
	for (AActor* Candidate : PerceivedActors)
	{
		if (!WCCombatant::AreHostile(GhostPawn, Candidate))
		{
			continue;
		}
		const float DistanceSquared = FVector::DistSquared2D(
			GhostPawn->GetActorLocation(), Candidate->GetActorLocation());
		if (DistanceSquared < NearestDistanceSquared)
		{
			NearestDistanceSquared = DistanceSquared;
			NearestHostile = Candidate;
		}
	}
	if (!NearestHostile)
	{
		return false;
	}
	EnterChase(NearestHostile);
	return true;
}

bool AWCGhostAIController::IsActorCurrentlyPerceived(const AActor* Actor) const
{
	if (!SightPerception || !IsValid(Actor))
	{
		return false;
	}
	TArray<AActor*> PerceivedActors;
	SightPerception->GetCurrentlyPerceivedActors(
		UAISense_Sight::StaticClass(), PerceivedActors);
	return PerceivedActors.Contains(Actor);
}

bool AWCGhostAIController::IsTargetUsable() const
{
	return GhostPawn && WCCombatant::AreHostile(GhostPawn, CurrentTarget);
}
