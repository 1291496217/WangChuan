#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WCProgressionGate.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class WANGCHUAN_API AWCProgressionGate : public AActor
{
	GENERATED_BODY()

public:
	AWCProgressionGate();

	UFUNCTION(BlueprintCallable, Category = "Progression Gate")
	void OpenGate();

	UFUNCTION(BlueprintPure, Category = "Progression Gate")
	bool IsGateOpen() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression Gate|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression Gate|Components")
	TObjectPtr<UStaticMeshComponent> GateMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression Gate")
	FVector OpenOffset = FVector(0.0f, 0.0f, 500.0f);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
		Category = "Progression Gate")
	bool bIsOpen = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "Progression Gate",
		meta = (DisplayName = "On Gate Opened"))
	void BP_OnGateOpened();

private:
	FVector ClosedRelativeLocation = FVector::ZeroVector;
};
