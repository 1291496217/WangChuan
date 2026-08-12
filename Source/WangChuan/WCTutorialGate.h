#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WCTutorialGate.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class WANGCHUAN_API AWCTutorialGate : public AActor
{
	GENERATED_BODY()

public:
	AWCTutorialGate();

	UFUNCTION(BlueprintCallable, Category = "Tutorial Gate")
	void OpenGate();

	UFUNCTION(BlueprintPure, Category = "Tutorial Gate")
	bool IsGateOpen() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial Gate|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial Gate|Components")
	TObjectPtr<UStaticMeshComponent> GateMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Gate")
	FVector OpenOffset = FVector(0.0f, 0.0f, 500.0f);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Tutorial Gate")
	bool bIsOpen = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial Gate",
		meta = (DisplayName = "On Gate Opened"))
	void BP_OnGateOpened();

private:
	FVector ClosedRelativeLocation = FVector::ZeroVector;
};
