#include "WCGameSaveGame.h"

// ******************** Construction ********************

UWCGameSaveGame::UWCGameSaveGame()
{
	SaveVersion = CurrentSaveVersion;
	CurrentCheckpointID = NAME_None;
}
