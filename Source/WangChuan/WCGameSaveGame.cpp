#include "WCGameSaveGame.h"

UWCGameSaveGame::UWCGameSaveGame()
{
	SaveVersion = CurrentSaveVersion;
	CurrentCheckpointID = NAME_None;
}