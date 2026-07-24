#include "LanternPuzzlePiece.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#include "WCCharacter.h"

// Sets default values
ALanternPuzzlePiece::ALanternPuzzlePiece()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(
		TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	LanternMesh = CreateDefaultSubobject<UStaticMeshComponent>(
		TEXT("LanternMesh"));
	LanternMesh->SetupAttachment(SceneRoot);

	/*
	* 第一版本由 InteractionSphere 负责交互。
	* 
	* Placeholder Mesh 不阻玩家，
	* 避免三盏灯在灰盒阶段形成意外碰撞。
	*/
	LanternMesh->SetCollisionEnabled(
		ECollisionEnabled::NoCollision);

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(
		TEXT("InteractionSphere"));

	InteractionSphere->SetupAttachment(SceneRoot);

	InteractionSphere->SetSphereRadius(170.0f);

	InteractionSphere->SetCollisionEnabled(
		ECollisionEnabled::QueryOnly);

	InteractionSphere->SetCollisionResponseToAllChannels(
		ECR_Ignore);

	InteractionSphere->SetCollisionResponseToChannel(
		ECC_Pawn,
		ECR_Overlap
	);

	InteractionSphere->SetGenerateOverlapEvents(true);

	InteractionSphere->OnComponentBeginOverlap.AddDynamic(
		this,
		&ALanternPuzzlePiece::OnPlayerEnter
	);

	InteractionSphere->OnComponentEndOverlap.AddDynamic(
		this,
		&ALanternPuzzlePiece::OnPlayerExit
	);

	LanternLight = CreateDefaultSubobject<UPointLightComponent>(
		TEXT("LanternLight")
	);

	LanternLight->SetupAttachment(SceneRoot);

	/*
	* 初始状态保持关闭。
	* 
	* 灯光强度，颜色和半径由 Blueprint 配置。
	*/
	LanternLight->SetVisibility(false);

	PieceID = 0;
	bInteractionEnabled = false;
	bIsLit = false;
}

void ALanternPuzzlePiece::BeginPlay()
{
	Super::BeginPlay();
	
	SetLanternLit(false);

	SetInteractionEnabled(
		bEnableInteractionOnBeginPlay
	);
}

void ALanternPuzzlePiece::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(
		LanternFeedbackTimerHandle
	);

	ClearPlayerInteractionIfNeeded();

	Super::EndPlay(EndPlayReason);
}

void ALanternPuzzlePiece::Interact()
{
	if (!bInteractionEnabled)
	{
		return;
	}

	AWCCharacter* Player =
		Cast<AWCCharacter>(
			UGameplayStatics::GetPlayerCharacter(
				this,
				0
			)
		);

	if (!Player)
	{
		return;
	}

	if (Player->GetIsDead())
	{
		return;
	}

	/*
	* 一次玩家输入先产生本地反馈，
	* 再通知 Puzzle Controller。
	*/
	PlayLanternFeedback();

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Lantern Puzzle Piece [%d] interacted."
		),
		PieceID
	);

	OnLanternInteracted.Broadcast(this);
}

FString ALanternPuzzlePiece::GetInteractionPrompt()
{
	if (!bInteractionEnabled)
	{
		return FString();
	}

	return InteractionPrompt;
}

void ALanternPuzzlePiece::SetInteractionEnabled(
	bool bEnabled)
{
	/*
	* 即便状态没有变化，
	* Enabled 状态下也允许重新检查 Prompt。
	*/
	if (bInteractionEnabled == bEnabled)
	{
		if (bEnabled) 
		{
			RefreshPromptForOverlappingPlayer();
		}

		return;
	}

	bInteractionEnabled = bEnabled;

	if (!bInteractionEnabled)
	{
		ClearPlayerInteractionIfNeeded();
		return;
	}

	RefreshPromptForOverlappingPlayer();	
}

void ALanternPuzzlePiece::SetLanternLit(
	bool bNewLit)
{
	/*
	* 外部主动设置灯光状态时，
	* 应取消尚未结束的临时 Feedback Timer。
	* 
	* 例如谜题完成后调用 SetLanternLit(ture),
	* 旧 Timer 不应稍后再次把灯关闭。
	*/
	GetWorldTimerManager().ClearTimer(
		LanternFeedbackTimerHandle
	);

	bIsLit = bNewLit;

	if (LanternLight)
	{
		LanternLight->SetVisibility(bIsLit);
	}

	BP_OnLanternLitChanged(bIsLit);
}

void ALanternPuzzlePiece::PlayLanternFeedback()
{
	/*
	* Preview 阶段也需要调用本函数。
	* 因此这里不检查 bInteractionEnabled。
	*/
	SetLanternLit(true);

	PlayLanternTone();

	GetWorldTimerManager().SetTimer(
		LanternFeedbackTimerHandle,
		this,
		&ALanternPuzzlePiece::FinishLanternFeedback,
		FeedbackDuration,
		false
	);
}

int32 ALanternPuzzlePiece::GetPieceID() const
{
	return PieceID;
}

bool ALanternPuzzlePiece::GetIsLanternLit() const
{
	return bIsLit;
}

bool ALanternPuzzlePiece::GetIsInteractionEnabled() const
{
	return bInteractionEnabled;
}

void ALanternPuzzlePiece::OnPlayerEnter(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bInteractionEnabled)
	{
		return;
	}

	AWCCharacter* Player =
		Cast<AWCCharacter>(OtherActor);

	if (!Player)
	{
		return;
	}

	if (Player->GetIsDead())
	{
		return;
	}

	/*
	* 保持与现有 Echo Relic 相同的交互模式。
	*/
	Player->CurrentInteractable = this;

	Player->ShowInteractionPrompt(
		GetInteractionPrompt()
	);
}

void ALanternPuzzlePiece::OnPlayerExit(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	AWCCharacter* Player =
		Cast<AWCCharacter>(OtherActor);

	if (!Player)
	{
		return;
	}

	/*
	* 只清理本灯持有的 CurrentInteractable,
	* 避免离开一盏灯时清除另一盏灯的 Prompt。
	*/
	if (Player->CurrentInteractable != this)
	{
		return;
	}

	Player->CurrentInteractable = nullptr;
	Player->HideInteractionPrompt();
}

void ALanternPuzzlePiece::ClearPlayerInteractionIfNeeded()
{
	AWCCharacter* Player =
		Cast<AWCCharacter>(
			UGameplayStatics::GetPlayerCharacter(
				this,
				0
			)
		);

	if (!Player)
	{
		return;
	}

	if (Player->CurrentInteractable != this)
	{
		return;
	}

	Player->CurrentInteractable = nullptr;
	Player->HideInteractionPrompt();
}

void ALanternPuzzlePiece::RefreshPromptForOverlappingPlayer()
{
	if (!bInteractionEnabled)
	{
		return;
	}

	if (!InteractionSphere)
	{
		return;
	}

	AWCCharacter* Player =
		Cast<AWCCharacter>(
			UGameplayStatics::GetPlayerCharacter(
				this,
				0
			)
		);

	if (!Player)
	{
		return;
	}

	if (Player->GetIsDead())
	{
		return;
	}

	if (!InteractionSphere
		->IsOverlappingActor(Player))
	{
		return;
	}

	/*
	* 状态变化时不主动覆盖另一个有效交互五
	*/
	if (Player->CurrentInteractable &&
		Player->CurrentInteractable != this)
	{
		return;
	}

	Player->CurrentInteractable = this;

	Player->ShowInteractionPrompt(
		GetInteractionPrompt()
	);
}

void ALanternPuzzlePiece::PlayLanternTone()
{
	if (!LanternTone)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		this,
		LanternTone,
		GetActorLocation()
	);
}

void ALanternPuzzlePiece::FinishLanternFeedback()
{
	SetLanternLit(false);
}


