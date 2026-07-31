#include "WCGameInstance.h"

#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogWCSaveGame, Log, All);

const FString UWCGameInstance::SaveSlotName =
TEXT("WangChuan_Save_01");

void UWCGameInstance::Init()
{
	Super::Init();

	LoadedSaveData = nullptr;

	UE_LOG(
		LogWCSaveGame,
		Display,
		TEXT("WCGameInstance initialized. Slot='%s', UserIndex=%d."),
		*SaveSlotName,
		SaveUserIndex
	);
}

UWCGameSaveGame* UWCGameInstance::CreateNewSave()
{
	USaveGame* NewSaveObject =
		UGameplayStatics::CreateSaveGameObject(
			UWCGameSaveGame::StaticClass()
		);

	UWCGameSaveGame* NewSave =
		Cast<UWCGameSaveGame>(NewSaveObject);

	if (!IsValid(NewSave))
	{
		UE_LOG(
			LogWCSaveGame,
			Error,
			TEXT("CreateNewSave failed: unable to create UWCGameSaveGame.")
		);

		return nullptr;
	}

	LoadedSaveData = NewSave;

	UE_LOG(
		LogWCSaveGame,
		Display,
		TEXT("Created new in-memory save. Version=%d. No disk write performed."),
		LoadedSaveData->SaveVersion
	);

	return LoadedSaveData;
}

bool UWCGameInstance::HasSavedGame() const
{
	return UGameplayStatics::DoesSaveGameExist(
		SaveSlotName,
		SaveUserIndex
	);
}

bool UWCGameInstance::SaveCurrentGame()
{
	if (!IsValid(LoadedSaveData))
	{
		UE_LOG(
			LogWCSaveGame,
			Warning,
			TEXT(
				"SaveCurrentGame rejected: LoadedSaveData is invalid. "
				"Call CreateNewSave or LoadSavedGame first."
			)
		);

		return false;
	}

	/*
	* 当前版本没有迁移系统。
	* 所有新保存的数据统一写为当前格式版本。
	*/
	LoadedSaveData->SaveVersion =
		UWCGameSaveGame::CurrentSaveVersion;

	const bool bSaveSucceeded =
		UGameplayStatics::SaveGameToSlot(
			LoadedSaveData,
			SaveSlotName,
			SaveUserIndex
		);

	if (!bSaveSucceeded)
	{
		UE_LOG(
			LogWCSaveGame,
			Error,
			TEXT("Failed to save slot '%s'."),
			*SaveSlotName
		);

		return false;
	}

	UE_LOG(
		LogWCSaveGame,
		Display,
		TEXT(
			"Saved slot '%s'. Version=%d, Checkpoint='%s', "
			"NPCs=%d, Objectives=%d, Encounters=%d, Echoes=%d."
		),
		*SaveSlotName,
		LoadedSaveData->SaveVersion,
		*LoadedSaveData->CurrentCheckpointID.ToString(),
		LoadedSaveData->StoryNPCStates.Num(),
		LoadedSaveData->ObjectiveStates.Num(),
		LoadedSaveData->EncounterStates.Num(),
		LoadedSaveData->RecordedMemoryEchoes.Num()
	);

	return true;
}

bool UWCGameInstance::LoadSavedGame()
{
	if (!HasSavedGame())
	{
		UE_LOG(
			LogWCSaveGame,
			Warning,
			TEXT("LoadSavedGame rejected: slot '%s' does not exist."),
			*SaveSlotName
		);

		return false;
	}

	USaveGame* RawLoadedSave =
		UGameplayStatics::LoadGameFromSlot(
			SaveSlotName,
			SaveUserIndex
		);

	UWCGameSaveGame* LoadedGame =
		Cast<UWCGameSaveGame>(RawLoadedSave);

	if (!IsValid(LoadedGame))
	{
		UE_LOG(
			LogWCSaveGame,
			Error,
			TEXT(
				"LoadSavedGame failed: slot '%s' did not contain "
				"a valid UWCGameSaveGame."
			),
			*SaveSlotName
		);

		return false;
	}

	if (LoadedGame->SaveVersion !=
		UWCGameSaveGame::CurrentSaveVersion)
	{
		UE_LOG(
			LogWCSaveGame,
			Error,
			TEXT(
				"LoadSavedGame rejected: unsupported SaveVersion %d. "
				"Current supported version is %d."
			),
			LoadedGame->SaveVersion,
			UWCGameSaveGame::CurrentSaveVersion
		);

		return false;
	}

	/*
	* 只有在读取、类型检查和版本检查全部成功后，
	* 才替换当前内存中的工作存档。
	*/
	LoadedSaveData = LoadedGame;

	UE_LOG(
		LogWCSaveGame,
		Display,
		TEXT(
			"Loaded slot '%s'. Version=%d, Checkpoint='%s', "
			"NPCs=%d, Objectives=%d, Encounters=%d, Echoes=%d."
		),
		*SaveSlotName,
		LoadedSaveData->SaveVersion,
		*LoadedSaveData->CurrentCheckpointID.ToString(),
		LoadedSaveData->StoryNPCStates.Num(),
		LoadedSaveData->ObjectiveStates.Num(),
		LoadedSaveData->EncounterStates.Num(),
		LoadedSaveData->RecordedMemoryEchoes.Num()
	);

	return true;
}

bool UWCGameInstance::DeleteSavedGame()
{
	if (!HasSavedGame())
	{
		/*
		* 删除目标已经满足：
		* 磁盘中不存在该 Slot。
		*
		* 同时清理当前内存对象，让函数保持幂等。
		*/
		LoadedSaveData = nullptr;

		UE_LOG(
			LogWCSaveGame,
			Display,
			TEXT(
				"DeleteSavedGame: slot '%s' was already absent. "
				"In-memory save data was cleared."
			),
			*SaveSlotName
		);

		return true;
	}

	const bool bDeleteSucceeded =
		UGameplayStatics::DeleteGameInSlot(
			SaveSlotName,
			SaveUserIndex
		);

	if (!bDeleteSucceeded)
	{
		UE_LOG(
			LogWCSaveGame,
			Error,
			TEXT("Failed to delete save slot '%s'."),
			*SaveSlotName
		);

		return false;
	}

	LoadedSaveData = nullptr;

	UE_LOG(
		LogWCSaveGame,
		Display,
		TEXT("Deleted save slot '%s'."),
		*SaveSlotName
	);

	return true;
}

UWCGameSaveGame* UWCGameInstance::GetLoadedSaveData() const
{
	return LoadedSaveData;
}

FString UWCGameInstance::GetSaveSlotName() const
{
	return SaveSlotName;
}

int32 UWCGameInstance::GetSaveUserIndex() const
{
	return SaveUserIndex;
}