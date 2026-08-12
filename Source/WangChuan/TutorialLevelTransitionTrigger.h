#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialLevelTransitionTrigger.generated.h"

class APlayerController;
class AWCTutorialGate;
class UBoxComponent;

UCLASS()
class WANGCHUAN_API ATutorialLevelTransitionTrigger : public AActor
{
	GENERATED_BODY()

public:
	ATutorialLevelTransitionTrigger();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial Transition|Components")
	TObjectPtr<UBoxComponent> TransitionBox;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Tutorial Transition")
	TObjectPtr<AWCTutorialGate> RequiredOpenGate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Transition")
	FName TargetLevelName = TEXT("LandTemple_Prologue_Greybox");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Transition",
		meta = (ClampMin = "0.0"))
	float FadeDuration = 0.75f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
		Category = "Tutorial Transition")
	bool bTriggered = false;

	UFUNCTION()
	void HandleTransitionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

private:
	TWeakObjectPtr<APlayerController> TransitioningPlayerController;
	FTimerHandle OpenLevelTimerHandle;

	void OpenTargetLevel();
};
