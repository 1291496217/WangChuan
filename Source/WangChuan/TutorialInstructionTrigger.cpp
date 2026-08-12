#include "TutorialInstructionTrigger.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "WCCharacter.h"

ATutorialInstructionTrigger::ATutorialInstructionTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(SceneRoot);
	TriggerBox->SetBoxExtent(FVector(250.0f, 250.0f, 150.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->OnComponentBeginOverlap.AddUniqueDynamic(
		this, &ATutorialInstructionTrigger::OnTriggerEntered);
}

void ATutorialInstructionTrigger::OnTriggerEntered(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	AWCCharacter* Player = Cast<AWCCharacter>(OtherActor);
	if (!Player || !Player->IsLocallyControlled())
	{
		return;
	}

	if (Player->ShowTutorialInstruction(
		InstructionID, InstructionTitle, InstructionBody) && bTriggerOnce)
	{
		TriggerBox->SetGenerateOverlapEvents(false);
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
