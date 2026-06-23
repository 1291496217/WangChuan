// Fill out your copyright notice in the Description page of Project Settings.


#include "MemoryFragment.h"
#include "WCCharacter.h"
#include "Engine/Engine.h"

// Sets default values
AMemoryFragment::AMemoryFragment()
{
	// create fragment mesh
	FragmentMesh = CreateDefaultSubobject<UStaticMeshComponent>(
		TEXT("FragmentMesh"));
	// create interaction sphere
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(
		TEXT("InteractionSphere"));

	RootComponent = FragmentMesh;

	InteractionSphere->SetupAttachment(
		RootComponent);

	InteractionSphere->SetSphereRadius(
		200.0f);

	InteractionSphere->OnComponentBeginOverlap.AddDynamic(
		this, &AMemoryFragment::OnPlayerEnter);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(
		this, &AMemoryFragment::OnPlayerExit);
}

// Called when the game starts or when spawned
void AMemoryFragment::BeginPlay()
{
	Super::BeginPlay();
}

void AMemoryFragment::Interact() 
{
	// get the player
	AWCCharacter* Player = Cast<AWCCharacter>(
		GetWorld()->GetFirstPlayerController()->GetPawn());
	// collect IDs
	if (Player)
	{
		Player->CollectedFragments.Add(
			FragmentID);
		if (Player->CollectedFragments.Num() >= 3) {
			GEngine->AddOnScreenDebugMessage(
				-1,
				30.0f,
				FColor::Green,
				TEXT(
					"《安静的孩子》\n\n"
					"校车侧翻后的很长一段时间里，\n"
					"她一直待在医院。\n\n"
					"同学们擦破皮，扭伤脚，哭闹着。\n"
					"她安慰着他们。\n"
					"<马上就会好起来啦> 她总是这么懂事，成熟。\n\n"
					"但，\n"
					"没人回应她。\n\n"
					"同学听不见她。\n"
					"医生看不见她。\n\n"
					"直到同学们一个个离开医院。\n\n"
					"妈妈....\n"
					"我今天也很乖。\n\n"
					"要快点来接我哦。"
				)
			);
		}
	}
	GEngine->AddOnScreenDebugMessage(
		-1,
		3.0f,
		FColor::Cyan,
		FragmentText
	);
	Destroy();
}

void AMemoryFragment::OnPlayerEnter(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	AWCCharacter* Player =
		Cast<AWCCharacter>(OtherActor);
	if (Player)
	{
		Player->CurrentInteractable = this;

		Player->ShowInteractionPrompt(
			GetInteractionPrompt());
	}
}

void AMemoryFragment::OnPlayerExit(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	AWCCharacter* Player =
		Cast<AWCCharacter>(OtherActor);

	if (Player)
	{
		Player->CurrentInteractable = nullptr;

		Player->HideInteractionPrompt();
	}
}

FString AMemoryFragment::GetInteractionPrompt(){
	return TEXT("[E] 收集记忆");
}

