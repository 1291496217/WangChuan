#include "EchoRelic.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

#include "WCCharacter.h"

// ******************** Construction ********************

AEchoRelic::AEchoRelic()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot =
		CreateDefaultSubobject<USceneComponent>(
			TEXT("SceneRoot")
		);
	SetRootComponent(SceneRoot);

	RelicMesh =
		CreateDefaultSubobject<UStaticMeshComponent>(
			TEXT("RelicMesh")
		);
	RelicMesh->SetupAttachment(SceneRoot);

	/*
	* 第一版本由 InteractionSphere 负责交互。
	* 避免临时铃铛 Mesh 阻挡玩家。
	*/
	RelicMesh->SetCollisionEnabled(
		ECollisionEnabled::NoCollision
	);

	InteractionSphere =
		CreateDefaultSubobject<USphereComponent>(
			TEXT("InteractionSphere")
		);

	InteractionSphere->SetupAttachment(SceneRoot);

	InteractionSphere->SetSphereRadius(170.0f);

	InteractionSphere->SetCollisionEnabled(
		ECollisionEnabled::QueryOnly
	);

	InteractionSphere
		->SetCollisionResponseToAllChannels(
			ECR_Ignore
		);

	InteractionSphere
		->SetCollisionResponseToChannel(
			ECC_Pawn,
			ECR_Overlap
		);

	InteractionSphere
		->SetGenerateOverlapEvents(true);

	InteractionSphere
		->OnComponentBeginOverlap.AddDynamic(
			this,
			&AEchoRelic::OnPlayerEnter
		);

	InteractionSphere
		->OnComponentEndOverlap.AddDynamic(
			this,
			&AEchoRelic::OnPlayerExit
		);
}

// ******************** Interaction ********************

void AEchoRelic::Interact()
{
	if (RelicState ==
		EEchoRelicState::Activated)
	{
		return;
	}

	if (bActivationInProgress)
	{
		return;
	}

	/*
	* Locked 状态允许玩家调查，
	* 但不会显示 Memory Echo。
	*/
	if (RelicState ==
		EEchoRelicState::locked)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				3.0f,
				FColor::Silver,
				LockedInteractionText.ToString()
			);
		}
		return;
	}

	if (RelicState !=
		EEchoRelicState::Available)
	{
		return;
	}

	AWCCharacter* Player =
		Cast<AWCCharacter>(
			UGameplayStatics::GetPlayerCharacter(
				this,
				0
			)
		);

	if (!Player)
	{
		return;
	}

	if (Player->GetIsDead())
	{
		return;
	}

	const bool bStarted =
		Player->StartMemoryEcho(
			MemoryEchoData,
			this
		);

	if (!bStarted)
	{
		return;
	}

	bActivationInProgress = true;
}

FString AEchoRelic::GetInteractionPrompt()
{
	switch (RelicState)
	{
	case EEchoRelicState::locked:
		return LockedInteractionPrompt;

	case EEchoRelicState::Available:
		return AvailableInteractionPrompt;

	case EEchoRelicState::Activated:
	default:
		return FString();
	}
}

bool AEchoRelic::UnlockRelic()
{
	if (RelicState !=
		EEchoRelicState::locked)
	{
		return false;
	}

	RelicState =
		EEchoRelicState::Available;

	/*
	* 玩家如果已经站在铃铛附近，
	* Prompt 需要从 Examine 更新为 Listen。
	*/
	RefreshPromptForOverlappingPlayer();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Cyan,
			TEXT(
				"Echo Relic Unlocked: "
				"Memory Echo is now available."
			)
		);
	}

	return true;
}

void AEchoRelic::ConfirmEchoRead()
{
	if (RelicState !=
		EEchoRelicState::Available)
	{
		return;
	}

	if (!bActivationInProgress)
	{
		return;
	}

	bActivationInProgress = false;

	RelicState =
		EEchoRelicState::Activated;

	ClearPlayerInteractionIfNeeded();

	DisableRelicInteraction();

	OnEchoActivated.Broadcast(this);
}

void AEchoRelic::CancelEchoRead()
{
	if (RelicState !=
		EEchoRelicState::Available)
	{
		return;
	}

	bActivationInProgress = false;

	RefreshPromptForOverlappingPlayer();
}

EEchoRelicState
// ******************** Getters ********************

AEchoRelic::GetRelicState() const
{
	return RelicState;
}

FMemoryEchoData
AEchoRelic::GetMemoryEchoData() const
{
	return MemoryEchoData;
}

FName AEchoRelic::GetEchoID() const
{
	return MemoryEchoData.EchoID;
}

// ******************** Events ********************

void AEchoRelic::OnPlayerEnter(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (RelicState ==
		EEchoRelicState::Activated)
	{
		return;
	}

	AWCCharacter* Player =
		Cast<AWCCharacter>(OtherActor);

	if (!Player)
	{
		return;
	}

	if (Player->GetIsDead())
	{
		return;
	}

	Player->CurrentInteractable = this;

	Player->ShowInteractionPrompt(
		GetInteractionPrompt()
	);
}

void AEchoRelic::OnPlayerExit(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	AWCCharacter* Player =
		Cast<AWCCharacter>(OtherActor);

	if (!Player)
	{
		return;
	}

	if (Player->CurrentInteractable ==
		this)
	{
		Player->CurrentInteractable =
			nullptr;

		Player->HideInteractionPrompt();
	}
}

// ******************** Helpers ********************

void AEchoRelic::ClearPlayerInteractionIfNeeded()
{
	AWCCharacter* Player =
		Cast<AWCCharacter>(
			UGameplayStatics::GetPlayerCharacter(
				this,
				0
			)
		);
	if (!Player)
	{
		return;
	}

	if (Player->CurrentInteractable != this)
	{
		return;
	}

	Player->CurrentInteractable = nullptr;

	Player->HideInteractionPrompt();
}

void AEchoRelic::DisableRelicInteraction()
{
	if (!InteractionSphere)
	{
		return;
	}

	InteractionSphere
		->SetGenerateOverlapEvents(false);

	InteractionSphere
		->SetCollisionEnabled(
			ECollisionEnabled::NoCollision
		);
}

void AEchoRelic::RefreshPromptForOverlappingPlayer()
{
	if (!InteractionSphere)
	{
		return;
	}

	AWCCharacter* Player =
		Cast<AWCCharacter>(
			UGameplayStatics::GetPlayerCharacter(
				this,
				0
			)
		);

	if (!Player)
	{
		return;
	}

	if (!InteractionSphere
		->IsOverlappingActor(Player))
	{
		return;
	}

	/*
	* 不覆盖另一个更近的 Interactable。
	*/
	if (Player->CurrentInteractable &&
		Player->CurrentInteractable != this)
	{
		return;
	}

	Player->CurrentInteractable = this;

	Player->ShowInteractionPrompt(
		GetInteractionPrompt()
	);
}

