# 《忘川河畔》Week 7 — Day 1 Advanced Progress

**Project:** WangChuan / 《忘川河畔》  
**Active Project:** `WangChuan_UE58_Migration`  
**Internal Module:** `WangChuan`  
**Engine:** Unreal Engine 5.8  
**Date:** 2026-07-30  
**Week Theme:** Story Persistence, SaveGame & Resume Flow  
**Milestone:** Day 1 Advanced / Original Day 2  
**Day Theme:** SaveGame Data and GameInstance API

---

## Day Goal

Day 1 Advanced established the first functional persistence infrastructure for the project.

The goal was not yet to save or restore the real Quiet Child world state. Instead, this milestone created and independently verified the two lower layers required by all later persistence work:

```text
Serializable Save Data
→ UWCGameSaveGame

Disk Save / Load Entry
→ UWCGameInstance
```

The completed implementation supports:

- One fixed Save Slot
- Save-format version tracking
- Story NPC, Objective, Encounter, Journal, and Checkpoint data containers
- Creation of a new in-memory save
- Synchronous disk save
- Synchronous disk load
- Save-slot existence checks
- Idempotent save deletion
- Safe access to the currently loaded save object
- Cross-PIE persistence validation

This milestone deliberately did **not** collect or restore live Story Actors.

---

# Completed Files

Created:

```text
Source/WangChuan/WCSaveTypes.h
Source/WangChuan/WCSaveTypes.cpp
Source/WangChuan/WCGameSaveGame.h
Source/WangChuan/WCGameSaveGame.cpp
Source/WangChuan/WCGameInstance.h
Source/WangChuan/WCGameInstance.cpp
```

`WCSaveTypes.cpp` was added during Visual Studio troubleshooting as a minimal translation unit:

```cpp
#include "WCSaveTypes.h"
```

It does not add gameplay behavior or change the save-data model.

---

# 1. Save Data Structures

Created reusable SaveGame structures in:

```text
WCSaveTypes.h
```

## `FWCSavedStoryNPCState`

Fields:

```cpp
FName StoryNPCID;
int32 StoryStage;
FName AnchorID;
```

This structure stores the stable result of a Story NPC's progression:

```text
Which NPC
→ Which Story Stage
→ Which stable Story Anchor
```

It intentionally does not store:

- Current dialogue line
- `Relocating`
- `EventResolved`
- Pending relocation state
- Relocation timers
- Niagara playback
- Runtime Actor pointers

The future restore path will place the NPC directly into a stable `Available` state at the saved Stage and Anchor.

---

## `FWCSavedObjectiveState`

Fields:

```cpp
FName ObjectiveID;
bool bCompleted;
```

The first save format stores only the final completion fact.

It intentionally does not store:

- `bIsActive`
- Lantern Preview index
- Current sequence input
- Wrong-input Reset state
- Timer progress
- Temporary lantern light state

Restore rule:

```text
Objective incomplete
→ restore its normal initial runtime state

Objective completed
→ restore the final completed state silently
```

---

## `FWCSavedEncounterState`

Fields:

```cpp
FName EncounterID;
bool bCompleted;
```

Objective completion and Encounter completion remain separate persistent facts.

This preserves the valid intermediate state:

```text
Objective Completed
+ Encounter Incomplete
→ Echo Relic Available
```

The structure does not save Actor references or duplicated derived state.

---

# 2. `UWCGameSaveGame`

Created:

```cpp
UWCGameSaveGame : public USaveGame
```

The class acts only as a serializable data container.

## Save Version

Configured:

```cpp
static constexpr int32 CurrentSaveVersion = 1;
int32 SaveVersion = 1;
```

Week 7 does not implement a migration framework, but every save records its format version so unsupported data can be rejected explicitly rather than interpreted incorrectly.

---

## Checkpoint Data

Added:

```cpp
FName CurrentCheckpointID;
```

This field prepares for a later stable checkpoint system.

Day 1 Advanced only verified that the value can survive a disk round trip. No `AWCPlayerCheckpoint` Actor or player relocation was implemented yet.

---

## Story Arrays

Added:

```cpp
TArray<FWCSavedStoryNPCState> StoryNPCStates;
TArray<FWCSavedObjectiveState> ObjectiveStates;
TArray<FWCSavedEncounterState> EncounterStates;
```

These arrays will later be populated by the Story Persistence Coordinator using the stable IDs confirmed on Day 1.

---

## Journal Records

Added:

```cpp
TArray<FMemoryEchoData> RecordedMemoryEchoes;
```

The first save format stores complete Memory Echo data because the project does not yet have a global Echo Data Registry.

Current tradeoff:

```text
Save complete FMemoryEchoData
→ simple and reliable Journal restoration
```

Possible future direction:

```text
Save EchoID only
→ look up content through a centralized DataAsset registry
```

The current implementation prioritizes completion and reliability over prematurely building a larger content database.

---

# 3. `UWCGameInstance`

Created:

```cpp
UWCGameInstance : public UGameInstance
```

Configured as the project's active Game Instance in:

```text
Project Settings
→ Maps & Modes
→ Game Instance Class
→ WCGameInstance
```

The class provides the persistent disk-access entry point while avoiding ownership of level Actors.

---

## Fixed Save Slot

Configured:

```text
Slot Name: WangChuan_Save_01
User Index: 0
```

Week 7 continues to use one slot only.

No multi-slot UI, cloud save, thumbnail, playtime counter, or slot metadata system was added.

---

## Runtime Save Object

Added:

```cpp
UPROPERTY(Transient)
TObjectPtr<UWCGameSaveGame> LoadedSaveData;
```

`LoadedSaveData` represents the current in-memory working save.

It is distinct from the disk slot:

```text
Disk Slot exists
≠
Save object is currently loaded in memory
```

Possible valid states include:

```text
HasSavedGame() == true
LoadedSaveData == nullptr
```

when a disk save exists but has not been loaded during the current GameInstance.

Another valid state is:

```text
HasSavedGame() == false
LoadedSaveData != nullptr
```

after creating a new in-memory save but before writing it to disk.

---

# Implemented GameInstance API

## `Init()`

Initializes the in-memory save pointer and logs:

- GameInstance startup
- Save Slot name
- User index

This also verifies that the project is actually using `UWCGameInstance` rather than the default engine GameInstance.

---

## `CreateNewSave()`

Creates a new `UWCGameSaveGame` object in memory.

Behavior:

```text
Create new save object
→ assign LoadedSaveData
→ do not write to disk automatically
```

Separating creation from disk writing prevents an accidental `CreateNewSave()` call from immediately overwriting an existing save file.

---

## `HasSavedGame()`

Checks whether:

```text
WangChuan_Save_01
```

currently exists on disk.

It does not create or load data.

---

## `SaveCurrentGame()`

Writes the current `LoadedSaveData` to the fixed Slot.

Safety behavior:

- Rejects saving when `LoadedSaveData` is invalid
- Does not silently create an empty save
- Updates `SaveVersion` to the current supported format
- Logs the number of saved NPC, Objective, Encounter, and Echo records
- Returns an explicit success or failure result

This avoids the dangerous case:

```text
Old disk save exists
→ load was accidentally skipped
→ empty save silently created
→ old progress overwritten
```

---

## `LoadSavedGame()`

Loads the fixed Slot into memory.

Validation order:

```text
Check Slot existence
→ Load temporary USaveGame object
→ Cast to UWCGameSaveGame
→ Validate SaveVersion
→ assign LoadedSaveData only after all checks pass
```

If loading or validation fails, an existing valid `LoadedSaveData` is not replaced by an invalid object.

---

## `DeleteSavedGame()`

Deletes the fixed disk Slot and clears `LoadedSaveData`.

The operation is idempotent:

```text
Delete once
and
Delete again
→ same final state: no save exists
```

Calling the function when the Slot is already absent is treated as a successful final result rather than a gameplay error.

---

## Read Accessors

Added:

```cpp
GetLoadedSaveData()
GetSaveSlotName()
GetSaveUserIndex()
```

These provide safe access for future testing and for the later Story Persistence Coordinator.

---

# Independent Persistence Test

A temporary Level Blueprint test path was used to validate the SaveGame and GameInstance layers without depending on the existing Story Actors.

## Save Test

The test created a new in-memory save and wrote independent test data including:

```text
Checkpoint ID:
Week7.Day2.PersistenceTest
```

and an Objective record:

```text
Objective ID:
Week7.DebugObjective

Completed:
true
```

The save operation succeeded and created:

```text
Saved/SaveGames/WangChuan_Save_01.sav
```

---

## Cross-PIE Load Test

The test did not stop at saving and loading during one PIE session.

Validation sequence:

```text
Start PIE
→ create and save test data
→ stop PIE
→ start a new PIE session
→ load from disk
→ read the stored values
```

The new PIE session successfully recovered:

- Save Version
- Checkpoint ID
- Objective array
- Objective ID
- Objective completion Boolean

This confirmed that the data was persisted to disk rather than only surviving in the original GameInstance memory.

---

## Delete Test

The Save Slot was deleted successfully.

Repeated deletion remained safe.

Loading after deletion failed cleanly as expected and did not:

- Crash
- Produce `Accessed None`
- Create a replacement blank save
- Restore stale in-memory data
- Generate a Blueprint Runtime Error

---

# Visual Studio IntelliSense Issue

## Symptom

After creating the new SaveGame C++ files through Visual Studio Solution Explorer:

- `Build Solution` succeeded
- UnrealBuildTool compiled and linked successfully
- Visual Studio still displayed many red underlines
- Error List showed a large number of apparent errors

Many reported locations were inside Unreal Engine 5.8 template and engine headers, including:

```text
StaticAssertCompleteType.h
IsContiguousContainer.h
```

---

## Diagnosis

The new persistence implementation did not contain a real compilation failure.

Confirmed facts:

- UnrealBuildTool compiled successfully
- Generated headers existed
- `UnrealEditor-WangChuan.dll` linked successfully
- The reported errors came from Visual Studio IntelliSense
- The `.vs` database contained stale or excessive cached parsing data
- Header-only `WCSaveTypes.h` lacked its own translation unit, making standalone IntelliSense parsing less stable

The issue was therefore classified as:

```text
Visual Studio IntelliSense false positives
```

rather than:

```text
C++ / UnrealBuildTool compilation errors
```

---

## Resolution

Completed troubleshooting steps:

1. Closed Visual Studio.
2. Moved the old `.vs` IntelliSense cache to a temporary backup location.
3. Regenerated the Visual Studio project files through Unreal Engine 5.8 UnrealBuildTool.
4. Added:

   ```text
   Source/WangChuan/WCSaveTypes.cpp
   ```

   containing:

   ```cpp
   #include "WCSaveTypes.h"
   ```

5. Regenerated project files and performed a full build.
6. Set Visual Studio C/C++ IntelliSense:

   ```text
   Disable Squiggles = True
   ```

7. Set Error List source to:

   ```text
   Build Only
   ```

The extra `.cpp` file only supplies a stable translation unit for tooling and does not alter runtime logic.

---

## Final Visual Studio Validation

Final result:

```text
0 Errors
0 Warnings
0 Messages
```

Additional confirmation:

- `WCSaveTypes.cpp` participated in compilation
- `UnrealEditor-WangChuan.dll` linked successfully
- Error underlines disappeared
- The Error List now emphasizes real build results instead of IntelliSense template false positives

---

# Build and Runtime Verification

## Compilation

Passed:

```text
WangChuanEditor
Win64
Development
```

Verified:

- UnrealHeaderTool generated the required headers
- All new translation units compiled
- The project module linked successfully
- No real C++ compile errors were present

---

## Editor Configuration

Passed:

- `WCGameInstance` was available in Project Settings
- The project successfully used it as the active Game Instance
- PIE initialization reached `UWCGameInstance::Init()`

---

## Save / Load Validation

Passed:

- New in-memory save creation
- Fixed Slot existence check
- Synchronous save
- `.sav` file creation
- New-PIE-session load
- Struct and array serialization
- `FName` serialization
- Boolean serialization
- Save Version validation path
- Loaded save access
- Slot deletion
- Repeated deletion safety
- Missing-slot load rejection

---

## Output Log

No new project C++ error, Blueprint error, Blueprint Runtime Error, or `Accessed None` was introduced by the Day 1 Advanced implementation.

The Visual Studio error display issue was separately resolved as an IntelliSense/tooling problem.

---

# Architectural Understanding

## SaveGame Is Data, Not Gameplay

`UWCGameSaveGame` stores facts but does not decide what those facts mean inside the level.

It cannot:

- Find Quiet Child
- Complete an Objective
- Activate a Relic
- Move an NPC
- Open the Journal
- Play relocation VFX

---

## GameInstance Is the Disk Gateway

`UWCGameInstance` owns the current in-memory SaveGame object and the disk API.

It does not permanently own:

```cpp
AWCStoryNPC*
AStoryObjectiveBase*
AStoryEncounter*
AEchoRelic*
AWCCharacter*
```

Those Actors belong to the current World and may be destroyed when a map is unloaded or PIE ends.

---

## Disk Data and World Restoration Are Separate Phases

The completed flow currently ends at:

```text
Disk
→ UWCGameSaveGame
→ UWCGameInstance::LoadedSaveData
```

The project does not yet perform:

```text
LoadedSaveData
→ current level Story Actors
```

That boundary is intentional.

---

## Save Facts Remain Minimal

The save format currently stores:

```text
NPC Stage and Anchor
Objective completion
Encounter completion
Journal Echo records
Checkpoint ID
```

It does not store transitional states such as:

```text
Relocating
Previewing
Resetting
Current Dialogue Line
Current Puzzle Input
Timer Progress
Current Lock-On Target
Attack Montage Progress
```

---

# Scope Preserved

Day 1 Advanced did not add:

- Story Actor scanning
- Stable-ID duplicate validation at runtime
- World-state capture
- Story-state restoration
- Restore APIs
- Persistence Coordinator
- Checkpoint Actors
- Player relocation
- Auto Save
- New Game UI
- Save / Load menu
- Multi-slot support
- Async Save
- Save-version migration

Existing combat, dialogue, Story Encounter, Echo Relic, Journal, puzzle, NPC relocation, and animation systems were not changed by this milestone.

---

# Final Result

Day 1 Advanced / Original Day 2 is complete.

The project now has a verified persistence foundation:

```text
Stable Story IDs
→ Serializable Save Structs
→ UWCGameSaveGame
→ UWCGameInstance
→ Fixed Disk Slot
→ Cross-PIE Save / Load
```

The system has proven that structured Story data can survive stopping and restarting PIE.

The next planned milestone is:

```text
Original Day 3 — World State Capture
```

That phase should create the lightweight Story Persistence Coordinator and begin collecting real stable state from:

- Quiet Child
- Objective 01
- Objective 02
- Encounter 01
- Encounter 02
- Recorded Memory Echoes

It should still avoid restoring the world until the capture path and duplicate-ID validation are independently verified.
