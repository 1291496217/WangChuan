// Fill out your copyright notice in the Description page of Project Settings.


#include "WCStoryNPC.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
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

void AWCStoryNPC::Interact()
{
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
				5.0f,
				FColor::Red,
				WarningMessage
			);
		}
		return;
	}
	AWCCharacter* Player = Cast<AWCCharacter>(
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

	Player->StartDialogue(
		this,
		CurrentDialogue
	);
}

FString AWCStoryNPC::GetInteractionPrompt()
{
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

void AWCStoryNPC::OnPlayerEnter(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	AWCCharacter* Player =
		Cast<AWCCharacter>(OtherActor);

	if (!Player)
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

