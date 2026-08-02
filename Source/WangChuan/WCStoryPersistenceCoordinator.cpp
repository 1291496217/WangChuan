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

	TGuardValue<bool> StartupRestoreGuard(
		bStartupRestoreRequestInProgress,
		true
	);

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
	 * ----------------------------------------------------
	 * 1. SaveGame 基础有效性
	 * ----------------------------------------------------
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
	 * 当前 Version 2 仍然是：
	 *
	 * 单地图
	 * 完整 Story 快照
	 *
	 * 因此 NPC、Objective、Encounter 的存档数量
	 * 必须与当前地图中的正式 Actor 数量一致。
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
	 * ----------------------------------------------------
	 * 2. Player Checkpoint 基础验证
	 * ----------------------------------------------------
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

	AWCPlayerCheckpoint* const*
		CurrentCheckpointPtr =
		WorldCheckpoints.Find(
			SaveData->CurrentCheckpointID
		);

	if (!CurrentCheckpointPtr ||
		!IsValid(*CurrentCheckpointPtr))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Restore validation failed: "
				"Current Checkpoint [%s] does not "
				"exist in the current World."
			),
			*SaveData
			->CurrentCheckpointID
			.ToString()
		);

		return false;
	}

	if (SaveData
		->UnlockedCheckpointIDs
		.Num() == 0)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Restore validation failed: "
				"UnlockedCheckpointIDs is empty."
			)
		);

		return false;
	}

	TSet<FName> SavedUnlockedCheckpointIDs;

	for (const FName CheckpointID :
	SaveData->UnlockedCheckpointIDs)
	{
		if (CheckpointID.IsNone())
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"Unlocked Checkpoint ID is None."
				)
			);

			return false;
		}

		if (SavedUnlockedCheckpointIDs.Contains(
			CheckpointID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"duplicate unlocked "
					"Checkpoint ID [%s]."
				),
				*CheckpointID.ToString()
			);

			return false;
		}

		AWCPlayerCheckpoint* const*
			CheckpointPtr =
			WorldCheckpoints.Find(
				CheckpointID
			);

		if (!CheckpointPtr ||
			!IsValid(*CheckpointPtr))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"unlocked Checkpoint [%s] "
					"does not exist in the "
					"current World."
				),
				*CheckpointID.ToString()
			);

			return false;
		}

		SavedUnlockedCheckpointIDs.Add(
			CheckpointID
		);
	}

	if (!SavedUnlockedCheckpointIDs.Contains(
		SaveData->CurrentCheckpointID))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Restore validation failed: "
				"Current Checkpoint [%s] is not "
				"contained in "
				"UnlockedCheckpointIDs."
			),
			*SaveData
			->CurrentCheckpointID
			.ToString()
		);

		return false;
	}

	/*
	 * ----------------------------------------------------
	 * 3. 准备 ID Set 与完成状态 Map
	 * ----------------------------------------------------
	 */

	TSet<FName> SavedStoryNPCIDs;
	TSet<FName> SavedObjectiveIDs;
	TSet<FName> SavedEncounterIDs;
	TSet<FName> SavedJournalEchoIDs;

	TMap<FName, bool>
		SavedObjectiveCompletion;

	TMap<FName, bool>
		SavedEncounterCompletion;

	/*
	 * ----------------------------------------------------
	 * 4. Story NPC Records
	 * ----------------------------------------------------
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
					"saved Story NPC [%s] does "
					"not exist in the current World."
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
					"Story NPC [%s] has negative "
					"StoryStage=%d."
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
					"Story NPC [%s] has "
					"AnchorID None."
				),
				*NPCState
				.StoryNPCID
				.ToString()
			);

			return false;
		}

		AStoryAnchor* const* AnchorPtr =
			WorldAnchors.Find(
				NPCState.AnchorID
			);

		if (!AnchorPtr ||
			!IsValid(*AnchorPtr))
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
				*NPCState.AnchorID.ToString(),
				*NPCState
				.StoryNPCID
				.ToString()
			);

			return false;
		}

		/*
		 * Anchor 不仅要存在于地图，
		 * 还必须属于这个 NPC 配置的 StoryAnchors。
		 */

		if (!(*StoryNPCPtr)->HasStoryAnchorID(
			NPCState.AnchorID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"Anchor [%s] is not assigned "
					"to Story NPC [%s]."
				),
				*NPCState.AnchorID.ToString(),
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
	 * ----------------------------------------------------
	 * 5. Objective Records
	 * ----------------------------------------------------
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

		AStoryObjectiveBase* const*
			ObjectivePtr =
			WorldObjectives.Find(
				ObjectiveState.ObjectiveID
			);

		if (!ObjectivePtr ||
			!IsValid(*ObjectivePtr))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"saved Objective [%s] does "
					"not exist in the current World."
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
	 * ----------------------------------------------------
	 * 6. Encounter Records
	 * ----------------------------------------------------
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

		AStoryEncounter* const*
			EncounterPtr =
			WorldEncounters.Find(
				EncounterState.EncounterID
			);

		if (!EncounterPtr ||
			!IsValid(*EncounterPtr))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"saved Encounter [%s] does "
					"not exist in the current World."
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
	 * ----------------------------------------------------
	 * 7. Journal Echo Records
	 * ----------------------------------------------------
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
				*EchoData.EchoID.ToString()
			);

			return false;
		}

		AEchoRelic* const* EchoRelicPtr =
			WorldEchoRelics.Find(
				EchoData.EchoID
			);

		if (!EchoRelicPtr ||
			!IsValid(*EchoRelicPtr))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"Journal Echo [%s] does not "
					"have a matching Echo Relic "
					"in the current World."
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
	 * ----------------------------------------------------
	 * 8. Encounter → Objective → Echo Relic
	 *    关系及状态一致性验证
	 * ----------------------------------------------------
	 */

	TSet<FName> EncounterObjectiveIDs;
	TSet<FName> EncounterRelicIDs;

	for (const TPair<FName, AStoryEncounter*>&
		EncounterPair :
		WorldEncounters)
	{
		const FName EncounterID =
			EncounterPair.Key;

		AStoryEncounter* Encounter =
			EncounterPair.Value;

		if (!IsValid(Encounter))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"Encounter [%s] Actor "
					"pointer is invalid."
				),
				*EncounterID.ToString()
			);

			return false;
		}

		AStoryObjectiveBase* Objective =
			Encounter->GetStoryObjective();

		AEchoRelic* EchoRelic =
			Encounter->GetEchoRelic();

		if (!IsValid(Objective))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"Encounter [%s] has an "
					"invalid Story Objective."
				),
				*EncounterID.ToString()
			);

			return false;
		}

		if (!IsValid(EchoRelic))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"Encounter [%s] has an "
					"invalid Echo Relic."
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

		if (!WorldObjectives.Contains(
			ObjectiveID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"Encounter [%s] Objective "
					"[%s] is not registered in "
					"the World Objective map."
				),
				*EncounterID.ToString(),
				*ObjectiveID.ToString()
			);

			return false;
		}

		if (!WorldEchoRelics.Contains(
			EchoID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"Encounter [%s] Echo Relic "
					"[%s] is not registered in "
					"the World Echo map."
				),
				*EncounterID.ToString(),
				*EchoID.ToString()
			);

			return false;
		}

		/*
		 * 当前 Demo 中，一个 Objective 应只属于
		 * 一个 Encounter。
		 */

		if (EncounterObjectiveIDs.Contains(
			ObjectiveID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"Objective [%s] is assigned "
					"to more than one Encounter."
				),
				*ObjectiveID.ToString()
			);

			return false;
		}

		EncounterObjectiveIDs.Add(
			ObjectiveID
		);

		/*
		 * 当前 Demo 中，一个 Echo Relic 应只属于
		 * 一个 Encounter。
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
					"Objective [%s] used by "
					"Encounter [%s] has no "
					"saved record."
				),
				*ObjectiveID.ToString(),
				*EncounterID.ToString()
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
					"Encounter [%s] has no "
					"saved completion record."
				),
				*EncounterID.ToString()
			);

			return false;
		}

		/*
		 * Encounter 不可能在其 Objective
		 * 尚未完成时已经完成。
		 */

		if (*SavedEncounterComplete &&
			!*SavedObjectiveComplete)
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Restore validation failed: "
					"Encounter [%s] is complete "
					"while Objective [%s] is "
					"incomplete."
				),
				*EncounterID.ToString(),
				*ObjectiveID.ToString()
			);

			return false;
		}

		/*
		 * 当前 Story 流程中：
		 *
		 * Encounter 完成
		 * ⇔ 玩家已确认阅读该 Encounter 的 Echo
		 * ⇔ Journal 包含对应 Echo
		 */

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
					"Restore validation failed: "
					"Encounter [%s] and Journal "
					"Echo [%s] disagree. "
					"EncounterCompleted=%s, "
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

	/*
	 * 当前完整快照要求：
	 * 每个正式 Objective 都被一个 Encounter 使用。
	 */

	if (EncounterObjectiveIDs.Num() !=
		WorldObjectives.Num())
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Restore validation failed: "
				"not every Story Objective is "
				"assigned to exactly one Encounter. "
				"EncounterObjectives=%d, "
				"WorldObjectives=%d."
			),
			EncounterObjectiveIDs.Num(),
			WorldObjectives.Num()
		);

		return false;
	}

	/*
	 * 每个正式 Echo Relic 都必须归属于
	 * 一个且仅一个 Encounter。
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
				"EncounterRelics=%d, "
				"WorldRelics=%d."
			),
			EncounterRelicIDs.Num(),
			WorldEchoRelics.Num()
		);

		return false;
	}

	/*
	 * ----------------------------------------------------
	 * 9. 最终成功日志
	 * ----------------------------------------------------
	 */

	UE_LOG(
		LogWCStoryPersistence,
		Display,
		TEXT(
			"Loaded Save validation succeeded. "
			"Version=%d, "
			"NPCs=%d, Objectives=%d, "
			"Encounters=%d, JournalEchoes=%d, "
			"UnlockedCheckpoints=%d, "
			"CurrentCheckpoint=[%s]."
		),
		SaveData->SaveVersion,
		SaveData->StoryNPCStates.Num(),
		SaveData->ObjectiveStates.Num(),
		SaveData->EncounterStates.Num(),
		SaveData->RecordedMemoryEchoes.Num(),
		SaveData->UnlockedCheckpointIDs.Num(),
		*SaveData
		->CurrentCheckpointID
		.ToString()
	);

	return true;
}

bool AWCStoryPersistenceCoordinator::
LoadAndRestoreWorldState()
{
	if (!bStartupRestoreRequestInProgress &&
		!bAllowDirectDebugPersistenceActions)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Display,
			TEXT(
				"Direct Load/Restore request ignored. "
				"Player-facing loading occurs only during startup."
			)
		);

		return false;
	}

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
	* Checkpoint 的 Runtime Progress 与派生视觉必须在玩家移动前恢复。
	*
	* 这不会写入磁盘，也不会播放一次性的休憩成功表现。
	*/
	Player->ApplyRuntimeCheckpointProgress(
		SaveData->CurrentCheckpointID,
		SaveData->UnlockedCheckpointIDs
	);

	RefreshCheckpointPresentations(
		SaveData->UnlockedCheckpointIDs
	);

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

bool AWCStoryPersistenceCoordinator::SaveAtCheckpoint(
	AWCPlayerCheckpoint* Checkpoint)
{
	if (!IsValid(Checkpoint) ||
		Checkpoint->GetWorld() != GetWorld() ||
		Checkpoint->GetCheckpointID().IsNone() ||
		bRestoreInProgress)
	{
		return false;
	}

	AWCCharacter* Player =
		Cast<AWCCharacter>(
			UGameplayStatics::GetPlayerCharacter(
				this,
				0
			)
		);

	if (!IsValid(Player) ||
		!Player->CanUseCheckpoint())
	{
		ShowPersistenceMessage(
			TEXT("Cannot rest here right now."),
			FColor::Yellow
		);

		return false;
	}

	if (!Checkpoint->IsPlayerWithinInteractionRange(
		Player))
	{
		return false;
	}

	/*
	* Capture 也会再次校验当前 Checkpoint，
	* 这里先拒绝不安全的候选休憩点。
	*/
	FTransform SafeTransform;

	if (!Checkpoint->BuildSafeResumeTransform(
		Player,
		SafeTransform))
	{
		return false;
	}

	const FName PreviousCurrentID =
		Player->GetCurrentCheckpointID();

	const TArray<FName> PreviousUnlockedIDs =
		Player->GetUnlockedCheckpointIDs();

	TArray<FName> CandidateUnlockedIDs =
		PreviousUnlockedIDs;

	CandidateUnlockedIDs.AddUnique(
		Checkpoint->GetCheckpointID()
	);

	Player->ApplyRuntimeCheckpointProgress(
		Checkpoint->GetCheckpointID(),
		CandidateUnlockedIDs
	);

	TGuardValue<bool> SaveRequestGuard(
		bCheckpointSaveRequestInProgress,
		true
	);

	if (!CaptureAndSaveWorldState())
	{
		Player->ApplyRuntimeCheckpointProgress(
			PreviousCurrentID,
			PreviousUnlockedIDs
		);

		RefreshCheckpointPresentations(
			PreviousUnlockedIDs
		);

		UE_LOG(
			LogWCStoryPersistence,
			Warning,
			TEXT(
				"Checkpoint save failed. Runtime progress "
				"was rolled back to [%s]."
			),
			*PreviousCurrentID.ToString()
		);

		return false;
	}

	RefreshCheckpointPresentations(
		CandidateUnlockedIDs
	);

	ShowPersistenceMessage(
		FString::Printf(
			TEXT("Memory anchored at %s."),
			*Checkpoint
				->GetCheckpointDisplayName()
				.ToString()
		),
		FColor::Green
	);

	return true;
}

TArray<FWCCheckpointTravelOption>
AWCStoryPersistenceCoordinator::
GetCheckpointTravelOptions() const
{
	TArray<FWCCheckpointTravelOption> Options;

	const AWCCharacter* Player =
		Cast<AWCCharacter>(
			UGameplayStatics::GetPlayerCharacter(
				this,
				0
			)
		);

	if (!IsValid(Player))
	{
		return Options;
	}

	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AWCPlayerCheckpoint::StaticClass(),
		FoundActors
	);

	Options.Reserve(FoundActors.Num());

	for (AActor* Actor : FoundActors)
	{
		const AWCPlayerCheckpoint* Checkpoint =
			Cast<AWCPlayerCheckpoint>(Actor);

		if (!IsValid(Checkpoint) ||
			Checkpoint->GetCheckpointID().IsNone())
		{
			continue;
		}

		FWCCheckpointTravelOption Option;
		Option.CheckpointID =
			Checkpoint->GetCheckpointID();
		Option.DisplayName =
			Checkpoint->GetCheckpointDisplayName();
		Option.bUnlocked =
			Player->HasUnlockedCheckpoint(
				Option.CheckpointID
			);
		Option.bCurrent =
			Player->GetCurrentCheckpointID() ==
			Option.CheckpointID;
		Option.TravelOrder =
			Checkpoint->GetTravelOrder();

		Options.Add(Option);
	}

	Options.Sort(
		[](
			const FWCCheckpointTravelOption& A,
			const FWCCheckpointTravelOption& B)
		{
			return A.TravelOrder < B.TravelOrder;
		}
	);

	return Options;
}

AWCPlayerCheckpoint*
AWCStoryPersistenceCoordinator::FindCheckpointByID(
	FName CheckpointID) const
{
	if (CheckpointID.IsNone())
	{
		return nullptr;
	}

	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AWCPlayerCheckpoint::StaticClass(),
		FoundActors
	);

	AWCPlayerCheckpoint* Match = nullptr;

	for (AActor* Actor : FoundActors)
	{
		AWCPlayerCheckpoint* Checkpoint =
			Cast<AWCPlayerCheckpoint>(Actor);

		if (!IsValid(Checkpoint) ||
			Checkpoint->GetCheckpointID() !=
			CheckpointID)
		{
			continue;
		}

		if (IsValid(Match))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Checkpoint lookup failed: duplicate ID [%s]."
				),
				*CheckpointID.ToString()
			);

			return nullptr;
		}

		Match = Checkpoint;
	}

	return Match;
}

bool AWCStoryPersistenceCoordinator::
TravelPlayerToCheckpoint(
	FName TargetCheckpointID)
{
	if (TargetCheckpointID.IsNone() ||
		bRestoreInProgress)
	{
		return false;
	}

	AWCCharacter* Player =
		Cast<AWCCharacter>(
			UGameplayStatics::GetPlayerCharacter(
				this,
				0
			)
		);

	/*
	* Fast Travel 是菜单中的玩家主动操作，
	* 不是可以从任意 Blueprint 后台触发的自动传送。
	*/
	if (!IsValid(Player) ||
		!Player->GetIsCheckpointMenuOpen() ||
		!Player->HasUnlockedCheckpoint(
			TargetCheckpointID
		))
	{
		return false;
	}

	const FName SourceCheckpointID =
		Player->GetCurrentCheckpointID();

	if (SourceCheckpointID.IsNone() ||
		SourceCheckpointID == TargetCheckpointID ||
		!Player->HasUnlockedCheckpoint(
			SourceCheckpointID
		))
	{
		return false;
	}

	AWCPlayerCheckpoint* SourceCheckpoint =
		FindCheckpointByID(SourceCheckpointID);

	AWCPlayerCheckpoint* TargetCheckpoint =
		FindCheckpointByID(TargetCheckpointID);

	if (!IsValid(SourceCheckpoint) ||
		!IsValid(TargetCheckpoint))
	{
		return false;
	}

	FTransform SourceTransform;
	FTransform TargetTransform;

	if (!SourceCheckpoint->BuildSafeResumeTransform(
			Player,
			SourceTransform) ||
		!TargetCheckpoint->BuildSafeResumeTransform(
			Player,
			TargetTransform))
	{
		return false;
	}

	/*
	* 所有验证完成后再关闭菜单和清理旧交互状态。
	*/
	Player->CloseCheckpointMenu();
	Player->CurrentInteractable = nullptr;
	Player->HideInteractionPrompt();

	if (!Player->ApplySavedCheckpointState(
		TargetCheckpointID,
		TargetTransform))
	{
		SourceCheckpoint
			->RefreshPlayerInteractionIfOverlapping();
		return false;
	}

	TGuardValue<bool> SaveRequestGuard(
		bCheckpointSaveRequestInProgress,
		true
	);

	if (!CaptureAndSaveWorldState())
	{
		/*
		* 磁盘保存失败时，将位置与 Current ID 一起回滚。
		*/
		if (!Player->ApplySavedCheckpointState(
			SourceCheckpointID,
			SourceTransform))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"Fast Travel rollback failed for source "
					"Checkpoint [%s]."
				),
				*SourceCheckpointID.ToString()
			);
		}

		SourceCheckpoint
			->RefreshPlayerInteractionIfOverlapping();

		return false;
	}

	TargetCheckpoint
		->RefreshPlayerInteractionIfOverlapping();

	ShowPersistenceMessage(
		FString::Printf(
			TEXT("Traveled to %s."),
			*TargetCheckpoint
				->GetCheckpointDisplayName()
				.ToString()
		),
		FColor::Cyan
	);

	return true;
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
	if (!bCheckpointSaveRequestInProgress &&
		!bAllowDirectDebugPersistenceActions)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Display,
			TEXT(
				"Direct World-state capture ignored. "
				"Use a visible Checkpoint or Fast Travel."
			)
		);

		return false;
	}

	/*
	* Capture 只允许在非 Restore 阶段执行。
	*
	* 避免：
	* Restore 正在修改 World
	* → 同时 Capture 半恢复状态
	* → 覆盖磁盘中的稳定存档
	*/
	if (bRestoreInProgress)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Warning,
			TEXT(
				"World-state capture rejected: "
				"a World Restore is currently in progress."
			)
		);

		ShowPersistenceMessage(
			TEXT(
				"Save failed: World Restore is in progress."
			),
			FColor::Yellow
		);

		return false;
	}

	/*
	* 首先验证当前 World 中所有正式持久化 Actor 的 ID。
	*
	* Day4 当前应包括：
	* - Story NPC
	* - Objective
	* - Encounter
	* - Echo Relic
	* - Story Anchor
	* - Player Checkpoint
	*/
	if (!ValidateWorldPersistenceIDs())
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"World-state capture failed: "
				"World persistence ID validation failed."
			)
		);

		ShowPersistenceMessage(
			TEXT(
				"Save failed: invalid World persistence IDs."
			),
			FColor::Red
		);

		return false;
	}

	UWCGameInstance* GameInstance =
		GetWCGameInstance();

	if (!IsValid(GameInstance))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"World-state capture failed: "
				"active GameInstance is not "
				"a valid UWCGameInstance."
			)
		);

		ShowPersistenceMessage(
			TEXT(
				"Save failed: invalid GameInstance."
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
				"Player 0 is not a valid AWCCharacter."
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
	* ----------------------------------------------------------------
	* Phase 1:
	* 构造 Checkpoint 候选数据
	* ----------------------------------------------------------------
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

	const TArray<FName>&
		RuntimeUnlockedCheckpointIDs =
		Player->GetUnlockedCheckpointIDs();

	if (RuntimeUnlockedCheckpointIDs.IsEmpty())
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"World-state capture failed: "
				"Player has no unlocked Checkpoints."
			)
		);

		ShowPersistenceMessage(
			TEXT(
				"Save failed: no unlocked Checkpoints."
			),
			FColor::Red
		);

		return false;
	}

	/*
	* 复制到本地数组。
	*
	* 后续所有验证都针对这个候选快照，
	* 而不是直接修改 SaveData。
	*/
	TArray<FName> CapturedUnlockedCheckpointIDs;

	CapturedUnlockedCheckpointIDs.Reserve(
		RuntimeUnlockedCheckpointIDs.Num()
	);

	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AWCPlayerCheckpoint::StaticClass(),
		FoundActors
	);

	TMap<FName, AWCPlayerCheckpoint*>
		WorldCheckpoints;

	for (AActor* Actor : FoundActors)
	{
		AWCPlayerCheckpoint* Checkpoint =
			Cast<AWCPlayerCheckpoint>(Actor);

		if (!IsValid(Checkpoint))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"invalid Player Checkpoint Actor."
				)
			);

			return false;
		}

		const FName CheckpointID =
			Checkpoint->GetCheckpointID();

		if (CheckpointID.IsNone())
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"Checkpoint Actor [%s] has ID None."
				),
				*Checkpoint->GetName()
			);

			return false;
		}

		WorldCheckpoints.Add(
			CheckpointID,
			Checkpoint
		);
	}

	AWCPlayerCheckpoint* const*
		CapturedCheckpointPtr =
		WorldCheckpoints.Find(
			CapturedCheckpointID
		);

	if (!CapturedCheckpointPtr ||
		!IsValid(*CapturedCheckpointPtr))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"World-state capture failed: "
				"active Checkpoint [%s] does not exist "
				"in the current World."
			),
			*CapturedCheckpointID.ToString()
		);

		ShowPersistenceMessage(
			TEXT(
				"Save failed: active Checkpoint is invalid."
			),
			FColor::Red
		);

		return false;
	}

	/*
	* 当前 Checkpoint 还必须能提供一个安全恢复位置。
	*
	* SaveAtCheckpoint() 已经会提前验证一次，
	* 这里再次验证是为了保证 Capture 本身仍然独立安全。
	*/
	FTransform TestResumeTransform;

	if (!(*CapturedCheckpointPtr)
		->BuildSafeResumeTransform(
			Player,
			TestResumeTransform
		))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"World-state capture failed: "
				"Checkpoint [%s] cannot build a "
				"safe Resume Transform."
			),
			*CapturedCheckpointID.ToString()
		);

		ShowPersistenceMessage(
			TEXT(
				"Save failed: unsafe Checkpoint position."
			),
			FColor::Red
		);

		return false;
	}

	TSet<FName>
		CapturedUnlockedCheckpointIDSet;

	bool bUnlockedListContainsCurrent =
		false;

	for (const FName CheckpointID :
	RuntimeUnlockedCheckpointIDs)
	{
		if (CheckpointID.IsNone())
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"UnlockedCheckpointIDs contains None."
				)
			);

			return false;
		}

		if (CapturedUnlockedCheckpointIDSet.Contains(
			CheckpointID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"duplicate unlocked Checkpoint ID [%s]."
				),
				*CheckpointID.ToString()
			);

			return false;
		}

		if (!WorldCheckpoints.Contains(
			CheckpointID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"unlocked Checkpoint [%s] does not "
					"exist in the current World."
				),
				*CheckpointID.ToString()
			);

			return false;
		}

		CapturedUnlockedCheckpointIDSet.Add(
			CheckpointID
		);

		CapturedUnlockedCheckpointIDs.Add(
			CheckpointID
		);

		if (CheckpointID ==
			CapturedCheckpointID)
		{
			bUnlockedListContainsCurrent =
				true;
		}
	}

	if (!bUnlockedListContainsCurrent)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"World-state capture failed: "
				"CurrentCheckpointID [%s] is not present "
				"in UnlockedCheckpointIDs."
			),
			*CapturedCheckpointID.ToString()
		);

		ShowPersistenceMessage(
			TEXT(
				"Save failed: current Checkpoint is not unlocked."
			),
			FColor::Red
		);

		return false;
	}

	/*
	* ----------------------------------------------------------------
	* Phase 2:
	* 构造 Story NPC 候选快照
	* ----------------------------------------------------------------
	*/

	TArray<FWCSavedStoryNPCState>
		CapturedStoryNPCStates;

	TSet<FName> CapturedStoryNPCIDs;

	FoundActors.Reset();

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AWCStoryNPC::StaticClass(),
		FoundActors
	);

	CapturedStoryNPCStates.Reserve(
		FoundActors.Num()
	);

	for (AActor* Actor : FoundActors)
	{
		AWCStoryNPC* StoryNPC =
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

		const FName StoryNPCID =
			StoryNPC->GetStoryNPCID();

		if (StoryNPCID.IsNone() ||
			CapturedStoryNPCIDs.Contains(
				StoryNPCID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"invalid or duplicate Story NPC "
					"ID [%s]."
				),
				*StoryNPCID.ToString()
			);

			return false;
		}

		const EStoryNPCState StoryState =
			StoryNPC->GetStoryState();

		/*
		* 不允许保存 NPC 正在推进故事的中间状态。
		*
		* 否则可能出现：
		* Encounter 已完成
		* + NPC 仍在旧 Stage / Anchor
		*/
		if (StoryState ==
			EStoryNPCState::EventResolved ||
			StoryState ==
			EStoryNPCState::Relocating)
		{
			UE_LOG(
				LogWCStoryPersistence,
				Warning,
				TEXT(
					"World-state capture rejected: "
					"Story NPC [%s] is in transitional "
					"state [%d]."
				),
				*StoryNPCID.ToString(),
				static_cast<int32>(
					StoryState
					)
			);

			ShowPersistenceMessage(
				TEXT(
					"Cannot save while the story is changing."
				),
				FColor::Yellow
			);

			return false;
		}

		const int32 StoryStage =
			StoryNPC->GetCurrentStoryStage();

		const FName AnchorID =
			StoryNPC->GetCurrentStoryAnchorID();

		if (StoryStage < 0 ||
			AnchorID.IsNone())
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"Story NPC [%s] has invalid "
					"Stage=%d or Anchor=[%s]."
				),
				*StoryNPCID.ToString(),
				StoryStage,
				*AnchorID.ToString()
			);

			return false;
		}

		FWCSavedStoryNPCState SavedNPCState;

		SavedNPCState.StoryNPCID =
			StoryNPCID;

		SavedNPCState.StoryStage =
			StoryStage;

		SavedNPCState.AnchorID =
			AnchorID;

		CapturedStoryNPCIDs.Add(
			StoryNPCID
		);

		CapturedStoryNPCStates.Add(
			MoveTemp(SavedNPCState)
		);
	}

	/*
	* ----------------------------------------------------------------
	* Phase 3:
	* 构造 Objective 候选快照
	* ----------------------------------------------------------------
	*/

	TArray<FWCSavedObjectiveState>
		CapturedObjectiveStates;

	TSet<FName> CapturedObjectiveIDs;

	FoundActors.Reset();

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AStoryObjectiveBase::StaticClass(),
		FoundActors
	);

	CapturedObjectiveStates.Reserve(
		FoundActors.Num()
	);

	for (AActor* Actor : FoundActors)
	{
		AStoryObjectiveBase* Objective =
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

		const FName ObjectiveID =
			Objective->GetObjectiveID();

		if (ObjectiveID.IsNone() ||
			CapturedObjectiveIDs.Contains(
				ObjectiveID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"invalid or duplicate Objective "
					"ID [%s]."
				),
				*ObjectiveID.ToString()
			);

			return false;
		}

		FWCSavedObjectiveState
			SavedObjectiveState;

		SavedObjectiveState.ObjectiveID =
			ObjectiveID;

		SavedObjectiveState.bCompleted =
			Objective
			->GetIsObjectiveComplete();

		CapturedObjectiveIDs.Add(
			ObjectiveID
		);

		CapturedObjectiveStates.Add(
			MoveTemp(
				SavedObjectiveState
			)
		);
	}

	/*
	* ----------------------------------------------------------------
	* Phase 4:
	* 构造 Encounter 候选快照
	* ----------------------------------------------------------------
	*/

	TArray<FWCSavedEncounterState>
		CapturedEncounterStates;

	TSet<FName> CapturedEncounterIDs;

	FoundActors.Reset();

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AStoryEncounter::StaticClass(),
		FoundActors
	);

	CapturedEncounterStates.Reserve(
		FoundActors.Num()
	);

	for (AActor* Actor : FoundActors)
	{
		AStoryEncounter* Encounter =
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

		const FName EncounterID =
			Encounter->GetEncounterID();

		if (EncounterID.IsNone() ||
			CapturedEncounterIDs.Contains(
				EncounterID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"invalid or duplicate Encounter "
					"ID [%s]."
				),
				*EncounterID.ToString()
			);

			return false;
		}

		FWCSavedEncounterState
			SavedEncounterState;

		SavedEncounterState.EncounterID =
			EncounterID;

		SavedEncounterState.bCompleted =
			Encounter
			->GetIsEncounterCompleted();

		CapturedEncounterIDs.Add(
			EncounterID
		);

		CapturedEncounterStates.Add(
			MoveTemp(
				SavedEncounterState
			)
		);
	}

	/*
	* ----------------------------------------------------------------
	* Phase 5:
	* 构造 Journal 候选快照
	* ----------------------------------------------------------------
	*/

	TSet<FName> WorldEchoIDs;

	FoundActors.Reset();

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AEchoRelic::StaticClass(),
		FoundActors
	);

	for (AActor* Actor : FoundActors)
	{
		const AEchoRelic* EchoRelic =
			Cast<AEchoRelic>(Actor);

		if (!IsValid(EchoRelic))
		{
			return false;
		}

		const FName EchoID =
			EchoRelic->GetEchoID();

		if (EchoID.IsNone())
		{
			return false;
		}

		WorldEchoIDs.Add(EchoID);
	}

	const TArray<FMemoryEchoData>&
		RuntimeMemoryEchoes =
		Player->GetRecordedMemoryEchoes();

	TArray<FMemoryEchoData>
		CapturedMemoryEchoes;

	CapturedMemoryEchoes.Reserve(
		RuntimeMemoryEchoes.Num()
	);

	TSet<FName> CapturedMemoryEchoIDs;

	for (const FMemoryEchoData& EchoData :
		RuntimeMemoryEchoes)
	{
		if (EchoData.EchoID.IsNone())
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"Journal contains an Echo "
					"with ID None."
				)
			);

			return false;
		}

		if (CapturedMemoryEchoIDs.Contains(
			EchoData.EchoID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"duplicate Journal Echo ID [%s]."
				),
				*EchoData.EchoID.ToString()
			);

			return false;
		}

		if (!WorldEchoIDs.Contains(
			EchoData.EchoID))
		{
			UE_LOG(
				LogWCStoryPersistence,
				Error,
				TEXT(
					"World-state capture failed: "
					"Journal Echo [%s] has no matching "
					"Echo Relic in the current World."
				),
				*EchoData.EchoID.ToString()
			);

			return false;
		}

		CapturedMemoryEchoIDs.Add(
			EchoData.EchoID
		);

		/*
		* 保留玩家 Journal 中的现有顺序。
		*/
		CapturedMemoryEchoes.Add(
			EchoData
		);
	}

	/*
	* ----------------------------------------------------------------
	* Phase 6:
	* 准备可写入的 SaveGame Object
	* ----------------------------------------------------------------
	*/

	UWCGameSaveGame* SaveData =
		GameInstance->GetLoadedSaveData();

	if (!IsValid(SaveData))
	{
		if (GameInstance->HasSavedGame())
		{
			/*
			* 如果磁盘中已经有存档，则必须先加载。
			*
			* 加载失败时不能直接创建空存档覆盖它，
			* 尤其是旧 Version 存档或损坏存档。
			*/
			if (!GameInstance->LoadSavedGame())
			{
				UE_LOG(
					LogWCStoryPersistence,
					Error,
					TEXT(
						"World-state capture failed: "
						"an existing disk save could "
						"not be loaded safely."
					)
				);

				ShowPersistenceMessage(
					TEXT(
						"Save failed: existing save is incompatible."
					),
					FColor::Red
				);

				return false;
			}
		}
		else
		{
			/*
			* CreateNewSave 的返回类型即使是 void 或 bool，
			* 都可以直接调用，随后通过 Getter 验证结果。
			*/
			GameInstance->CreateNewSave();
		}

		SaveData =
			GameInstance->GetLoadedSaveData();
	}

	if (!IsValid(SaveData))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"World-state capture failed: "
				"no valid LoadedSaveData is available."
			)
		);

		ShowPersistenceMessage(
			TEXT(
				"Save failed: could not create save data."
			),
			FColor::Red
		);

		return false;
	}

	/*
	* ----------------------------------------------------------------
	* Phase 7:
	* 备份当前内存 SaveData
	* ----------------------------------------------------------------
	*
	* 如果 SaveCurrentGame() 最后写盘失败，
	* 不仅磁盘应保持旧状态，内存中的 LoadedSaveData
	* 也应回滚。
	*/

	const int32 PreviousSaveVersion =
		SaveData->SaveVersion;

	const FName PreviousCheckpointID =
		SaveData->CurrentCheckpointID;

	const TArray<FName>
		PreviousUnlockedCheckpointIDs =
		SaveData->UnlockedCheckpointIDs;

	const TArray<FWCSavedStoryNPCState>
		PreviousStoryNPCStates =
		SaveData->StoryNPCStates;

	const TArray<FWCSavedObjectiveState>
		PreviousObjectiveStates =
		SaveData->ObjectiveStates;

	const TArray<FWCSavedEncounterState>
		PreviousEncounterStates =
		SaveData->EncounterStates;

	const TArray<FMemoryEchoData>
		PreviousMemoryEchoes =
		SaveData->RecordedMemoryEchoes;

	/*
	* ----------------------------------------------------------------
	* Phase 8:
	* 一次性提交完整候选快照到内存 SaveData
	* ----------------------------------------------------------------
	*/

	SaveData->SaveVersion =
		UWCGameSaveGame::CurrentSaveVersion;

	SaveData->CurrentCheckpointID =
		CapturedCheckpointID;

	SaveData->UnlockedCheckpointIDs =
		CapturedUnlockedCheckpointIDs;

	SaveData->StoryNPCStates =
		CapturedStoryNPCStates;

	SaveData->ObjectiveStates =
		CapturedObjectiveStates;

	SaveData->EncounterStates =
		CapturedEncounterStates;

	SaveData->RecordedMemoryEchoes =
		CapturedMemoryEchoes;

	/*
	* ----------------------------------------------------------------
	* Phase 9:
	* 只调用一次磁盘写入
	* ----------------------------------------------------------------
	*/

	if (!GameInstance->SaveCurrentGame())
	{
		/*
		* 磁盘写入失败：
		* 恢复之前的内存 SaveData。
		*
		* SaveAtCheckpoint() 随后还会负责回滚
		* Player Runtime Checkpoint 状态。
		*/
		SaveData->SaveVersion =
			PreviousSaveVersion;

		SaveData->CurrentCheckpointID =
			PreviousCheckpointID;

		SaveData->UnlockedCheckpointIDs =
			PreviousUnlockedCheckpointIDs;

		SaveData->StoryNPCStates =
			PreviousStoryNPCStates;

		SaveData->ObjectiveStates =
			PreviousObjectiveStates;

		SaveData->EncounterStates =
			PreviousEncounterStates;

		SaveData->RecordedMemoryEchoes =
			PreviousMemoryEchoes;

		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"World-state capture failed: "
				"SaveCurrentGame could not write "
				"the candidate snapshot to disk. "
				"LoadedSaveData was rolled back."
			)
		);

		ShowPersistenceMessage(
			TEXT(
				"Save failed: disk write error."
			),
			FColor::Red
		);

		return false;
	}

	UE_LOG(
		LogWCStoryPersistence,
		Display,
		TEXT(
			"World-state capture succeeded. "
			"Version=%d, Checkpoint=[%s], "
			"UnlockedCheckpoints=%d, NPCs=%d, "
			"Objectives=%d, Encounters=%d, Echoes=%d."
		),
		SaveData->SaveVersion,
		*SaveData->CurrentCheckpointID.ToString(),
		SaveData->UnlockedCheckpointIDs.Num(),
		SaveData->StoryNPCStates.Num(),
		SaveData->ObjectiveStates.Num(),
		SaveData->EncounterStates.Num(),
		SaveData->RecordedMemoryEchoes.Num()
	);

	ShowPersistenceMessage(
		FString::Printf(
			TEXT(
				"Memory Saved: %s | "
				"Rest Points %d | Echoes %d"
			),
			*SaveData
			->CurrentCheckpointID
			.ToString(),
			SaveData
			->UnlockedCheckpointIDs
			.Num(),
			SaveData
			->RecordedMemoryEchoes
			.Num()
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

bool AWCStoryPersistenceCoordinator::InitializeDefaultCheckpointForNewGame()
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
				"Default Checkpoint initialization failed: "
				"Player 0 is not a valid AWCCharacter."
			)
		);

		return false;
	}

	TArray<AActor*> FoundCheckpointActors;

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AWCPlayerCheckpoint::StaticClass(),
		FoundCheckpointActors
	);

	if (FoundCheckpointActors.Num() == 0)
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Default Checkpoint initialization failed: "
				"no AWCPlayerCheckpoint exists "
				"in the current World."
			)
		);

		return false;
	}

	AWCPlayerCheckpoint* DefaultCheckpoint =
		nullptr;

	int32 DefaultCheckpointCount = 0;

	for (AActor* FoundActor :
		FoundCheckpointActors)
	{
		AWCPlayerCheckpoint* Checkpoint =
			Cast<AWCPlayerCheckpoint>(
				FoundActor
			);

		if (!IsValid(Checkpoint))
		{
			continue;
		}

		if (!Checkpoint
			->GetIsDefaultCheckpoint())
		{
			continue;
		}

		++DefaultCheckpointCount;
		DefaultCheckpoint = Checkpoint;
	}

	if (DefaultCheckpointCount != 1 ||
		!IsValid(DefaultCheckpoint))
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Default Checkpoint initialization failed: "
				"expected exactly one Default Checkpoint, "
				"but found %d."
			),
			DefaultCheckpointCount
		);

		return false;
	}

	const FName DefaultCheckpointID =
		DefaultCheckpoint->GetCheckpointID();

	if (DefaultCheckpointID.IsNone())
	{
		UE_LOG(
			LogWCStoryPersistence,
			Error,
			TEXT(
				"Default Checkpoint initialization failed: "
				"Default Checkpoint Actor [%s] "
				"has CheckpointID None."
			),
			*DefaultCheckpoint->GetName()
		);

		return false;
	}

	/*
	 * New Game 的 Runtime Checkpoint 状态：
	 *
	 * - CurrentCheckpointID = Default Checkpoint
	 * - UnlockedCheckpointIDs = [Default Checkpoint]
	 *
	 * 这里只初始化 Runtime State。
	 * 不移动玩家，也不创建或写入 SaveGame。
	 */
	TArray<FName> InitialUnlockedCheckpointIDs;

	InitialUnlockedCheckpointIDs.Add(
		DefaultCheckpointID
	);

	Player->ApplyRuntimeCheckpointProgress(
		DefaultCheckpointID,
		InitialUnlockedCheckpointIDs
	);

	/*
	 * 根据 Runtime Unlocked IDs 恢复所有归魂碑的表现。
	 *
	 * 默认归魂碑显示为已解锁；
	 * 其他归魂碑显示为未解锁。
	 */
	RefreshCheckpointPresentations(
		InitialUnlockedCheckpointIDs
	);

	UE_LOG(
		LogWCStoryPersistence,
		Display,
		TEXT(
			"New Game Checkpoint state initialized. "
			"Current=[%s], UnlockedCount=%d. "
			"Player was not moved and no disk save "
			"was performed."
		),
		*DefaultCheckpointID.ToString(),
		InitialUnlockedCheckpointIDs.Num()
	);

	ShowPersistenceMessage(
		FString::Printf(
			TEXT(
				"Rest point initialized: %s"
			),
			*DefaultCheckpoint
			->GetCheckpointDisplayName()
			.ToString()
		),
		FColor::Silver
	);

	return true;
}

void AWCStoryPersistenceCoordinator::
RefreshCheckpointPresentations(
	const TArray<FName>&
	UnlockedCheckpointIDs) const
{
	TArray<AActor*> FoundActors;

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
			continue;
		}

		Checkpoint
			->ApplyUnlockedPresentation(
				UnlockedCheckpointIDs
				.Contains(
					Checkpoint
					->GetCheckpointID()
				)
			);
	}
}
