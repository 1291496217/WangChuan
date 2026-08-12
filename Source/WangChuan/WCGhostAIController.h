#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "WCGhostAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class AGhostEnemy;
class AWCCharacter;

/**
 * Minimal Ghost controller. It owns sight, NavMesh requests, lost-sight handling and leash.
 * Health, attacks, hit reaction, death and objectives remain on AGhostEnemy.
 */
UCLASS()
class WANGCHUAN_API AWCGhostAIController : public AAIController
{
	GENERATED_BODY()

public:
	AWCGhostAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnMoveCompleted(FAIRequestID RequestID,
		const FPathFollowingResult& Result) override;

	AWCCharacter* GetTargetPlayer() const;
	void HandleDamageAggro(AWCCharacter* DamageInstigator);
	void PauseForHitReaction();
	void ResumeAfterAttack();
	void HandlePawnDeath();
	void BeginReturnHome();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	UAIPerceptionComponent* SightPerception;

private:
	UPROPERTY()
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY()
	AGhostEnemy* GhostPawn;

	UPROPERTY()
	AWCCharacter* TargetPlayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Debug",
		meta = (AllowPrivateAccess = "true"))
	FVector LastSeenPlayerLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Debug",
		meta = (AllowPrivateAccess = "true"))
	bool bCurrentlySeesPlayer = false;

	bool bLeashReturnLocked = false;
	bool bMoveFailureLogged = false;

	FTimerHandle LostSightTimerHandle;
	FTimerHandle InvestigationTimerHandle;
	FTimerHandle LeashHysteresisTimerHandle;
	FTimerHandle MoveRetryTimerHandle;

	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void ConfigureSightFromPawn();
	void EnterChase(AWCCharacter* PlayerCharacter);
	void BeginInvestigation(const FVector& LastSeenLocation);
	void FinishLostSightGrace();
	void FinishInvestigation();
	void FinishLeashHysteresis();
	void IssueChaseMove();
	void RetryChaseMove();
	void RetryReturnHome();
	void ClearAITimers();
	bool IsTargetUsable() const;
};
