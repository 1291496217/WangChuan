#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StoryAnchor.generated.h"

class USceneComponent;
class UArrowComponent;

/*
* 轻量 Story Anchor。
*
* 负责在关卡中标记：
* - NPC 应该出现的位置。
* - NPC 应该面向的方向。
*
* 不负责：
* - 移动 NPC, 具体剧情。
*/
UCLASS()
class WANGCHUAN_API AStoryAnchor : public AActor
{
	GENERATED_BODY()

public:
	// ******************** Construction ********************

	AStoryAnchor();

	// ******************** Getters ********************

	/*
	* 返回 Anchor 当前的世界 Transform。
	*/
	UFUNCTION(BlueprintPure, Category = "Story Anchor")
	FTransform GetAnchorTransform() const;

	/*
	* 返回用于编辑器识别的 Anchor ID。
	*/
	UFUNCTION(BlueprintPure, Category = "Story Anchor")
	FName GetAnchorID() const;

protected:
	// ******************** Components ********************

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story Anchor|Components")
	USceneComponent* SceneRoot;

	/*
	* 编辑器中的朝向指示箭头。
	*
	* NPC 重现后的面向方向由 Actor Rotation 决定。
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story Anchor|Components")
	UArrowComponent* FacingArrow;

	// ******************** Configuration ********************

	/*
	* 用于区分多个 Story Anchor。
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Anchor")
	FName AnchorID = NAME_None;
};
