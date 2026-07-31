
#include "WCStoryNPC.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"

#include "StoryAnchor.h"
#include "WCCharacter.h"

// ******************** Construction ********************

AWCStoryNPC::AWCStoryNPC()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建根组件。
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// 创建 NPC 骨骼网格体。
	NPCMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("NPCMesh"));
	NPCMesh->SetupAttachment(SceneRoot);

	// NPC 暂时不依赖 Mesh 碰撞进行交互
	NPCMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 创建玩家交互范围。
	InteractionSphere =
		CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(SceneRoot);
	InteractionSphere->SetSphereRadius(300.0f);

	// 交互范围只用于 Query，不产生物理阻挡。
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionSphere->SetGenerateOverlapEvents(true);

	// 绑定进入和离开交互范围的事件。
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(
		this,
		&AWCStoryNPC::OnPlayerEnter);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(
		this,
		&AWCStoryNPC::OnPlayerExit);

}

// ******************** Lifecycle ********************

void AWCStoryNPC::BeginPlay()
{
	Super::BeginPlay();
}

void AWCStoryNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearRelocationTimers();

	Super::EndPlay(EndPlayReason);
}

// ******************** Interaction ********************

void AWCStoryNPC::Interact()
{
	/*
	* Relocating，Dormant 或其他不可交互状态下，
	* 不允许打开 Dialogue。
	*/
	if (StoryState != EStoryNPCState::Available)
	{
		return;
	}

	const FDialogueSequence CurrentDialogue = GetCurrentDialogueSequence();

	// 验证 Blueprint 中是否配置了当前阶段对话。
	if (CurrentDialogue.Lines.Num() == 0)
	{
		if (GEngine)
		{
			const FString WarningMessage =
				FString::Printf(
					TEXT(
						"%s: No dialogue configured "
						"for Story Stage %d."
					),
					*NPCDisplayName.ToString(),
					CurrentStoryStage
				);

			GEngine->AddOnScreenDebugMessage(
				-1,
				4.0f,
				FColor::Red,
				WarningMessage
			);
		}
		return;
	}
	AWCCharacter* Player = Cast<AWCCharacter>(
		UGameplayStatics::GetPlayerCharacter(this,0));

	if (!Player)
	{
		return;
	}

	if (Player->GetIsDead())
	{
		return;
	}

	Player->StartDialogue(
		this,
		CurrentDialogue
	);
}

FString AWCStoryNPC::GetInteractionPrompt()
{
	if (StoryState != EStoryNPCState::Available)
	{
		return FString();
	}

	return InteractionPrompt;
}

// ******************** Getters ********************

FText AWCStoryNPC::GetNPCDisplayName() const
{
	return NPCDisplayName;
}

FName AWCStoryNPC::GetStoryNPCID() const
{
	return StoryNPCID;
}

FDialogueSequence
AWCStoryNPC::GetCurrentDialogueSequence() const
{
	if (!DialogueByStage.IsValidIndex(CurrentStoryStage))
	{
		return FDialogueSequence();
	}

	return DialogueByStage[CurrentStoryStage];
}

int32 AWCStoryNPC::GetCurrentStoryStage() const
{
	return CurrentStoryStage;
}

FName AWCStoryNPC::GetCurrentStoryAnchorID() const
{
	if (!StoryAnchors.IsValidIndex(CurrentStoryStage))
	{
		return NAME_None;
	}

	const AStoryAnchor* CurrentAnchor =
		StoryAnchors[CurrentStoryStage];

	if (!IsValid(CurrentAnchor))
	{
		return NAME_None;
	}

	return CurrentAnchor->GetAnchorID();
}

EStoryNPCState
AWCStoryNPC::GetStoryState() const
{
	return StoryState;
}

// ******************** Relocation ********************

bool AWCStoryNPC::RelocateToStoryAnchor(
	AStoryAnchor* TargetAnchor,
	int32 NewStoryStage)
{
	if (!TargetAnchor)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				3.0f,
				FColor::Red,
				TEXT(
					"Story NPC relocation failed: "
					"TargetAnchor is null."
				)
			);
		}
		return false;
	}
	if (NewStoryStage < 0)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				3.0f,
				FColor::Red,
				TEXT(
					"Story NPC relocation failed: "
					"NewStoryStage cannot be negative."
				)
			);
		}
		return false;
	}
	/*
	* 防止同一个 NPC 在一次 Relocation 尚未完成时，
	* 再次启动新的 Relocation。
	*/
	if (StoryState == EStoryNPCState::Relocating ||
		GetWorldTimerManager().IsTimerActive(RelocationStartTimerHandle) ||
		GetWorldTimerManager().IsTimerActive(
			RelocationVFXObservationTimerHandle) ||
		GetWorldTimerManager().IsTimerActive(RelocationRevealTimerHandle))
	{
		return false;
	}

	AWCCharacter* Player =
		Cast<AWCCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

	/*
	* 不允许在 Dialogue 尚未关闭时移动 NPC。
	*/
	if (Player && Player->GetIsInDialogue())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Yellow,
				TEXT(
					"Close the dialogue before "
					"relocating the Story NPC."
				)
			);
		}
		return false;
	}

	/*
	* 缺少目标 Stage Dialogue 时不阻止移动，
	* 但给出配置警告。
	*/
	if (!DialogueByStage.IsValidIndex(NewStoryStage))
	{
		if (GEngine)
		{
			const FString WarningMessage =
				FString::Printf(
					TEXT(
						"Relocation warning: "
						"no dialogue configured "
						"for Story Stage %d."
					),
					NewStoryStage
				);

			GEngine->AddOnScreenDebugMessage(
				-1,
				4.0f,
				FColor::Yellow,
				WarningMessage
			);
		}
	}

	ClearRelocationTimers();

	/*
	* NPC 消失前清理旧位置的玩家交互引用。
	*/
	ClearPlayerInteractionIfNeeded();

	StoryStateBeforeRelocation = StoryState;
	StoryState = EStoryNPCState::Relocating;

	PendingStoryStage = NewStoryStage;
	PendingRelocationAnchor = TargetAnchor;
	PendingRelocationVFXLocation =
		GetActorLocation() + RelocationVFXOffset;

	/*
	* 关闭交互和碰撞。
	*/
	SetStoryNPCInteractionEnabled(false);

	/*
	* Gameplay 状态立即进入 Relocating，但旧位置的视觉表现延迟开始。
	* 这样 Prompt 会立刻消失，玩家仍能观察 NPC 的离场效果。
	*/
	SetNPCVisible(true);

	if (RelocationStartDelay <= KINDA_SMALL_NUMBER)
	{
		BeginRelocationVFXObservation();
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			RelocationStartTimerHandle,
			this,
			&AWCStoryNPC::BeginRelocationVFXObservation,
			RelocationStartDelay,
			false
		);
	}

	return true;
}

bool AWCStoryNPC::RelocateToStoryAnchorByIndex(
	int32 AnchorIndex, int32 NewStoryStage)
{
	if (!StoryAnchors.IsValidIndex(AnchorIndex))
	{
		if (GEngine)
		{
			const FString WarningMessage =
				FString::Printf(
					TEXT(
						"Story Anchor index %d "
						"is invalid."
					),
					AnchorIndex
				);

			GEngine->AddOnScreenDebugMessage(
				-1,
				3.0f,
				FColor::Red,
				WarningMessage
			);
		}
		return false;
	}

	AStoryAnchor* TargetAnchor = StoryAnchors[AnchorIndex];

	return RelocateToStoryAnchor(
		TargetAnchor, NewStoryStage
	);
}

void AWCStoryNPC::BeginRelocationVFXObservation()
{
	GetWorldTimerManager().ClearTimer(RelocationStartTimerHandle);

	if (StoryState != EStoryNPCState::Relocating ||
		!IsValid(PendingRelocationAnchor))
	{
		AbortRelocation(
			TEXT("target anchor became invalid before VFX observation"));
		return;
	}

	if (RelocationVFXSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			RelocationVFXSystem,
			PendingRelocationVFXLocation,
			FRotator::ZeroRotator,
			FVector::OneVector,
			true,
			true,
			ENCPoolMethod::None,
			true
		);
	}
	else if (!bWarnedMissingRelocationVFX)
	{
		bWarnedMissingRelocationVFX = true;
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"%s: RelocationVFXSystem is not configured. "
				"Relocation will continue."
			),
			*GetName()
		);
	}

	if (RelocationVFXObservationDuration <= KINDA_SMALL_NUMBER)
	{
		HideAndTeleport();
		return;
	}

	GetWorldTimerManager().SetTimer(
		RelocationVFXObservationTimerHandle,
		this,
		&AWCStoryNPC::HideAndTeleport,
		RelocationVFXObservationDuration,
		false
	);
}

void AWCStoryNPC::HideAndTeleport()
{
	GetWorldTimerManager().ClearTimer(
		RelocationVFXObservationTimerHandle);

	if (StoryState != EStoryNPCState::Relocating ||
		!IsValid(PendingRelocationAnchor))
	{
		AbortRelocation(TEXT("target anchor became invalid before teleport"));
		return;
	}

	SetNPCVisible(false);

	const FTransform TargetTransform =
		PendingRelocationAnchor->GetAnchorTransform();

	/*
	* Story Anchor 的 Rotation 表示最终玩家应该看到的
	* NPC 视觉朝向。抵消 Mesh 导入方向的 Relative Yaw。
	*/
	FRotator TargetActorRotation = TargetTransform.Rotator();
	TargetActorRotation.Yaw =
		FRotator::NormalizeAxis(
			TargetActorRotation.Yaw - MeshFacingYawOffset
		);

	SetActorLocationAndRotation(
		TargetTransform.GetLocation(),
		TargetActorRotation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);

	if (RelocationRevealDelay <= KINDA_SMALL_NUMBER)
	{
		FinishRelocation();
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			RelocationRevealTimerHandle,
			this,
			&AWCStoryNPC::FinishRelocation,
			RelocationRevealDelay,
			false
		);
	}
}

void AWCStoryNPC::FinishRelocation()
{
	if (StoryState != EStoryNPCState::Relocating)
	{
		ClearRelocationTimers();
		return;
	}

	if (PendingStoryStage != INDEX_NONE)
	{
		CurrentStoryStage =
			PendingStoryStage;
	}

	PendingStoryStage = INDEX_NONE;
	PendingRelocationAnchor = nullptr;
	PendingRelocationVFXLocation = FVector::ZeroVector;

	/*
	* 必须先恢复 Available，
	* 因为恢复 Collision 时可能立刻产生 BeginOverlap。
	*/
	StoryState = EStoryNPCState::Available;

	SetNPCVisible(true);

	SetStoryNPCInteractionEnabled(true);

	ClearRelocationTimers();
}

void AWCStoryNPC::AbortRelocation(const TCHAR* Reason)
{
	UE_LOG(
		LogTemp,
		Error,
		TEXT("%s: Story NPC relocation aborted because %s."),
		*GetName(),
		Reason
	);

	ClearRelocationTimers();
	PendingStoryStage = INDEX_NONE;
	PendingRelocationAnchor = nullptr;
	PendingRelocationVFXLocation = FVector::ZeroVector;
	SetNPCVisible(true);

	StoryState = StoryStateBeforeRelocation;
	SetStoryNPCInteractionEnabled(
		StoryState == EStoryNPCState::Available
	);
}

void AWCStoryNPC::ClearRelocationTimers()
{
	if (!GetWorld())
	{
		return;
	}

	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.ClearTimer(RelocationStartTimerHandle);
	TimerManager.ClearTimer(RelocationVFXObservationTimerHandle);
	TimerManager.ClearTimer(RelocationRevealTimerHandle);
}

void AWCStoryNPC::SetNPCVisible(bool bVisible)
{
	if (!NPCMesh)
	{
		return;
	}

	NPCMesh->SetVisibility(bVisible, true);
	NPCMesh->SetHiddenInGame(!bVisible, true);
}

void AWCStoryNPC::SetStoryNPCInteractionEnabled(bool bEnabled)
{
	SetActorEnableCollision(bEnabled);

	if (!InteractionSphere)
	{
		return;
	}

	if (bEnabled)
	{
		InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		InteractionSphere->SetGenerateOverlapEvents(true);

		/*
		* 如果玩家恰好站在目标 Anchor 附近，
		* 立即重新计算 Overlap。
		*/
		InteractionSphere->UpdateOverlaps();
	}
	else
	{
		InteractionSphere->SetGenerateOverlapEvents(false);

		InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AWCStoryNPC::ClearPlayerInteractionIfNeeded()
{
	AWCCharacter* Player =
		Cast<AWCCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

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

// ******************** Events ********************

void AWCStoryNPC::OnPlayerEnter(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	/*
	* 只有 Available 状态可以被玩家识别。
	*/
	if (StoryState != EStoryNPCState::Available)
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

	Player->ShowInteractionPrompt(GetInteractionPrompt());
}

void AWCStoryNPC::OnPlayerExit(
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

	/*
	* 只有当玩家当前记录的对象确实是这个 NPC 时，
	* 才会清除 CurrentInteractable。
	*
	* 避免多个交互范围重叠时，
	* 玩家离开其中一个范围却错误清除另一个对象。
	*/
	if (Player->CurrentInteractable == this)
	{
		Player->CurrentInteractable = nullptr;
		Player->HideInteractionPrompt();
	}
}

// ******************** Story Events ********************

void AWCStoryNPC::RecieveStoryEvent(FName EventID)
{
	if (EventID.IsNone())
	{
		return;
	}

	LastReceivedStoryEvent = EventID;

	if (StoryState !=
		EStoryNPCState::Relocating &&
		StoryState !=
		EStoryNPCState::ChapterComplete)
	{
		StoryState =
			EStoryNPCState::EventResolved;
	}

	if (GEngine)
	{
		const FString Message =
			FString::Printf(
				TEXT(
					"%s received Story Event: %s"
				),
				*NPCDisplayName.ToString(),
				*EventID.ToString()
			);

		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Cyan,
			Message
		);
	}
}

FName AWCStoryNPC::GetLastReceivedStoryEvent() const
{
	return LastReceivedStoryEvent;
}

bool AWCStoryNPC::ApplySavedStoryState(
	int32 SavedStoryStage,
	FName SavedAnchorID)
{
	if (SavedStoryStage < 0)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Story NPC [%s] restore failed: "
				"Story Stage cannot be negative."
			),
			*GetStoryNPCID().ToString()
		);

		return false;
	}

	if (SavedAnchorID.IsNone())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Story NPC [%s] restore failed: "
				"Anchor ID is None."
			),
			*GetStoryNPCID().ToString()
		);

		return false;
	}

	AStoryAnchor* TargetAnchor =
		FindStoryAnchorByID(
			SavedAnchorID
		);

	if (!IsValid(TargetAnchor))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Story NPC [%s] restore failed: "
				"Anchor [%s] was not found."
			),
			*GetStoryNPCID().ToString(),
			*SavedAnchorID.ToString()
		);

		return false;
	}

	AWCCharacter* Player =
		Cast<AWCCharacter>(
			UGameplayStatics::GetPlayerCharacter(
				this,
				0
			)
		);

	/*
	* Day6 会在正式 Load 前关闭或拒绝 Modal 状态。
	*
	* 当前接口不能在对话中直接移动对话来源。
	*/
	if (Player &&
		Player->GetIsInDialogue())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"Story NPC [%s] restore rejected "
				"because Dialogue is active."
			),
			*GetStoryNPCID().ToString()
		);

		return false;
	}

	/*
	* 所有验证先完成，再修改 World。
	*/
	ClearRelocationTimers();

	ClearPlayerInteractionIfNeeded();

	PendingStoryStage = INDEX_NONE;
	PendingRelocationAnchor = nullptr;
	PendingRelocationVFXLocation =
		FVector::ZeroVector;

	SetStoryNPCInteractionEnabled(false);

	const FTransform TargetTransform =
		TargetAnchor->GetAnchorTransform();

	FRotator TargetActorRotation =
		TargetTransform.Rotator();

	TargetActorRotation.Yaw =
		FRotator::NormalizeAxis(
			TargetActorRotation.Yaw -
			MeshFacingYawOffset
		);

	SetActorLocationAndRotation(
		TargetTransform.GetLocation(),
		TargetActorRotation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);

	CurrentStoryStage =
		SavedStoryStage;

	StoryState =
		EStoryNPCState::Available;

	StoryStateBeforeRelocation =
		EStoryNPCState::Available;

	/*
	* LastReceivedStoryEvent 是 Runtime Debug 状态，
	* 不属于保存事实。
	*/
	LastReceivedStoryEvent =
		NAME_None;

	SetNPCVisible(true);

	SetStoryNPCInteractionEnabled(true);

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Story NPC [%s] silently restored: "
			"Stage=%d, Anchor=[%s]."
		),
		*GetStoryNPCID().ToString(),
		CurrentStoryStage,
		*SavedAnchorID.ToString()
	);

	return true;
}

AStoryAnchor*
AWCStoryNPC::FindStoryAnchorByID(
	FName AnchorID) const
{
	if (AnchorID.IsNone())
	{
		return nullptr;
	}

	for (AStoryAnchor* StoryAnchor :
		StoryAnchors)
	{
		if (!IsValid(StoryAnchor))
		{
			continue;
		}

		if (StoryAnchor->GetAnchorID() ==
			AnchorID)
		{
			return StoryAnchor;
		}
	}

	return nullptr;
}
