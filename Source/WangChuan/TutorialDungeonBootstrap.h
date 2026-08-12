#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialDungeonBootstrap.generated.h"

class UTutorialFragmentWidget;
class UTutorialHUDWidget;
class UTutorialInstructionWidget;

UCLASS()
class WANGCHUAN_API ATutorialDungeonBootstrap : public AActor
{
	GENERATED_BODY()

public:
	ATutorialDungeonBootstrap();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial")
	TSubclassOf<UTutorialHUDWidget> TutorialHUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial")
	TSubclassOf<UTutorialFragmentWidget> TutorialFragmentWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial")
	TSubclassOf<UTutorialInstructionWidget> TutorialInstructionWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial", meta = (ClampMin = "1"))
	int32 TotalFragmentCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial")
	FText MoveLookInstructionTitle = FText::FromString(TEXT("Movement"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial", meta = (MultiLine = true))
	FText MoveLookInstructionBody = FText::FromString(TEXT("WASD — Move\nMouse — Look"));

private:
	void InitializeLocalTutorialPlayer();
};
