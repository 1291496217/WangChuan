#include "LanternSequencePuzzle.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#include "LanternPuzzlePiece.h"
#include "WCCharacter.h"

// ******************** Construction ********************

ALanternSequencePuzzle::ALanternSequencePuzzle()
{
	PrimaryActorTick.bCanEverTick = false;

	ActivationBox =
		CreateDefaultSubobject<UBoxComponent>(
			TEXT("ActivationBox")
		);

	ActivationBox->SetupAttachment(
		SceneRoot
	);

	ActivationBox->SetBoxExtent(
		FVector(
			350.0f,
			350.0f,
			150.0f
		)
	);

	ActivationBox->SetCollisionEnabled(
		ECollisionEnabled::QueryOnly
	);

	ActivationBox
		->SetCollisionResponseToAllChannels(
			ECR_Ignore
		);

	ActivationBox
		->SetCollisionResponseToChannel(
			ECC_Pawn,
			ECR_Overlap
		);

	ActivationBox
		->SetGenerateOverlapEvents(true);

	CorrectSequence =
	{
		0,
		2,
		1
	};

	PuzzleState =
		ELanternPuzzleState::Dormant;

	CurrentPreviewIndex = 0;
	bConfigurationValid = false;
}

// ******************** Lifecycle ********************

void ALanternSequencePuzzle::BeginPlay()
{
	Super::BeginPlay();

	if (ActivationBox)
	{
		ActivationBox
			->OnComponentBeginOverlap
			.AddUniqueDynamic(
				this,
				&ALanternSequencePuzzle::OnPuzzleAreaEntered
			);
	}

	PuzzleState =
		ELanternPuzzleState::Dormant;

	CurrentPreviewIndex = 0;

	CurrentPlayerInput.Empty();

	bConfigurationValid =
		ValidatePuzzleConfiguration();

	/*
	* 每个 Lantern Piece 会在 BeginPlay 中初始化自身灯光状态。
	*
	* 此处不能调用 SetAllLanternsLit()：Actor 的 BeginPlay 顺序没有保证，
	* Puzzle Controller 可能先于 Lantern Blueprint 创建动态材质实例。
	*/

	SetLanternInteractionEnabled(false);

	if (!bConfigurationValid)
	{
		if (ActivationBox)
		{
			ActivationBox->SetCollisionEnabled(
				ECollisionEnabled::NoCollision
			);
		}

		return;
	}

	BindLanternDelegates();

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Lantern Puzzle [%s] initialized "
			"and is waiting for player entry."
		),
		*ObjectiveID.ToString()
	);
}

void ALanternSequencePuzzle::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	ClearPuzzleTimers();

	UnbindLanternDelegates();

	if (ActivationBox)
	{
		ActivationBox
			->OnComponentBeginOverlap
			.RemoveDynamic(
				this,
				&ALanternSequencePuzzle::OnPuzzleAreaEntered
			);
	}

	Super::EndPlay(EndPlayReason);
}

// ******************** Objectives ********************

void ALanternSequencePuzzle::StartPuzzle()
{
	if (!bConfigurationValid)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Lantern Puzzle [%s] cannot start "
				"because its configuration is invalid."
			),
			*ObjectiveID.ToString()
		);

		return;
	}

	if (GetIsObjectiveComplete())
	{
		return;
	}

	if (PuzzleState !=
		ELanternPuzzleState::Dormant)
	{
		return;
	}

	ActivateObjective();

	if (!GetIsObjectiveActive())
	{
		return;
	}

	if (ActivationBox)
	{
		/*
		* 第一次启动后关闭 Trigger，
		* 避免玩家来回进出重复开始 Preview。
		*/
		ActivationBox->SetCollisionEnabled(
			ECollisionEnabled::NoCollision
		);
	}

	CurrentPlayerInput.Empty();

	SetAllLanternsLit(false);

	SetLanternInteractionEnabled(false);

	PuzzleState =
		ELanternPuzzleState::Previewing;

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Lantern Puzzle [%s] started."
		),
		*ObjectiveID.ToString()
	);

	if (InitialPreviewDelay <= 0.0f)
	{
		PlaySequencePreview();
		return;
	}

	GetWorldTimerManager().SetTimer(
		PreviewTimerHandle,
		this,
		&ALanternSequencePuzzle::
		PlaySequencePreview,
		InitialPreviewDelay,
		false
	);
}

void ALanternSequencePuzzle::ResetObjective()
{
	ClearPuzzleTimers();

	Super::ResetObjective();

	PuzzleState =
		ELanternPuzzleState::Dormant;

	CurrentPreviewIndex = 0;

	CurrentPlayerInput.Empty();

	SetAllLanternsLit(false);

	SetLanternInteractionEnabled(false);

	bConfigurationValid =
		ValidatePuzzleConfiguration();

	if (ActivationBox)
	{
		ActivationBox->SetCollisionEnabled(
			bConfigurationValid
			? ECollisionEnabled::QueryOnly
			: ECollisionEnabled::NoCollision
		);
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Lantern Puzzle [%s] fully reset."
		),
		*ObjectiveID.ToString()
	);
}

bool ALanternSequencePuzzle::ValidatePuzzleConfiguration()
{
	bool bIsValid = true;

	if (LanternPieces.IsEmpty())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Lantern Puzzle [%s] has "
				"an empty LanternPieces array."
			),
			*ObjectiveID.ToString()
		);

		bIsValid = false;
	}

	TSet<ALanternPuzzlePiece*>
		UniqueLanternActors;

	TSet<int32> UniquePieceIDs;

	for (int32 Index = 0;
		Index < LanternPieces.Num();
		++Index)
	{
		ALanternPuzzlePiece* Lantern =
			LanternPieces[Index];

		if (!IsValid(Lantern))
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT(
					"Lantern Puzzle [%s] has "
					"a null Lantern Piece at index %d."
				),
				*ObjectiveID.ToString(),
				Index
			);

			bIsValid = false;
			continue;
		}

		if (UniqueLanternActors.Contains(Lantern))
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT(
					"Lantern Puzzle [%s] contains "
					"the same Lantern Actor more than once: %s."
				),
				*ObjectiveID.ToString(),
				*Lantern->GetName()
			);

			bIsValid = false;
			continue;
		}

		UniqueLanternActors.Add(
			Lantern
		);

		const int32 PieceID =
			Lantern->GetPieceID();

		if (UniquePieceIDs.Contains(
			PieceID))
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT(
					"Lantern Puzzle [%s] contains "
					"duplicate PieceID [%d]."
				),
				*ObjectiveID.ToString(),
				PieceID
			);

			bIsValid = false;
			continue;
		}

		UniquePieceIDs.Add(
			PieceID
		);
	}

	if (CorrectSequence.IsEmpty())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Lantern Puzzle [%s] has "
				"an empty CorrectSequence."
			),
			*ObjectiveID.ToString()
		);

		bIsValid = false;
	}
	else
	{
		for (int32 SequenceIndex = 0;
			SequenceIndex <
			CorrectSequence.Num();
			++SequenceIndex)
		{
			const int32 RequiredPieceID =
				CorrectSequence[SequenceIndex];

			if (!UniquePieceIDs.Contains(
				RequiredPieceID))
			{
				UE_LOG(
					LogTemp,
					Error,
					TEXT(
						"Lantern Puzzle [%s] "
						"CorrectSequence index %d "
						"references missing PieceID [%d]."
					),
					*ObjectiveID.ToString(),
					SequenceIndex,
					RequiredPieceID
				);

				bIsValid = false;
			}
		}
	}

	if (bIsValid)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT(
				"Lantern Puzzle [%s] "
				"configuration is valid."
			),
			*ObjectiveID.ToString()
		);
	}

	return bIsValid;
}

ELanternPuzzleState
// ******************** Getters ********************

ALanternSequencePuzzle::GetPuzzleState() const
{
	return PuzzleState;
}

int32 ALanternSequencePuzzle::GetCurrentInputCount() const
{
	return CurrentPlayerInput.Num();
}

// ******************** Events ********************

void ALanternSequencePuzzle::OnPuzzleAreaEntered(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
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

	if (PuzzleState != ELanternPuzzleState::Dormant)
	{
		return;
	}

	StartPuzzle();
}

void ALanternSequencePuzzle::HandleLanternInteracted(
	ALanternPuzzlePiece* InteractedLantern)
{
	if (PuzzleState !=
		ELanternPuzzleState::
		AwaitingInput)
	{
		return;
	}

	if (!IsValid(InteractedLantern))
	{
		return;
	}

	if (!IsConfiguredLantern(InteractedLantern))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"Lantern Puzzle [%s] received "
				"input from an unconfigured Lantern: %s."
			),
			*ObjectiveID.ToString(),
			*InteractedLantern->GetName()
		);

		return;
	}

	const int32 InputIndex =
		CurrentPlayerInput.Num();

	if (!CorrectSequence.IsValidIndex(
		InputIndex))
	{
		return;
	}

	const int32 SubmittedPieceID =
		InteractedLantern->GetPieceID();

	const int32 ExpectedPieceID =
		CorrectSequence[InputIndex];

	if (SubmittedPieceID ==
		ExpectedPieceID)
	{
		HandleCorrectInput(
			InteractedLantern
		);
	}
	else
	{
		HandleWrongInput(
			InteractedLantern
		);
	}
}

// ******************** Preview ********************

void ALanternSequencePuzzle::PlaySequencePreview()
{
	if (!GetIsObjectiveActive())
	{
		return;
	}

	if (GetIsObjectiveComplete())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(
		PreviewTimerHandle
	);

	GetWorldTimerManager().ClearTimer(
		ResetTimerHandle
	);

	PuzzleState =
		ELanternPuzzleState::Previewing;

	CurrentPlayerInput.Empty();

	CurrentPreviewIndex = 0;

	SetLanternInteractionEnabled(false);

	SetAllLanternsLit(false);

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Lantern Puzzle [%s] "
			"sequence preview started."
		),
		*ObjectiveID.ToString()
	);

	AdvanceSequencePreview();
}

void ALanternSequencePuzzle::AdvanceSequencePreview()
{
	if (PuzzleState !=
		ELanternPuzzleState::Previewing)
	{
		return;
	}

	if (CurrentPreviewIndex >=
		CorrectSequence.Num())
	{
		FinishSequencePreview();
		return;
	}

	const int32 PreviewPieceID =
		CorrectSequence[CurrentPreviewIndex];

	ALanternPuzzlePiece* Lantern =
		FindLanternByPieceID(
			PreviewPieceID
		);

	if (!IsValid(Lantern))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Lantern Puzzle [%s] could not "
				"find PieceID [%d] during preview."
			),
			*ObjectiveID.ToString(),
			PreviewPieceID
		);

		bConfigurationValid = false;

		SetLanternInteractionEnabled(false);

		SetAllLanternsLit(false);

		PuzzleState =
			ELanternPuzzleState::Dormant;

		return;
	}

	Lantern
		->PlayLanternFeedbackForDuration(
			PreviewLightDuration
		);

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Lantern Puzzle [%s] preview step %d: "
			"PieceID [%d]."
		),
		*ObjectiveID.ToString(),
		CurrentPreviewIndex,
		PreviewPieceID
	);

	++CurrentPreviewIndex;

	const float PreviewStepInterval =
		FMath::Max(
			PreviewLightDuration +
			PreviewGapDuration,
			0.05f
		);

	GetWorldTimerManager().SetTimer(
		PreviewTimerHandle,
		this,
		&ALanternSequencePuzzle::
		AdvanceSequencePreview,
		PreviewStepInterval,
		false
	);
}

void ALanternSequencePuzzle::FinishSequencePreview()
{
	if (PuzzleState !=
		ELanternPuzzleState::Previewing)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(
		PreviewTimerHandle
	);

	SetAllLanternsLit(false);

	CurrentPlayerInput.Empty();

	CurrentPreviewIndex = 0;

	PuzzleState =
		ELanternPuzzleState::AwaitingInput;

	SetLanternInteractionEnabled(true);

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Lantern Puzzle [%s] preview finished. "
			"Awaiting player input."
		),
		*ObjectiveID.ToString()
	);
}

// ******************** Input Evaluation ********************

void ALanternSequencePuzzle::HandleCorrectInput(
	ALanternPuzzlePiece* InteractedLantern)
{
	if (!IsValid(InteractedLantern))
	{
		return;
	}

	const int32 PieceID =
		InteractedLantern->GetPieceID();

	CurrentPlayerInput.Add(
		PieceID
	);

	/*
	* ALanternPuzzlePiece::Interact() 已先播放
	* 临时 Feedback。
	*
	* 这里再次 SetLanternLit(true)，
	* 会清除临时熄灯 Timer，
	* 让正确输入保持点亮。
	*/
	InteractedLantern->SetLanternLit(
		true
	);

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Lantern Puzzle [%s] correct input: "
			"PieceID [%d]. Progress: %d / %d."
		),
		*ObjectiveID.ToString(),
		PieceID,
		CurrentPlayerInput.Num(),
		CorrectSequence.Num()
	);

	if (CurrentPlayerInput.Num() >=
		CorrectSequence.Num())
	{
		FinishPuzzle();
	}
}

void ALanternSequencePuzzle::HandleWrongInput(
	ALanternPuzzlePiece* InteractedLantern)
{
	if (PuzzleState !=
		ELanternPuzzleState::AwaitingInput)
	{
		return;
	}

	PuzzleState =
		ELanternPuzzleState::Resetting;

	SetLanternInteractionEnabled(false);

	const int32 WrongPieceID =
		IsValid(InteractedLantern)
		? InteractedLantern->GetPieceID()
		: INDEX_NONE;

	const int32 InputIndex =
		CurrentPlayerInput.Num();

	const int32 ExpectedPieceID =
		CorrectSequence.IsValidIndex(
			InputIndex
		)
		? CorrectSequence[InputIndex]
		: INDEX_NONE;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"Lantern Puzzle [%s] wrong input. "
			"Received [%d], expected [%d]."
		),
		*ObjectiveID.ToString(),
		WrongPieceID,
		ExpectedPieceID
	);

	PlayWrongInputSound(
		InteractedLantern
	);

	ResetPuzzleInput();

	if (ResetDelay <= 0.0f)
	{
		PlaySequencePreview();
		return;
	}

	GetWorldTimerManager().SetTimer(
		ResetTimerHandle,
		this,
		&ALanternSequencePuzzle::
		PlaySequencePreview,
		ResetDelay,
		false
	);
}

void ALanternSequencePuzzle::
ResetPuzzleInput()
{
	CurrentPlayerInput.Empty();

	SetAllLanternsLit(false);
}

void ALanternSequencePuzzle::FinishPuzzle()
{
	if (PuzzleState ==
		ELanternPuzzleState::Completed)
	{
		return;
	}

	if (GetIsObjectiveComplete())
	{
		return;
	}

	if (!GetIsObjectiveActive())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"Lantern Puzzle [%s] cannot finish "
				"because its Objective is inactive."
			),
			*ObjectiveID.ToString()
		);

		return;
	}

	ClearPuzzleTimers();

	SetLanternInteractionEnabled(false);

	SetAllLanternsLit(true);

	PuzzleState =
		ELanternPuzzleState::Completed;

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Lantern Puzzle [%s] solved."
		),
		*ObjectiveID.ToString()
	);

	CompleteObjective();
}

// ******************** Helpers ********************

void ALanternSequencePuzzle::BindLanternDelegates()
{
	for (ALanternPuzzlePiece* Lantern
		: LanternPieces)
	{
		if (!IsValid(Lantern))
		{
			continue;
		}

		Lantern->OnLanternInteracted
			.AddUniqueDynamic(
				this,
				&ALanternSequencePuzzle::HandleLanternInteracted
			);
	}
}

void ALanternSequencePuzzle::UnbindLanternDelegates()
{
	for (ALanternPuzzlePiece* Lantern
		: LanternPieces)
	{
		if (!IsValid(Lantern))
		{
			continue;
		}

		Lantern->OnLanternInteracted
			.RemoveDynamic(
				this,
				&ALanternSequencePuzzle::HandleLanternInteracted
			);
	}
}

void ALanternSequencePuzzle::SetAllLanternsLit(bool bLit)
{
	for (ALanternPuzzlePiece* Lantern
		: LanternPieces)
	{
		if (!IsValid(Lantern))
		{
			continue;
		}

		Lantern->SetLanternLit(bLit);
	}
}

void ALanternSequencePuzzle::SetLanternInteractionEnabled(bool bEnabled)
{
	for (ALanternPuzzlePiece* Lantern
		: LanternPieces)
	{
		if (!IsValid(Lantern))
		{
			continue;
		}

		Lantern->SetInteractionEnabled(bEnabled);
	}
}

ALanternPuzzlePiece*
ALanternSequencePuzzle::FindLanternByPieceID(int32 PieceID) const
{
	for (ALanternPuzzlePiece* Lantern
		: LanternPieces)
	{
		if (!IsValid(Lantern))
		{
			continue;
		}

		if (Lantern->GetPieceID() ==
			PieceID)
		{
			return Lantern;
		}
	}

	return nullptr;
}

bool ALanternSequencePuzzle::IsConfiguredLantern(
	const ALanternPuzzlePiece* Lantern) const
{
	if (!IsValid(Lantern))
	{
		return false;
	}

	for (const ALanternPuzzlePiece*
		ConfiguredLantern
		: LanternPieces)
	{
		if (ConfiguredLantern ==
			Lantern)
		{
			return true;
		}
	}

	return false;
}

void ALanternSequencePuzzle::ClearPuzzleTimers()
{
	GetWorldTimerManager().ClearTimer(
		PreviewTimerHandle
	);

	GetWorldTimerManager().ClearTimer(
		ResetTimerHandle
	);
}

void ALanternSequencePuzzle::PlayWrongInputSound(
	const ALanternPuzzlePiece* WrongLantern) const
{
	if (!WrongInputSound)
	{
		return;
	}

	const FVector SoundLocation =
		IsValid(WrongLantern)
		? WrongLantern->GetActorLocation()
		: GetActorLocation();

	UGameplayStatics::PlaySoundAtLocation(
		this,
		WrongInputSound,
		SoundLocation
	);
}








