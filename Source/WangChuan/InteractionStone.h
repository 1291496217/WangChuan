#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Interactable.h"
#include "InteractionStone.generated.h"

UCLASS()
class WANGCHUAN_API AInteractionStone : public AActor, public IInteractable
{
	GENERATED_BODY()

protected:
	// ******************** Components ********************

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StoneMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* InteractionSphere;

	// ******************** Events ********************

	UFUNCTION()
	void OnPlayerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
					   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
					   const FHitResult& SweepResult);

	UFUNCTION()
	void OnPlayerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
					  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// ******************** Lifecycle ********************

	virtual void BeginPlay() override;

public:
	// ******************** Construction ********************

	AInteractionStone();

	// ******************** Interaction ********************

	virtual void Interact() override;

	virtual FString GetInteractionPrompt() override;
};
