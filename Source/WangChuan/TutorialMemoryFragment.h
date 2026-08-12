#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "TutorialMemoryFragment.generated.h"

class AWCCharacter;
class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class WANGCHUAN_API ATutorialMemoryFragment : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	ATutorialMemoryFragment();

	virtual void Interact() override;
	virtual FString GetInteractionPrompt() override;

	UFUNCTION(BlueprintCallable, Category = "Tutorial Fragment|Testing")
	void TriggerTutorialInteraction();

	UFUNCTION(BlueprintCallable, Category = "Tutorial Fragment")
	void SetFragmentAvailable(bool bNewAvailable);

	UFUNCTION(BlueprintPure, Category = "Tutorial Fragment")
	FName GetFragmentID() const;

	UFUNCTION(BlueprintPure, Category = "Tutorial Fragment")
	bool GetIsAvailable() const;

	UFUNCTION(BlueprintPure, Category = "Tutorial Fragment")
	bool GetIsCollected() const;

	UFUNCTION(BlueprintPure, Category = "Tutorial Fragment")
	bool GetStartAvailable() const;

	UFUNCTION(BlueprintPure, Category = "Tutorial Fragment")
	bool GetIsCurrentInteractableForOverlappingPlayer() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial Fragment|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial Fragment|Components")
	TObjectPtr<UStaticMeshComponent> FragmentMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial Fragment|Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Fragment")
	FName FragmentID = TEXT("Tutorial.Fragment01");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Fragment")
	FText DisplayTitle = FText::FromString(TEXT("Memory Fragment"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Fragment", meta = (MultiLine = true))
	FText DisplayText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Fragment")
	bool bStartAvailable = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Tutorial Fragment")
	bool bIsAvailable = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Tutorial Fragment")
	bool bCollected = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Fragment")
	FString InteractionPrompt = TEXT("[E] Read Memory");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Fragment")
	bool bShowInteractHintOnFirstOverlap = false;

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

private:
	TWeakObjectPtr<AWCCharacter> OverlappingPlayer;

	void ClearPlayerInteractionIfNeeded();
	void RefreshInteractionForOverlappingPlayer();
};
