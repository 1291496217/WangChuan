#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WCPlayerCheckpoint.h"
#include "WCCheckpointMenuWidget.generated.h"

class AWCCharacter;
class AWCStoryPersistenceCoordinator;
class UButton;
class UTextBlock;

/*
* 归魂碑菜单的强类型 C++ 基类。
*
* 具体布局由 WBP_CheckpointMenu 实现；本类只保存菜单会话引用，
* 并向 Blueprint 暴露已排序的传送选项和受控请求入口。
*/
UCLASS()
class WANGCHUAN_API UWCCheckpointMenuWidget :
	public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeCheckpointMenu(
		AWCCharacter* InPlayer,
		AWCPlayerCheckpoint* InSourceCheckpoint,
		AWCStoryPersistenceCoordinator* InCoordinator
	);

	UFUNCTION(
		BlueprintPure,
		Category = "Checkpoint Menu"
	)
	const TArray<FWCCheckpointTravelOption>&
		GetTravelOptions() const;

	UFUNCTION(
		BlueprintCallable,
		Category = "Checkpoint Menu"
	)
	void RequestTravelToCheckpoint(
		FName TargetCheckpointID
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "Checkpoint Menu"
	)
	void RequestCloseMenu();

	UFUNCTION(
		BlueprintImplementableEvent,
		Category = "Checkpoint Menu"
	)
	void OnCheckpointMenuInitialized();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<AWCCharacter> PlayerOwner;

	UPROPERTY(Transient)
	TObjectPtr<AWCPlayerCheckpoint> SourceCheckpoint;

	UPROPERTY(Transient)
	TObjectPtr<AWCStoryPersistenceCoordinator> Coordinator;

	UPROPERTY(Transient)
	TArray<FWCCheckpointTravelOption> TravelOptions;

	/*
	* WBP 尚未配置时使用的原生三行后备界面。
	* Blueprint 若拥有自己的 WidgetTree，不会被替换。
	*/
	UPROPERTY(Transient)
	TArray<TObjectPtr<UButton>> DestinationButtons;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> DestinationLabels;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CloseButton;

	void BuildFallbackWidgetTree();
	void ResolveWidgetReferences();
	void RefreshPresentation();
	void RequestTravelOptionByIndex(int32 OptionIndex);

	UFUNCTION()
	void HandleDestination0Clicked();

	UFUNCTION()
	void HandleDestination1Clicked();

	UFUNCTION()
	void HandleDestination2Clicked();

	UFUNCTION()
	void HandleCloseClicked();
};
