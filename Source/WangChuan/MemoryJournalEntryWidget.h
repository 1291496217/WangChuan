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
* One reusable selectable row in the Memory Journal.
*/
UCLASS()
class WANGCHUAN_API UMemoryJournalEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeEntry(
		int32 InEntryIndex,
		const FText& InTitle
	);

	void SetEntrySelected(bool bSelected);

	UPROPERTY(BlueprintAssignable, Category = "Memory Journal|Entry")
	FOnMemoryJournalEntrySelected OnEntrySelected;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UBorder> SelectionBorder;

	UPROPERTY(Transient)
	TObjectPtr<UButton> EntryButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> EntryTitleText;

	int32 EntryIndex = INDEX_NONE;
	FText EntryTitle;
	bool bIsSelected = false;

	void BuildWidgetTree();
	void ApplySelectionVisual();

	UFUNCTION()
	void HandleEntryClicked();
};
