// Copyright Epic Games, Inc. All Rights Reserved.

#include "WangChuanCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

// ******************** Construction ********************

AWangChuanCharacter::AWangChuanCharacter()
{
	// 设置角色碰撞胶囊尺寸。
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// 控制器旋转仅影响摄像机，不直接旋转角色。
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 配置角色移动方向与旋转速度。
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// 以下移动参数可在角色 Blueprint 中调整，无需重新编译。
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// 创建可在发生碰撞时自动收缩的摄像机弹簧臂。
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// 创建跟随摄像机并挂接到弹簧臂末端。
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// 骨骼网格体与 Anim Blueprint 由派生的 ThirdPersonCharacter Blueprint 配置，
	// 避免在 C++ 中直接引用内容资产。
}

// ******************** Lifecycle ********************

void AWangChuanCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// ******************** Input ********************

void AWangChuanCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// 添加默认输入映射上下文。
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
					PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// 绑定输入动作。
	if (UEnhancedInputComponent* EnhancedInputComponent =
			Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{

		// 跳跃。
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this,
										   &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this,
										   &ACharacter::StopJumping);

		// 移动。
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this,
										   &AWangChuanCharacter::Move);

		// 视角。
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this,
										   &AWangChuanCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error,
			   TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."),
			   *GetNameSafe(this));
	}
}

void AWangChuanCharacter::Move(const FInputActionValue& Value)
{
	// 输入值为二维向量。
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// 根据控制器朝向计算水平移动方向。
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AWangChuanCharacter::Look(const FInputActionValue& Value)
{
	// 输入值为二维向量。
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// 将水平和垂直分量分别应用到控制器。
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
