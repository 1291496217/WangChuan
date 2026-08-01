#include "WCStoryPersistenceCoordinator.h"

#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#include "EchoRelic.h"
#include "StoryAnchor.h"
#include "StoryEncounter.h"
#include "StoryObjectiveBase.h"
#include "WCCharacter.h"
#include "WCGameInstance.h"
#include "WCGameSaveGame.h"
#include "WCStoryNPC.h"


DEFINE_LOG_CATEGORY_STATIC(
	LogWCStoryPersistence,
	Log,
	All
);

AWCStoryPersistenceCoordinator::
AWCStoryPersistenceCoordinator()
{
	PrimaryActorTick.bCanEverTick = false;

	SetActorEnableCollision(false);
}

void AWCStoryPersistenceCoordinator::BeginPlay()
{
	Super::BeginPlay();

	bRestoreInProgress = false;
	bHasRestoredLoadedWorld = false;
	SuccessfulRestoreCount = 0;

	UE_LOG(
		LogWCStoryPersistence,
		Display,
		TEXT(
			"Story Persistence Coordinator initialized. "
			"AutoLoad=%s."
		),
		bAutoLoadAndRestoreOnBeginPlay
		? TEXT("True")
		: TEXT("False")
	);

	if (!bAutoLoadAndRestoreOnBeginPlay)
	{
		return;
	}

	/*
	* 等关卡 Actor 和 Blueprint BeginPlay 完成后，
	* 再统一执行 Load / Restore。
	*/
	DeferredRestoreTimerHandle =
		GetWorldTimerManager()
		.SetTimerForNextTick(
			FTimerDelegate::CreateUObject(
				this,
				&AWCStoryPersistenceCoordinator::
				HandleDeferredAutoRestore
			)
		);
}

void AWCStoryPersistenceCoordinator::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(
		DeferredRestoreTimerHandle
	);

	Super::EndPlay(EndPlayReason);
}

void AWCStoryPersistenceCoordinator::
HandleDeferredAutoRestore()
{
	UWCGameInstance* GameInstance =
		GetWCGameInstance();

	if (!IsValid(GameInstance))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Deferred Restore failed: "
				"active GameInstance is not "
				"UWCGameInstance."
			)
		);

		return;
	}

	if (!GameInstance->HasSavedGame())
	{
		/*
		* 没有 Save 是正常的新游戏状态，
		* 不是 Error。
		*/
		UE_LOG(
			LogWCStoryPersistence,
			Display,
			TEXT(
				"No save slot found. "
				"Default new-game World remains active."
			)
		);

		ShowPersistenceMessage(
			TEXT(
				"No save found — starting new game."
			),
			FColor::Silver
		);

		return;
	}

	LoadAndRestoreWorldState();
}

bool AWCStoryPersistenceCoordinator::BuildWorldActorMaps(
	TMap<FName, AWCStoryNPC*>&
	OutStoryNPCs,
	TMap<FName, AStoryObjectiveBase*>&
	OutObjectives,
	TMap<FName, AStoryEncounter*>&
	OutEncounters,
	TMap<FName, AEchoRelic*>&
	OutEchoRelics,
	TMap<FName, AStoryAnchor*>&
	OutAnchors
) const
{
	OutStoryNPCs.Reset();
	OutObjectives.Reset();
	OutEncounters.Reset();
	OutEchoRelics.Reset();
	OutAnchors.Reset();

	/*
	* 先复用 Day2 已完成的完整 ID 验证。
	*/
	if (!ValidateWorldPersistenceIDs())
	{
		return false;
	}

	TArray<AActor*> FoundActors;

	/*
	* Story NPCs
	*/
	UGameplayStatics::GetAllActorsOfClass(
		this,
		AWCStoryNPC::StaticClass(),
		FoundActors
	);

	for (AActor* Actor : FoundActors)
	{
		AWCStoryNPC* StoryNPC =
			Cast<AWCStoryNPC>(Actor);

		if (!IsValid(StoryNPC))
		{
			return false;
		}

		OutStoryNPCs.Add(
			StoryNPC->GetStoryNPCID(),
			StoryNPC
		);
	}

	/*
	* Objectives
	*/
	FoundActors.Reset();

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AStoryObjectiveBase::StaticClass(),
		FoundActors
	);

	for (AActor* Actor : FoundActors)
	{
		AStoryObjectiveBase* Objective =
			Cast<AStoryObjectiveBase>(Actor);

		if (!IsValid(Objective))
		{
			return false;
		}

		OutObjectives.Add(
			Objective->GetObjectiveID(),
			Objective
		);
	}

	/*
	* Encounters
	*/
	FoundActors.Reset();

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AStoryEncounter::StaticClass(),
		FoundActors
	);

	for (AActor* Actor : FoundActors)
	{
		AStoryEncounter* Encounter =
			Cast<AStoryEncounter>(Actor);

		if (!IsValid(Encounter))
		{
			return false;
		}

		OutEncounters.Add(
			Encounter->GetEncounterID(),
			Encounter
		);
	}

	/*
	* Echo Relics
	*/
	FoundActors.Reset();

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AEchoRelic::StaticClass(),
		FoundActors
	);

	for (AActor* Actor : FoundActors)
	{
		AEchoRelic* EchoRelic =
			Cast<AEchoRelic>(Actor);

		if (!IsValid(EchoRelic))
		{
			return false;
		}

		OutEchoRelics.Add(
			EchoRelic->GetEchoID(),
			EchoRelic
		);
	}

	/*
	* Anchors
	*/
	FoundActors.Reset();

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AStoryAnchor::StaticClass(),
		FoundActors
	);

	for (AActor* Actor : FoundActors)
	{
		AStoryAnchor* StoryAnchor =
			Cast<AStoryAnchor>(Actor);

		if (!IsValid(StoryAnchor))
		{
			return false;
		}

		OutAnchors.Add(
			StoryAnchor->GetAnchorID(),
			StoryAnchor
		);
	}

	UE_LOG(
		LogWCStoryPersistence,
		Display,
		TEXT(
			"Built World Persistence maps: "
			"NPCs=%d, Objectives=%d, Encounters=%d, "
			"Echoes=%d, Anchors=%d."
		),
		OutStoryNPCs.Num(),
		OutObjectives.Num(),
		OutEncounters.Num(),
		OutEchoRelics.Num(),
		OutAnchors.Num()
	);

	return true;
}

bool AWCStoryPersistenceCoordinator::ValidateLoadedSaveDataForWorld(
	const UWCGameSaveGame* SaveData,
	const TMap<FName, AWCStoryNPC*>&
	WorldStoryNPCs,
	const TMap<FName, AStoryObjectiveBase*>&
	WorldObjectives,
	const TMap<FName, AStoryEncounter*>&
	WorldEncounters,
	const TMap<FName, AEchoRelic*>&
	WorldEchoRelics,
	const TMap<FName, AStoryAnchor*>&
	WorldAnchors
) const
{
	if (!IsValid(SaveData))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Restore validation failed: "
				"SaveData is invalid."
			)
		);

		return false;
	}

	if (SaveData->SaveVersion !=
		UWCGameSaveGame::CurrentSaveVersion)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Restore validation failed: "
				"SaveVersion=%d, Supported=%d."
			),
			SaveData->SaveVersion,
			UWCGameSaveGame::CurrentSaveVersion
		);

		return false;
	}

	/*
	* 当前 Save V1 是完整单地图快照。
	* Record 数量应与当前正式 World Actor 数量一致。
	*/
	if (SaveData->StoryNPCStates.Num() !=
		WorldStoryNPCs.Num() ||
		SaveData->ObjectiveStates.Num() !=
		WorldObjectives.Num() ||
		SaveData->EncounterStates.Num() !=
		WorldEncounters.Num())
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Restore validation failed: "
				"snapshot/world counts differ. "
				"Save NPC/O/E=%d/%d/%d, "
				"World NPC/O/E=%d/%d/%d."
			),
			SaveData->StoryNPCStates.Num(),
			SaveData->ObjectiveStates.Num(),
			SaveData->EncounterStates.Num(),
			WorldStoryNPCs.Num(),
			WorldObjectives.Num(),
			WorldEncounters.Num()
		);

		return false;
	}

	TSet<FName> SavedStoryNPCIDs;
	TSet<FName> SavedObjectiveIDs;
	TSet<FName> SavedEncounterIDs;
	TSet<FName> SavedJournalEchoIDs;

	TMap<FName, bool>
		SavedObjectiveCompletion;

	TMap<FName, bool>
		SavedEncounterCompletion;

	/*
	* NPC records
	*/
	for (const FWCSavedStoryNPCState& NPCState :
		SaveData->StoryNPCStates)
	{
		if (NPCState.StoryNPCID.IsNone() ||
			SavedStoryNPCIDs.Contains(
				NPCState.StoryNPCID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Invalid or duplicate saved NPC ID [%s]."
				),
				*NPCState.StoryNPCID.ToString()
			);

			return false;
		}

		AWCStoryNPC* const* StoryNPCPtr =
			WorldStoryNPCs.Find(
				NPCState.StoryNPCID
			);

		if (!StoryNPCPtr ||
			!IsValid(*StoryNPCPtr))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Saved NPC [%s] does not exist "
					"in the current World."
				),
				*NPCState.StoryNPCID.ToString()
			);

			return false;
		}

		if (NPCState.StoryStage < 0 ||
			NPCState.AnchorID.IsNone() ||
			!WorldAnchors.Contains(
				NPCState.AnchorID) ||
			!(*StoryNPCPtr)->HasStoryAnchorID(
				NPCState.AnchorID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Saved NPC [%s] has invalid "
					"Stage=%d or Anchor=[%s]."
				),
				*NPCState.StoryNPCID.ToString(),
				NPCState.StoryStage,
				*NPCState.AnchorID.ToString()
			);

			return false;
		}

		SavedStoryNPCIDs.Add(
			NPCState.StoryNPCID
		);
	}

	/*
	* Objective records
	*/
	for (const FWCSavedObjectiveState&
		ObjectiveState :
		SaveData->ObjectiveStates)
	{
		if (ObjectiveState.ObjectiveID.IsNone() ||
			SavedObjectiveIDs.Contains(
				ObjectiveState.ObjectiveID) ||
			!WorldObjectives.Contains(
				ObjectiveState.ObjectiveID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Invalid, duplicate, or missing "
					"saved Objective ID [%s]."
				),
				*ObjectiveState.ObjectiveID.ToString()
			);

			return false;
		}

		SavedObjectiveIDs.Add(
			ObjectiveState.ObjectiveID
		);

		SavedObjectiveCompletion.Add(
			ObjectiveState.ObjectiveID,
			ObjectiveState.bCompleted
		);
	}

	/*
	* Encounter records
	*/
	for (const FWCSavedEncounterState&
		EncounterState :
		SaveData->EncounterStates)
	{
		if (EncounterState.EncounterID.IsNone() ||
			SavedEncounterIDs.Contains(
				EncounterState.EncounterID) ||
			!WorldEncounters.Contains(
				EncounterState.EncounterID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Invalid, duplicate, or missing "
					"saved Encounter ID [%s]."
				),
				*EncounterState.EncounterID.ToString()
			);

			return false;
		}

		SavedEncounterIDs.Add(
			EncounterState.EncounterID
		);

		SavedEncounterCompletion.Add(
			EncounterState.EncounterID,
			EncounterState.bCompleted
		);
	}

	/*
	* Journal records
	*/
	for (const FMemoryEchoData& EchoData :
		SaveData->RecordedMemoryEchoes)
	{
		if (EchoData.EchoID.IsNone() ||
			SavedJournalEchoIDs.Contains(
				EchoData.EchoID) ||
			!WorldEchoRelics.Contains(
				EchoData.EchoID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Invalid, duplicate, or missing "
					"Journal Echo ID [%s]."
				),
				*EchoData.EchoID.ToString()
			);

			return false;
		}

		SavedJournalEchoIDs.Add(
			EchoData.EchoID
		);
	}

	/*
	* 验证 Encounter → Objective → Relic 关系，
	* 同时验证数据事实的一致性。
	*/
	TSet<FName> EncounterRelicIDs;

	for (const TPair<FName, AStoryEncounter*>& Pair :
		WorldEncounters)
	{
		const FName EncounterID = Pair.Key;
		AStoryEncounter* Encounter = Pair.Value;

		if (!IsValid(Encounter))
		{
			return false;
		}

		AStoryObjectiveBase* Objective =
			Encounter->GetStoryObjective();

		AEchoRelic* EchoRelic =
			Encounter->GetEchoRelic();

		if (!IsValid(Objective) ||
			!IsValid(EchoRelic))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Encounter [%s] has invalid "
					"Objective or Echo Relic reference."
				),
				*EncounterID.ToString()
			);

			return false;
		}

		const FName ObjectiveID =
			Objective->GetObjectiveID();

		const FName EchoID =
			EchoRelic->GetEchoID();

		const bool* SavedObjectiveComplete =
			SavedObjectiveCompletion.Find(
				ObjectiveID
			);

		const bool* SavedEncounterComplete =
			SavedEncounterCompletion.Find(
				EncounterID
			);

		if (!SavedObjectiveComplete ||
			!SavedEncounterComplete)
		{
			return false;
		}

		if (EncounterRelicIDs.Contains(EchoID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Echo Relic [%s] is assigned to "
					"more than one Encounter."
				),
				*EchoID.ToString()
			);

			return false;
		}

		EncounterRelicIDs.Add(EchoID);

		if (*SavedEncounterComplete &&
			!*SavedObjectiveComplete)
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Saved Encounter [%s] is complete "
					"while Objective [%s] is incomplete."
				),
				*EncounterID.ToString(),
				*ObjectiveID.ToString()
			);

			return false;
		}

		const bool bJournalContainsEcho =
			SavedJournalEchoIDs.Contains(
				EchoID
			);

		if (*SavedEncounterComplete !=
			bJournalContainsEcho)
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Encounter [%s] and Journal Echo "
					"[%s] disagree. Encounter=%s, "
					"JournalContainsEcho=%s."
				),
				*EncounterID.ToString(),
				*EchoID.ToString(),
				*SavedEncounterComplete
				? TEXT("True")
				: TEXT("False"),
				bJournalContainsEcho
				? TEXT("True")
				: TEXT("False")
			);

			return false;
		}
	}

	if (EncounterRelicIDs.Num() !=
		WorldEchoRelics.Num())
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Not every Echo Relic is assigned "
				"to exactly one Encounter."
			)
		);

		return false;
	}

	UE_LOG(
		LogWCStoryPersistence,
		Display,
		TEXT(
			"Loaded Save validation succeeded."
		)
	);

	return true;
}

bool AWCStoryPersistenceCoordinator::LoadAndRestoreWorldState()
{
	UWCGameInstance* GameInstance =
		GetWCGameInstance();

	if (!IsValid(GameInstance))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Load/Restore failed: "
				"invalid WCGameInstance."
			)
		);

		return false;
	}

	if (!GameInstance->HasSavedGame())
	{
		UE_LOG(
			LogWCStoryPersistence,
			Display,
			TEXT(
				"Load/Restore skipped: "
				"no save slot exists."
			)
		);

		return false;
	}

	/*
	* 每次明确 Load 都重新从磁盘读取，
	* 不依赖可能过期的旧 LoadedSaveData。
	*/
	if (!GameInstance->LoadSavedGame())
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Load/Restore failed while "
				"reading the disk save."
			)
		);

		return false;
	}

	return RestoreLoadedWorldState();
}

bool AWCStoryPersistenceCoordinator::RestoreLoadedWorldState()
{
	if (bRestoreInProgress)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Warning,
			TEXT(
				"Restore request rejected: "
				"a Restore is already in progress."
			)
		);

		return false;
	}

	TGuardValue<bool> RestoreGuard(
		bRestoreInProgress,
		true
	);

	UWCGameInstance* GameInstance =
		GetWCGameInstance();

	if (!IsValid(GameInstance))
	{
		return false;
	}

	UWCGameSaveGame* SaveData =
		GameInstance->GetLoadedSaveData();

	if (!IsValid(SaveData))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Restore failed: "
				"LoadedSaveData is invalid."
			)
		);

		return false;
	}

	AWCCharacter* Player =
		Cast<AWCCharacter>(
			UGameplayStatics::GetPlayerCharacter(
				this,
				0
			)
		);

	if (!IsValid(Player))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Restore failed: Player 0 is not "
				"a valid AWCCharacter."
			)
		);

		return false;
	}

	TMap<FName, AWCStoryNPC*>
		WorldStoryNPCs;

	TMap<FName, AStoryObjectiveBase*>
		WorldObjectives;

	TMap<FName, AStoryEncounter*>
		WorldEncounters;

	TMap<FName, AEchoRelic*>
		WorldEchoRelics;

	TMap<FName, AStoryAnchor*>
		WorldAnchors;

	if (!BuildWorldActorMaps(
		WorldStoryNPCs,
		WorldObjectives,
		WorldEncounters,
		WorldEchoRelics,
		WorldAnchors))
	{
		ShowPersistenceMessage(
			TEXT(
				"Restore failed: World ID map invalid."
			),
			FColor::Red
		);

		return false;
	}

	/*
	* 所有 Save / World 数据先完整验证。
	* 验证成功前不修改任何 Actor。
	*/
	if (!ValidateLoadedSaveDataForWorld(
		SaveData,
		WorldStoryNPCs,
		WorldObjectives,
		WorldEncounters,
		WorldEchoRelics,
		WorldAnchors))
	{
		ShowPersistenceMessage(
			TEXT(
				"Restore failed: Save data is "
				"incompatible with this World."
			),
			FColor::Red
		);

		return false;
	}

	UE_LOG(
		LogWCStoryPersistence,
		Display,
		TEXT(
			"Beginning ordered silent World Restore."
		)
	);

	/*
	* Phase 1: Objectives
	*
	* Enemy defeated presentation and Lantern final
	* presentation are restored through subclass hooks.
	*/
	for (const FWCSavedObjectiveState&
		ObjectiveState :
		SaveData->ObjectiveStates)
	{
		AStoryObjectiveBase* const* ObjectivePtr =
			WorldObjectives.Find(
				ObjectiveState.ObjectiveID
			);

		check(ObjectivePtr);

		(*ObjectivePtr)->ApplySavedObjectiveState(
			ObjectiveState.bCompleted
		);
	}

	/*
	* Phase 2: Encounters
	*
	* Encounter derives ObjectiveResolved from the
	* Objective state restored above.
	*/
	for (const FWCSavedEncounterState&
		EncounterState :
		SaveData->EncounterStates)
	{
		AStoryEncounter* const* EncounterPtr =
			WorldEncounters.Find(
				EncounterState.EncounterID
			);

		check(EncounterPtr);

		(*EncounterPtr)->ApplySavedEncounterState(
			EncounterState.bCompleted
		);
	}

	/*
	* Phase 3: Relics
	*
	* Relic State is derived rather than read from
	* a duplicated SaveGame field.
	*/
	for (const TPair<FName, AStoryEncounter*>& Pair :
		WorldEncounters)
	{
		AStoryEncounter* Encounter = Pair.Value;
		AStoryObjectiveBase* Objective =
			Encounter->GetStoryObjective();
		AEchoRelic* EchoRelic =
			Encounter->GetEchoRelic();

		EEchoRelicState DerivedRelicState =
			EEchoRelicState::locked;

		if (Encounter
			->GetIsEncounterCompleted())
		{
			DerivedRelicState =
				EEchoRelicState::Activated;
		}
		else if (Objective
			->GetIsObjectiveComplete())
		{
			DerivedRelicState =
				EEchoRelicState::Available;
		}

		EchoRelic->ApplySavedRelicState(
			DerivedRelicState
		);

		UE_LOG(
			LogWCStoryPersistence,
			Display,
			TEXT(
				"Derived Relic [%s] State=%d "
				"from Objective [%s] and "
				"Encounter [%s]."
			),
			*EchoRelic->GetEchoID().ToString(),
			static_cast<int32>(
				DerivedRelicState
				),
			*Objective
			->GetObjectiveID()
			.ToString(),
			*Encounter
			->GetEncounterID()
			.ToString()
		);
	}

	/*
	* Phase 4: Story NPCs
	*/
	for (const FWCSavedStoryNPCState& NPCState :
		SaveData->StoryNPCStates)
	{
		AWCStoryNPC* const* StoryNPCPtr =
			WorldStoryNPCs.Find(
				NPCState.StoryNPCID
			);

		check(StoryNPCPtr);

		if (!(*StoryNPCPtr)
			->ApplySavedStoryState(
				NPCState.StoryStage,
				NPCState.AnchorID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore failed while applying "
					"NPC [%s]."
				),
				*NPCState.StoryNPCID.ToString()
			);

			return false;
		}
	}

	/*
	* Phase 5: Journal
	*/
	Player->ApplySavedMemoryEchoes(
		SaveData->RecordedMemoryEchoes
	);

	if (Player
		->GetRecordedMemoryEchoes()
		.Num() !=
		SaveData->RecordedMemoryEchoes.Num())
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Restore failed: Journal record count "
				"does not match the saved data."
			)
		);

		return false;
	}

	bHasRestoredLoadedWorld = true;
	++SuccessfulRestoreCount;

	UE_LOG(
		LogWCStoryPersistence,
		Display,
		TEXT(
			"Silent World Restore succeeded. "
			"RestoreCount=%d, NPCs=%d, "
			"Objectives=%d, Encounters=%d, "
			"Echoes=%d."
		),
		SuccessfulRestoreCount,
		SaveData->StoryNPCStates.Num(),
		SaveData->ObjectiveStates.Num(),
		SaveData->EncounterStates.Num(),
		SaveData->RecordedMemoryEchoes.Num()
	);

	ShowPersistenceMessage(
		FString::Printf(
			TEXT(
				"World Restored: NPCs %d | "
				"Objectives %d | Encounters %d | "
				"Echoes %d"
			),
			SaveData->StoryNPCStates.Num(),
			SaveData->ObjectiveStates.Num(),
			SaveData->EncounterStates.Num(),
			SaveData->RecordedMemoryEchoes.Num()
		),
		FColor::Green
	);

	PrintLoadedSaveSummary();

	return true;
}

bool AWCStoryPersistenceCoordinator::GetHasRestoredLoadedWorld() const
{
	return bHasRestoredLoadedWorld;
}

UWCGameInstance*
AWCStoryPersistenceCoordinator::GetWCGameInstance() const
{
	return Cast<UWCGameInstance>(
		GetGameInstance()
	);
}

bool AWCStoryPersistenceCoordinator::RegisterPersistenceID(
	FName ID,
	const AActor* OwnerActor,
	const TCHAR* TypeLabel,
	TSet<FName>& SeenIDs
) const
{
	if (!IsValid(OwnerActor))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Persistence validation failed: "
				"invalid %s Actor."
			),
			TypeLabel
		);

		return false;
	}

	if (ID.IsNone())
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Persistence validation failed: "
				"%s Actor [%s] has ID None."
			),
			TypeLabel,
			*GetNameSafe(OwnerActor)
		);

		return false;
	}

	if (SeenIDs.Contains(ID))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Persistence validation failed: "
				"duplicate %s ID [%s]. "
				"Current Actor: [%s]."
			),
			TypeLabel,
			*ID.ToString(),
			*GetNameSafe(OwnerActor)
		);

		return false;
	}

	SeenIDs.Add(ID);

	UE_LOG(
		LogWCStoryPersistence,
		Verbose,
		TEXT(
			"Validated %s ID [%s] on Actor [%s]."
		),
		TypeLabel,
		*ID.ToString(),
		*GetNameSafe(OwnerActor)
	);

	return true;
}

bool AWCStoryPersistenceCoordinator::
ValidateWorldPersistenceIDs() const
{
	bool bAllIDsValid = true;

	TArray<AActor*> FoundActors;

	/*
	* Story NPC IDs
	*/
	UGameplayStatics::GetAllActorsOfClass(
		this,
		AWCStoryNPC::StaticClass(),
		FoundActors
	);

	if (FoundActors.Num() == 0)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Persistence validation failed: "
				"no AWCStoryNPC Actor found."
			)
		);

		bAllIDsValid = false;
	}

	TSet<FName> StoryNPCIDs;

	for (AActor* Actor : FoundActors)
	{
		const AWCStoryNPC* StoryNPC =
			Cast<AWCStoryNPC>(Actor);

		if (!StoryNPC)
		{
			bAllIDsValid = false;
			continue;
		}

		if (!RegisterPersistenceID(
			StoryNPC->GetStoryNPCID(),
			StoryNPC,
			TEXT("Story NPC"),
			StoryNPCIDs
		))
		{
			bAllIDsValid = false;
		}
	}

	/*
	* Objective IDs
	*/
	FoundActors.Reset();

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AStoryObjectiveBase::StaticClass(),
		FoundActors
	);

	if (FoundActors.Num() == 0)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Persistence validation failed: "
				"no AStoryObjectiveBase Actor found."
			)
		);

		bAllIDsValid = false;
	}

	TSet<FName> ObjectiveIDs;

	for (AActor* Actor : FoundActors)
	{
		const AStoryObjectiveBase* Objective =
			Cast<AStoryObjectiveBase>(Actor);

		if (!Objective)
		{
			bAllIDsValid = false;
			continue;
		}

		if (!RegisterPersistenceID(
			Objective->GetObjectiveID(),
			Objective,
			TEXT("Objective"),
			ObjectiveIDs
		))
		{
			bAllIDsValid = false;
		}
	}

	/*
	* Encounter IDs
	*/
	FoundActors.Reset();

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AStoryEncounter::StaticClass(),
		FoundActors
	);

	if (FoundActors.Num() == 0)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Persistence validation failed: "
				"no AStoryEncounter Actor found."
			)
		);

		bAllIDsValid = false;
	}

	TSet<FName> EncounterIDs;

	for (AActor* Actor : FoundActors)
	{
		const AStoryEncounter* Encounter =
			Cast<AStoryEncounter>(Actor);

		if (!Encounter)
		{
			bAllIDsValid = false;
			continue;
		}

		if (!RegisterPersistenceID(
			Encounter->GetEncounterID(),
			Encounter,
			TEXT("Encounter"),
			EncounterIDs
		))
		{
			bAllIDsValid = false;
		}
	}

	/*
	* Echo IDs
	*
	* Echo State 当前不独立保存，
	* 但 EchoID 仍然是 Journal 与未来 Restore 的关键身份。
	*/
	FoundActors.Reset();

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AEchoRelic::StaticClass(),
		FoundActors
	);

	if (FoundActors.Num() == 0)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Persistence validation failed: "
				"no AEchoRelic Actor found."
			)
		);

		bAllIDsValid = false;
	}

	TSet<FName> EchoIDs;

	for (AActor* Actor : FoundActors)
	{
		const AEchoRelic* EchoRelic =
			Cast<AEchoRelic>(Actor);

		if (!EchoRelic)
		{
			bAllIDsValid = false;
			continue;
		}

		if (!RegisterPersistenceID(
			EchoRelic->GetEchoID(),
			EchoRelic,
			TEXT("Echo Relic"),
			EchoIDs
		))
		{
			bAllIDsValid = false;
		}
	}

	/*
	* Anchor IDs
	*/
	FoundActors.Reset();

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AStoryAnchor::StaticClass(),
		FoundActors
	);

	if (FoundActors.Num() == 0)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Persistence validation failed: "
				"no AStoryAnchor Actor found."
			)
		);

		bAllIDsValid = false;
	}

	TSet<FName> AnchorIDs;

	for (AActor* Actor : FoundActors)
	{
		const AStoryAnchor* StoryAnchor =
			Cast<AStoryAnchor>(Actor);

		if (!StoryAnchor)
		{
			bAllIDsValid = false;
			continue;
		}

		if (!RegisterPersistenceID(
			StoryAnchor->GetAnchorID(),
			StoryAnchor,
			TEXT("Story Anchor"),
			AnchorIDs
		))
		{
			bAllIDsValid = false;
		}
	}

	if (bAllIDsValid)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Display,
			TEXT(
				"Persistence ID validation succeeded. "
				"NPCs=%d, Objectives=%d, Encounters=%d, "
				"Echoes=%d, Anchors=%d."
			),
			StoryNPCIDs.Num(),
			ObjectiveIDs.Num(),
			EncounterIDs.Num(),
			EchoIDs.Num(),
			AnchorIDs.Num()
		);
	}
	else
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Persistence ID validation failed. "
				"World state will not be saved."
			)
		);
	}

	return bAllIDsValid;
}

bool AWCStoryPersistenceCoordinator::
CaptureAndSaveWorldState()
{
	UWCGameInstance* GameInstance =
		GetWCGameInstance();

	if (!IsValid(GameInstance))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"World-state capture failed: "
				"active GameInstance is not UWCGameInstance."
			)
		);

		ShowPersistenceMessage(
			TEXT("Save failed: invalid WCGameInstance."),
			FColor::Red
		);

		return false;
	}

	/*
	* 必须先验证整个世界。
	*
	* 若存在空 ID 或重复 ID，
	* 不允许写出一份无法可靠恢复的存档。
	*/
	if (!ValidateWorldPersistenceIDs())
	{
		ShowPersistenceMessage(
			TEXT("Save failed: Persistence ID validation failed."),
			FColor::Red
		);

		return false;
	}

	AWCCharacter* Player =
		Cast<AWCCharacter>(
			UGameplayStatics::GetPlayerCharacter(
				this,
				0
			)
		);

	if (!IsValid(Player))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"World-state capture failed: "
				"player 0 is not a valid AWCCharacter."
			)
		);

		return false;
	}

	/*
	* 先收集到局部数组。
	*
	* 不直接边扫描边修改 LoadedSaveData，
	* 避免采集中途失败后留下半份新数据。
	*/
	TArray<FWCSavedStoryNPCState>
		CapturedStoryNPCStates;

	TArray<FWCSavedObjectiveState>
		CapturedObjectiveStates;

	TArray<FWCSavedEncounterState>
		CapturedEncounterStates;

	TArray<FMemoryEchoData>
		CapturedMemoryEchoes;

	TArray<AActor*> FoundActors;

	/*
	* Capture Story NPCs
	*/
	UGameplayStatics::GetAllActorsOfClass(
		this,
		AWCStoryNPC::StaticClass(),
		FoundActors
	);

	for (AActor* Actor : FoundActors)
	{
		const AWCStoryNPC* StoryNPC =
			Cast<AWCStoryNPC>(Actor);

		if (!IsValid(StoryNPC))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"invalid Story NPC Actor."
				)
			);

			return false;
		}

		const EStoryNPCState CurrentState =
			StoryNPC->GetStoryState();

		/*
		* 不保存瞬时转移过程。
		*
		* Encounter 会先进入 EventResolved，
		* 然后进入 Relocating，最后回到 Available。
		*/
		if (CurrentState ==
			EStoryNPCState::EventResolved ||
			CurrentState ==
			EStoryNPCState::Relocating)
		{
			UE_LOG(
				LogWCStoryPersistence,
				Warning,
				TEXT(
					"World-state capture rejected: "
					"Story NPC [%s] is in transitional "
					"state [%d]. Wait for relocation "
					"to finish before saving."
				),
				*StoryNPC->GetStoryNPCID().ToString(),
				static_cast<int32>(CurrentState)
			);

			ShowPersistenceMessage(
				TEXT(
					"Save postponed: NPC relocation "
					"is still in progress."
				),
				FColor::Yellow
			);

			return false;
		}

		const int32 StoryStage =
			StoryNPC->GetCurrentStoryStage();

		const FName AnchorID =
			StoryNPC->GetCurrentStoryAnchorID();

		if (StoryStage < 0)
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"Story NPC [%s] has invalid Stage %d."
				),
				*StoryNPC->GetStoryNPCID().ToString(),
				StoryStage
			);

			return false;
		}

		if (AnchorID.IsNone())
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"Story NPC [%s] Stage %d has no "
					"valid stable Anchor ID."
				),
				*StoryNPC->GetStoryNPCID().ToString(),
				StoryStage
			);

			return false;
		}

		FWCSavedStoryNPCState SavedState;
		SavedState.StoryNPCID =
			StoryNPC->GetStoryNPCID();
		SavedState.StoryStage = StoryStage;
		SavedState.AnchorID = AnchorID;

		CapturedStoryNPCStates.Add(SavedState);

		UE_LOG(
			LogWCStoryPersistence,
			Display,
			TEXT(
				"Captured Story NPC [%s]: "
				"Stage=%d, Anchor=[%s]."
			),
			*SavedState.StoryNPCID.ToString(),
			SavedState.StoryStage,
			*SavedState.AnchorID.ToString()
		);
	}

	/*
	* Capture Objectives
	*/
	FoundActors.Reset();

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AStoryObjectiveBase::StaticClass(),
		FoundActors
	);

	for (AActor* Actor : FoundActors)
	{
		const AStoryObjectiveBase* Objective =
			Cast<AStoryObjectiveBase>(Actor);

		if (!IsValid(Objective))
		{
			return false;
		}

		FWCSavedObjectiveState SavedState;
		SavedState.ObjectiveID =
			Objective->GetObjectiveID();
		SavedState.bCompleted =
			Objective->GetIsObjectiveComplete();

		CapturedObjectiveStates.Add(SavedState);

		UE_LOG(
			LogWCStoryPersistence,
			Display,
			TEXT(
				"Captured Objective [%s]: "
				"Completed=%s."
			),
			*SavedState.ObjectiveID.ToString(),
			SavedState.bCompleted
			? TEXT("True")
			: TEXT("False")
		);
	}

	/*
	* Capture Encounters
	*/
	FoundActors.Reset();

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AStoryEncounter::StaticClass(),
		FoundActors
	);

	for (AActor* Actor : FoundActors)
	{
		const AStoryEncounter* Encounter =
			Cast<AStoryEncounter>(Actor);

		if (!IsValid(Encounter))
		{
			return false;
		}

		FWCSavedEncounterState SavedState;
		SavedState.EncounterID =
			Encounter->GetEncounterID();
		SavedState.bCompleted =
			Encounter->GetIsEncounterCompleted();

		CapturedEncounterStates.Add(SavedState);

		UE_LOG(
			LogWCStoryPersistence,
			Display,
			TEXT(
				"Captured Encounter [%s]: "
				"Completed=%s."
			),
			*SavedState.EncounterID.ToString(),
			SavedState.bCompleted
			? TEXT("True")
			: TEXT("False")
		);
	}

	/*
	* Capture Journal Echoes
	*/
	TSet<FName> RecordedEchoIDs;

	for (const FMemoryEchoData& EchoData :
		Player->GetRecordedMemoryEchoes())
	{
		if (EchoData.EchoID.IsNone())
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"Recorded Memory Echo has ID None."
				)
			);

			return false;
		}

		if (RecordedEchoIDs.Contains(
			EchoData.EchoID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"duplicate recorded EchoID [%s]."
				),
				*EchoData.EchoID.ToString()
			);

			return false;
		}

		RecordedEchoIDs.Add(EchoData.EchoID);
		CapturedMemoryEchoes.Add(EchoData);

		UE_LOG(
			LogWCStoryPersistence,
			Display,
			TEXT(
				"Captured Journal Echo [%s]: [%s]."
			),
			*EchoData.EchoID.ToString(),
			*EchoData.Title.ToString()
		);
	}

	/*
	* 准备内存中的工作 SaveGame。
	*
	* 若当前没有 LoadedSaveData：
	* - 磁盘已有 Slot → 先加载
	* - 磁盘没有 Slot → 创建新 Save
	*/
	UWCGameSaveGame* SaveData =
		GameInstance->GetLoadedSaveData();

	if (!IsValid(SaveData))
	{
		if (GameInstance->HasSavedGame())
		{
			if (!GameInstance->LoadSavedGame())
			{
				UE_LOG(
					LogWCStoryPersistence,
					Error,
					TEXT(
						"World-state capture failed: "
						"existing disk save could not be loaded."
					)
				);

				return false;
			}

			SaveData =
				GameInstance->GetLoadedSaveData();
		}
		else
		{
			SaveData =
				GameInstance->CreateNewSave();
		}
	}

	if (!IsValid(SaveData))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"World-state capture failed: "
				"no valid UWCGameSaveGame is available."
			)
		);

		return false;
	}

	/*
	* 全部 Capture 成功后，才替换旧快照。
	*
	* Reset 保证不会把同一 Actor 在每次保存时
	* 重复追加到旧数组。
	*/
	SaveData->StoryNPCStates.Reset(
		CapturedStoryNPCStates.Num()
	);
	SaveData->StoryNPCStates.Append(
		CapturedStoryNPCStates
	);

	SaveData->ObjectiveStates.Reset(
		CapturedObjectiveStates.Num()
	);
	SaveData->ObjectiveStates.Append(
		CapturedObjectiveStates
	);

	SaveData->EncounterStates.Reset(
		CapturedEncounterStates.Num()
	);
	SaveData->EncounterStates.Append(
		CapturedEncounterStates
	);

	SaveData->RecordedMemoryEchoes.Reset(
		CapturedMemoryEchoes.Num()
	);
	SaveData->RecordedMemoryEchoes.Append(
		CapturedMemoryEchoes
	);

	/*
	* Checkpoint 系统今天尚未实现。
	*
	* 明确清理 Day1 Advanced 可能遗留的
	* PersistenceTest 值，避免把测试 ID 当成正式进度。
	*/
	SaveData->CurrentCheckpointID =
		NAME_None;

	if (!GameInstance->SaveCurrentGame())
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"World-state capture succeeded in memory, "
				"but disk save failed."
			)
		);

		ShowPersistenceMessage(
			TEXT("World captured, but disk save failed."),
			FColor::Red
		);

		return false;
	}

	UE_LOG(
		LogWCStoryPersistence,
		Display,
		TEXT(
			"World-state capture and save succeeded. "
			"NPCs=%d, Objectives=%d, Encounters=%d, "
			"Echoes=%d."
		),
		SaveData->StoryNPCStates.Num(),
		SaveData->ObjectiveStates.Num(),
		SaveData->EncounterStates.Num(),
		SaveData->RecordedMemoryEchoes.Num()
	);

	ShowPersistenceMessage(
		TEXT("Story world saved successfully."),
		FColor::Green
	);

	return true;
}

void AWCStoryPersistenceCoordinator::
PrintLoadedSaveSummary() const
{
	const UWCGameInstance* GameInstance =
		GetWCGameInstance();

	if (!IsValid(GameInstance))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Cannot print save summary: "
				"invalid WCGameInstance."
			)
		);

		return;
	}

	const UWCGameSaveGame* SaveData =
		GameInstance->GetLoadedSaveData();

	if (!IsValid(SaveData))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Warning,
			TEXT(
				"Cannot print save summary: "
				"no save is currently loaded."
			)
		);

		return;
	}

	UE_LOG(
		LogWCStoryPersistence,
		Display,
		TEXT(
			"========== WC SAVE SUMMARY =========="
		)
	);

	UE_LOG(
		LogWCStoryPersistence,
		Display,
		TEXT(
			"Version=%d, Checkpoint=[%s], "
			"NPCs=%d, Objectives=%d, "
			"Encounters=%d, Echoes=%d."
		),
		SaveData->SaveVersion,
		*SaveData->CurrentCheckpointID.ToString(),
		SaveData->StoryNPCStates.Num(),
		SaveData->ObjectiveStates.Num(),
		SaveData->EncounterStates.Num(),
		SaveData->RecordedMemoryEchoes.Num()
	);

	for (const FWCSavedStoryNPCState& NPCState :
		SaveData->StoryNPCStates)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Display,
			TEXT(
				"NPC [%s]: Stage=%d, Anchor=[%s]."
			),
			*NPCState.StoryNPCID.ToString(),
			NPCState.StoryStage,
			*NPCState.AnchorID.ToString()
		);
	}

	for (const FWCSavedObjectiveState& ObjectiveState :
		SaveData->ObjectiveStates)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Display,
			TEXT(
				"Objective [%s]: Completed=%s."
			),
			*ObjectiveState.ObjectiveID.ToString(),
			ObjectiveState.bCompleted
			? TEXT("True")
			: TEXT("False")
		);
	}

	for (const FWCSavedEncounterState& EncounterState :
		SaveData->EncounterStates)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Display,
			TEXT(
				"Encounter [%s]: Completed=%s."
			),
			*EncounterState.EncounterID.ToString(),
			EncounterState.bCompleted
			? TEXT("True")
			: TEXT("False")
		);
	}

	for (const FMemoryEchoData& EchoData :
		SaveData->RecordedMemoryEchoes)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Display,
			TEXT(
				"Journal Echo [%s]: [%s]."
			),
			*EchoData.EchoID.ToString(),
			*EchoData.Title.ToString()
		);
	}

	UE_LOG(
		LogWCStoryPersistence,
		Display,
		TEXT(
			"====================================="
		)
	);

	ShowPersistenceMessage(
		FString::Printf(
			TEXT(
				"Loaded Save: NPCs %d | Objectives %d | "
				"Encounters %d | Echoes %d"
			),
			SaveData->StoryNPCStates.Num(),
			SaveData->ObjectiveStates.Num(),
			SaveData->EncounterStates.Num(),
			SaveData->RecordedMemoryEchoes.Num()
		),
		FColor::Cyan
	);
}

void AWCStoryPersistenceCoordinator::ShowPersistenceMessage(
	const FString& Message,
	const FColor& Color
) const
{
	if (!bShowOnScreenDebug ||
		!GEngine)
	{
		return;
	}

	GEngine->AddOnScreenDebugMessage(
		-1,
		5.0f,
		Color,
		Message
	);
}