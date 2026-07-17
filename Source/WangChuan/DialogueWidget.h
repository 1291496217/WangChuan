// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StoryTypes.h"
#include "DialogueWidget.generated.h"

class AWCCharacter;
class AWCStoryNPC;
class UButton;
class UTextBlock;
/**
 * 最小通用对话 Widget 基类。
 * 
 * 负责：
 * - 保存当前 Dialogue Sequence
 * - 显示当前行
 * - 推进到下一行
 * - 最后一行后请求玩家关闭对话。
 * 
 * 不负责：
 * - Story Stage 推进
 * - 分支选择，任务条件， NPC移动， 具体剧情逻辑
 */
UCLASS()
class WANGCHUAN_API UDialogueWidget 
	: public UUserWidget
{
	GENERATED_BODY()
	
public:
	/*
	* 使用一组新的对话数据初始化 Widget。
	*/
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(
		const FDialogueSequence& NewDialogueSequence,
		AWCStoryNPC* NewStoryNPC,
		AWCCharacter* NewPlayerOwner
	);

	/*
	* 前进到下一行。
	* 
	* 可由 Contine Button 或玩家 E 键调用
	*/
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void AdvanceDialogue();

protected:
	virtual void NativeConstruct() override;

	/*
	* Blueprint Widget 中必须存在同名 TextBlock。
	*/
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SpeakerNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DialogueText;

	UPROPERTY(meta = (BindWidget))
	UButton* ContinueButton;

	/*
	* Continue Button 中的文字
	* 
	* 最后一行会从 Continue 改为 Close。
	*/
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ContinueButtonLabel;

	/*
	* 允许玩家随时主动关闭对话。
	*/
	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

	/*
	* 当前正在播放的 Dialogue Sequence。
	*/
	UPROPERTY()
	FDialogueSequence ActiveDialogueSequence;

	/*
	* 当前对话来源 NPC。
	*/
	UPROPERTY()
	AWCStoryNPC* StoryNPCOwner = nullptr;

	/*
	* 当前本地玩家。
	*/
	UPROPERTY()
	AWCCharacter* PlayerOwner = nullptr;

	/*
	* 当前显示的对话行。
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	int32 CurrentLineIndex = 0;

	/*
	* 将当前行写入UI。
	*/
	void DisplayCurrentLine();

	/*
	* 请求 Player 结束当前对话。
	*/
	void RequestCloseDialogue();

	UFUNCTION()
	void HandleContinueClicked();

	UFUNCTION()
	void HandleCloseClicked();
};
