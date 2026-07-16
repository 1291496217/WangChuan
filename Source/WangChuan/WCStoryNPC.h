// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "StoryTypes.h"
#include "WCStoryNPC.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class USphereComponent;
class UPrimitiveComponent;

/*
* 所有轻量叙事 NPC 的通用 C++ 基类
* 
* 负责：
* - 接入已有的 IInteractable 系统
* - 提供交互范围
* - 保存 NPC 名称，当前故事阶段， 当前行为状态， Blueprint 可配置的阶段对话。
* 
* 不负责：
* - 写死剧情，铃铛逻辑
* - 管理具体敌人, 完整任务系统
*/
UCLASS()
class WANGCHUAN_API AWCStoryNPC : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AWCStoryNPC();

	/*
	* IInteractable 接口
	*/
	virtual void Interact() override;

	virtual FString GetInteractionPrompt() override;

	/*
	* 提供给 Dialogue Widget 使用。
	*/
	UFUNCTION(BlueprintPure, Category = "Story NPC|Identity")
	FText GetNPCDisplayName() const;

	/*
	* 根据 CurrentStoryStage 返回当前阶段的对话。
	*
	* 当数组中不存在对应阶段时，返回一个空的Sequence。
	*/
	UFUNCTION(BlueprintPure, Category = "Story NPC|Dialogue")
	FDialogueSequence GetCurrentDialogueSequence() const;

protected:
	// Actor 根组件。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story NPC|Components")
	USceneComponent* SceneRoot;

	// NPC 骨骼模型。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story NPC|Components")
	USkeletalMeshComponent* NPCMesh;

	// 玩家进入后用于触发交互提示的范围。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story NPC|Components")
	USphereComponent* InteractionSphere;

	// NPC显示名称。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story NPC|Identity")
	FText NPCDisplayName;

	// 交互提示。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story NPC|Interaction")
	FString InteractionPrompt = TEXT("[E] Approach");

	// 当前故事阶段。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story NPC|State")
	int32 CurrentStoryStage = 0;

	// 当前通用行为状态。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story NPC|State")
	EStoryNPCState StoryState = EStoryNPCState::Available;

	// 数组下标与 CurrentStoryStage 对应。EX：DialogueByStage[0] = Stage 0 Dialogue.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story NPC|Dialogue")
	TArray<FDialogueSequence> DialogueByStage;

	// 玩家进入交互范围。
	UFUNCTION()
	void OnPlayerEnter(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// 玩家离开交互范围
	UFUNCTION()
	void OnPlayerExit(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);
};
