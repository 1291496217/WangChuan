// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "WangChuanCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config = Game)
class AWangChuanCharacter : public ACharacter
{
	GENERATED_BODY()

	// ******************** Components ********************

	// 将摄像机定位在角色后方的弹簧臂。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera,
			  meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	// 跟随摄像机。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera,
			  meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	// ******************** Input ********************

	// 默认输入映射上下文。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input,
			  meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	// 跳跃输入动作。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input,
			  meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	// 移动输入动作。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input,
			  meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	// 视角输入动作。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input,
			  meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

public:
	// ******************** Construction ********************

	AWangChuanCharacter();

protected:
	// ******************** Input ********************

	// 处理移动输入。
	void Move(const FInputActionValue& Value);

	// 处理视角输入。
	void Look(const FInputActionValue& Value);

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ******************** Lifecycle ********************

	virtual void BeginPlay();

public:
	// ******************** Getters ********************

	// 返回 CameraBoom 子对象。
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const
	{
		return CameraBoom;
	}

	// 返回 FollowCamera 子对象。
	FORCEINLINE class UCameraComponent* GetFollowCamera() const
	{
		return FollowCamera;
	}
};
