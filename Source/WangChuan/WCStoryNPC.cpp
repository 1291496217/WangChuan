// Fill out your copyright notice in the Description page of Project Settings.


#include "WCStoryNPC.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"

#include "StoryAnchor.h"
#include "WCCharacter.h"

// Sets default values
AWCStoryNPC::AWCStoryNPC()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// NPC Mesh
	NPCMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("NPCMesh"));
	NPCMesh->SetupAttachment(SceneRoot);

	// NPC 暂时不依赖 Mesh 碰撞进行交互
	NPCMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Interaction Sphere
	InteractionSphere =
		CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(SceneRoot);
	InteractionSphere->SetSphereRadius(300.0f);

	// 交互范围只用于 Query， 不产生物理阻挡。
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionSphere->SetGenerateOverlapEvents(true);

	// Bind overlap events
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(
		this,
		&AWCStoryNPC::OnPlayerEnter);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(
		this,
		&AWCStoryNPC::OnPlayerExit);

}

void AWCStoryNPC::BeginPlay()
{
	Super::BeginPlay();

	InitializeRelocationMaterials();
	SetRelocationFadeOpacity(1.0f);
}

void AWCStoryNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearRelocationTimers();

	Super::EndPlay(EndPlayReason);
}

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

FText AWCStoryNPC::GetNPCDisplayName() const
{
	return NPCDisplayName;
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

EStoryNPCState
AWCStoryNPC::GetStoryState() const
{
	return StoryState;
}

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
		GetWorldTimerManager().IsTimerActive(RelocationFadeTimerHandle) ||
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

	/*
	* 关闭交互和碰撞。
	*/
	SetStoryNPCInteractionEnabled(false);

	/*
	* Gameplay 状态立即进入 Relocating，但旧位置的视觉表现延迟开始。
	* 这样 Prompt 会立刻消失，玩家仍能观察 NPC 的离场效果。
	*/
	SetActorHiddenInGame(false);
	SetRelocationFadeOpacity(1.0f);

	if (RelocationStartDelay <= KINDA_SMALL_NUMBER)
	{
		BeginRelocationFade();
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			RelocationStartTimerHandle,
			this,
			&AWCStoryNPC::BeginRelocationFade,
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

void AWCStoryNPC::InitializeRelocationMaterials()
{
	RelocationMaterialInstances.Reset();
	bRelocationFadeSupported = false;

	if (!NPCMesh || RelocationFadeParameterName.IsNone())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("%s: relocation fade is unavailable because the mesh or parameter name is invalid."),
			*GetName()
		);
		return;
	}

	const int32 MaterialCount = NPCMesh->GetNumMaterials();
	if (MaterialCount <= 0)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("%s: relocation fade is unavailable because NPCMesh has no materials."),
			*GetName()
		);
		return;
	}

	/*
	* 先验证所有可见 Slot，再创建 MID，避免只有部分身体淡出。
	*/
	for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
	{
		UMaterialInterface* Material = NPCMesh->GetMaterial(MaterialIndex);
		float ParameterValue = 0.0f;
		const bool bHasFadeParameter =
			Material &&
			Material->GetScalarParameterValue(
				FHashedMaterialParameterInfo(RelocationFadeParameterName),
				ParameterValue
			);

		if (!bHasFadeParameter)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("%s: material slot %d does not expose scalar parameter '%s'. Relocation will use the safe instant-hide fallback."),
				*GetName(),
				MaterialIndex,
				*RelocationFadeParameterName.ToString()
			);
			return;
		}
	}

	for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
	{
		if (UMaterialInstanceDynamic* MaterialInstance =
			NPCMesh->CreateAndSetMaterialInstanceDynamic(MaterialIndex))
		{
			RelocationMaterialInstances.Add(MaterialInstance);
		}
	}

	bRelocationFadeSupported =
		RelocationMaterialInstances.Num() == MaterialCount;

	if (!bRelocationFadeSupported)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("%s: not all relocation material instances could be created. Relocation will use the safe instant-hide fallback."),
			*GetName()
		);
		RelocationMaterialInstances.Reset();
	}
}

void AWCStoryNPC::SetRelocationFadeOpacity(float NewOpacity)
{
	if (!bRelocationFadeSupported)
	{
		return;
	}

	const float ClampedOpacity = FMath::Clamp(NewOpacity, 0.0f, 1.0f);
	for (UMaterialInstanceDynamic* MaterialInstance : RelocationMaterialInstances)
	{
		if (IsValid(MaterialInstance))
		{
			MaterialInstance->SetScalarParameterValue(
				RelocationFadeParameterName,
				ClampedOpacity
			);
		}
	}
}

void AWCStoryNPC::BeginRelocationFade()
{
	GetWorldTimerManager().ClearTimer(RelocationStartTimerHandle);

	if (StoryState != EStoryNPCState::Relocating ||
		!IsValid(PendingRelocationAnchor))
	{
		AbortRelocation(TEXT("target anchor became invalid before the fade started"));
		return;
	}

	if (RelocationVFXSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			RelocationVFXSystem,
			GetActorLocation() + RelocationVFXOffset,
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
			TEXT("%s: RelocationVFXSystem is not configured. Fade and relocation will continue."),
			*GetName()
		);
	}

	if (RelocationFadeDuration <= KINDA_SMALL_NUMBER)
	{
		CompleteFadeAndTeleport();
		return;
	}

	if (!bRelocationFadeSupported)
	{
		GetWorldTimerManager().SetTimer(
			RelocationFadeTimerHandle,
			this,
			&AWCStoryNPC::CompleteFadeAndTeleport,
			RelocationFadeDuration,
			false
		);
		return;
	}

	RelocationFadeStartTime = GetWorld()->GetTimeSeconds();
	SetRelocationFadeOpacity(1.0f);

	GetWorldTimerManager().SetTimer(
		RelocationFadeTimerHandle,
		this,
		&AWCStoryNPC::UpdateRelocationFade,
		FMath::Max(RelocationFadeUpdateInterval, 0.01f),
		true
	);
}

void AWCStoryNPC::UpdateRelocationFade()
{
	if (StoryState != EStoryNPCState::Relocating)
	{
		ClearRelocationTimers();
		return;
	}

	const float ElapsedTime =
		GetWorld()->GetTimeSeconds() - RelocationFadeStartTime;
	const float LinearAlpha =
		FMath::Clamp(ElapsedTime / RelocationFadeDuration, 0.0f, 1.0f);
	const float SmoothAlpha =
		LinearAlpha * LinearAlpha * (3.0f - 2.0f * LinearAlpha);

	SetRelocationFadeOpacity(1.0f - SmoothAlpha);

	if (LinearAlpha >= 1.0f)
	{
		CompleteFadeAndTeleport();
	}
}

void AWCStoryNPC::CompleteFadeAndTeleport()
{
	GetWorldTimerManager().ClearTimer(RelocationFadeTimerHandle);

	if (StoryState != EStoryNPCState::Relocating ||
		!IsValid(PendingRelocationAnchor))
	{
		AbortRelocation(TEXT("target anchor became invalid before teleport"));
		return;
	}

	SetRelocationFadeOpacity(0.0f);
	SetActorHiddenInGame(true);

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

	/*
	* Actor 仍隐藏时恢复材质，确保新 Anchor reveal 时完全可见。
	*/
	SetRelocationFadeOpacity(1.0f);

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

	/*
	* 必须先恢复 Available，
	* 因为恢复 Collision 时可能立刻产生 BeginOverlap。
	*/
	StoryState = EStoryNPCState::Available;

	SetRelocationFadeOpacity(1.0f);
	SetActorHiddenInGame(false);

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
	SetRelocationFadeOpacity(1.0f);
	SetActorHiddenInGame(false);

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
	TimerManager.ClearTimer(RelocationFadeTimerHandle);
	TimerManager.ClearTimer(RelocationRevealTimerHandle);
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
