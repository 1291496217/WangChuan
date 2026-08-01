#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WCPlayerCheckpoint.generated.h"

class AWCCharacter;
class UArrowComponent;
class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;

/*
* 玩家稳定恢复点。
*
* Checkpoint Actor 负责：
* - 提供稳定 CheckpointID
* - 在玩家进入范围时更新 Runtime CurrentCheckpointID
* - 提供经过地面校准的 Resume Transform
*
* 不负责：
* - 写入磁盘
* - Capture Story World
* - 自动保存
* - 自动加载
*/
UCLASS()
class WANGCHUAN_API AWCPlayerCheckpoint :
	public AActor
{
	GENERATED_BODY()

public:
	AWCPlayerCheckpoint();

	UFUNCTION(
		BlueprintPure,
		Category = "Checkpoint"
	)
	FName GetCheckpointID() const;

	UFUNCTION(
		BlueprintPure,
		Category = "Checkpoint"
	)
	bool GetIsDefaultCheckpoint() const;

	/*
	* 根据 Checkpoint 的 ResumeArrow 和玩家 Capsule，
	* 生成安全的玩家 Actor Transform。
	*
	* ResumeArrow 的位置代表玩家脚底附近的位置。
	*/
	bool BuildSafeResumeTransform(
		const AWCCharacter* Player,
		FTransform& OutResumeTransform
	) const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Components"
	)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Components"
	)
	TObjectPtr<UBoxComponent> ActivationBox;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Components"
	)
	TObjectPtr<UArrowComponent> ResumeArrow;

	/*
	* 永久稳定的语义 ID。
	*
	* 一旦用于正式 SaveGame，不应随意重命名。
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
	* 新游戏没有存档时，
	* Coordinator 会将玩家的 Runtime Checkpoint
	* 初始化为该 ID，但不会强制移动玩家。
	*/
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Identity"
	)
	bool bIsDefaultCheckpoint = false;

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

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Checkpoint|Debug"
	)
	bool bShowActivationDebug = true;

private:
	UFUNCTION()
	void HandleActivationBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};