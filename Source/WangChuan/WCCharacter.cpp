// Fill out your copyright notice in the Description page of Project Settings.


#include "WCCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Controller.h"
#include "Math/RotationMatrix.h"

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

	if (UEnhancedInputComponent* EnhancedInput =
		Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(
			MoveAction,
			ETriggerEvent::Triggered,
			this,
			&AWCCharacter::Move);
		EnhancedInput->BindAction(
			LookAction,
			ETriggerEvent::Triggered,
			this,
			&AWCCharacter::Look);
		EnhancedInput->BindAction(
			JumpAction,
			ETriggerEvent::Triggered,
			this,
			&AWCCharacter::Jump);
		EnhancedInput->BindAction(
			JumpAction,
			ETriggerEvent::Completed,
			this,
			&AWCCharacter::StopJumping);
		EnhancedInput->BindAction(
			InteractAction,
			ETriggerEvent::Started,
			this,
			&AWCCharacter::Interact);
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