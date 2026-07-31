
#include "StoryAnchor.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"

// ******************** Construction ********************

AStoryAnchor::AStoryAnchor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));

	SetRootComponent(SceneRoot);

	FacingArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("FacingArrow"));

	FacingArrow->SetupAttachment(SceneRoot);

	FacingArrow->ArrowSize = 1.5f;

	FacingArrow->SetHiddenInGame(true);

	SetActorEnableCollision(false);

}

// ******************** Getters ********************

FTransform AStoryAnchor::GetAnchorTransform() const
{
	return GetActorTransform();
}

FName AStoryAnchor::GetAnchorID() const
{
	return AnchorID;
}
