#include "WCCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Controller.h"
#include "Math/RotationMatrix.h"
#include "Animation/AnimInstance.h"
#include "GhostEnemy.h"
#include "WCStoryNPC.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "DialogueWidget.h"
#include "Camera/PlayerCameraManager.h"

#include "EchoRelic.h"
#include "MemoryEchoWidget.h"
#include "MemoryJournalWidget.h"
#include "WCCheckpointMenuWidget.h"
#include "WCStoryPersistenceCoordinator.h"

// ******************** Construction ********************

AWCCharacter::AWCCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建SpringArm组件
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	// 挂到角色根节点
	CameraBoom->SetupAttachment(RootComponent);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	// 把Camera挂到SpringArm
	FollowCamera->SetupAttachment(CameraBoom);

	CameraBoom->TargetArmLength = 400.0f; // 摄像机距离角色400厘米
	CameraBoom->bUsePawnControlRotation = true; // 鼠标旋转控制SpringArm旋转
	FollowCamera->bUsePawnControlRotation = false; // 只让Boom旋转，Camera跟着Boom

	// 控制器旋转不直接驱动角色旋转。
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	// 角色朝移动方向旋转。
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// 设置角色转向速度。
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	bShowAttackDebug = false;
}

// ******************** Lifecycle ********************

void AWCCharacter::BeginPlay()
{
	Super::BeginPlay();
	NormalWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	Health = MaxHealth;
	bIsDead = false;

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
					PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (PlayerHUDClass)
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetController());

		if (PlayerController)
		{
			PlayerHUDWidget = CreateWidget<UUserWidget>(PlayerController, PlayerHUDClass);

			if (PlayerHUDWidget)
			{
				PlayerHUDWidget->AddToViewport();
			}
		}
	}

	if (PlayerHitFlashWidgetClass)
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetController());

		if (PlayerController)
		{
			PlayerHitFlashWidget =
				CreateWidget<UPlayerHitFlashWidget>(PlayerController, PlayerHitFlashWidgetClass);

			if (PlayerHitFlashWidget)
			{
				PlayerHitFlashWidget->AddToViewport(10);
			}
		}
	}
}

void AWCCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ActiveCheckpointMenuWidget)
	{
		ActiveCheckpointMenuWidget->RemoveFromParent();
	}

	ActiveCheckpointMenuWidget = nullptr;
	bIsCheckpointMenuOpen = false;

	if (ActiveMemoryJournalWidget)
	{
		ActiveMemoryJournalWidget->RemoveFromParent();
	}

	ActiveMemoryJournalWidget = nullptr;
	bIsMemoryJournalOpen = false;

	Super::EndPlay(EndPlayReason);
}

void AWCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateLockOn(DeltaTime);
}

// ******************** Initialization ********************

void AWCCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this,
											   &AWCCharacter::Move);
		}
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this,
											   &AWCCharacter::Look);
		}
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this,
											   &AWCCharacter::HandleJumpStarted);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this,
											   &AWCCharacter::HandleJumpCompleted);
		}
		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this,
											   &AWCCharacter::Interact);
		}
		if (JournalAction)
		{
			EnhancedInputComponent->BindAction(JournalAction, ETriggerEvent::Started, this,
											   &AWCCharacter::ToggleMemoryJournal);
		}
		if (AttackAction)
		{
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this,
											   &AWCCharacter::Attack);
		}
		if (HeavyAttackAction)
		{
			EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this,
											   &AWCCharacter::HeavyAttack);
		}
		if (LockOnAction)
		{
			EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started, this,
											   &AWCCharacter::ToggleLockOn);
		}
	}
}

// ******************** Input ********************

void AWCCharacter::Move(const FInputActionValue& Value)
{
	if (!CanAct())
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();

		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);

		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AWCCharacter::HandleJumpStarted()
{
	if (!CanStartJump())
	{
		return;
	}

	Jump();
}

void AWCCharacter::HandleJumpCompleted()
{
	StopJumping();
}

void AWCCharacter::Look(const FInputActionValue& Value)
{
	if (!CanAct())
	{
		return;
	}
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);

		AddControllerPitchInput(LookAxisVector.Y);
	}
}

// ******************** Interaction ********************

void AWCCharacter::Interact()
{
	/*
	 * Checkpoint Menu 打开时，E 键只负责关闭菜单。
	 */
	if (bIsCheckpointMenuOpen)
	{
		CloseCheckpointMenu();
		return;
	}

	if (bIsMemoryJournalOpen)
	{
		return;
	}

	/*
	 * Memory Echo 状态下，
	 * E 键用于翻页和关闭。
	 */
	if (bIsViewingMemoryEcho)
	{
		if (ActiveMemoryEchoWidget)
		{
			ActiveMemoryEchoWidget->AdvanceMemoryEcho();
		}

		return;
	}

	/*
	 * Dialogue 状态下，
	 * E 键用于推进 Dialogue。
	 */
	if (bIsInDialogue)
	{
		if (ActiveDialogueWidget)
		{
			ActiveDialogueWidget->AdvanceDialogue();
		}

		return;
	}

	/*
	 * 玩家死亡等状态下不能开始交互。
	 */
	if (!CanAct())
	{
		return;
	}

	/*
	 * 避免攻击动作途中打开对话。
	 */
	if (bIsAttacking)
	{
		return;
	}

	if (CurrentInteractable)
	{
		CurrentInteractable->Interact();
	}
}

void AWCCharacter::ShowInteractionPrompt(const FString& Prompt)
{
	CurrentPrompt = Prompt;
	GEngine->AddOnScreenDebugMessage(1, 9999.0f, FColor::Green, CurrentPrompt);
}

void AWCCharacter::HideInteractionPrompt()
{
	GEngine->RemoveOnScreenDebugMessage(1);
}

// ******************** Memory Journal ********************

void AWCCharacter::ToggleMemoryJournal()
{
	UE_LOG(LogTemp, Log, TEXT("Memory Journal input received (open=%s)."),
		   bIsMemoryJournalOpen ? TEXT("true") : TEXT("false"));

	if (bIsCheckpointMenuOpen)
	{
		return;
	}

	if (bIsMemoryJournalOpen)
	{
		CloseMemoryJournal();
		return;
	}

	OpenMemoryJournal();
}

bool AWCCharacter::OpenMemoryJournal()
{
	if (bIsDead || bIsInDialogue || bIsViewingMemoryEcho || bIsMemoryJournalOpen ||
		bIsCheckpointMenuOpen || bIsAttacking ||
		bIsInCombat || bIsLockedOn)
	{
		if (bIsInCombat || bIsLockedOn)
		{
			UE_LOG(LogTemp, Log,
				   TEXT("Memory Journal open rejected while "
						"the player is in combat."));
		}
		return false;
	}

	if (!MemoryJournalWidgetClass)
	{
		UE_LOG(LogTemp, Warning,
			   TEXT("Memory Journal cannot open because "
					"MemoryJournalWidgetClass is not assigned."));
		return false;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return false;
	}

	if (!ActiveMemoryJournalWidget)
	{
		ActiveMemoryJournalWidget =
			CreateWidget<UMemoryJournalWidget>(PlayerController, MemoryJournalWidgetClass);
	}

	if (!ActiveMemoryJournalWidget)
	{
		return false;
	}

	bIsMemoryJournalOpen = true;

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	StopJumping();
	HideInteractionPrompt();

	ActiveMemoryJournalWidget->InitializeJournal(this, RecordedMemoryEchoes);
	ActiveMemoryJournalWidget->AddToViewport(30);

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(ActiveMemoryJournalWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);

	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);

	UE_LOG(LogTemp, Log, TEXT("Memory Journal opened with %d recorded echo(es)."),
		   RecordedMemoryEchoes.Num());

	return true;
}

void AWCCharacter::CloseMemoryJournal()
{
	if (!bIsMemoryJournalOpen && !ActiveMemoryJournalWidget)
	{
		return;
	}

	if (ActiveMemoryJournalWidget)
	{
		ActiveMemoryJournalWidget->RemoveFromParent();
	}

	bIsMemoryJournalOpen = false;

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);

		if (!bIsDead && !bIsInDialogue && !bIsViewingMemoryEcho)
		{
			PlayerController->SetIgnoreMoveInput(false);
			PlayerController->SetIgnoreLookInput(false);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Memory Journal closed."));
}

// ******************** Dialogue ********************

bool AWCCharacter::StartDialogue(AWCStoryNPC* StoryNPC, const FDialogueSequence& DialogueSequence)
{
	if (bIsDead)
	{
		return false;
	}

	if (bIsInDialogue || bIsViewingMemoryEcho || bIsMemoryJournalOpen ||
		bIsCheckpointMenuOpen)
	{
		return false;
	}

	if (!StoryNPC)
	{
		return false;
	}

	if (DialogueSequence.Lines.Num() == 0)
	{
		return false;
	}

	if (!DialogueWidgetClass)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red,
											 TEXT("DialogueWidgetClass is not assigned."));
		}
		return false;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (!PlayerController)
	{
		return false;
	}

	UDialogueWidget* NewDialogueWidget =
		CreateWidget<UDialogueWidget>(PlayerController, DialogueWidgetClass);

	if (!NewDialogueWidget)
	{
		return false;
	}

	/*
	 * 对话开始前解除锁定。
	 */
	if (bIsLockedOn)
	{
		UnlockTarget();
	}

	bIsInDialogue = true;

	ActiveDialogueNPC = StoryNPC;
	ActiveDialogueWidget = NewDialogueWidget;

	/*
	 * 停止玩家当前移动。
	 */
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->Velocity = FVector::ZeroVector;

	StopJumping();

	/*
	 * 隐藏世界交互 Prompt。
	 */
	HideInteractionPrompt();

	/*
	 * 先加入 Viewport，
	 * 确保 BindWidget 与 NativeConstruct 已完成。
	 */
	ActiveDialogueWidget->AddToViewport(20);

	ActiveDialogueWidget->StartDialogue(DialogueSequence, StoryNPC, this);

	/*
	 * GameAndUI:
	 * - 鼠标可以点击 Continue/ Close
	 * - 未被 UI 消耗的 E 仍可进入 Interact（）
	 */
	FInputModeGameAndUI InputMode;

	InputMode.SetWidgetToFocus(ActiveDialogueWidget->TakeWidget());

	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	InputMode.SetHideCursorDuringCapture(false);

	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;

	/*
	 * 阻止移动与镜头输入。
	 *
	 * 攻击，锁定，Journal 和 Jump 由 CanAct() 或各自函数保护。
	 */
	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);

	return true;
}

void AWCCharacter::EndDialogue()
{
	if (!bIsInDialogue && !ActiveDialogueWidget)
	{
		return;
	}

	/*
	 * 在清空前保存 NPC 引用。
	 * 用于对话关闭后恢复 Prompt。
	 */
	AWCStoryNPC* PreviousDialogueNPC = ActiveDialogueNPC;

	if (ActiveDialogueWidget)
	{
		ActiveDialogueWidget->RemoveFromParent();
	}

	ActiveDialogueWidget = nullptr;
	ActiveDialogueNPC = nullptr;
	bIsInDialogue = false;

	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController)
	{
		// 无论是否死亡，都移除 Dialogue 鼠标状态。
		PlayerController->bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);

		/*
		 * 玩家正常存活时恢复控制。
		 *
		 * 玩家死亡时不恢复，
		 * Die（）会继续禁用输入和移动。
		 */
		if (!bIsDead)
		{
			PlayerController->SetIgnoreMoveInput(false);
			PlayerController->SetIgnoreLookInput(false);
		}
	}

	/*
	 * 如果玩家关闭对话后仍在 NPC 范围内，
	 * 重新显示 [E] Approach。
	 *
	 * 如果玩家已被清出范围，
	 * CurrentInteractable 不再是该 NPC，
	 * 则不会恢复 Prompt。
	 */
	if (!bIsDead && PreviousDialogueNPC && CurrentInteractable == PreviousDialogueNPC)
	{
		ShowInteractionPrompt(PreviousDialogueNPC->GetInteractionPrompt());
	}
}

// ******************** Memory Echo ********************

bool AWCCharacter::StartMemoryEcho(const FMemoryEchoData& EchoData, AEchoRelic* SourceRelic)
{
	if (bIsDead)
	{
		return false;
	}

	if (bIsInDialogue || bIsViewingMemoryEcho || bIsMemoryJournalOpen ||
		bIsCheckpointMenuOpen)
	{
		return false;
	}

	if (bIsAttacking)
	{
		return false;
	}

	if (!SourceRelic)
	{
		return false;
	}

	if (EchoData.EchoID.IsNone())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
											 TEXT("Memory Echo has no EchoID."));
		}
		return false;
	}

	if (!MemoryEchoWidgetClass)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red,
											 TEXT("MemoryEchoWidgetClass "
												  "is not assigned."));
		}

		return false;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (!PlayerController)
	{
		return false;
	}

	UMemoryEchoWidget* NewWidget =
		CreateWidget<UMemoryEchoWidget>(PlayerController, MemoryEchoWidgetClass);

	if (!NewWidget)
	{
		return false;
	}

	if (bIsLockedOn)
	{
		UnlockTarget();
	}

	ExitCombatState();

	bIsViewingMemoryEcho = true;

	ActiveEchoRelic = SourceRelic;
	ActiveMemoryEchoData = EchoData;
	ActiveMemoryEchoWidget = NewWidget;

	GetCharacterMovement()->StopMovementImmediately();

	GetCharacterMovement()->Velocity = FVector::ZeroVector;

	StopJumping();

	HideInteractionPrompt();

	ActiveMemoryEchoWidget->AddToViewport(25);

	ActiveMemoryEchoWidget->StartMemoryEcho(EchoData, this);

	FInputModeGameAndUI InputMode;

	InputMode.SetWidgetToFocus(ActiveMemoryEchoWidget->TakeWidget());

	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	InputMode.SetHideCursorDuringCapture(false);

	PlayerController->SetInputMode(InputMode);

	PlayerController->bShowMouseCursor = true;

	PlayerController->SetIgnoreMoveInput(true);

	return true;
}

void AWCCharacter::EndMemoryEcho(bool bCompleted)
{
	if (!bIsViewingMemoryEcho && !ActiveMemoryEchoWidget)
	{
		return;
	}

	/*
	 * 在清空状态前保留数据。
	 */
	AEchoRelic* PreviousRelic = ActiveEchoRelic;

	const FMemoryEchoData CompletedEchoData = ActiveMemoryEchoData;

	if (ActiveMemoryEchoWidget)
	{
		ActiveMemoryEchoWidget->RemoveFromParent();
	}

	ActiveMemoryEchoWidget = nullptr;
	ActiveEchoRelic = nullptr;
	ActiveMemoryEchoData = FMemoryEchoData();

	bIsViewingMemoryEcho = false;

	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = false;

		FInputModeGameOnly InputMode;

		PlayerController->SetInputMode(InputMode);

		if (!bIsDead)
		{
			PlayerController->SetIgnoreMoveInput(false);
			PlayerController->SetIgnoreLookInput(false);
		}
	}

	/*
	 * 玩家存活且完整阅读：
	 * 记录 Journal，并确认 Relic。
	 */
	if (bCompleted && !bIsDead)
	{
		RecordMemoryEcho(CompletedEchoData);

		if (PreviousRelic)
		{
			PreviousRelic->ConfirmEchoRead();
		}

		return;
	}

	/*
	 * 玩家死亡或 UI 异常关闭。
	 * 不完成 Story Event。
	 */
	if (PreviousRelic)
	{
		PreviousRelic->CancelEchoRead();
	}
}

bool AWCCharacter::HasRecordedMemoryEcho(FName EchoID) const
{
	if (EchoID.IsNone())
	{
		return false;
	}

	for (const FMemoryEchoData& Echo : RecordedMemoryEchoes)
	{
		if (Echo.EchoID == EchoID)
		{
			return true;
		}
	}
	return false;
}

bool AWCCharacter::RecordMemoryEcho(const FMemoryEchoData& EchoData)
{
	if (EchoData.EchoID.IsNone())
	{
		return false;
	}

	if (HasRecordedMemoryEcho(EchoData.EchoID))
	{
		return false;
	}

	RecordedMemoryEchoes.Add(EchoData);

	if (bIsMemoryJournalOpen && ActiveMemoryJournalWidget)
	{
		ActiveMemoryJournalWidget->RefreshJournalEntries(RecordedMemoryEchoes);
	}

	UE_LOG(LogTemp, Log, TEXT("Memory Echo recorded: [%s] %s."), *EchoData.EchoID.ToString(),
		   *EchoData.Title.ToString());

	return true;
}

const TArray<FMemoryEchoData>&
AWCCharacter::GetRecordedMemoryEchoes() const
{
	return RecordedMemoryEchoes;
}

bool AWCCharacter::GetIsMemoryJournalOpen() const
{
	return bIsMemoryJournalOpen;
}


void AWCCharacter::ApplySavedMemoryEchoes(
	const TArray<FMemoryEchoData>&
	SavedEchoes)
{
	/*
	* 不在其他 Story Modal 运行中替换数据。
	*
	* 正式 Load 流程之后还会增加统一 Modal 安全处理。
	*/
	if (bIsInDialogue ||
		bIsViewingMemoryEcho)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"Memory Echo restore rejected because "
				"a Story UI mode is active."
			)
		);

		return;
	}

	if (bIsMemoryJournalOpen ||
		ActiveMemoryJournalWidget)
	{
		CloseMemoryJournal();
	}

	RecordedMemoryEchoes.Reset(
		SavedEchoes.Num()
	);

	TSet<FName> RestoredEchoIDs;

	for (const FMemoryEchoData& EchoData :
		SavedEchoes)
	{
		if (EchoData.EchoID.IsNone())
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT(
					"Skipped saved Memory Echo "
					"with ID None."
				)
			);

			continue;
		}

		if (RestoredEchoIDs.Contains(
			EchoData.EchoID))
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT(
					"Skipped duplicate saved "
					"Memory Echo [%s]."
				),
				*EchoData.EchoID.ToString()
			);

			continue;
		}

		RestoredEchoIDs.Add(
			EchoData.EchoID
		);

		/*
		* 不调用 RecordMemoryEcho()。
		*
		* Restore 直接恢复已经确认的记录，
		* 不需要模拟“刚刚获得 Echo”。
		*/
		RecordedMemoryEchoes.Add(
			EchoData
		);
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Memory Journal silently restored "
			"with %d Echo record(s)."
		),
		RecordedMemoryEchoes.Num()
	);
}
// ******************** Combat ********************

void AWCCharacter::Attack()
{
	if (!CanStartGroundAttack())
	{
		return;
	}
	if (bIsAttacking)
	{
		if (bCanBufferLightAttack)
		{
			bHasBufferedLightAttack = true;
		}
		return;
	}

	bIsAttacking = true;
	bHasProcessedAttackHit = false;

	FaceLockOnTargetInstantly();

	bIsCurrentAttackHeavy = false;
	bCanBufferLightAttack = false;
	bHasBufferedLightAttack = false;

	GetWorldTimerManager().ClearTimer(ComboWindowOpenTimerHandle);
	GetWorldTimerManager().ClearTimer(ComboWindowCloseTimerHandle);

	EnterCombatState();

	const int32 ComboIndex = CurrentLightComboIndex;

	CurrentAttackData = GetLightComboAttackData(ComboIndex);

	bIsCurrentAttackFinisher = (ComboIndex == MaxLightComboIndex - 1);

	const FName SectionName = GetLightComboSectionName(ComboIndex);

	CurrentAttackMontage = LightAttackMontage;
	PlayLightAttackMontage(SectionName);

	StartAttackTimer(CurrentAttackData.Duration);

	const float WindowOpenTime = FMath::Clamp(CurrentAttackData.Duration * ComboWindowOpenRatio,
											  0.0f, CurrentAttackData.Duration);

	GetWorldTimerManager().SetTimer(ComboWindowOpenTimerHandle, this,
									&AWCCharacter::OpenLightComboWindow, WindowOpenTime, false);

	AdvancedLightCombo();

	GetWorldTimerManager().ClearTimer(ComboResetTimerHandle);
	GetWorldTimerManager().SetTimer(ComboResetTimerHandle, this, &AWCCharacter::ResetLightCombo,
									ComboResetTime, false);
}

void AWCCharacter::HeavyAttack()
{
	if (!CanStartGroundAttack())
	{
		return;
	}

	if (bIsAttacking)
	{
		return;
	}

	bIsAttacking = true;
	bHasProcessedAttackHit = false;

	FaceLockOnTargetInstantly();

	bIsCurrentAttackHeavy = true;
	bIsCurrentAttackFinisher = false;

	ClearLightComboBuffer();
	EnterCombatState();
	ResetLightCombo();

	CurrentAttackData = HeavyAttackData;

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;

	CurrentAttackMontage = HeavyAttackMontage;

	if (AnimInstance && HeavyAttackMontage)
	{
		AnimInstance->Montage_Play(HeavyAttackMontage);
	}

	StartAttackTimer(CurrentAttackData.Duration);
}

bool AWCCharacter::IsAnyAttackActive() const
{
	return bIsAttacking;
}

bool AWCCharacter::CanStartGroundAttack() const
{
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	return CanAct() && MovementComponent && MovementComponent->IsMovingOnGround() && !bPressedJump;
}

bool AWCCharacter::CanStartJump() const
{
	return CanAct() && !IsAnyAttackActive() && CanJump();
}

void AWCCharacter::CancelActiveAttackForAirborneTransition()
{
	if (!IsAnyAttackActive())
	{
		return;
	}

	StopCurrentAttackMontage();
	EndHitStop();

	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	GetWorldTimerManager().ClearTimer(ComboResetTimerHandle);
	GetWorldTimerManager().ClearTimer(ComboWindowOpenTimerHandle);
	GetWorldTimerManager().ClearTimer(ComboWindowCloseTimerHandle);

	bIsAttacking = false;
	bHasProcessedAttackHit = false;
	bIsCurrentAttackHeavy = false;
	bIsCurrentAttackFinisher = false;
	CurrentAttackMontage = nullptr;
	CurrentLightComboIndex = 0;
	ClearLightComboBuffer();
}

void AWCCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (MovementComponent && MovementComponent->IsFalling() && IsAnyAttackActive())
	{
		CancelActiveAttackForAirborneTransition();
	}
}

void AWCCharacter::PerformCurrentAttackTrace()
{
	if (!CanStartGroundAttack() || !IsAnyAttackActive())
	{
		return;
	}

	FVector ForwardVector = GetActorForwardVector();

	FVector Start = GetActorLocation() + FVector(0.0f, 0.0f, 40.0f) + ForwardVector * 60.0f;

	FVector End = Start + ForwardVector * CurrentAttackData.Range;

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	FHitResult HitResult;

	EDrawDebugTrace::Type DebugType =
		bShowAttackDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	bool bHit = UKismetSystemLibrary::BoxTraceSingle(
		this, Start, End, CurrentAttackData.BoxHalfSize, GetActorRotation(),
		UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore, DebugType,
		HitResult, true, FLinearColor::Red, FLinearColor::Green, 2.0f);

	bool bHitEnemy = false;

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();

		ShowAttackHitDebug(HitActor);

		AGhostEnemy* GhostEnemy = Cast<AGhostEnemy>(HitActor);

		if (GhostEnemy)
		{
			FVector HitDirection = GetActorForwardVector();

			GhostEnemy->TakeHit(CurrentAttackData.Damage, HitDirection,
								CurrentAttackData.KnockbackStrength);

			if (bIsCurrentAttackHeavy)
			{
				PlayHeavyAttackHitFeedback(HitResult);
				ApplyHitStop(GhostEnemy, HeavyHitStopDuration);
			}
			else if (bIsCurrentAttackFinisher)
			{
				PlayComboFinisherFeedback(HitResult);
				ApplyHitStop(GhostEnemy, FinisherHitStopDuration);
			}
			else
			{
				PlayAttackHitEffect(HitResult);
				ApplyHitStop(GhostEnemy, LightHitStopDuration);
			}

			bHitEnemy = true;
		}
	}

	if (bHitEnemy)
	{
		if (!bIsCurrentAttackFinisher && !bIsCurrentAttackHeavy)
		{
			PlayAttackHitSound();
		}
	}
	else
	{
		PlayAttackWhiffSound();
	}
}

void AWCCharacter::OnPlayerAttackHitNotify()
{
	if (!CanStartGroundAttack() || !IsAnyAttackActive())
	{
		return;
	}
	if (bHasProcessedAttackHit)
	{
		return;
	}

	bHasProcessedAttackHit = true;

	PerformCurrentAttackTrace();
}

void AWCCharacter::EndAttack()
{
	StopCurrentAttackMontage();

	bIsAttacking = false;
	bHasProcessedAttackHit = false;
	bIsCurrentAttackHeavy = false;
	CurrentAttackMontage = nullptr;

	bCanBufferLightAttack = false;
	GetWorldTimerManager().ClearTimer(ComboWindowOpenTimerHandle);
	GetWorldTimerManager().ClearTimer(ComboWindowCloseTimerHandle);

	if (bHasBufferedLightAttack)
	{
		ConsumeBufferedLightAttack();
	}
	else
	{
		bHasBufferedLightAttack = false;
		bIsCurrentAttackFinisher = false;
	}
}

void AWCCharacter::ReceiveDamage(float DamageAmount)
{
	if (bIsDead)
	{
		return;
	}

	if (bIsMemoryJournalOpen)
	{
		CloseMemoryJournal();
	}

	Health -= DamageAmount;

	if (Health < 0.0f)
	{
		Health = 0.0f;
	}

	if (Health <= 0.0f)
	{
		Die();
		return;
	}

	PlayPlayerHitFeedback();
}

// ******************** Combat Audio ********************

void AWCCharacter::PlayAttackHitSound()
{
	if (AttackHitSound == nullptr)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(this, AttackHitSound, GetActorLocation());
}

void AWCCharacter::PlayAttackWhiffSound()
{
	if (AttackWhiffSound == nullptr)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(this, AttackWhiffSound, GetActorLocation());
}

// ******************** State Queries ********************

float AWCCharacter::GetHealth() const
{
	return Health;
}

float AWCCharacter::GetHealthPercent() const
{
	if (MaxHealth <= 0.0f)
	{
		return 0.0f;
	}
	return Health / MaxHealth;
}

bool AWCCharacter::GetIsDead() const
{
	return bIsDead;
}

bool AWCCharacter::GetIsInCombat() const
{
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	/*
	 * bIsInCombat 是战斗计时与 Journal 限制使用的持久 Gameplay 状态。
	 * 此 Blueprint Getter 为 ABP_Manny 提供地面 Combat Idle 选择依据，
	 * 因此角色在空中时不应暴露该视觉状态。
	 */
	return bIsInCombat && MovementComponent && MovementComponent->IsMovingOnGround();
}

bool AWCCharacter::GetIsLockedOn() const
{
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	/*
	 * 在空中保留 bIsLockedOn 与 CurrentLockOnTarget，
	 * 以维持摄像机跟踪并在落地后恢复状态。
	 * 此 Blueprint Getter 为 ABP_Manny 提供地面 Lock-On Strafe 选择依据，
	 * 因此只抑制空中动画请求。
	 */
	return bIsLockedOn && MovementComponent && MovementComponent->IsMovingOnGround();
}

bool AWCCharacter::GetIsInDialogue() const
{
	return bIsInDialogue;
}

bool AWCCharacter::GetIsViewingMemoryEcho() const
{
	return bIsViewingMemoryEcho;
}

void AWCCharacter::Die()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	bIsAttacking = false;

	/*
	 * 玩家死亡时立即关闭 Dialogue UI。
	 */
	if (bIsInDialogue || ActiveDialogueWidget)
	{
		EndDialogue();
	}

	if (bIsViewingMemoryEcho || ActiveMemoryEchoWidget)
	{
		EndMemoryEcho(false);
	}

	if (bIsMemoryJournalOpen || ActiveMemoryJournalWidget)
	{
		CloseMemoryJournal();
	}

	if (bIsCheckpointMenuOpen || ActiveCheckpointMenuWidget)
	{
		CloseCheckpointMenu();
	}

	UnlockTarget();
	bHasProcessedAttackHit = false;
	bIsCurrentAttackHeavy = false;
	bIsCurrentAttackFinisher = false;

	EndHitStop();

	if (PlayerDeathSound)
	{
		UGameplayStatics::PlaySound2D(this, PlayerDeathSound);
	}

	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	GetWorldTimerManager().ClearTimer(ComboResetTimerHandle);
	GetWorldTimerManager().ClearTimer(CombatIdleTimerHandle);
	ClearLightComboBuffer();

	CurrentLightComboIndex = 0;
	bIsInCombat = false;

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	GetCharacterMovement()->DisableMovement();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController)
	{
		DisableInput(PlayerController);
	}

	// 进入布娃娃状态。
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	USkeletalMeshComponent* CharacterMesh = GetMesh();

	if (CharacterMesh)
	{
		CharacterMesh->SetCollisionProfileName(TEXT("Ragdoll"));

		CharacterMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		CharacterMesh->SetSimulatePhysics(true);

		CharacterMesh->WakeAllRigidBodies();
	}
}

bool AWCCharacter::CanAct() const
{
	if (bIsDead)
	{
		return false;
	}

	if (bIsInDialogue)
	{
		return false;
	}

	if (bIsViewingMemoryEcho)
	{
		return false;
	}

	if (bIsMemoryJournalOpen)
	{
		return false;
	}

	if (bIsCheckpointMenuOpen)
	{
		return false;
	}

	return true;
}

void AWCCharacter::PlayLightAttackMontage(FName SectionName)
{
	if (LightAttackMontage == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;

	if (AnimInstance == nullptr)
	{
		return;
	}

	AnimInstance->Montage_Play(LightAttackMontage);
	AnimInstance->Montage_JumpToSection(SectionName, LightAttackMontage);
}

void AWCCharacter::StartAttackTimer(float Duration)
{
	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AWCCharacter::EndAttack, Duration,
									false);
}

void AWCCharacter::ShowAttackHitDebug(AActor* HitActor)
{
	if (!bShowAttackDebug)
	{
		return;
	}
	if (HitActor == nullptr)
	{
		return;
	}
	if (GEngine == nullptr)
	{
		return;
	}
	FString HitMessage = FString::Printf(TEXT("Hit Actor: %s"), *HitActor->GetName());
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, HitMessage);
}

void AWCCharacter::PlayAttackHitEffect(const FHitResult& HitResult)
{
	if (AttackHitEffect == nullptr)
	{
		return;
	}

	FVector SpawnLocation = HitResult.ImpactPoint;

	if (SpawnLocation.IsNearlyZero())
	{
		SpawnLocation = HitResult.Location;
	}

	UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(), AttackHitEffect, SpawnLocation, FRotator::ZeroRotator,
		FVector(AttackHitEffectScale, AttackHitEffectScale, AttackHitEffectScale));
}

void AWCCharacter::PlayPlayerHitFeedback()
{
	if (PlayerHurtSound)
	{
		UGameplayStatics::PlaySound2D(this, PlayerHurtSound);
	}

	if (PlayerHitFlashWidget)
	{
		PlayerHitFlashWidget->PlayHitFlash();
	}

	if (HitCameraShakeClass)
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetController());

		if (PlayerController && PlayerController->PlayerCameraManager)
		{

			PlayerController->PlayerCameraManager->StartCameraShake(HitCameraShakeClass);
		}
	}
}

void AWCCharacter::PlayComboFinisherFeedback(const FHitResult& HitResult)
{
	if (ComboFinisherHitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ComboFinisherHitSound, HitResult.ImpactPoint);
	}
	else
	{
		PlayAttackHitSound();
	}

	if (ComboFinisherHitEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ComboFinisherHitEffect,
												 HitResult.ImpactPoint,
												 HitResult.ImpactNormal.Rotation());
	}
	else
	{
		PlayAttackHitEffect(HitResult);
	}

	if (ComboFinisherCameraShakeClass)
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetController());

		if (PlayerController && PlayerController->PlayerCameraManager)
		{
			PlayerController->PlayerCameraManager->StartCameraShake(ComboFinisherCameraShakeClass);
		}
	}
}

void AWCCharacter::PlayHeavyAttackHitFeedback(const FHitResult& HitResult)
{
	if (HeavyAttackHitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HeavyAttackHitSound, HitResult.ImpactPoint);
	}
	else
	{
		PlayAttackHitSound();
	}

	if (HeavyAttackHitEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HeavyAttackHitEffect,
												 HitResult.ImpactPoint,
												 HitResult.ImpactNormal.Rotation());
	}
	else
	{
		PlayAttackHitEffect(HitResult);
	}

	if (HeavyAttackCameraShakeClass)
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetController());

		if (PlayerController && PlayerController->PlayerCameraManager)
		{
			PlayerController->PlayerCameraManager->StartCameraShake(HeavyAttackCameraShakeClass);
		}
	}
}

// ******************** Combo ********************

void AWCCharacter::ResetLightCombo()
{
	CurrentLightComboIndex = 0;

	GetWorldTimerManager().ClearTimer(ComboResetTimerHandle);

	ClearLightComboBuffer();
}

FName AWCCharacter::GetLightComboSectionName(int32 ComboIndex) const
{
	switch (ComboIndex)
	{
	case 0:
		return FName(TEXT("Light_1"));
	case 1:
		return FName(TEXT("Light_2"));
	case 2:
		return FName(TEXT("Light_3"));
	case 3:
		return FName(TEXT("Light_4"));
	default:
		return FName(TEXT("Light_1"));
	}
}

FPlayerAttackData AWCCharacter::GetLightComboAttackData(int32 ComboIndex) const
{
	if (LightComboAttackData.IsValidIndex(ComboIndex))
	{
		return LightComboAttackData[ComboIndex];
	}

	return LightAttackData;
}

void AWCCharacter::AdvancedLightCombo()
{
	CurrentLightComboIndex++;

	if (CurrentLightComboIndex >= MaxLightComboIndex)
	{
		CurrentLightComboIndex = 0;
	}
}

void AWCCharacter::StopCurrentAttackMontage(float BlendOutTime)
{
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;

	if (AnimInstance == nullptr || CurrentAttackMontage == nullptr)
	{
		return;
	}
	if (AnimInstance->Montage_IsPlaying(CurrentAttackMontage))
	{
		AnimInstance->Montage_Stop(BlendOutTime, CurrentAttackMontage);
	}
}

void AWCCharacter::EnterCombatState()
{
	bIsInCombat = true;

	GetWorldTimerManager().ClearTimer(CombatIdleTimerHandle);
	GetWorldTimerManager().SetTimer(CombatIdleTimerHandle, this, &AWCCharacter::ExitCombatState,
									CombatIdleDuration, false);
}

void AWCCharacter::ExitCombatState()
{
	bIsInCombat = false;

	GetWorldTimerManager().ClearTimer(CombatIdleTimerHandle);
}

// ******************** Combo Input ********************

void AWCCharacter::OpenLightComboWindow()
{
	if (!CanStartGroundAttack() || !IsAnyAttackActive())
	{
		return;
	}

	bCanBufferLightAttack = true;
}

void AWCCharacter::CloseLightComboWindow()
{
	bCanBufferLightAttack = false;
}

void AWCCharacter::ClearLightComboBuffer()
{
	bCanBufferLightAttack = false;
	bHasBufferedLightAttack = false;

	GetWorldTimerManager().ClearTimer(ComboWindowOpenTimerHandle);
	GetWorldTimerManager().ClearTimer(ComboWindowCloseTimerHandle);
}

void AWCCharacter::ConsumeBufferedLightAttack()
{
	if (!CanStartGroundAttack())
	{
		ClearLightComboBuffer();
		return;
	}

	if (!bHasBufferedLightAttack)
	{
		ClearLightComboBuffer();
		return;
	}

	bHasBufferedLightAttack = false;
	bCanBufferLightAttack = false;

	Attack();
}

void AWCCharacter::ApplyHitStop(AActor* HitActor, float Duration)
{
	if (!bEnableHitStop)
	{
		return;
	}
	if (Duration <= 0.0f)
	{
		return;
	}
	if (HitActor == nullptr)
	{
		return;
	}
	// 先清除上一次 Hit Stop。
	EndHitStop();

	HitStopTargetActor = HitActor;

	CustomTimeDilation = HitStopTimeDilation;
	HitActor->CustomTimeDilation = HitStopTimeDilation;

	GetWorldTimerManager().SetTimer(HitStopTimerHandle, this, &AWCCharacter::EndHitStop, Duration,
									false);
}

void AWCCharacter::EndHitStop()
{
	CustomTimeDilation = 1.0f;

	if (HitStopTargetActor.IsValid())
	{
		HitStopTargetActor->CustomTimeDilation = 1.0f;
	}

	HitStopTargetActor = nullptr;

	GetWorldTimerManager().ClearTimer(HitStopTimerHandle);
}

// ******************** Lock-On ********************

void AWCCharacter::ToggleLockOn()
{
	if (!CanAct())
	{
		return;
	}
	if (bIsLockedOn)
	{
		UnlockTarget();
		return;
	}
	LockOnToTarget();
}

void AWCCharacter::LockOnToTarget()
{
	AGhostEnemy* Target = FindBestLockOnTarget();

	if (Target == nullptr)
	{
		if (GEngine && bShowAttackDebug)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow,
											 TEXT("No Lock-on target found"));
		}
		return;
	}

	CurrentLockOnTarget = Target;
	bIsLockedOn = true;

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = LockOnWalkSpeed;

	if (GEngine && bShowAttackDebug)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, TEXT("Locked On"));
	}
}

void AWCCharacter::UnlockTarget()
{
	CurrentLockOnTarget = nullptr;
	bIsLockedOn = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;

	if (GEngine && bShowAttackDebug)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, TEXT("Lock-On Released"));
	}
}

bool AWCCharacter::IsLockOnTargetValid() const
{
	if (!bIsLockedOn || CurrentLockOnTarget == nullptr)
	{
		return false;
	}
	if (!IsValid(CurrentLockOnTarget))
	{
		return false;
	}
	if (CurrentLockOnTarget->GetIsDead())
	{
		return false;
	}

	const float Distance =
		FVector::Dist(GetActorLocation(), CurrentLockOnTarget->GetActorLocation());

	if (Distance > LockOnBreakDistance)
	{
		return false;
	}

	return true;
}

void AWCCharacter::UpdateLockOn(float DeltaTime)
{
	if (!bIsLockedOn)
	{
		return;
	}

	if (!IsLockOnTargetValid())
	{
		UnlockTarget();
		return;
	}

	if (Controller == nullptr || CurrentLockOnTarget == nullptr)
	{
		return;
	}

	FVector TargetLocation = CurrentLockOnTarget->GetActorLocation();
	FVector PlayerLocation = GetActorLocation();

	FVector DirectionToTarget = TargetLocation - PlayerLocation;
	DirectionToTarget.Z = 0.0f;

	if (DirectionToTarget.IsNearlyZero())
	{
		return;
	}

	const FRotator TargetRotation = DirectionToTarget.Rotation();

	const FRotator CurrentRotation = GetActorRotation();

	const FRotator NewRotation =
		FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, LockOnRotationInterpSpeed);

	SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));

	FRotator ControlRotation = Controller->GetControlRotation();
	ControlRotation.Yaw = NewRotation.Yaw;
	Controller->SetControlRotation(ControlRotation);
}

AGhostEnemy* AWCCharacter::FindBestLockOnTarget() const
{
	UWorld* World = GetWorld();

	if (World == nullptr)
	{
		return nullptr;
	}

	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(World, AGhostEnemy::StaticClass(), FoundEnemies);

	AGhostEnemy* BestTarget = nullptr;
	float BestScore = TNumericLimits<float>::Max();

	FVector PlayerLocation = GetActorLocation();

	FVector CameraForward = GetActorForwardVector();

	if (FollowCamera)
	{
		CameraForward = FollowCamera->GetForwardVector();
	}

	for (AActor* Actor : FoundEnemies)
	{
		AGhostEnemy* Enemy = Cast<AGhostEnemy>(Actor);

		if (Enemy == nullptr)
		{
			continue;
		}
		if (!IsValid(Enemy))
		{
			continue;
		}
		if (Enemy->GetIsDead())
		{
			continue;
		}

		FVector ToEnemy = Enemy->GetActorLocation() - PlayerLocation;
		const float Distance = ToEnemy.Size();

		if (Distance > LockOnRadius)
		{
			continue;
		}

		ToEnemy.Normalize();

		const float CameraDot = FVector::DotProduct(CameraForward, ToEnemy);

		if (CameraDot < LockOnMinCameraDot)
		{
			continue;
		}

		const float Score = Distance * (1.0f - CameraDot + 0.1f);

		if (Score < BestScore)
		{
			BestScore = Score;
			BestTarget = Enemy;
		}
	}

	return BestTarget;
}

void AWCCharacter::FaceLockOnTargetInstantly()
{
	if (!bIsLockedOn || CurrentLockOnTarget == nullptr)
	{
		return;
	}

	FVector DirectionToTarget = CurrentLockOnTarget->GetActorLocation() - GetActorLocation();

	DirectionToTarget.Z = 0.0f;

	if (DirectionToTarget.IsNearlyZero())
	{
		return;
	}

	SetActorRotation(DirectionToTarget.Rotation());
}

void AWCCharacter::SetCurrentCheckpointID(
	FName NewCheckpointID)
{
	if (NewCheckpointID.IsNone())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"SetCurrentCheckpointID rejected: "
				"Checkpoint ID is None."
			)
		);

		return;
	}

	if (CurrentCheckpointID ==
		NewCheckpointID)
	{
		return;
	}

	CurrentCheckpointID =
		NewCheckpointID;

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"Player Runtime Checkpoint set to [%s]."
		),
		*CurrentCheckpointID.ToString()
	);
}

FName AWCCharacter::
GetCurrentCheckpointID() const
{
	return CurrentCheckpointID;
}

bool AWCCharacter::ApplySavedCheckpointState(
	FName SavedCheckpointID,
	const FTransform& ResumeTransform)
{
	if (SavedCheckpointID.IsNone())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Player Checkpoint Restore failed: "
				"Checkpoint ID is None."
			)
		);

		return false;
	}

	UCharacterMovementComponent*
		MovementComponent =
		GetCharacterMovement();

	if (!IsValid(MovementComponent))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Player Checkpoint Restore failed: "
				"CharacterMovement is invalid."
			)
		);

		return false;
	}

	/*
	* 清理跳跃与上一位置遗留的移动输入。
	*/
	StopJumping();
	ConsumeMovementInputVector();

	/*
	* TeleportPhysics 不会自动替我们清除速度，
	* 所以在移动前后都显式停止 Movement。
	*/
	MovementComponent
		->StopMovementImmediately();

	FRotator TargetRotation =
		ResumeTransform.Rotator();

	TargetRotation.Pitch = 0.0f;
	TargetRotation.Roll = 0.0f;
	TargetRotation.Normalize();

	const bool bMoveSucceeded =
		SetActorLocationAndRotation(
			ResumeTransform.GetLocation(),
			TargetRotation,
			false,
			nullptr,
			ETeleportType::TeleportPhysics
		);

	if (!bMoveSucceeded)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Player Checkpoint Restore failed: "
				"SetActorLocationAndRotation "
				"returned false."
			)
		);

		return false;
	}

	MovementComponent
		->StopMovementImmediately();

	MovementComponent->SetMovementMode(
		MOVE_Walking
	);

	if (AController* PlayerController =
		GetController())
	{
		PlayerController->SetControlRotation(
			TargetRotation
		);
	}

	CurrentCheckpointID =
		SavedCheckpointID;

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"Player silently restored to "
			"Checkpoint [%s] at %s."
		),
		*CurrentCheckpointID.ToString(),
		*GetActorLocation().ToCompactString()
	);

	return true;
}

bool AWCCharacter::UnlockCheckpoint(FName CheckpointID)
{
	if (CheckpointID.IsNone())
	{
		return false;
	}

	if (UnlockedCheckpointIDs.Contains(
		CheckpointID))
	{
		return false;
	}

	UnlockedCheckpointIDs.Add(
		CheckpointID
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"Player unlocked Checkpoint [%s]."
		),
		*CheckpointID.ToString()
	);

	return true;
}

bool AWCCharacter::HasUnlockedCheckpoint(FName CheckpointID) const
{
	return !CheckpointID.IsNone() &&
		UnlockedCheckpointIDs.Contains(
			CheckpointID
		);
}

const TArray<FName>&
AWCCharacter::GetUnlockedCheckpointIDs()
const
{
	return UnlockedCheckpointIDs;
}

void AWCCharacter::
ApplyRuntimeCheckpointProgress(
	FName NewCurrentCheckpointID,
	const TArray<FName>&
	NewUnlockedCheckpointIDs)
{
	CurrentCheckpointID =
		NewCurrentCheckpointID;

	UnlockedCheckpointIDs.Reset(
		NewUnlockedCheckpointIDs.Num()
	);

	TSet<FName> SeenIDs;

	for (const FName CheckpointID :
	NewUnlockedCheckpointIDs)
	{
		if (CheckpointID.IsNone() ||
			SeenIDs.Contains(CheckpointID))
		{
			continue;
		}

		SeenIDs.Add(CheckpointID);
		UnlockedCheckpointIDs.Add(
			CheckpointID
		);
	}
}

bool AWCCharacter::CanUseCheckpoint() const
{
	const UCharacterMovementComponent*
		MovementComponent =
		GetCharacterMovement();

	return
		!bIsDead &&
		!bIsInDialogue &&
		!bIsViewingMemoryEcho &&
		!bIsMemoryJournalOpen &&
		!bIsCheckpointMenuOpen &&
		!bIsAttacking &&
		!bIsInCombat &&
		!bIsLockedOn &&
		MovementComponent &&
		MovementComponent->IsMovingOnGround();
}

bool AWCCharacter::OpenCheckpointMenu(
	AWCPlayerCheckpoint* SourceCheckpoint,
	AWCStoryPersistenceCoordinator* Coordinator)
{
	if (bIsDead ||
		bIsInDialogue ||
		bIsViewingMemoryEcho ||
		bIsMemoryJournalOpen ||
		bIsCheckpointMenuOpen ||
		bIsAttacking ||
		bIsInCombat ||
		bIsLockedOn ||
		!IsValid(SourceCheckpoint) ||
		!IsValid(Coordinator))
	{
		return false;
	}

	APlayerController* PlayerController =
		Cast<APlayerController>(GetController());

	if (!IsValid(PlayerController))
	{
		return false;
	}

	TSubclassOf<UWCCheckpointMenuWidget>
		EffectiveWidgetClass =
			CheckpointMenuWidgetClass;

	if (!EffectiveWidgetClass)
	{
		EffectiveWidgetClass =
			UWCCheckpointMenuWidget::StaticClass();
	}

	UWCCheckpointMenuWidget* Widget =
		CreateWidget<UWCCheckpointMenuWidget>(
			PlayerController,
			EffectiveWidgetClass
		);

	if (!IsValid(Widget))
	{
		return false;
	}

	bIsCheckpointMenuOpen = true;
	ActiveCheckpointMenuWidget = Widget;

	if (UCharacterMovementComponent* MovementComponent =
		GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	StopJumping();
	HideInteractionPrompt();

	Widget->AddToViewport(35);
	Widget->InitializeCheckpointMenu(
		this,
		SourceCheckpoint,
		Coordinator
	);

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(
		Widget->TakeWidget()
	);
	InputMode.SetLockMouseToViewportBehavior(
		EMouseLockMode::DoNotLock
	);
	InputMode.SetHideCursorDuringCapture(false);

	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);

	return true;
}

void AWCCharacter::CloseCheckpointMenu()
{
	if (!bIsCheckpointMenuOpen &&
		!ActiveCheckpointMenuWidget)
	{
		return;
	}

	if (ActiveCheckpointMenuWidget)
	{
		ActiveCheckpointMenuWidget->RemoveFromParent();
	}

	ActiveCheckpointMenuWidget = nullptr;
	bIsCheckpointMenuOpen = false;

	APlayerController* PlayerController =
		Cast<APlayerController>(GetController());

	if (IsValid(PlayerController))
	{
		PlayerController->bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);

		if (!bIsDead &&
			!bIsInDialogue &&
			!bIsViewingMemoryEcho &&
			!bIsMemoryJournalOpen)
		{
			PlayerController->SetIgnoreMoveInput(false);
			PlayerController->SetIgnoreLookInput(false);
		}
	}

	if (!bIsDead && CurrentInteractable)
	{
		ShowInteractionPrompt(
			CurrentInteractable->GetInteractionPrompt()
		);
	}
}

bool AWCCharacter::GetIsCheckpointMenuOpen() const
{
	return bIsCheckpointMenuOpen;
}
