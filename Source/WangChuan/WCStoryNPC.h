// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "StoryTypes.h"
#include "WCStoryNPC.generated.h"

class AStoryAnchor;
class UNiagaraSystem;
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
* - 在 Story Anchors 之间进行轻量转移。
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

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

	UFUNCTION(BlueprintPure, Category = "Story NPC|Story")
	int32 GetCurrentStoryStage() const;

	UFUNCTION(BlueprintPure, Category = "Story NPC|Story")
	EStoryNPCState GetStoryState() const;

	UFUNCTION(BlueprintCallable, Category = "Story NPC|Story")
	void RecieveStoryEvent(FName EventID);

	UFUNCTION(BlueprintPure, Category = "Story NPC|Story")
	FName GetLastReceivedStoryEvent() const;

	/*
	* 将 NPC 转移到指定Story Anchor。
	* 
	* TargetAnchor： NPC 的目标位置与朝向。
	* 
	* NewStoryStage： NPC 重新出现后使用的故事阶段。
	*/
	UFUNCTION(BlueprintCallable, Category = "Story NPC|Relocation")
	bool RelocateToStoryAnchor(AStoryAnchor* TargetAnchor, int32 NewStoryStage);

	/*
	* 根据 StoryAnchors 数组下标进行转移。
	* 
	* 主要用于 Blueprint 配置与 Day3 测试。
	*/
	UFUNCTION(BlueprintCallable, Category = "Story NPC|Relocation")
	bool RelocateToStoryAnchorByIndex(int32 AnchorIndex, int32 NewStoryStage);

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story NPC|Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story NPC|Components")
	USkeletalMeshComponent* NPCMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story NPC|Components")
	USphereComponent* InteractionSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story NPC|Identity")
	FText NPCDisplayName;

	// Interaction
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story NPC|Interaction")
	FString InteractionPrompt = TEXT("[E] Approach");

	// Story
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story NPC|State")
	int32 CurrentStoryStage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story NPC|State")
	EStoryNPCState StoryState = EStoryNPCState::Available;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story NPC|Story")
	FName LastReceivedStoryEvent = NAME_None;

	// Dialogue
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story NPC|Dialogue")
	TArray<FDialogueSequence> DialogueByStage;

	// Relocation
	/*
	* 当前 NPC 可以使用的关卡 Anchor。
	*/
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "StoryNPC|Relocation")
	TArray<AStoryAnchor*> StoryAnchors;

	/*
	* NPC 移动到位置后，
	* 等待多久再重新显示。
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, 
		Category = "Story NPC|Relocation|Timing", meta = (ClampMin = "0.0"))
	float RelocationRevealDelay = 0.5f;

	/*
	* 进入 Relocating 后，在旧位置生成 VFX 前的反应时间。
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Story NPC|Relocation|Timing", meta = (ClampMin = "0.0"))
	float RelocationStartDelay = 0.35f;

	/*
	* VFX 生成后，NPC Mesh 在旧位置继续保持可见的观察时间。
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Story NPC|Relocation|Timing", meta = (ClampMin = "0.0"))
	float RelocationVFXObservationDuration = 0.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Story NPC|Relocation|VFX")
	TObjectPtr<UNiagaraSystem> RelocationVFXSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Story NPC|Relocation|VFX")
	FVector RelocationVFXOffset = FVector(0.0f, 0.0f, 80.0f);

	/*
	* Relocating 期间暂存下一 Story Stage。
	*/
	UPROPERTY()
	int32 PendingStoryStage = INDEX_NONE;

	UPROPERTY(Transient)
	TObjectPtr<AStoryAnchor> PendingRelocationAnchor;

	FTimerHandle RelocationStartTimerHandle;
	FTimerHandle RelocationVFXObservationTimerHandle;
	FTimerHandle RelocationRevealTimerHandle;

	FVector PendingRelocationVFXLocation = FVector::ZeroVector;
	EStoryNPCState StoryStateBeforeRelocation = EStoryNPCState::Available;
	bool bWarnedMissingRelocationVFX = false;

	/*
	* NPC Mesh 相对于 Actor Forward 的视觉朝向偏移。
	* 
	* 例如：
	* NPCMesh Relative Yaw = 150.
	* 这里则配置 150.
	* 
	* Relocation 时会自动抵消该角度。
	* 然最终模型视觉朝向与 Story Achor 箭头一致。
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story NPC|Relocation", 
		meta = (ClampMin = "-180.0", ClampMax = "180.0"))
	float MeshFacingYawOffset = 0.0f;

	// Overlap

	UFUNCTION()
	void OnPlayerEnter(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnPlayerExit(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	/*
	* Relocation 延迟结束后的恢复函数。
	*/
	void FinishRelocation();

	void BeginRelocationVFXObservation();
	void HideAndTeleport();
	void AbortRelocation(const TCHAR* Reason);
	void ClearRelocationTimers();
	void SetNPCVisible(bool bVisible);

	/*
	* 统一开启或关闭交互碰撞。
	*/
	void SetStoryNPCInteractionEnabled(
		bool bEnabled
	);

	/*
	* NPC 消失前清除玩家当前交互引用与 Prompt。
	*/
	void ClearPlayerInteractionIfNeeded();
};
