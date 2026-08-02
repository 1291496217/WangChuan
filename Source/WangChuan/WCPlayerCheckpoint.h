#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"

#include "WCPlayerCheckpoint.generated.h"

class AWCCharacter;
class AWCStoryPersistenceCoordinator;

class UArrowComponent;
class UBoxComponent;
class UPointLightComponent;
class UPrimitiveComponent;
class USceneComponent;
class UStaticMeshComponent;


/*
* 单个休憩点在传送菜单中的运行时显示数据。
*
* 该结构不进入 SaveGame。
*
* Persistent Source of Truth:
* - UWCGameSaveGame::UnlockedCheckpointIDs
* - UWCGameSaveGame::CurrentCheckpointID
*
* 本结构只负责将当前运行时状态整理给 UI。
*/
USTRUCT(BlueprintType)
struct FWCCheckpointTravelOption
{
	GENERATED_BODY()

	UPROPERTY(
		BlueprintReadOnly,
		Category = "Checkpoint Travel"
	)
	FName CheckpointID = NAME_None;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "Checkpoint Travel"
	)
	FText DisplayName;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "Checkpoint Travel"
	)
	bool bUnlocked = false;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "Checkpoint Travel"
	)
	bool bCurrent = false;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "Checkpoint Travel"
	)
	int32 TravelOrder = 0;
};


/*
* 玩家可见、可交互的休憩点 / 存档点。
*
* 当前设计职责：
* - 提供稳定 CheckpointID
* - 提供玩家可见的 Mesh、Light 与 Blueprint Presentation
* - 在玩家进入范围时提供 Interaction Prompt
* - 玩家按 E 后向 Persistence Coordinator 请求完整保存
* - 为 Resume / Fast Travel 提供安全 Transform
* - 根据 Runtime Unlocked 状态更新视觉表现
*
* 不负责：
* - 直接调用 SaveGameToSlot
* - 自动保存
* - 在 Overlap 时直接解锁
* - 捕获 Story World
* - 恢复 Story World
* - 复活敌人或重置谜题
*/
UCLASS()
class WANGCHUAN_API AWCPlayerCheckpoint
	: public AActor,
	public IInteractable
{
	GENERATED_BODY()

public:
	AWCPlayerCheckpoint();

	// ---------------------------------------------------------------------
	// IInteractable
	// ---------------------------------------------------------------------

	/*
	* 玩家主动按下 Interact 时调用。
	*
	* 正常流程：
	* Checkpoint Interact
	* → Coordinator SaveAtCheckpoint
	* → 保存成功
	* → 更新已解锁表现
	* → 打开 Checkpoint Menu
	*/
	virtual void Interact() override;

	/*
	* 根据当前是否已经解锁，返回不同交互提示。
	*/
	virtual FString GetInteractionPrompt() override;


	// ---------------------------------------------------------------------
	// Identity
	// ---------------------------------------------------------------------

	UFUNCTION(
		BlueprintPure,
		Category = "Checkpoint|Identity"
	)
	FName GetCheckpointID() const;

	UFUNCTION(
		BlueprintPure,
		Category = "Checkpoint|Identity"
	)
	bool GetIsDefaultCheckpoint() const;


	// ---------------------------------------------------------------------
	// Display / Travel Data
	// ---------------------------------------------------------------------

	UFUNCTION(
		BlueprintPure,
		Category = "Checkpoint|Display"
	)
	FText GetCheckpointDisplayName() const;

	UFUNCTION(
		BlueprintPure,
		Category = "Checkpoint|State"
	)
	bool GetIsUnlocked() const;

	UFUNCTION(
		BlueprintPure,
		Category = "Checkpoint|Travel"
	)
	int32 GetTravelOrder() const;


	// ---------------------------------------------------------------------
	// Interaction Range
	// ---------------------------------------------------------------------

	/*
	* 检查指定玩家当前是否仍在此休憩点的交互范围内。
	*
	* Coordinator 在执行 SaveAtCheckpoint 前会再次验证，
	* 防止玩家已经离开范围但仍通过过期引用发起保存。
	*/
	UFUNCTION(
		BlueprintPure,
		Category = "Checkpoint|Interaction"
	)
	bool IsPlayerWithinInteractionRange(
		const AWCCharacter* Player
	) const;

	/*
	* 玩家在传送后可能直接出现在目标 Checkpoint 的
	* ActivationBox 内。
	*
	* 此函数重新建立 CurrentInteractable 和 Prompt，
	* 但不会保存、解锁或自动打开菜单。
	*/
	UFUNCTION(
		BlueprintCallable,
		Category = "Checkpoint|Interaction"
	)
	void RefreshPlayerInteractionIfOverlapping();


	// ---------------------------------------------------------------------
	// Safe Resume Transform
	// ---------------------------------------------------------------------

	/*
	* 根据 ResumeArrow 和玩家 Capsule，
	* 构建经过地面校准的安全 Character Transform。
	*
	* ResumeArrow 的位置表示玩家脚底附近的位置，
	* 而不是 Character Actor 的 Capsule 中心。
	*
	* 流程：
	* ResumeArrow
	* → 上下 Ground Trace
	* → 地面 Impact Point
	* → 加上 Capsule Half Height
	* → 保留 Arrow Yaw
	* → 清除 Pitch / Roll
	*/
	bool BuildSafeResumeTransform(
		const AWCCharacter* Player,
		FTransform& OutResumeTransform
	) const;


	// ---------------------------------------------------------------------
	// Runtime Presentation
	// ---------------------------------------------------------------------

	/*
	* 静默应用当前休憩点的已解锁表现。
	*
	* 该函数不会：
	* - 修改 Player 的 UnlockedCheckpointIDs
	* - 写入 SaveGame
	* - 打开 UI
	* - 播放一次性保存逻辑
	*
	* Coordinator 会根据 Runtime / SaveGame 中的
	* UnlockedCheckpointIDs 调用本函数。
	*/
	UFUNCTION(
		BlueprintCallable,
		Category = "Checkpoint|Presentation"
	)
	void ApplyUnlockedPresentation(
		bool bUnlocked
	);


protected:
	virtual void BeginPlay() override;


	// ---------------------------------------------------------------------
	// Components
	// ---------------------------------------------------------------------

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Components"
	)
	TObjectPtr<USceneComponent> SceneRoot;

	/*
	* 休憩点的可见主体。
	*
	* 具体 Static Mesh、材质和 Low-Poly 美术表现
	* 由 BP_SoulRestPoint 配置。
	*/
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Components"
	)
	TObjectPtr<UStaticMeshComponent> CheckpointMesh;

	/*
	* 用于显示休憩点已解锁或未解锁状态。
	*
	* C++ 负责基础 Intensity，
	* Blueprint 可以进一步切换材质、Niagara 或颜色。
	*/
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Components"
	)
	TObjectPtr<UPointLightComponent> CheckpointLight;

	/*
	* 玩家靠近后进入交互范围。
	*
	* Overlap 只负责：
	* - 设置 CurrentInteractable
	* - 显示 Prompt
	*
	* 不负责自动激活或自动保存。
	*/
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Components"
	)
	TObjectPtr<UBoxComponent> ActivationBox;

	/*
	* 表示玩家 Resume / Travel 后的脚底位置和面对方向。
	*
	* Pitch 和 Roll 不会应用给 Character。
	*/
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Components"
	)
	TObjectPtr<UArrowComponent> ResumeArrow;


	// ---------------------------------------------------------------------
	// Stable Identity
	// ---------------------------------------------------------------------

	/*
	* 永久稳定的语义 ID。
	*
	* 一旦进入正式 SaveGame，不应随意重命名。
	*
	* 示例：
	* WangChuan.Checkpoint.Start
	* WangChuan.Checkpoint.AfterEncounter01
	* WangChuan.Checkpoint.AfterEncounter02
	*/
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Identity"
	)
	FName CheckpointID = NAME_None;

	/*
	* 当前地图必须且只能有一个 Default Checkpoint。
	*
	* 无存档的新游戏中：
	* - 玩家仍从 PlayerStart 出现
	* - Coordinator 将此 Checkpoint 初始化为 Runtime Current
	* - 不自动创建磁盘存档
	*/
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Identity"
	)
	bool bIsDefaultCheckpoint = false;


	// ---------------------------------------------------------------------
	// Display
	// ---------------------------------------------------------------------

	/*
	* 玩家可见名称。
	*
	* 可以随时修改，不影响稳定 CheckpointID。
	*
	* 示例：
	* 黄泉路起点
	* 铃音余迹
	* 五灯尽处
	*/
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Display"
	)
	FText CheckpointDisplayName =
		FText::FromString(
			TEXT("Soul Marker")
		);

	/*
	* 休憩点未解锁时显示的交互提示。
	*/
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Interaction"
	)
	FString LockedInteractionPrompt =
		TEXT("[E] Kindle Soul Marker");

	/*
	* 休憩点已经解锁时显示的交互提示。
	*/
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Interaction"
	)
	FString UnlockedInteractionPrompt =
		TEXT("[E] Rest / Travel");

	/*
	* 未解锁时的基础灯光强度。
	*/
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Display",
		meta = (ClampMin = "0.0")
	)
	float LockedLightIntensity = 150.0f;

	/*
	* 已解锁时的基础灯光强度。
	*/
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Display",
		meta = (ClampMin = "0.0")
	)
	float UnlockedLightIntensity = 2500.0f;

	/*
	* 当前运行时的派生视觉状态。
	*
	* 不进入 SaveGame。
	*
	* Source of Truth：
	* Player / SaveGame 的 UnlockedCheckpointIDs。
	*/
	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Transient,
		Category = "Checkpoint|State"
	)
	bool bIsUnlocked = false;


	// ---------------------------------------------------------------------
	// Travel
	// ---------------------------------------------------------------------

	/*
	* Checkpoint Menu 中的排序值。
	*
	* 示例：
	* Start = 0
	* AfterEncounter01 = 1
	* AfterEncounter02 = 2
	*/
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Travel"
	)
	int32 TravelOrder = 0;


	// ---------------------------------------------------------------------
	// Ground Safety
	// ---------------------------------------------------------------------

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Ground Safety",
		meta = (ClampMin = "0.0")
	)
	float GroundTraceUpDistance = 100.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Ground Safety",
		meta = (ClampMin = "0.0")
	)
	float GroundTraceDownDistance = 300.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Ground Safety",
		meta = (ClampMin = "0.0")
	)
	float GroundClearance = 2.0f;


	// ---------------------------------------------------------------------
	// Debug
	// ---------------------------------------------------------------------

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Debug"
	)
	bool bShowActivationDebug = true;


	// ---------------------------------------------------------------------
	// Blueprint Presentation Events
	// ---------------------------------------------------------------------

	/*
	* 每次 ApplyUnlockedPresentation() 时调用。
	*
	* Blueprint 可根据 bUnlocked：
	* - 切换发光材质
	* - 调整 Niagara
	* - 切换魂火状态
	* - 调整额外灯光
	*
	* 该事件可能在 Load Restore 时调用，
	* 因此不应在这里执行一次性 Story 推进。
	*/
	UFUNCTION(
		BlueprintImplementableEvent,
		Category = "Checkpoint|Presentation"
	)
	void OnCheckpointPresentationChanged(
		bool bUnlocked
	);

	/*
	* 只有玩家主动使用休憩点且保存成功后调用。
	*
	* bFirstUnlock = true：
	* - 可以播放较明显的首次点亮效果
	*
	* bFirstUnlock = false：
	* - 可以播放较轻的再次休憩脉冲
	*
	* 不应在此事件中再次调用 Save。
	*/
	UFUNCTION(
		BlueprintImplementableEvent,
		Category = "Checkpoint|Presentation"
	)
	void OnCheckpointSaveSucceeded(
		bool bFirstUnlock
	);


private:
	// ---------------------------------------------------------------------
	// Overlap Callbacks
	// ---------------------------------------------------------------------

	UFUNCTION()
	void HandleActivationBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void HandleActivationBoxEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex
	);


	// ---------------------------------------------------------------------
	// Runtime References
	// ---------------------------------------------------------------------

	/*
	* 当前地图唯一的 Story Persistence Coordinator。
	*
	* Checkpoint 不直接访问磁盘。
	* 它只请求 Coordinator 执行完整稳定快照。
	*/
	UPROPERTY(Transient)
	TObjectPtr<AWCStoryPersistenceCoordinator>
		CachedPersistenceCoordinator;
};