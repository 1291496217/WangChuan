// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "MemoryFragment.generated.h"

UCLASS()
class WANGCHUAN_API AMemoryFragment : public AActor, public IInteractable
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FragmentMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* InteractionSphere;

	// Fragment ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory")
	int32 FragmentID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory")
	FString FragmentText;

	UFUNCTION()
	void OnPlayerEnter(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnPlayerExit(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	virtual void BeginPlay() override;

public:	
	AMemoryFragment();

	virtual void Interact() override;

	virtual FString GetInteractionPrompt() override;

};
