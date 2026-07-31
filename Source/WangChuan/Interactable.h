#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// 为所有可交互 Actor 提供统一的反射接口。
UINTERFACE(MinimalAPI)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

class WANGCHUAN_API IInteractable
{
	GENERATED_BODY()

public:
	// ******************** Public Interface ********************

	virtual void Interact() {};

	virtual FString GetInteractionPrompt() = 0;
};
