// Fill out your copyright notice in the Description page of Project Settings.


#include "WCCharacter.h"
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
}

// Called when the game starts or when spawned
void AWCCharacter::BeginPlay()
{
	Super::BeginPlay();
	// 游戏开始 -》找到玩家 -》找到Enhanced Input -》加载IMC Default
	if (APlayerController* PlayerController =
		Cast<APlayerController>(GetController())) {
		if (UEnhancedInputLocalPlayerSubsystem * Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
				Subsystem->AddMappingContext(
					DefaultMappingContext, 0);
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

	void AWCCharacter::Attack() {
		// display attack debug
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Red,
				TEXT("Attack")
			);
		}
		if (LightAttackMontage) {
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

			if (AnimInstance) {
				AnimInstance->Montage_Play(LightAttackMontage);
			}
		}
		else {
			if (GEngine) {
				GEngine->AddOnScreenDebugMessage(
					-1,
					2.0f,
					FColor::Yellow,
					TEXT("LightAttackMontage is not assigned")
				);
			}
		}

		PerformAttackTrace();
	}

	void AWCCharacter::PerformAttackTrace() {

		FVector ForwardVector = GetActorForwardVector(); // Direction

		FVector Start = GetActorLocation() // Start point
			+ FVector(0.0f, 0.0f, 40.0f)
			+ ForwardVector * 60.0f;

		FVector End = Start + ForwardVector * AttackRange; // End point

		TArray<AActor*> ActorsToIgnore; // Ignore player it self 
		ActorsToIgnore.Add(this); 

		FHitResult HitResult;

		bool bHit = UKismetSystemLibrary::BoxTraceSingle( // only return the first thing hit
			this,
			Start,
			End,
			AttackBoxHalfSize,
			GetActorRotation(),
			UEngineTypes::ConvertToTraceType(ECC_Visibility), // Trace Channel
			false,
			ActorsToIgnore,
			EDrawDebugTrace::ForDuration, // Display the Box Trace 
			HitResult,
			true,
			FLinearColor::Red,
			FLinearColor::Green,
			2.0f
		);

		if (bHit) {
			AActor* HitActor = HitResult.GetActor();

			if (HitActor) {
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

			AGhostEnemy* GhostEnemy = Cast<AGhostEnemy>(HitActor);

			if (GhostEnemy) {
				GhostEnemy->TakeHit(AttackDamage);
			}
		}
	}

