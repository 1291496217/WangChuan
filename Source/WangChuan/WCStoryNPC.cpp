// Fill out your copyright notice in the Description page of Project Settings.


#include "WCStoryNPC.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
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

	if (!GEngine)
	{
		return;
	}
	
	// 验证 Blueprint 中是否配置了当前阶段对话。
	if (CurrentDialogue.Lines.Num() == 0) {
		const FString WarningMessage =
			FString::Printf(
				TEXT(
					"%s\n"
					"Story Stage: %d\n"
					"No dialogue configured for this stage."
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

		return;
	}

	// Day1 临时将所有对话合并成一条 Debug Message
	FString DialogueDebugMessage =
		FString::Printf(
			TEXT(
				"%s\n"
				"Story Stage: %d\n"
			),
			*NPCDisplayName.ToString(),
			CurrentStoryStage
		);

	for (const FDialogueLine& DialogueLine : CurrentDialogue.Lines)
	{
		const FString Speaker =
			DialogueLine.SpeakerName.IsEmpty()
			? NPCDisplayName.ToString()
			: DialogueLine.SpeakerName.ToString();

		DialogueDebugMessage += FString::Printf(
			TEXT("%s: %s\n"),
			*Speaker,
			*DialogueLine.DialogueText.ToString()
		);
	}

	GEngine->AddOnScreenDebugMessage(
		-1,
		8.0f,
		FColor::Cyan,
		DialogueDebugMessage
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

