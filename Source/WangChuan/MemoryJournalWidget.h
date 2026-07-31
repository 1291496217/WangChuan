#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StoryTypes.h"
#include "MemoryJournalWidget.generated.h"

class AWCCharacter;
class UButton;
class UScrollBox;
class UTextBlock;
class UVerticalBox;
class UMemoryJournalEntryWidget;

/*
* 轻量 Memory Journal 展示层。
*
* 记录数据仍由 AWCCharacter 持有；此 Widget 仅按记录顺序显示传入快照并处理选择。
*/
UCLASS()
class WANGCHUAN_API UMemoryJournalWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ******************** Public Interface ********************

	void InitializeJournal(
		AWCCharacter* InPlayerOwner,
		const TArray<FMemoryEchoData>& InEntries
	);

	void RefreshJournalEntries(
		const TArray<FMemoryEchoData>& InEntries
	);

protected:
	// ******************** Lifecycle ********************

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	// ******************** Configuration ********************

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Memory Journal")
	TSubclassOf<UMemoryJournalEntryWidget> EntryWidgetClass;

private:
	// ******************** Runtime State ********************

	UPROPERTY(Transient)
	TObjectPtr<AWCCharacter> PlayerOwner;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> EntryListBox;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SelectedEntryTitle;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SelectedEntryBody;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> EmptyStateText;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMemoryJournalEntryWidget>> EntryWidgets;

	UPROPERTY(Transient)
	TArray<FMemoryEchoData> DisplayedEntries;

	int32 SelectedEntryIndex = INDEX_NONE;

	// ******************** Helpers ********************

	void BuildWidgetTree();
	void RebuildEntryList();
	void SelectEntry(int32 EntryIndex);
	void RefreshDetailPanel();

	// ******************** Events ********************

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleEntrySelected(int32 EntryIndex);
};
