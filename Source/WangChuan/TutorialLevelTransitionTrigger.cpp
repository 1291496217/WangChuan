#include "TutorialLevelTransitionTrigger.h"

#include "WCProgressionGate.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ATutorialLevelTransitionTrigger::ATutorialLevelTransitionTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TransitionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TransitionBox"));
	SetRootComponent(TransitionBox);
	TransitionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TransitionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TransitionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TransitionBox->SetGenerateOverlapEvents(true);
	TransitionBox->SetBoxExtent(FVector(180.0f, 180.0f, 150.0f));
}

void ATutorialLevelTransitionTrigger::BeginPlay()
{
	Super::BeginPlay();
	bTriggered = false;
	TransitionBox->OnComponentBeginOverlap.AddUniqueDynamic(
		this, &ATutorialLevelTransitionTrigger::HandleTransitionOverlap);

	if (TargetLevelName.IsNone())
	{
		UE_LOG(LogTemp, Error,
			TEXT("TutorialLevelTransitionTrigger '%s' has no TargetLevelName."), *GetName());
	}
	if (!IsValid(RequiredOpenGate))
	{
		UE_LOG(LogTemp, Error,
			TEXT("TutorialLevelTransitionTrigger '%s' requires an Exit Gate reference."),
			*GetName());
	}
}

void ATutorialLevelTransitionTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(OpenLevelTimerHandle);
	if (TransitionBox)
	{
		TransitionBox->OnComponentBeginOverlap.RemoveDynamic(
			this, &ATutorialLevelTransitionTrigger::HandleTransitionOverlap);
	}
	Super::EndPlay(EndPlayReason);
}

void ATutorialLevelTransitionTrigger::HandleTransitionOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bTriggered || TargetLevelName.IsNone() || !IsValid(RequiredOpenGate) ||
		!RequiredOpenGate->IsGateOpen())
	{
		return;
	}

	APawn* PlayerPawn = Cast<APawn>(OtherActor);
	APlayerController* PlayerController = PlayerPawn
		? Cast<APlayerController>(PlayerPawn->GetController())
		: nullptr;
	if (!IsValid(PlayerController))
	{
		return;
	}

	bTriggered = true;
	TransitioningPlayerController = PlayerController;
	TransitionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlayerPawn->DisableInput(PlayerController);

	if (PlayerController->PlayerCameraManager)
	{
		PlayerController->PlayerCameraManager->StartCameraFade(
			0.0f, 1.0f, FadeDuration, FLinearColor::Black, false, true);
	}

	if (FadeDuration <= 0.0f)
	{
		OpenTargetLevel();
		return;
	}

	GetWorldTimerManager().SetTimer(OpenLevelTimerHandle, this,
		&ATutorialLevelTransitionTrigger::OpenTargetLevel, FadeDuration, false);
}

void ATutorialLevelTransitionTrigger::OpenTargetLevel()
{
	GetWorldTimerManager().ClearTimer(OpenLevelTimerHandle);
	UGameplayStatics::OpenLevel(this, TargetLevelName);
}
