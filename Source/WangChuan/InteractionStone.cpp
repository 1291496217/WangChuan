#include "InteractionStone.h"
#include "WCCharacter.h"
#include "Engine/Engine.h"

// ******************** Construction ********************

AInteractionStone::AInteractionStone()
{
	PrimaryActorTick.bCanEverTick = false;

	StoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StoneMesh"));

	RootComponent = StoneMesh;

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(200.0f);

	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AInteractionStone::OnPlayerEnter);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AInteractionStone::OnPlayerExit);
}

// ******************** Lifecycle ********************

void AInteractionStone::BeginPlay()
{
	Super::BeginPlay();
}

// ******************** Interaction ********************

void AInteractionStone::Interact()
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
									 TEXT("There are some Memory here..."));
}

// ******************** Events ********************

void AInteractionStone::OnPlayerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
									  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
									  bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Enter the Interactive Range"));

	AWCCharacter* Player = Cast<AWCCharacter>(OtherActor);
	if (Player)
	{
		Player->CurrentInteractable = this;

		Player->ShowInteractionPrompt(GetInteractionPrompt());
	}
}

void AInteractionStone::OnPlayerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
									 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Exit the Interative Range"));
	AWCCharacter* Player = Cast<AWCCharacter>(OtherActor);

	if (Player)
	{
		Player->CurrentInteractable = nullptr;

		Player->HideInteractionPrompt();
	}
}

// ******************** Getters ********************

FString AInteractionStone::GetInteractionPrompt()
{
	return TEXT("[E] 调查");
}
