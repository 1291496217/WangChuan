// Fill out your copyright notice in the Description page of Project Settings.


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
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "Camera/PlayerCameraManager.h"

// Sets default values
AWCCharacter::AWCCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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

	// Do not rotate the character directly with the controller
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	// Rotate the character toward movement direction.
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// Roate speed
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	bShowAttackDebug = false;
}

// Called when the game starts or when spawned
void AWCCharacter::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	bIsDead = false;

	if (APlayerController* PlayerController =
		Cast<APlayerController>(GetController())) {
		if (UEnhancedInputLocalPlayerSubsystem * Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
				Subsystem->AddMappingContext(
					DefaultMappingContext, 0);
		}
	}

	if (PlayerHUDClass) {
		APlayerController* PlayerController =
			Cast<APlayerController>(GetController());

		if (PlayerController) {
			PlayerHUDWidget = CreateWidget<UUserWidget>(
				PlayerController,
				PlayerHUDClass
			);

			if (PlayerHUDWidget) {
				PlayerHUDWidget->AddToViewport();
			}
		}
	}

	if (PlayerHitFlashWidgetClass) {
		APlayerController* PlayerController =
			Cast<APlayerController>(GetController());

		if (PlayerController) {
			PlayerHitFlashWidget = CreateWidget<UPlayerHitFlashWidget>(
				PlayerController,
				PlayerHitFlashWidgetClass
			);

			if (PlayerHitFlashWidget) {
				PlayerHitFlashWidget->AddToViewport(10);
			}
		}
	}
}

// Called every frame
void AWCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AWCCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent) 
	{
		if (MoveAction) {
			EnhancedInputComponent->BindAction(
				MoveAction,
				ETriggerEvent::Triggered,
				this,
				&AWCCharacter::Move);
		}
		if (LookAction) {
			EnhancedInputComponent->BindAction(
				LookAction,
				ETriggerEvent::Triggered,
				this,
				&AWCCharacter::Look);
		}
		if (JumpAction) {
			EnhancedInputComponent->BindAction(
				JumpAction,
				ETriggerEvent::Triggered,
				this,
				&AWCCharacter::Jump);
			EnhancedInputComponent->BindAction(
				JumpAction,
				ETriggerEvent::Completed,
				this,
				&AWCCharacter::StopJumping);
		}
		if (InteractAction) {
			EnhancedInputComponent->BindAction(
				InteractAction,
				ETriggerEvent::Started,
				this,
				&AWCCharacter::Interact);
		}
		if (JournalAction) {
			EnhancedInputComponent->BindAction(
				JournalAction,
				ETriggerEvent::Started,
				this,
				&AWCCharacter::ShowMemoryJournal);
		}
		if (AttackAction) {
			EnhancedInputComponent->BindAction(
				AttackAction,
				ETriggerEvent::Started,
				this,
				&AWCCharacter::Attack);
		}
		if (HeavyAttackAction) {
			EnhancedInputComponent->BindAction(
				HeavyAttackAction,
				ETriggerEvent::Started,
				this,
				&AWCCharacter::HeavyAttack);
		}
	}
}

	void AWCCharacter::Move(const FInputActionValue & Value)
	{
		FVector2D MovementVector =
			Value.Get<FVector2D>();

		if (Controller != nullptr)
		{
			const FRotator Rotation =
				Controller->GetControlRotation();

			const FRotator YawRotation(
				0,
				Rotation.Yaw,
				0);

			const FVector ForwardDirection =
				FRotationMatrix(YawRotation)
				.GetUnitAxis(EAxis::X);

			const FVector RightDirection =
				FRotationMatrix(YawRotation)
				.GetUnitAxis(EAxis::Y);

			AddMovementInput(
				ForwardDirection,
				MovementVector.Y);

			AddMovementInput(
				RightDirection,
				MovementVector.X);
		}
	}

	void AWCCharacter::Look(
		const FInputActionValue & Value)
	{
		FVector2D LookAxisVector =
			Value.Get<FVector2D>();

		if (Controller != nullptr)
		{
			AddControllerYawInput(
				LookAxisVector.X);

			AddControllerPitchInput(
				LookAxisVector.Y);
		}
	}

	void AWCCharacter::Interact()
	{
		if (CurrentInteractable)
		{
			CurrentInteractable->Interact();
		}
	}

	void AWCCharacter::ShowInteractionPrompt(
		const FString& Prompt) {
		CurrentPrompt = Prompt;
		GEngine->AddOnScreenDebugMessage(
			1,
			9999.0f,
			FColor::Green,
			CurrentPrompt);
	}

	void AWCCharacter::HideInteractionPrompt() {
		GEngine->RemoveOnScreenDebugMessage(1);
	}

	void AWCCharacter::ShowMemoryJournal() {
		FString StatusText = TEXT("未完成");

		if (CollectedFragments.Num() >= 3) {
			StatusText = TEXT("已解锁");
		}

		FString JournalText = FString::Printf(
			TEXT(
				"记忆日志\n\n"
				"《安静的孩子》\n"
				"碎片：%d / 3\n"
				"状态：%s"
			),
			CollectedFragments.Num(),
			*StatusText
		);

		if (CollectedFragments.Num() >= 3) {
			JournalText += TEXT(
				"\n\n"
				"妈妈....."
				"我今天也很乖。\n"
				"要快点来接我哦。"
			);
		}
		
		GEngine->AddOnScreenDebugMessage(
			2,
			8.0f,
			FColor::Cyan,
			JournalText);
	}

//*****************Combat********************
	void AWCCharacter::Attack() {
		if (!CanAct()) {
			return;
		}
		if (bIsAttacking) {
			if (bCanBufferLightAttack) {
				bHasBufferedLightAttack = true;
			}
			return;
		}

		bIsAttacking = true;
		bHasProcessedAttackHit = false;
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

		const float WindowOpenTime =
			FMath::Clamp(CurrentAttackData.Duration * ComboWindowOpenRatio,
				0.0f,
				CurrentAttackData.Duration);

		GetWorldTimerManager().SetTimer(
			ComboWindowOpenTimerHandle,
			this,
			&AWCCharacter::OpenLightComboWindow,
			WindowOpenTime,
			false
		);

		AdvancedLightCombo();

		GetWorldTimerManager().ClearTimer(ComboResetTimerHandle);
		GetWorldTimerManager().SetTimer(
			ComboResetTimerHandle,
			this,
			&AWCCharacter::ResetLightCombo,
			ComboResetTime,
			false
		);
	}

	void AWCCharacter::HeavyAttack() {
		if (!CanAct()) {
			return;
		}

		if (bIsAttacking) {
			return;
		}

		bIsAttacking = true;
		bHasProcessedAttackHit = false;
		bIsCurrentAttackFinisher = false;

		ClearLightComboBuffer();
		EnterCombatState();
		ResetLightCombo();

		CurrentAttackData = HeavyAttackData;

		UAnimInstance* AnimInstance =
			GetMesh()
			? GetMesh()->GetAnimInstance()
			: nullptr;

		CurrentAttackMontage = HeavyAttackMontage;

		if (AnimInstance && HeavyAttackMontage) {
			AnimInstance->Montage_Play(HeavyAttackMontage);
		}

		StartAttackTimer(CurrentAttackData.Duration);
	}

	void AWCCharacter::PerformCurrentAttackTrace() {
		FVector ForwardVector = GetActorForwardVector();

		FVector Start = GetActorLocation()
			+ FVector(0.0f, 0.0f, 40.0f)
			+ ForwardVector * 60.0f;

		FVector End = Start + ForwardVector * CurrentAttackData.Range;

		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(this);

		FHitResult HitResult;

		EDrawDebugTrace::Type DebugType = bShowAttackDebug
			? EDrawDebugTrace::ForDuration
			: EDrawDebugTrace::None;

		bool bHit = UKismetSystemLibrary::BoxTraceSingle(
			this,
			Start,
			End,
			CurrentAttackData.BoxHalfSize,
			GetActorRotation(),
			UEngineTypes::ConvertToTraceType(ECC_Visibility),
			false,
			ActorsToIgnore,
			DebugType,
			HitResult,
			true,
			FLinearColor::Red,
			FLinearColor::Green,
			2.0f
		);

		bool bHitEnemy = false;

		if (bHit) {
			AActor* HitActor = HitResult.GetActor();

			ShowAttackHitDebug(HitActor);

			AGhostEnemy* GhostEnemy = Cast<AGhostEnemy>(HitActor);

			if (GhostEnemy) {
				FVector HitDirection = GetActorForwardVector();

				GhostEnemy->TakeHit(
					CurrentAttackData.Damage,
					HitDirection,
					CurrentAttackData.KnockbackStrength
				);

				if (bIsCurrentAttackFinisher) {
					PlayComboFinisherFeedback(HitResult);
				}
				else {
					PlayAttackHitEffect(HitResult);
				}

				bHitEnemy = true;
			}
		}

		if (bHitEnemy) {
			if (!bIsCurrentAttackFinisher) {
				PlayAttackHitSound();
			}
		}
		else {
			PlayAttackWhiffSound();
		}
	}

	void AWCCharacter::OnPlayerAttackHitNotify() {
		if (!CanAct()) {
			return;
		}
		if (!bIsAttacking) {
			return;
		}
		if (bHasProcessedAttackHit) {
			return;
		}

		bHasProcessedAttackHit = true;

		PerformCurrentAttackTrace();
	}

	void AWCCharacter::EndAttack() {
		StopCurrentAttackMontage();

		bIsAttacking = false;
		bHasProcessedAttackHit = false;
		CurrentAttackMontage = nullptr;

		bCanBufferLightAttack = false;
		GetWorldTimerManager().ClearTimer(ComboWindowOpenTimerHandle);
		GetWorldTimerManager().ClearTimer(ComboWindowCloseTimerHandle);

		if (bHasBufferedLightAttack) {
			ConsumeBufferedLightAttack();
		}
		else {
			bHasBufferedLightAttack = false;
			bIsCurrentAttackFinisher = false;
		}
	}

	void AWCCharacter::ReceiveDamage(float DamageAmount) {
		if (bIsDead) {
			return;
		}

		Health -= DamageAmount;

		if (Health < 0.0f) {
			Health = 0.0f;
		}

		if (Health <= 0.0f) {
			Die();
			return;
		}

		PlayPlayerHitFeedback();

	}

	// Combat Audios
	void AWCCharacter::PlayAttackHitSound() {
		if (AttackHitSound == nullptr) {
			return;
		}

		UGameplayStatics::PlaySoundAtLocation(
			this,
			AttackHitSound,
			GetActorLocation()
		);
	}

	void AWCCharacter::PlayAttackWhiffSound() {
		if (AttackWhiffSound == nullptr) {
			return;
		}

		UGameplayStatics::PlaySoundAtLocation(
			this,
			AttackWhiffSound,
			GetActorLocation()
		);
	}

//*******************Getters**********************************
	float AWCCharacter::GetHealth() const {
		return Health;
	}

	float AWCCharacter::GetHealthPercent() const {
		if (MaxHealth <= 0.0f) {
			return 0.0f;
		}
		return Health / MaxHealth;
	}

	bool AWCCharacter::GetIsDead() const {
		return bIsDead;
	}

	bool AWCCharacter::GetIsInCombat() const {
		return bIsInCombat;
	}


	void AWCCharacter::Die() {
		if (bIsDead) {
			return;
		}

		bIsDead = true;
		bIsAttacking = false;
		bHasProcessedAttackHit = false;
		bIsCurrentAttackFinisher = false;

		if (PlayerDeathSound) {
			UGameplayStatics::PlaySound2D(
				this,
				PlayerDeathSound
			);
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

		APlayerController* PlayerController =
			Cast<APlayerController>(GetController());

		if (PlayerController) {
			DisableInput(PlayerController);
		}

		// Ragdoll
		GetCapsuleComponent()->SetCollisionEnabled(
			ECollisionEnabled::NoCollision
		);

		USkeletalMeshComponent* CharacterMesh = GetMesh();

		if (CharacterMesh) {
			CharacterMesh->SetCollisionProfileName(
				TEXT("Ragdoll")
			);

			CharacterMesh->SetCollisionEnabled(
				ECollisionEnabled::QueryAndPhysics
			);

			CharacterMesh->SetSimulatePhysics(true);

			CharacterMesh->WakeAllRigidBodies();
		}
	}

	bool AWCCharacter::CanAct() const {
		if (bIsDead) {
			return false;
		}
		return true;
	}

	void AWCCharacter::PlayLightAttackMontage(FName SectionName) {
		if (LightAttackMontage == nullptr) {
			return;
		}

		UAnimInstance* AnimInstance = GetMesh()
			? GetMesh()->GetAnimInstance()
			: nullptr;

		if (AnimInstance == nullptr) {
			return;
		}

		AnimInstance->Montage_Play(LightAttackMontage);
		AnimInstance->Montage_JumpToSection(SectionName, LightAttackMontage);
	}

	void AWCCharacter::StartAttackTimer(float Duration) {
		GetWorldTimerManager().SetTimer(
			AttackTimerHandle,
			this,
			&AWCCharacter::EndAttack,
			Duration,
			false
		);
	}

	void AWCCharacter::ShowAttackHitDebug(AActor* HitActor) {
		if (!bShowAttackDebug) {
			return;
		}
		if (HitActor == nullptr) {
			return;
		}
		if (GEngine == nullptr) {
			return;
		}
		FString HitMessage = FString::Printf(
			TEXT("Hit Actor: %s"),
			*HitActor->GetName()
		);
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Green,
			HitMessage
		);
	}

	void AWCCharacter::PlayAttackHitEffect(
		const FHitResult& HitResult) {
		if (AttackHitEffect == nullptr) {
			return;
		}

		FVector SpawnLocation = HitResult.ImpactPoint;

		if (SpawnLocation.IsNearlyZero()) {
			SpawnLocation = HitResult.Location;
		}

		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			AttackHitEffect,
			SpawnLocation,
			FRotator::ZeroRotator,
			FVector(
				AttackHitEffectScale,
				AttackHitEffectScale,
				AttackHitEffectScale
			)
		);
	}

	void AWCCharacter::PlayPlayerHitFeedback() {
		if (PlayerHurtSound) {
			UGameplayStatics::PlaySound2D(
				this,
				PlayerHurtSound
			);
		}

		if (PlayerHitFlashWidget) {
			PlayerHitFlashWidget->PlayHitFlash();
		}

		if (HitCameraShakeClass) {
			APlayerController* PlayerController =
				Cast<APlayerController>(GetController());

			if (PlayerController &&
				PlayerController->PlayerCameraManager) {

				PlayerController->PlayerCameraManager->StartCameraShake(
					HitCameraShakeClass
				);
			}
		}
	}

	void AWCCharacter::PlayComboFinisherFeedback(const FHitResult& HitResult) {
		if (ComboFinisherHitSound) {
			UGameplayStatics::PlaySoundAtLocation(
				this,
				ComboFinisherHitSound,
				HitResult.ImpactPoint
			);
		}
		else {
			PlayAttackHitSound();
		}

		if (ComboFinisherHitEffect) {
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ComboFinisherHitEffect,
				HitResult.ImpactPoint,
				HitResult.ImpactNormal.Rotation()
			);
		}
		else {
			PlayAttackHitEffect(HitResult);
		}

		if (ComboFinisherCameraShakeClass) {
			APlayerController* PlayerController =
				Cast<APlayerController>(GetController());

			if (PlayerController && PlayerController->PlayerCameraManager) {
				PlayerController->PlayerCameraManager->StartCameraShake(
					ComboFinisherCameraShakeClass
				);
			}
		}
	}

	// Combo Helpers
	void AWCCharacter::ResetLightCombo() {
		CurrentLightComboIndex = 0;

		GetWorldTimerManager().ClearTimer(ComboResetTimerHandle);

		ClearLightComboBuffer();
	}

	FName AWCCharacter::GetLightComboSectionName(int32 ComboIndex) const {
		switch (ComboIndex) {
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

	FPlayerAttackData AWCCharacter::GetLightComboAttackData(int32 ComboIndex) const {
		if (LightComboAttackData.IsValidIndex(ComboIndex)) {
			return LightComboAttackData[ComboIndex];
		}

		return LightAttackData;
	}

	void AWCCharacter::AdvancedLightCombo() {
		CurrentLightComboIndex++;

		if (CurrentLightComboIndex >= MaxLightComboIndex) {
			CurrentLightComboIndex = 0;
		}
	}

	void AWCCharacter::StopCurrentAttackMontage(float BlendOutTime) {
		UAnimInstance* AnimInstance = GetMesh()
			? GetMesh()->GetAnimInstance()
			: nullptr;

		if (AnimInstance == nullptr || CurrentAttackMontage == nullptr) {
			return;
		}
		if (AnimInstance->Montage_IsPlaying(CurrentAttackMontage)) {
			AnimInstance->Montage_Stop(BlendOutTime, CurrentAttackMontage);
		}
	}

	void AWCCharacter::EnterCombatState() {
		bIsInCombat = true;

		GetWorldTimerManager().ClearTimer(CombatIdleTimerHandle);
		GetWorldTimerManager().SetTimer(
			CombatIdleTimerHandle,
			this,
			&AWCCharacter::ExitCombatState,
			CombatIdleDuration,
			false
		);
	}

	void AWCCharacter::ExitCombatState() {
		bIsInCombat = false;

		GetWorldTimerManager().ClearTimer(CombatIdleTimerHandle);
	}

	// Combo Window / Input Buffer
	void AWCCharacter::OpenLightComboWindow() {
		if (!CanAct()) {
			return;
		}

		if (!bIsAttacking) {
			return;
		}

		bCanBufferLightAttack = true;
	}

	void AWCCharacter::CloseLightComboWindow() {
		bCanBufferLightAttack = false;
	}

	void AWCCharacter::ClearLightComboBuffer() {
		bCanBufferLightAttack = false;
		bHasBufferedLightAttack = false;

		GetWorldTimerManager().ClearTimer(ComboWindowOpenTimerHandle);
		GetWorldTimerManager().ClearTimer(ComboWindowCloseTimerHandle);
	}

	void AWCCharacter::ConsumeBufferedLightAttack() {
		if (!CanAct()) {
			ClearLightComboBuffer();
			return;
		}

		if (!bHasBufferedLightAttack) {
			ClearLightComboBuffer();
			return;
		}

		bHasBufferedLightAttack = false;
		bCanBufferLightAttack = false;

		Attack();
	}