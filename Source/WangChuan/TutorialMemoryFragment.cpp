#include "TutorialMemoryFragment.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "WCCharacter.h"

ATutorialMemoryFragment::ATutorialMemoryFragment()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	FragmentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FragmentMesh"));
	FragmentMesh->SetupAttachment(SceneRoot);
	FragmentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FragmentMesh->SetRelativeScale3D(FVector(0.45f, 0.45f, 0.75f));
	FragmentMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		FragmentMesh->SetStaticMesh(SphereMesh.Object);
	}

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(SceneRoot);
	InteractionSphere->SetSphereRadius(200.0f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionSphere->SetGenerateOverlapEvents(true);

	InteractionSphere->OnComponentBeginOverlap.AddUniqueDynamic(
		this, &ATutorialMemoryFragment::OnPlayerEnter);
	InteractionSphere->OnComponentEndOverlap.AddUniqueDynamic(
		this, &ATutorialMemoryFragment::OnPlayerExit);
}

void ATutorialMemoryFragment::BeginPlay()
{
	Super::BeginPlay();
	SetFragmentAvailable(bStartAvailable);
}

void ATutorialMemoryFragment::Interact()
{
	if (!bIsAvailable || bCollected)
	{
		return;
	}

	AWCCharacter* Player = OverlappingPlayer.Get();
	if (!Player && GetWorld() && GetWorld()->GetFirstPlayerController())
	{
		Player = Cast<AWCCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
	}

	if (!Player)
	{
		return;
	}

	if (!Player->CollectTutorialFragment(FragmentID, DisplayTitle, DisplayText))
	{
		return;
	}

	bCollected = true;
	ClearPlayerInteractionIfNeeded();
	SetFragmentAvailable(false);
}

void ATutorialMemoryFragment::TriggerTutorialInteraction()
{
	Interact();
}

FString ATutorialMemoryFragment::GetInteractionPrompt()
{
	return bIsAvailable && !bCollected ? InteractionPrompt : FString();
}

void ATutorialMemoryFragment::SetFragmentAvailable(bool bNewAvailable)
{
	if (bCollected)
	{
		bNewAvailable = false;
	}

	bIsAvailable = bNewAvailable;
	SetActorHiddenInGame(!bIsAvailable);

	InteractionSphere->SetGenerateOverlapEvents(bIsAvailable);
	InteractionSphere->SetCollisionEnabled(
		bIsAvailable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);

	if (!bIsAvailable)
	{
		ClearPlayerInteractionIfNeeded();
		return;
	}

	RefreshInteractionForOverlappingPlayer();
}

FName ATutorialMemoryFragment::GetFragmentID() const
{
	return FragmentID;
}

bool ATutorialMemoryFragment::GetIsAvailable() const
{
	return bIsAvailable;
}

bool ATutorialMemoryFragment::GetIsCollected() const
{
	return bCollected;
}

bool ATutorialMemoryFragment::GetStartAvailable() const
{
	return bStartAvailable;
}

bool ATutorialMemoryFragment::GetIsCurrentInteractableForOverlappingPlayer() const
{
	const AWCCharacter* Player = OverlappingPlayer.Get();
	return Player && Player->CurrentInteractable == this;
}

void ATutorialMemoryFragment::OnPlayerEnter(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	AWCCharacter* Player = Cast<AWCCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	OverlappingPlayer = Player;
	RefreshInteractionForOverlappingPlayer();
}

void ATutorialMemoryFragment::OnPlayerExit(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	AWCCharacter* Player = Cast<AWCCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	if (Player->CurrentInteractable == this)
	{
		Player->CurrentInteractable = nullptr;
		Player->HideInteractionPrompt();
	}

	if (OverlappingPlayer.Get() == Player)
	{
		OverlappingPlayer.Reset();
	}
}

void ATutorialMemoryFragment::ClearPlayerInteractionIfNeeded()
{
	AWCCharacter* Player = OverlappingPlayer.Get();
	if (Player && Player->CurrentInteractable == this)
	{
		Player->CurrentInteractable = nullptr;
		Player->HideInteractionPrompt();
	}
}

void ATutorialMemoryFragment::RefreshInteractionForOverlappingPlayer()
{
	if (!bIsAvailable || bCollected)
	{
		return;
	}

	AWCCharacter* Player = OverlappingPlayer.Get();
	if (!Player)
	{
		TArray<AActor*> OverlappingActors;
		InteractionSphere->GetOverlappingActors(OverlappingActors, AWCCharacter::StaticClass());
		if (!OverlappingActors.IsEmpty())
		{
			Player = Cast<AWCCharacter>(OverlappingActors[0]);
			OverlappingPlayer = Player;
		}
	}

	if (!Player || !Player->CanReceiveTutorialInteraction())
	{
		return;
	}

	if (Player->CurrentInteractable && Player->CurrentInteractable != this)
	{
		return;
	}

	Player->CurrentInteractable = this;
	Player->ShowInteractionPrompt(GetInteractionPrompt());

	// Control teaching is owned by TutorialInstructionTrigger. The normal
	// world interaction prompt remains active after the instruction closes.
}
