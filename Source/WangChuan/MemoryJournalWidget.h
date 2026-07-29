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
* Lightweight Memory Journal presentation.
*
* Recorded data remains owned by AWCCharacter. This Widget only displays the
* supplied recording-order snapshot and handles selection.
*/
UCLASS()
class WANGCHUAN_API UMemoryJournalWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeJournal(
		AWCCharacter* InPlayerOwner,
		const TArray<FMemoryEchoData>& InEntries
	);

	void RefreshJournalEntries(
		const TArray<FMemoryEchoData>& InEntries
	);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Memory Journal")
	TSubclassOf<UMemoryJournalEntryWidget> EntryWidgetClass;

private:
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

	void BuildWidgetTree();
	void RebuildEntryList();
	void SelectEntry(int32 EntryIndex);
	void RefreshDetailPanel();

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleEntrySelected(int32 EntryIndex);
};
