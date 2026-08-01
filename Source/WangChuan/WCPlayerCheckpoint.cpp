#include "WCPlayerCheckpoint.h"

#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

#include "WCCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(
	LogWCPlayerCheckpoint,
	Log,
	All
);

AWCPlayerCheckpoint::AWCPlayerCheckpoint()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot =
		CreateDefaultSubobject<USceneComponent>(
			TEXT("SceneRoot")
		);

	SetRootComponent(SceneRoot);

	ActivationBox =
		CreateDefaultSubobject<UBoxComponent>(
			TEXT("ActivationBox")
		);

	ActivationBox->SetupAttachment(SceneRoot);

	ActivationBox->SetBoxExtent(
		FVector(100.0f, 100.0f, 100.0f)
	);

	ActivationBox->SetCollisionEnabled(
		ECollisionEnabled::QueryOnly
	);

	ActivationBox->SetCollisionResponseToAllChannels(
		ECR_Ignore
	);

	ActivationBox->SetCollisionResponseToChannel(
		ECC_Pawn,
		ECR_Overlap
	);

	ActivationBox->SetGenerateOverlapEvents(true);

	/*
	* AddDynamic 会将函数指针表达式字符串化为 UFUNCTION 名称。
	* 不要在作用域运算符与函数名之间换行或插入空白，
	* 否则运行时可能尝试绑定带前导空格的错误函数名。
	*/
	ActivationBox->OnComponentBeginOverlap.AddDynamic(
		this,
		&AWCPlayerCheckpoint::HandleActivationBoxBeginOverlap
	);

	ResumeArrow =
		CreateDefaultSubobject<UArrowComponent>(
			TEXT("ResumeArrow")
		);

	ResumeArrow->SetupAttachment(SceneRoot);

	ResumeArrow->SetRelativeLocation(
		FVector::ZeroVector
	);

	ResumeArrow->ArrowSize = 1.5f;
	ResumeArrow->ArrowColor = FColor::Green;
}

void AWCPlayerCheckpoint::BeginPlay()
{
	Super::BeginPlay();

	if (CheckpointID.IsNone())
	{
		UE_LOG(
			LogWCPlayerCheckpoint,
			Error,
			TEXT(
				"Player Checkpoint Actor [%s] "
				"has CheckpointID None."
			),
			*GetName()
		);
	}
}

FName AWCPlayerCheckpoint::
GetCheckpointID() const
{
	return CheckpointID;
}

bool AWCPlayerCheckpoint::
GetIsDefaultCheckpoint() const
{
	return bIsDefaultCheckpoint;
}

bool AWCPlayerCheckpoint::
BuildSafeResumeTransform(
	const AWCCharacter* Player,
	FTransform& OutResumeTransform
) const
{
	OutResumeTransform =
		FTransform::Identity;

	if (!IsValid(Player) ||
		!IsValid(ResumeArrow) ||
		!GetWorld())
	{
		return false;
	}

	const UCapsuleComponent* Capsule =
		Player->GetCapsuleComponent();

	if (!IsValid(Capsule))
	{
		return false;
	}

	const FVector ResumeLocation =
		ResumeArrow->GetComponentLocation();

	const FVector TraceStart =
		ResumeLocation +
		FVector::UpVector *
		GroundTraceUpDistance;

	const FVector TraceEnd =
		ResumeLocation -
		FVector::UpVector *
		GroundTraceDownDistance;

	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(
			WCCheckpointGroundTrace
		),
		false,
		this
	);

	QueryParams.AddIgnoredActor(Player);

	FHitResult GroundHit;

	const bool bFoundGround =
		GetWorld()->LineTraceSingleByChannel(
			GroundHit,
			TraceStart,
			TraceEnd,
			ECC_Visibility,
			QueryParams
		);

	if (!bFoundGround ||
		!GroundHit.bBlockingHit)
	{
		UE_LOG(
			LogWCPlayerCheckpoint,
			Error,
			TEXT(
				"Checkpoint [%s] could not find "
				"valid ground below ResumeArrow."
			),
			*CheckpointID.ToString()
		);

		return false;
	}

	const float CapsuleHalfHeight =
		Capsule
		->GetScaledCapsuleHalfHeight();

	const FVector TargetActorLocation =
		GroundHit.ImpactPoint +
		FVector::UpVector *
		(CapsuleHalfHeight +
			GroundClearance);

	FRotator TargetRotation =
		ResumeArrow
		->GetComponentRotation();

	/*
	* Checkpoint 只决定玩家平面朝向。
	* 不把 Arrow 的 Pitch / Roll 应用给 Character。
	*/
	TargetRotation.Pitch = 0.0f;
	TargetRotation.Roll = 0.0f;
	TargetRotation.Normalize();

	OutResumeTransform = FTransform(
		TargetRotation,
		TargetActorLocation,
		FVector::OneVector
	);

	return true;
}

void AWCPlayerCheckpoint::
HandleActivationBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	AWCCharacter* Player =
		Cast<AWCCharacter>(OtherActor);

	if (!IsValid(Player) ||
		CheckpointID.IsNone())
	{
		return;
	}

	/*
	* 激活前确认 Resume Transform 本身有效。
	*
	* 避免玩家保存到一个无法安全恢复的位置。
	*/
	FTransform TestResumeTransform;

	if (!BuildSafeResumeTransform(
		Player,
		TestResumeTransform))
	{
		UE_LOG(
			LogWCPlayerCheckpoint,
			Error,
			TEXT(
				"Checkpoint [%s] activation "
				"rejected because its Resume "
				"Transform is invalid."
			),
			*CheckpointID.ToString()
		);

		return;
	}

	Player->SetCurrentCheckpointID(
		CheckpointID
	);

	UE_LOG(
		LogWCPlayerCheckpoint,
		Display,
		TEXT(
			"Player activated Checkpoint [%s]. "
			"No disk save was performed."
		),
		*CheckpointID.ToString()
	);

	if (bShowActivationDebug &&
		GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Green,
			FString::Printf(
				TEXT(
					"Checkpoint Activated: %s"
				),
				*CheckpointID.ToString()
			)
		);
	}
}
