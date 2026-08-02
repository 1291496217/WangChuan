#include "WCPlayerCheckpoint.h"

#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

#include "WCCharacter.h"
#include "WCStoryPersistenceCoordinator.h"

DEFINE_LOG_CATEGORY_STATIC(
	LogWCPlayerCheckpoint,
	Log,
	All
);

AWCPlayerCheckpoint::AWCPlayerCheckpoint()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot =
		CreateDefaultSubobject<USceneComponent>(
			TEXT("SceneRoot")
		);

	SetRootComponent(SceneRoot);

	CheckpointMesh =
		CreateDefaultSubobject<UStaticMeshComponent>(
			TEXT("CheckpointMesh")
		);

	CheckpointMesh->SetupAttachment(SceneRoot);

	/*
	* The visible rest-point mesh should not block the player.
	* If the final art needs collision, use a separate, deliberately
	* configured collision component rather than coupling it to the mesh.
	*/
	CheckpointMesh->SetCollisionEnabled(
		ECollisionEnabled::NoCollision
	);

	CheckpointMesh->SetCanEverAffectNavigation(false);

	/*
	* 原生 Actor 的可见后备模型。
	* BP_SoulRestPoint 指定自定义 Mesh 后会覆盖此默认值。
	*/
	static ConstructorHelpers::FObjectFinder<UStaticMesh>
		DefaultCheckpointMesh(
			TEXT("/Engine/BasicShapes/Cylinder.Cylinder")
		);

	if (DefaultCheckpointMesh.Succeeded())
	{
		CheckpointMesh->SetStaticMesh(
			DefaultCheckpointMesh.Object
		);
		CheckpointMesh->SetRelativeScale3D(
			FVector(0.65f, 0.65f, 1.25f)
		);
	}

	CheckpointLight =
		CreateDefaultSubobject<UPointLightComponent>(
			TEXT("CheckpointLight")
		);

	CheckpointLight->SetupAttachment(SceneRoot);
	CheckpointLight->SetIntensity(
		LockedLightIntensity
	);
	CheckpointLight->SetRelativeLocation(
		FVector(0.0f, 0.0f, 110.0f)
	);
	CheckpointLight->SetLightColor(
		FLinearColor(0.25f, 0.75f, 1.0f)
	);

	ActivationBox =
		CreateDefaultSubobject<UBoxComponent>(
			TEXT("ActivationBox")
		);

	ActivationBox->SetupAttachment(SceneRoot);

	ActivationBox->SetBoxExtent(
		FVector(125.0f, 125.0f, 120.0f)
	);

	ActivationBox->SetCollisionEnabled(
		ECollisionEnabled::QueryOnly
	);

	ActivationBox->SetCollisionResponseToAllChannels(
		ECR_Ignore
	);

	ActivationBox->SetCollisionResponseToChannel(
		ECC_Pawn,
		ECR_Overlap
	);

	ActivationBox->SetGenerateOverlapEvents(true);
	ActivationBox->SetCanEverAffectNavigation(false);

	ResumeArrow =
		CreateDefaultSubobject<UArrowComponent>(
			TEXT("ResumeArrow")
		);

	ResumeArrow->SetupAttachment(SceneRoot);
	ResumeArrow->SetRelativeLocation(
		FVector::ZeroVector
	);

	ResumeArrow->ArrowSize = 1.5f;
	ResumeArrow->ArrowColor = FColor::Green;
	ResumeArrow->SetHiddenInGame(true);
}

void AWCPlayerCheckpoint::BeginPlay()
{
	Super::BeginPlay();

	/*
	* Keep the full reflected member-function pointer contiguous.
	*
	* Do not split:
	* &AWCPlayerCheckpoint::HandleActivationBoxBeginOverlap
	* or:
	* &AWCPlayerCheckpoint::HandleActivationBoxEndOverlap
	*
	* between "::" and the function name. Dynamic-delegate macros
	* stringify the callback name for reflection.
	*/
	if (ActivationBox)
	{
		ActivationBox
			->OnComponentBeginOverlap
			.AddUniqueDynamic(
				this,
				&AWCPlayerCheckpoint::HandleActivationBoxBeginOverlap
			);

		ActivationBox
			->OnComponentEndOverlap
			.AddUniqueDynamic(
				this,
				&AWCPlayerCheckpoint::HandleActivationBoxEndOverlap
			);
	}

	if (CheckpointID.IsNone())
	{
		UE_LOG(
			LogWCPlayerCheckpoint,
			Error,
			TEXT(
				"Player Checkpoint Actor [%s] "
				"has CheckpointID None."
			),
			*GetName()
		);
	}

	CachedPersistenceCoordinator =
		Cast<AWCStoryPersistenceCoordinator>(
			UGameplayStatics::GetActorOfClass(
				this,
				AWCStoryPersistenceCoordinator::StaticClass()
			)
		);

	if (!IsValid(CachedPersistenceCoordinator))
	{
		UE_LOG(
			LogWCPlayerCheckpoint,
			Error,
			TEXT(
				"Checkpoint [%s] could not find "
				"AWCStoryPersistenceCoordinator."
			),
			*CheckpointID.ToString()
		);
	}

	/*
	* The Coordinator will reapply the authoritative unlocked
	* presentation after New Game initialization or SaveGame Restore.
	*/
	ApplyUnlockedPresentation(false);
}

FName AWCPlayerCheckpoint::GetCheckpointID() const
{
	return CheckpointID;
}

bool AWCPlayerCheckpoint::GetIsDefaultCheckpoint() const
{
	return bIsDefaultCheckpoint;
}

FText AWCPlayerCheckpoint::GetCheckpointDisplayName() const
{
	return CheckpointDisplayName;
}

bool AWCPlayerCheckpoint::GetIsUnlocked() const
{
	return bIsUnlocked;
}

int32 AWCPlayerCheckpoint::GetTravelOrder() const
{
	return TravelOrder;
}

bool AWCPlayerCheckpoint::BuildSafeResumeTransform(
	const AWCCharacter* Player,
	FTransform& OutResumeTransform
) const
{
	OutResumeTransform = FTransform::Identity;

	if (!IsValid(Player) ||
		!IsValid(ResumeArrow) ||
		!GetWorld())
	{
		UE_LOG(
			LogWCPlayerCheckpoint,
			Error,
			TEXT(
				"Checkpoint [%s] could not build a "
				"Resume Transform because Player, "
				"ResumeArrow, or World is invalid."
			),
			*CheckpointID.ToString()
		);

		return false;
	}

	const UCapsuleComponent* Capsule =
		Player->GetCapsuleComponent();

	if (!IsValid(Capsule))
	{
		UE_LOG(
			LogWCPlayerCheckpoint,
			Error,
			TEXT(
				"Checkpoint [%s] could not build a "
				"Resume Transform because the Player "
				"Capsule is invalid."
			),
			*CheckpointID.ToString()
		);

		return false;
	}

	const FVector ResumeLocation =
		ResumeArrow->GetComponentLocation();

	const FVector TraceStart =
		ResumeLocation +
		FVector::UpVector *
		GroundTraceUpDistance;

	const FVector TraceEnd =
		ResumeLocation -
		FVector::UpVector *
		GroundTraceDownDistance;

	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(
			WCCheckpointGroundTrace
		),
		false,
		this
	);

	QueryParams.AddIgnoredActor(Player);

	FHitResult GroundHit;

	const bool bFoundGround =
		GetWorld()->LineTraceSingleByChannel(
			GroundHit,
			TraceStart,
			TraceEnd,
			ECC_Visibility,
			QueryParams
		);

	if (!bFoundGround ||
		!GroundHit.bBlockingHit)
	{
		UE_LOG(
			LogWCPlayerCheckpoint,
			Error,
			TEXT(
				"Checkpoint [%s] could not find "
				"valid ground below ResumeArrow. "
				"TraceStart=%s, TraceEnd=%s."
			),
			*CheckpointID.ToString(),
			*TraceStart.ToCompactString(),
			*TraceEnd.ToCompactString()
		);

		return false;
	}

	const float CapsuleHalfHeight =
		Capsule->GetScaledCapsuleHalfHeight();

	const FVector TargetActorLocation =
		GroundHit.ImpactPoint +
		FVector::UpVector *
		(CapsuleHalfHeight +
			GroundClearance);

	FRotator TargetRotation =
		ResumeArrow->GetComponentRotation();

	/*
	* A Character resume point owns horizontal facing only.
	*/
	TargetRotation.Pitch = 0.0f;
	TargetRotation.Roll = 0.0f;
	TargetRotation.Normalize();

	OutResumeTransform = FTransform(
		TargetRotation,
		TargetActorLocation,
		FVector::OneVector
	);

	return true;
}

bool AWCPlayerCheckpoint::IsPlayerWithinInteractionRange(
	const AWCCharacter* Player
) const
{
	return IsValid(Player) &&
		IsValid(ActivationBox) &&
		ActivationBox->IsOverlappingActor(
			Player
		);
}

void AWCPlayerCheckpoint::Interact()
{
	AWCCharacter* Player =
		Cast<AWCCharacter>(
			UGameplayStatics::GetPlayerCharacter(
				this,
				0
			)
		);

	if (!IsValid(Player))
	{
		UE_LOG(
			LogWCPlayerCheckpoint,
			Warning,
			TEXT(
				"Checkpoint [%s] interaction rejected: "
				"Player 0 is not a valid AWCCharacter."
			),
			*CheckpointID.ToString()
		);

		return;
	}

	if (!Player->CanUseCheckpoint())
	{
		UE_LOG(
			LogWCPlayerCheckpoint,
			Display,
			TEXT(
				"Checkpoint [%s] interaction rejected: "
				"Player is not currently in a stable "
				"rest-point state."
			),
			*CheckpointID.ToString()
		);

		return;
	}

	if (!IsValid(CachedPersistenceCoordinator))
	{
		UE_LOG(
			LogWCPlayerCheckpoint,
			Error,
			TEXT(
				"Checkpoint [%s] interaction rejected: "
				"Persistence Coordinator is invalid."
			),
			*CheckpointID.ToString()
		);

		return;
	}

	if (!IsPlayerWithinInteractionRange(Player))
	{
		UE_LOG(
			LogWCPlayerCheckpoint,
			Warning,
			TEXT(
				"Checkpoint [%s] interaction rejected: "
				"Player is outside ActivationBox."
			),
			*CheckpointID.ToString()
		);

		return;
	}

	/*
	* Saving and unlocking are transactional inside the Coordinator.
	* This Actor does not write the SaveGame directly.
	*/
	const bool bFirstUnlock =
		!Player->HasUnlockedCheckpoint(
			CheckpointID
		);

	if (!CachedPersistenceCoordinator
		->SaveAtCheckpoint(this))
	{
		UE_LOG(
			LogWCPlayerCheckpoint,
			Warning,
			TEXT(
				"Checkpoint [%s] save request failed. "
				"No menu was opened."
			),
			*CheckpointID.ToString()
		);

		return;
	}

	/*
	* SaveAtCheckpoint normally refreshes all Checkpoint
	* presentations. Keep this as a safe fallback without
	* replaying the state event when already applied.
	*/
	if (!bIsUnlocked)
	{
		ApplyUnlockedPresentation(true);
	}

	OnCheckpointSaveSucceeded(
		bFirstUnlock
	);

	if (!Player->OpenCheckpointMenu(
		this,
		CachedPersistenceCoordinator))
	{
		/*
		* The save has already succeeded, so a UI failure must
		* not roll back persistent progress.
		*/
		UE_LOG(
			LogWCPlayerCheckpoint,
			Warning,
			TEXT(
				"Checkpoint [%s] saved successfully, "
				"but the Checkpoint Menu could not open."
			),
			*CheckpointID.ToString()
		);
	}
}

FString AWCPlayerCheckpoint::GetInteractionPrompt()
{
	return bIsUnlocked
		? UnlockedInteractionPrompt
		: LockedInteractionPrompt;
}

void AWCPlayerCheckpoint::ApplyUnlockedPresentation(
	bool bUnlocked
)
{
	bIsUnlocked = bUnlocked;

	if (CheckpointLight)
	{
		CheckpointLight->SetIntensity(
			bIsUnlocked
			? UnlockedLightIntensity
			: LockedLightIntensity
		);
	}

	/*
	* This Blueprint event should apply an idempotent state:
	* material, steady VFX, steady light, etc.
	*
	* One-shot activation feedback belongs in
	* OnCheckpointSaveSucceeded().
	*/
	OnCheckpointPresentationChanged(
		bIsUnlocked
	);

	RefreshPlayerInteractionIfOverlapping();

	UE_LOG(
		LogWCPlayerCheckpoint,
		Verbose,
		TEXT(
			"Checkpoint [%s] presentation applied. "
			"Unlocked=%s."
		),
		*CheckpointID.ToString(),
		bIsUnlocked
		? TEXT("True")
		: TEXT("False")
	);
}

void AWCPlayerCheckpoint::RefreshPlayerInteractionIfOverlapping()
{
	AWCCharacter* Player =
		Cast<AWCCharacter>(
			UGameplayStatics::GetPlayerCharacter(
				this,
				0
			)
		);

	if (!IsValid(Player) ||
		!IsPlayerWithinInteractionRange(Player))
	{
		return;
	}

	/*
	* Do not overwrite another interactable that currently owns
	* the player's prompt.
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

void AWCPlayerCheckpoint::HandleActivationBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	AWCCharacter* Player =
		Cast<AWCCharacter>(OtherActor);

	if (!IsValid(Player) ||
		Player->GetIsDead())
	{
		return;
	}

	/*
	* Overlap only establishes interaction ownership.
	*
	* It must not:
	* - unlock this Checkpoint;
	* - change CurrentCheckpointID;
	* - write a SaveGame;
	* - open the menu.
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

void AWCPlayerCheckpoint::HandleActivationBoxEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex
)
{
	AWCCharacter* Player =
		Cast<AWCCharacter>(OtherActor);

	if (!IsValid(Player))
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
