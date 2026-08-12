#include "WCProgressionGate.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

AWCProgressionGate::AWCProgressionGate()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	GateMesh->SetupAttachment(SceneRoot);
	GateMesh->SetCollisionProfileName(TEXT("BlockAll"));
	GateMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AWCProgressionGate::BeginPlay()
{
	Super::BeginPlay();
	bIsOpen = false;

	if (GateMesh)
	{
		ClosedRelativeLocation = GateMesh->GetRelativeLocation();
		GateMesh->SetRelativeLocation(ClosedRelativeLocation);
		GateMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void AWCProgressionGate::OpenGate()
{
	if (bIsOpen)
	{
		return;
	}

	bIsOpen = true;
	if (GateMesh)
	{
		GateMesh->SetRelativeLocation(ClosedRelativeLocation + OpenOffset);
		GateMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	BP_OnGateOpened();
}

bool AWCProgressionGate::IsGateOpen() const
{
	return bIsOpen;
}
