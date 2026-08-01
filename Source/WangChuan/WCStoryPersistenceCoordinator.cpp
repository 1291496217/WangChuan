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
#include "WCPlayerCheckpoint.h"


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

		ShowPersistenceMessage(
			TEXT(
				"Startup failed: invalid WCGameInstance."
			),
			FColor::Red
		);

		return;
	}

	if (!GameInstance->HasSavedGame())
	{
		/*
		* 没有 Save 是正常的新游戏状态，
		* 不是 Error。
		*
		* 不创建空存档，也不移动玩家。
		* 只初始化玩家当前 Runtime Checkpoint。
		*/
		UE_LOG(
			LogWCStoryPersistence,
			Display,
			TEXT(
				"No save slot found. "
				"Default new-game World remains active."
			)
		);

		if (!InitializeDefaultCheckpointForNewGame())
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"New Game startup failed: "
					"default Checkpoint could not "
					"be initialized."
				)
			);

			ShowPersistenceMessage(
				TEXT(
					"New Game Checkpoint "
					"initialization failed."
				),
				FColor::Red
			);

			return;
		}

		ShowPersistenceMessage(
			TEXT(
				"No save found — starting new game."
			),
			FColor::Silver
		);

		return;
	}

	if (!LoadAndRestoreWorldState())
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Deferred automatic Load/Restore failed."
			)
		);

		ShowPersistenceMessage(
			TEXT(
				"Automatic World Restore failed."
			),
			FColor::Red
		);
	}
}

bool AWCStoryPersistenceCoordinator::BuildWorldActorMaps(
	TMap<FName, AWCStoryNPC*>& OutStoryNPCs,
	TMap<FName, AStoryObjectiveBase*>& OutObjectives,
	TMap<FName, AStoryEncounter*>& OutEncounters,
	TMap<FName, AEchoRelic*>& OutEchoRelics,
	TMap<FName, AStoryAnchor*>& OutAnchors,
	TMap<FName, AWCPlayerCheckpoint*>& OutCheckpoints
) const
{
	OutStoryNPCs.Reset();
	OutObjectives.Reset();
	OutEncounters.Reset();
	OutEchoRelics.Reset();
	OutAnchors.Reset();
	OutCheckpoints.Reset();

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
* Story Anchors
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

	/*
	* Player Checkpoints
	*/
	FoundActors.Reset();

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AWCPlayerCheckpoint::StaticClass(),
		FoundActors
	);

	for (AActor* Actor : FoundActors)
	{
		AWCPlayerCheckpoint* Checkpoint =
			Cast<AWCPlayerCheckpoint>(Actor);

		if (!IsValid(Checkpoint))
		{
			return false;
		}

		OutCheckpoints.Add(
			Checkpoint->GetCheckpointID(),
			Checkpoint
		);
	}

	UE_LOG(
		LogWCStoryPersistence,
		Display,
		TEXT(
			"Built World Persistence maps: "
			"NPCs=%d, Objectives=%d, Encounters=%d, "
			"Echoes=%d, Anchors=%d, Checkpoints=%d."
		),
		OutStoryNPCs.Num(),
		OutObjectives.Num(),
		OutEncounters.Num(),
		OutEchoRelics.Num(),
		OutAnchors.Num(),
		OutCheckpoints.Num()
	);

	return true;
}

bool AWCStoryPersistenceCoordinator::
ValidateLoadedSaveDataForWorld(
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
	WorldAnchors,
	const TMap<FName, AWCPlayerCheckpoint*>&
	WorldCheckpoints
) const
{
	/*
	* 1. SaveGame 基础有效性
	*/
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
	* Save V1 是当前单地图的完整 Story 快照。
	*
	* NPC、Objective、Encounter 的记录数量
	* 应与当前正式 World 中的 Actor 数量一致。
	*
	* Echo Relic 不是独立存档数组；
	* Journal 只保存已经完成阅读的 Echo，
	* 所以这里不能要求 Journal 数量等于 Relic 数量。
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

	/*
	* 2. Checkpoint 验证
	*
	* 必须在任何 World Actor 被修改前，
	* 确认存档中的 Checkpoint ID 有效。
	*/
	if (SaveData->CurrentCheckpointID.IsNone())
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Restore validation failed: "
				"CurrentCheckpointID is None."
			)
		);

		return false;
	}

	if (!WorldCheckpoints.Contains(
		SaveData->CurrentCheckpointID))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Restore validation failed: "
				"Checkpoint [%s] does not exist "
				"in the current World."
			),
			*SaveData
			->CurrentCheckpointID
			.ToString()
		);

		return false;
	}

	/*
	* 用于检测 Save 内部的重复 ID。
	*/
	TSet<FName> SavedStoryNPCIDs;
	TSet<FName> SavedObjectiveIDs;
	TSet<FName> SavedEncounterIDs;
	TSet<FName> SavedJournalEchoIDs;

	/*
	* 用于之后验证：
	*
	* Encounter Complete
	* → Objective 必须 Complete
	*
	* Encounter Complete
	* ↔ Journal 必须包含对应 Echo
	*/
	TMap<FName, bool>
		SavedObjectiveCompletion;

	TMap<FName, bool>
		SavedEncounterCompletion;

	/*
	* 3. Story NPC Records
	*/
	for (const FWCSavedStoryNPCState& NPCState :
		SaveData->StoryNPCStates)
	{
		if (NPCState.StoryNPCID.IsNone())
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"saved Story NPC ID is None."
				)
			);

			return false;
		}

		if (SavedStoryNPCIDs.Contains(
			NPCState.StoryNPCID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"duplicate saved Story NPC "
					"ID [%s]."
				),
				*NPCState
				.StoryNPCID
				.ToString()
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
					"Restore validation failed: "
					"saved Story NPC [%s] "
					"does not exist in the "
					"current World."
				),
				*NPCState
				.StoryNPCID
				.ToString()
			);

			return false;
		}

		if (NPCState.StoryStage < 0)
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"saved Story NPC [%s] "
					"has invalid Stage=%d."
				),
				*NPCState
				.StoryNPCID
				.ToString(),
				NPCState.StoryStage
			);

			return false;
		}

		if (NPCState.AnchorID.IsNone())
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"saved Story NPC [%s] "
					"has AnchorID None."
				),
				*NPCState
				.StoryNPCID
				.ToString()
			);

			return false;
		}

		if (!WorldAnchors.Contains(
			NPCState.AnchorID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"saved Anchor [%s] for "
					"Story NPC [%s] does not "
					"exist in the current World."
				),
				*NPCState
				.AnchorID
				.ToString(),
				*NPCState
				.StoryNPCID
				.ToString()
			);

			return false;
		}

		/*
		* Anchor 不仅需要存在于地图中，
		* 还必须属于这个 NPC 的 StoryAnchors 配置。
		*/
		if (!(*StoryNPCPtr)->HasStoryAnchorID(
			NPCState.AnchorID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"Anchor [%s] is not configured "
					"for Story NPC [%s]."
				),
				*NPCState
				.AnchorID
				.ToString(),
				*NPCState
				.StoryNPCID
				.ToString()
			);

			return false;
		}

		SavedStoryNPCIDs.Add(
			NPCState.StoryNPCID
		);
	}

	/*
	* 4. Objective Records
	*/
	for (const FWCSavedObjectiveState&
		ObjectiveState :
		SaveData->ObjectiveStates)
	{
		if (ObjectiveState.ObjectiveID.IsNone())
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"saved Objective ID is None."
				)
			);

			return false;
		}

		if (SavedObjectiveIDs.Contains(
			ObjectiveState.ObjectiveID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"duplicate saved Objective "
					"ID [%s]."
				),
				*ObjectiveState
				.ObjectiveID
				.ToString()
			);

			return false;
		}

		if (!WorldObjectives.Contains(
			ObjectiveState.ObjectiveID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"saved Objective [%s] "
					"does not exist in the "
					"current World."
				),
				*ObjectiveState
				.ObjectiveID
				.ToString()
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
	* 5. Encounter Records
	*/
	for (const FWCSavedEncounterState&
		EncounterState :
		SaveData->EncounterStates)
	{
		if (EncounterState.EncounterID.IsNone())
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"saved Encounter ID is None."
				)
			);

			return false;
		}

		if (SavedEncounterIDs.Contains(
			EncounterState.EncounterID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"duplicate saved Encounter "
					"ID [%s]."
				),
				*EncounterState
				.EncounterID
				.ToString()
			);

			return false;
		}

		if (!WorldEncounters.Contains(
			EncounterState.EncounterID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"saved Encounter [%s] "
					"does not exist in the "
					"current World."
				),
				*EncounterState
				.EncounterID
				.ToString()
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
	* 6. Journal Records
	*
	* Journal 中的每个 Echo：
	* - ID 不能为 None
	* - 不能重复
	* - 必须对应当前 World 中的 Echo Relic
	*/
	for (const FMemoryEchoData& EchoData :
		SaveData->RecordedMemoryEchoes)
	{
		if (EchoData.EchoID.IsNone())
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"saved Journal Echo ID is None."
				)
			);

			return false;
		}

		if (SavedJournalEchoIDs.Contains(
			EchoData.EchoID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"duplicate saved Journal "
					"Echo ID [%s]."
				),
				*EchoData
				.EchoID
				.ToString()
			);

			return false;
		}

		if (!WorldEchoRelics.Contains(
			EchoData.EchoID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"saved Journal Echo [%s] "
					"does not exist in the "
					"current World."
				),
				*EchoData
				.EchoID
				.ToString()
			);

			return false;
		}

		SavedJournalEchoIDs.Add(
			EchoData.EchoID
		);
	}

	/*
	* 7. 验证 Encounter → Objective → Relic 关系
	*
	* 同时验证跨系统持久事实的一致性。
	*/
	TSet<FName> EncounterRelicIDs;

	for (const TPair<
		FName,
		AStoryEncounter*
	>& Pair : WorldEncounters)
	{
		const FName EncounterID =
			Pair.Key;

		AStoryEncounter* Encounter =
			Pair.Value;

		if (!IsValid(Encounter))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"Encounter [%s] Actor "
					"is invalid."
				),
				*EncounterID.ToString()
			);

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
					"Restore validation failed: "
					"Encounter [%s] has an "
					"invalid Objective or "
					"Echo Relic reference."
				),
				*EncounterID.ToString()
			);

			return false;
		}

		const FName ObjectiveID =
			Objective->GetObjectiveID();

		const FName EchoID =
			EchoRelic->GetEchoID();

		if (ObjectiveID.IsNone() ||
			EchoID.IsNone())
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"Encounter [%s] references "
					"an Objective or Echo with "
					"ID None."
				),
				*EncounterID.ToString()
			);

			return false;
		}

		/*
		* Encounter 配置中的 Objective / Relic
		* 也必须存在于已经建立的 World Maps 中。
		*/
		if (!WorldObjectives.Contains(
			ObjectiveID) ||
			!WorldEchoRelics.Contains(
				EchoID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"Encounter [%s] references "
					"Objective [%s] or Echo [%s] "
					"that is missing from the "
					"World maps."
				),
				*EncounterID.ToString(),
				*ObjectiveID.ToString(),
				*EchoID.ToString()
			);

			return false;
		}

		const bool* SavedObjectiveComplete =
			SavedObjectiveCompletion.Find(
				ObjectiveID
			);

		const bool* SavedEncounterComplete =
			SavedEncounterCompletion.Find(
				EncounterID
			);

		if (!SavedObjectiveComplete)
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"Encounter [%s] references "
					"Objective [%s], but the "
					"SaveGame has no matching "
					"Objective record."
				),
				*EncounterID.ToString(),
				*ObjectiveID.ToString()
			);

			return false;
		}

		if (!SavedEncounterComplete)
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"SaveGame has no matching "
					"record for Encounter [%s]."
				),
				*EncounterID.ToString()
			);

			return false;
		}

		/*
		* 当前 Demo 约定一个 Echo Relic
		* 只能属于一个 Encounter。
		*/
		if (EncounterRelicIDs.Contains(
			EchoID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"Echo Relic [%s] is assigned "
					"to more than one Encounter."
				),
				*EchoID.ToString()
			);

			return false;
		}

		EncounterRelicIDs.Add(EchoID);

		/*
		* Encounter 完成意味着其 Objective
		* 必须已经完成。
		*/
		if ((*SavedEncounterComplete) &&
			!(*SavedObjectiveComplete))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"saved Encounter [%s] is "
					"complete while Objective "
					"[%s] is incomplete."
				),
				*EncounterID.ToString(),
				*ObjectiveID.ToString()
			);

			return false;
		}

		/*
		* 当前剧情流程中：
		*
		* Encounter 完成
		* ↔ 对应 Echo 已完整阅读并记录到 Journal
		*/
		const bool bJournalContainsEcho =
			SavedJournalEchoIDs.Contains(
				EchoID
			);

		if ((*SavedEncounterComplete) !=
			bJournalContainsEcho)
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"Encounter [%s] and Journal "
					"Echo [%s] disagree. "
					"EncounterCompleted=%s, "
					"JournalContainsEcho=%s."
				),
				*EncounterID.ToString(),
				*EchoID.ToString(),
				(*SavedEncounterComplete)
				? TEXT("True")
				: TEXT("False"),
				bJournalContainsEcho
				? TEXT("True")
				: TEXT("False")
			);

			return false;
		}
	}

	/*
	* 每个 World Echo Relic 都必须且只能
	* 由一个 Encounter 管理。
	*/
	if (EncounterRelicIDs.Num() !=
		WorldEchoRelics.Num())
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Restore validation failed: "
				"not every Echo Relic is assigned "
				"to exactly one Encounter. "
				"Encounter Relics=%d, "
				"World Relics=%d."
			),
			EncounterRelicIDs.Num(),
			WorldEchoRelics.Num()
		);

		return false;
	}

	UE_LOG(
		LogWCStoryPersistence,
		Display,
		TEXT(
			"Loaded Save validation succeeded. "
			"Checkpoint=[%s], NPCs=%d, "
			"Objectives=%d, Encounters=%d, "
			"JournalEchoes=%d."
		),
		*SaveData
		->CurrentCheckpointID
		.ToString(),
		SaveData->StoryNPCStates.Num(),
		SaveData->ObjectiveStates.Num(),
		SaveData->EncounterStates.Num(),
		SaveData->RecordedMemoryEchoes.Num()
	);

	return true;
}

bool AWCStoryPersistenceCoordinator::
LoadAndRestoreWorldState()
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

bool AWCStoryPersistenceCoordinator::
RestoreLoadedWorldState()
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
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Restore failed: invalid WCGameInstance."
			)
		);

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

	TMap<FName, AWCPlayerCheckpoint*>
		WorldCheckpoints;

	if (!BuildWorldActorMaps(
		WorldStoryNPCs,
		WorldObjectives,
		WorldEncounters,
		WorldEchoRelics,
		WorldAnchors,
		WorldCheckpoints
	))
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
		WorldAnchors,
		WorldCheckpoints
	))
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

	/*
	* 在任何 World Actor 被修改前解析 Checkpoint，
	* 并完成地面 Trace 与安全落点计算。
	*/
	AWCPlayerCheckpoint* const* SavedCheckpointPtr =
		WorldCheckpoints.Find(
			SaveData->CurrentCheckpointID
		);

	if (!SavedCheckpointPtr ||
		!IsValid(*SavedCheckpointPtr))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Restore failed: saved Checkpoint "
				"could not be resolved."
			)
		);

		return false;
	}

	FTransform SafePlayerResumeTransform;

	if (!(*SavedCheckpointPtr)
		->BuildSafeResumeTransform(
			Player,
			SafePlayerResumeTransform
		))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Restore failed: Checkpoint [%s] "
				"could not build a safe "
				"Resume Transform."
			),
			*SaveData
				->CurrentCheckpointID
				.ToString()
		);

		return false;
	}

	UE_LOG(
		LogWCStoryPersistence,
		Display,
		TEXT(
			"Beginning ordered silent World Restore. "
			"Checkpoint=[%s]."
		),
		*SaveData->CurrentCheckpointID.ToString()
	);

	/*
	* Phase 1: Objectives
	*
	* Enemy defeated presentation and Lantern final
	* presentation are restored through subclass hooks.
	*/
	for (const FWCSavedObjectiveState& ObjectiveState :
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
	for (const FWCSavedEncounterState& EncounterState :
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

		if (Encounter->GetIsEncounterCompleted())
		{
			DerivedRelicState =
				EEchoRelicState::Activated;
		}
		else if (Objective->GetIsObjectiveComplete())
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
			static_cast<int32>(DerivedRelicState),
			*Objective->GetObjectiveID().ToString(),
			*Encounter->GetEncounterID().ToString()
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

		if (!(*StoryNPCPtr)->ApplySavedStoryState(
			NPCState.StoryStage,
			NPCState.AnchorID
		))
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

	if (Player->GetRecordedMemoryEchoes().Num() !=
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

	/*
	* Phase 6: Player Checkpoint
	*
	* 玩家必须最后恢复，确保进入目标位置时，
	* 碰撞、交互、NPC 和 Journal 均已处于最终状态。
	*/
	if (!Player->ApplySavedCheckpointState(
		SaveData->CurrentCheckpointID,
		SafePlayerResumeTransform
	))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Restore failed while applying "
				"Player Checkpoint [%s]."
			),
			*SaveData
				->CurrentCheckpointID
				.ToString()
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
			"RestoreCount=%d, Checkpoint=[%s], "
			"NPCs=%d, Objectives=%d, Encounters=%d, "
			"Echoes=%d."
		),
		SuccessfulRestoreCount,
		*SaveData->CurrentCheckpointID.ToString(),
		SaveData->StoryNPCStates.Num(),
		SaveData->ObjectiveStates.Num(),
		SaveData->EncounterStates.Num(),
		SaveData->RecordedMemoryEchoes.Num()
	);

	ShowPersistenceMessage(
		FString::Printf(
			TEXT(
				"World Restored | Checkpoint: %s | "
				"NPCs %d | Objectives %d | "
				"Encounters %d | Echoes %d"
			),
			*SaveData->CurrentCheckpointID.ToString(),
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

	/*
	* Player Checkpoint IDs
	*
	* 当前地图必须至少有一个 Checkpoint，
	* 且必须恰好有一个 Default Checkpoint。
	*/
	FoundActors.Reset();

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AWCPlayerCheckpoint::StaticClass(),
		FoundActors
	);

	if (FoundActors.Num() == 0)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Persistence validation failed: "
				"no AWCPlayerCheckpoint found."
			)
		);

		bAllIDsValid = false;
	}

	TSet<FName> CheckpointIDs;
	int32 DefaultCheckpointCount = 0;

	for (AActor* Actor : FoundActors)
	{
		const AWCPlayerCheckpoint* Checkpoint =
			Cast<AWCPlayerCheckpoint>(Actor);

		if (!IsValid(Checkpoint))
		{
			bAllIDsValid = false;
			continue;
		}

		if (!RegisterPersistenceID(
			Checkpoint->GetCheckpointID(),
			Checkpoint,
			TEXT("Player Checkpoint"),
			CheckpointIDs
		))
		{
			bAllIDsValid = false;
		}

		if (Checkpoint->GetIsDefaultCheckpoint())
		{
			++DefaultCheckpointCount;
		}
	}

	if (DefaultCheckpointCount != 1)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Persistence validation failed: "
				"expected exactly one Default "
				"Checkpoint, found %d."
			),
			DefaultCheckpointCount
		);

		bAllIDsValid = false;
	}

	if (bAllIDsValid)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Display,
			TEXT(
				"Persistence ID validation succeeded. "
				"NPCs=%d, Objectives=%d, Encounters=%d, "
				"Echoes=%d, Anchors=%d, Checkpoints=%d."
			),
			StoryNPCIDs.Num(),
			ObjectiveIDs.Num(),
			EncounterIDs.Num(),
			EchoIDs.Num(),
			AnchorIDs.Num(),
			CheckpointIDs.Num()
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
			TEXT(
				"Save failed: invalid WCGameInstance."
			),
			FColor::Red
		);

		return false;
	}

	/*
	* 必须先验证整个世界。
	*
	* 若存在空 ID、重复 ID、错误的 Default Checkpoint 数量，
	* 不允许写出一份无法可靠恢复的存档。
	*/
	if (!ValidateWorldPersistenceIDs())
	{
		ShowPersistenceMessage(
			TEXT(
				"Save failed: Persistence ID "
				"validation failed."
			),
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

		ShowPersistenceMessage(
			TEXT(
				"Save failed: invalid player."
			),
			FColor::Red
		);

		return false;
	}

	/*
	* Capture 当前 Runtime Checkpoint。
	*
	* Checkpoint 激活只更新 Player Runtime State。
	* 真正写入磁盘发生在本函数中。
	*/
	const FName CapturedCheckpointID =
		Player->GetCurrentCheckpointID();

	if (CapturedCheckpointID.IsNone())
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"World-state capture failed: "
				"Player CurrentCheckpointID is None."
			)
		);

		ShowPersistenceMessage(
			TEXT(
				"Save failed: no active Checkpoint."
			),
			FColor::Red
		);

		return false;
	}

	/*
	* CurrentCheckpointID 非 None 并不代表该 ID
	* 一定对应当前 World 中的有效 Checkpoint Actor。
	*
	* 因此在保存前再次确认。
	*/
	TArray<AActor*> CheckpointActors;

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AWCPlayerCheckpoint::StaticClass(),
		CheckpointActors
	);

	AWCPlayerCheckpoint* CapturedCheckpoint =
		nullptr;

	for (AActor* Actor : CheckpointActors)
	{
		AWCPlayerCheckpoint* Checkpoint =
			Cast<AWCPlayerCheckpoint>(Actor);

		if (!IsValid(Checkpoint))
		{
			continue;
		}

		if (Checkpoint->GetCheckpointID() ==
			CapturedCheckpointID)
		{
			CapturedCheckpoint = Checkpoint;
			break;
		}
	}

	if (!IsValid(CapturedCheckpoint))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"World-state capture failed: "
				"Checkpoint [%s] does not exist "
				"in the current World."
			),
			*CapturedCheckpointID.ToString()
		);

		ShowPersistenceMessage(
			TEXT(
				"Save failed: active Checkpoint "
				"does not exist."
			),
			FColor::Red
		);

		return false;
	}

	/*
	* 先将所有状态收集到局部数组。
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
	* Phase 1: Capture Story NPCs
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
		* Encounter 完成后，NPC 会经历：
		*
		* EventResolved
		* → Relocating
		* → Available
		*
		* 只有最终 Available 状态才是稳定存档点。
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
				*StoryNPC
				->GetStoryNPCID()
				.ToString(),
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
					"Story NPC [%s] has invalid "
					"Stage %d."
				),
				*StoryNPC
				->GetStoryNPCID()
				.ToString(),
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
				*StoryNPC
				->GetStoryNPCID()
				.ToString(),
				StoryStage
			);

			return false;
		}

		FWCSavedStoryNPCState SavedState;

		SavedState.StoryNPCID =
			StoryNPC->GetStoryNPCID();

		SavedState.StoryStage =
			StoryStage;

		SavedState.AnchorID =
			AnchorID;

		CapturedStoryNPCStates.Add(
			SavedState
		);

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
	* Phase 2: Capture Objectives
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
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"invalid Story Objective Actor."
				)
			);

			return false;
		}

		FWCSavedObjectiveState SavedState;

		SavedState.ObjectiveID =
			Objective->GetObjectiveID();

		SavedState.bCompleted =
			Objective->GetIsObjectiveComplete();

		CapturedObjectiveStates.Add(
			SavedState
		);

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
	* Phase 3: Capture Encounters
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
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"invalid Story Encounter Actor."
				)
			);

			return false;
		}

		FWCSavedEncounterState SavedState;

		SavedState.EncounterID =
			Encounter->GetEncounterID();

		SavedState.bCompleted =
			Encounter->GetIsEncounterCompleted();

		CapturedEncounterStates.Add(
			SavedState
		);

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
	* Phase 4: Capture Memory Journal
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

		RecordedEchoIDs.Add(
			EchoData.EchoID
		);

		CapturedMemoryEchoes.Add(
			EchoData
		);

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
	* 准备当前内存中的工作 SaveGame。
	*
	* 如果当前没有 LoadedSaveData：
	*
	* 磁盘已有 Slot
	* → 先加载旧存档，避免无意创建空对象覆盖数据。
	*
	* 磁盘没有 Slot
	* → 创建新的内存 SaveGame。
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
						"existing disk save could not "
						"be loaded."
					)
				);

				ShowPersistenceMessage(
					TEXT(
						"Save failed: existing save "
						"could not be loaded."
					),
					FColor::Red
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
				"no valid UWCGameSaveGame "
				"is available."
			)
		);

		ShowPersistenceMessage(
			TEXT(
				"Save failed: invalid SaveGame object."
			),
			FColor::Red
		);

		return false;
	}

	/*
	* 所有 Capture 和验证都成功之后，
	* 才替换 LoadedSaveData 中的旧快照。
	*
	* Reset + Append 可以防止重复保存时
	* 不断向旧数组追加相同 Actor。
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
	* 保存玩家最近激活的稳定 Checkpoint ID。
	*
	* 不保存玩家的原始 Transform。
	* 加载时由当前地图中的 Checkpoint Actor
	* 重新生成安全 Resume Transform。
	*/
	SaveData->CurrentCheckpointID =
		CapturedCheckpointID;

	UE_LOG(
		LogWCStoryPersistence,
		Display,
		TEXT(
			"Captured Player Checkpoint [%s]."
		),
		*SaveData
		->CurrentCheckpointID
		.ToString()
	);

	/*
	* 只在完整快照准备完成后写入磁盘。
	*/
	if (!GameInstance->SaveCurrentGame())
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"World-state capture succeeded "
				"in memory, but disk save failed."
			)
		);

		ShowPersistenceMessage(
			TEXT(
				"World captured, but disk save failed."
			),
			FColor::Red
		);

		return false;
	}

	UE_LOG(
		LogWCStoryPersistence,
		Display,
		TEXT(
			"World-state capture and save succeeded. "
			"Checkpoint=[%s], NPCs=%d, "
			"Objectives=%d, Encounters=%d, "
			"Echoes=%d."
		),
		*SaveData
		->CurrentCheckpointID
		.ToString(),
		SaveData->StoryNPCStates.Num(),
		SaveData->ObjectiveStates.Num(),
		SaveData->EncounterStates.Num(),
		SaveData->RecordedMemoryEchoes.Num()
	);

	ShowPersistenceMessage(
		FString::Printf(
			TEXT(
				"Story Saved | Checkpoint: %s | "
				"NPCs %d | Objectives %d | "
				"Encounters %d | Echoes %d"
			),
			*SaveData
			->CurrentCheckpointID
			.ToString(),
			SaveData->StoryNPCStates.Num(),
			SaveData->ObjectiveStates.Num(),
			SaveData->EncounterStates.Num(),
			SaveData->RecordedMemoryEchoes.Num()
		),
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

bool AWCStoryPersistenceCoordinator::
InitializeDefaultCheckpointForNewGame()
{
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
				"Default Checkpoint initialization "
				"failed: invalid Player."
			)
		);

		return false;
	}

	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AWCPlayerCheckpoint::StaticClass(),
		FoundActors
	);

	AWCPlayerCheckpoint*
		DefaultCheckpoint = nullptr;

	int32 DefaultCheckpointCount = 0;

	for (AActor* Actor : FoundActors)
	{
		AWCPlayerCheckpoint* Checkpoint =
			Cast<AWCPlayerCheckpoint>(Actor);

		if (!IsValid(Checkpoint) ||
			!Checkpoint
			->GetIsDefaultCheckpoint())
		{
			continue;
		}

		DefaultCheckpoint = Checkpoint;
		++DefaultCheckpointCount;
	}

	if (DefaultCheckpointCount != 1 ||
		!IsValid(DefaultCheckpoint))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Default Checkpoint initialization "
				"failed: expected exactly one "
				"Default Checkpoint, found %d."
			),
			DefaultCheckpointCount
		);

		return false;
	}

	Player->SetCurrentCheckpointID(
		DefaultCheckpoint->GetCheckpointID()
	);

	UE_LOG(
		LogWCStoryPersistence,
		Display,
		TEXT(
			"New Game Runtime Checkpoint initialized "
			"to [%s]. Player was not moved."
		),
		*DefaultCheckpoint
		->GetCheckpointID()
		.ToString()
	);

	return true;
}
