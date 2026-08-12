#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialInstructionTrigger.generated.h"

class UBoxComponent;
class USceneComponent;

UCLASS()
class WANGCHUAN_API ATutorialInstructionTrigger : public AActor
{
	GENERATED_BODY()

public:
	ATutorialInstructionTrigger();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial Instruction|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial Instruction|Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Instruction")
	FName InstructionID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Instruction")
	FText InstructionTitle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Instruction", meta = (MultiLine = true))
	FText InstructionBody;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Instruction")
	bool bTriggerOnce = true;

	UFUNCTION()
	void OnTriggerEntered(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
