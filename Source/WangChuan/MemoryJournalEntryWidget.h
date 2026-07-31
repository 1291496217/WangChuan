#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MemoryJournalEntryWidget.generated.h"

class UBorder;
class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnMemoryJournalEntrySelected,
	int32,
	EntryIndex
);

/*
* Memory Journal 中可复用、可选择的单行条目。
*/
UCLASS()
class WANGCHUAN_API UMemoryJournalEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ******************** Public Interface ********************

	void InitializeEntry(
		int32 InEntryIndex,
		const FText& InTitle
	);

	void SetEntrySelected(bool bSelected);

	// ******************** Events ********************

	UPROPERTY(BlueprintAssignable, Category = "Memory Journal|Entry")
	FOnMemoryJournalEntrySelected OnEntrySelected;

protected:
	// ******************** Lifecycle ********************

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

private:
	// ******************** UI ********************

	UPROPERTY(Transient)
	TObjectPtr<UBorder> SelectionBorder;

	UPROPERTY(Transient)
	TObjectPtr<UButton> EntryButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> EntryTitleText;

	// ******************** Runtime State ********************

	int32 EntryIndex = INDEX_NONE;
	FText EntryTitle;
	bool bIsSelected = false;

	// ******************** Helpers ********************

	void BuildWidgetTree();
	void ApplySelectionVisual();

	// ******************** Events ********************

	UFUNCTION()
	void HandleEntryClicked();
};
