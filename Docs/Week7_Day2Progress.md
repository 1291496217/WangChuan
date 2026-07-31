# 《忘川河畔》Week 7 — Day 2 Progress

**Project:** WangChuan / 《忘川河畔》  
**Active Project:** `WangChuan_UE58_Migration`  
**Internal Module:** `WangChuan`  
**Engine:** Unreal Engine 5.8  
**Date:** 2026-07-31  
**Week Theme:** Story Persistence, SaveGame & Resume Flow  
**Milestone:** Week 7 Day 2 / Original Day 3  
**Day Theme:** World State Capture

---

## Day Goal

Day 2 connected the SaveGame and GameInstance foundation from Day 1 Advanced to the real runtime Story world.

The target flow was:

```text
Current World stable Story facts
→ AWCStoryPersistenceCoordinator
→ UWCGameSaveGame
→ UWCGameInstance
→ WangChuan_Save_01.sav
```

This milestone intentionally implemented **Capture only**. Loaded data can be inspected in a new PIE session, but is not yet applied back to the World.

---

## Result Overview

Completed:

- `AWCStoryPersistenceCoordinator`
- Runtime validation for Story NPC, Objective, Encounter, Echo, and Anchor IDs
- Current Story Anchor lookup for `AWCStoryNPC`
- Read-only Journal access for persistence capture
- Story NPC state capture
- Objective completion capture
- Encounter completion capture
- Ordered Memory Echo capture
- Save rejection during transient NPC relocation
- Temporary-array capture before SaveGame mutation
- Complete snapshot replacement on repeated saves
- Fixed-slot disk save through `UWCGameInstance`
- Cross-PIE load and summary inspection
- Full compile, runtime, and manual code review

---

# Files

## Created

```text
Source/WangChuan/WCStoryPersistenceCoordinator.h
Source/WangChuan/WCStoryPersistenceCoordinator.cpp
```

## Modified

```text
Source/WangChuan/WCStoryNPC.h
Source/WangChuan/WCStoryNPC.cpp
Source/WangChuan/WCCharacter.h
Source/WangChuan/WCCharacter.cpp
```

The Day 1 Advanced data and disk layers remained unchanged:

```text
WCSaveTypes
UWCGameSaveGame
UWCGameInstance
```

No `WangChuan.Build.cs` change was required.

---

# 1. Story NPC Persistence Accessor

Added:

```cpp
FName GetCurrentStoryAnchorID() const;
```

The current stable Anchor is derived from:

```text
CurrentStoryStage
→ StoryAnchors[CurrentStoryStage]
→ AStoryAnchor::GetAnchorID()
```

Current mapping:

```text
Stage 0 → QuietChild.Anchor01
Stage 1 → QuietChild.Anchor02
Stage 2 → QuietChild.Anchor03
```

The function returns `NAME_None` for an invalid Stage index or invalid Anchor.

No duplicated mutable `CurrentAnchorID` field was introduced because the current Demo has a one-to-one Stage/Anchor relationship.

---

# 2. Journal Read Accessor

Added to `AWCCharacter`:

```cpp
const TArray<FMemoryEchoData>&
GetRecordedMemoryEchoes() const;
```

The Persistence Coordinator can read the player's completed Journal records without directly modifying protected player data.

The accessor:

- Returns a const reference
- Does not copy the array unnecessarily
- Does not add Echoes
- Does not refresh Journal UI
- Does not open Memory Echo UI
- Does not broadcast Story events

The player remains the runtime owner of `RecordedMemoryEchoes`.

---

# 3. Story Persistence Coordinator

Created:

```cpp
AWCStoryPersistenceCoordinator : public AActor
```

The Actor was placed once in `Prototype_Map`.

The class:

- Does not Tick
- Does not use collision
- Does not own Story progression
- Does not automatically save in `BeginPlay`
- Does not restore the World
- Does not call Gameplay transition functions

Public API:

```cpp
ValidateWorldPersistenceIDs()
CaptureAndSaveWorldState()
PrintLoadedSaveSummary()
```

---

# 4. Runtime ID Validation

The Coordinator scans and validates:

- `AWCStoryNPC`
- `AStoryObjectiveBase`
- `AStoryEncounter`
- `AEchoRelic`
- `AStoryAnchor`

Every persistence ID must:

```text
not be None
be unique within its Actor type
belong to an existing required Actor
```

Validated formal IDs:

## Story NPC

```text
QuietChild
```

## Objectives

```text
QuietChild.EnemyDefeat01
QuietChild.LanternPuzzle01
```

## Encounters

```text
QuietChild.Encounter01
QuietChild.Encounter02
```

## Echoes

```text
QuietChild.BellEcho01
QuietChild.LanternEcho02
```

## Anchors

```text
QuietChild.Anchor01
QuietChild.Anchor02
QuietChild.Anchor03
```

Expected runtime counts passed:

```text
NPCs = 1
Objectives = 2
Encounters = 2
Echoes = 2
Anchors = 3
```

Invalid or duplicate IDs cause capture to fail before the disk save is touched.

---

# 5. Stable World Capture

`CaptureAndSaveWorldState()` collects a complete World snapshot.

Each successful capture contains:

```text
1 Story NPC record
2 Objective records
2 Encounter records
0–2 Journal Echo records
```

Incomplete records are included explicitly. For example:

```text
QuietChild.LanternPuzzle01
Completed = False
```

This keeps the SaveGame representation complete and predictable.

---

## Story NPC Data

Captured:

```cpp
StoryNPCID
StoryStage
AnchorID
```

Not captured:

- `StoryState`
- Pending Story Stage
- Pending Anchor pointer
- Relocation timers
- Niagara playback
- Dialogue line index

---

## Objective Data

Captured:

```cpp
ObjectiveID
bCompleted
```

Not captured:

- `bIsActive`
- Required Enemy pointer
- Lantern Preview index
- Current puzzle input
- Reset timers
- Temporary lantern presentation

---

## Encounter Data

Captured:

```cpp
EncounterID
bCompleted
```

Not captured:

- `bStoryObjectiveCompleted`
- Objective pointer
- Relic pointer
- NPC pointer
- Story-event transition state

The intermediate state remains valid:

```text
Objective Completed = True
Encounter Completed = False
```

---

## Journal Data

The Coordinator copies `RecordedMemoryEchoes` in its existing order.

Every record is checked for:

```text
EchoID != None
no duplicate EchoID
```

Capture does not call `RecordMemoryEcho()` and therefore cannot replay UI, delegates, or Encounter completion.

---

# 6. Transaction-Like Capture

New state is first collected into local arrays:

```cpp
CapturedStoryNPCStates
CapturedObjectiveStates
CapturedEncounterStates
CapturedMemoryEchoes
```

Only after every record passes validation are the SaveGame arrays replaced.

This prevents partial in-memory mutation when a later capture step fails.

Final commit pattern:

```text
build complete candidate snapshot
→ validate all records
→ replace LoadedSaveData arrays
→ SaveCurrentGame once
```

Repeated saves use Reset/Append replacement rather than accumulating duplicate entries.

---

# 7. Transitional-State Save Protection

The Quiet Child Encounter completion flow includes:

```text
Encounter completed
→ EventResolved
→ Relocating
→ Stage/Anchor updated
→ Available
```

Saving during `EventResolved` or `Relocating` could store a mixed state such as:

```text
Encounter 01 Completed = True
NPC Stage = 0
NPC Anchor = Anchor01
```

The Coordinator therefore rejects capture during both transitional states.

The previous valid disk save remains unchanged. Saving succeeds after the NPC reaches its new stable Anchor and returns to `Available`.

---

# 8. SaveGame Commit

If no valid `LoadedSaveData` exists:

```text
existing disk Slot
→ load it

no disk Slot
→ create a new in-memory save
```

After capture succeeds, the Coordinator replaces:

```cpp
StoryNPCStates
ObjectiveStates
EncounterStates
RecordedMemoryEchoes
```

and calls:

```cpp
UWCGameInstance::SaveCurrentGame()
```

exactly once.

Checkpoint persistence remains outside this milestone:

```cpp
CurrentCheckpointID = NAME_None;
```

---

# 9. Loaded Save Summary

`PrintLoadedSaveSummary()` displays:

- Save Version
- Checkpoint ID
- Record counts
- NPC Stage and Anchor
- Objective completion values
- Encounter completion values
- Journal Echo IDs and titles

It only inspects `LoadedSaveData`.

It does not move NPCs, complete Objectives, activate Relics, restore the Journal, or modify the current World.

---

# Implementation Correction

During implementation, a malformed copied code block was identified in the on-screen summary call.

Incorrect fragment:

```cpp
"Loaded Save	ShowPersistenceMessage(
```

This was a duplicated copy/paste insertion and was not valid C++.

Correct implementation:

```cpp
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
```

The correction was compiled and manually reviewed before final acceptance.

This reinforces the Week 7 workflow:

```text
suggested/generated code
→ inspect exact syntax and ownership
→ compile
→ test side effects and results
→ accept only after review
```

---

# Test Results

## Build

Passed:

```text
WangChuanEditor
Win64
Development
```

No new module dependency was needed.

---

## Initial World Capture

Passed.

Expected state:

```text
NPC [QuietChild]
Stage = 0
Anchor = QuietChild.Anchor01
```

```text
EnemyDefeat01 = False
LanternPuzzle01 = False
Encounter01 = False
Encounter02 = False
Journal Echoes = 0
```

The Coordinator saved a complete initial snapshot.

---

## Objective Complete / Encounter Incomplete

Passed.

After defeating the required enemy but before reading Bell Echo 01:

```text
QuietChild.EnemyDefeat01 = True
QuietChild.Encounter01 = False
NPC Stage = 0
NPC Anchor = QuietChild.Anchor01
Journal Echoes = 0
```

This correctly preserves:

```text
Objective Condition Resolved
≠
Encounter Completed
```

---

## Transitional Save Rejection

Passed.

A capture request during Quiet Child relocation was rejected.

After relocation finished, capture succeeded normally.

---

## Encounter 01 Completed Snapshot

Passed.

Stable captured result:

```text
NPC [QuietChild]
Stage = 1
Anchor = QuietChild.Anchor02
```

```text
QuietChild.EnemyDefeat01 = True
QuietChild.LanternPuzzle01 = False
QuietChild.Encounter01 = True
QuietChild.Encounter02 = False
```

```text
Journal:
QuietChild.BellEcho01
```

Counts:

```text
NPCs = 1
Objectives = 2
Encounters = 2
Echoes = 1
```

---

## Cross-PIE Load Test

Passed.

Sequence:

```text
capture real Story World
→ save to disk
→ stop PIE
→ start new PIE
→ load Slot
→ print summary
```

The new PIE World still began in its default state, which is expected because Restore is not yet implemented.

The loaded SaveGame retained the correct Story Stage, Anchor, Objective, Encounter, and Journal data.

---

## Repeated Save Test

Passed.

Repeated captures did not append duplicates.

Counts remained stable:

```text
NPCs = 1
Objectives = 2
Encounters = 2
Echoes = actual recorded count
```

---

## Runtime Stability

Passed.

No new:

- C++ compile error
- Blueprint compile error
- Blueprint Runtime Error
- `Accessed None`
- Array bounds error
- Delegate error
- Timer error
- Persistence ID error

remained after the final correction and manual review.

---

# Architecture Review

## Persistent Facts Captured

```text
NPC stable Story Stage
NPC stable Anchor ID
Objective completion
Encounter completion
recorded Journal Echoes
```

## Derived State Not Saved Separately

```text
Objective incomplete
→ Relic Locked
```

```text
Objective complete + Encounter incomplete
→ Relic Available
```

```text
Encounter complete
→ Relic Activated
```

```text
Lantern Objective complete
→ Puzzle completed presentation
```

## Transitional State Excluded

```text
EventResolved
Relocating
Previewing
Resetting
current puzzle input
timer progress
dialogue line
Journal selection
Echo page
attack montage progress
Lock-On target
```

## Actor Pointers Remain Runtime-Only

No SaveGame field contains pointers to Story Actors.

The Coordinator reads current Actor instances and stores semantic IDs. Future Restore will use those IDs to match new instances in the loaded World.

---

# Scope Preserved

Day 2 did not implement:

- `ApplySavedStoryState()`
- `ApplySavedObjectiveState()`
- `ApplySavedEncounterState()`
- `ApplySavedRelicState()`
- Journal restoration
- Enemy defeated-state restoration
- Lantern completed-state restoration
- automatic startup restore
- auto-save
- player checkpoints
- Continue/New Game UI
- multi-slot support
- async save
- save-version migration

---

# Final Result

Week 7 Day 2 / Original Day 3 is complete.

The project now supports:

```text
Stable Story IDs
→ runtime World validation
→ stable fact capture
→ complete SaveGame snapshot
→ fixed-slot disk save
→ cross-PIE load and inspection
```

The capture path has been independently verified before introducing World Restore.

The next planned milestone is:

```text
Week 7 Day 3
→ Silent Restore Interfaces
```

Expected restore interfaces include:

```cpp
AWCStoryNPC::ApplySavedStoryState(...)
AStoryObjectiveBase::ApplySavedObjectiveState(...)
AStoryEncounter::ApplySavedEncounterState(...)
AEchoRelic::ApplySavedRelicState(...)
AWCCharacter::ApplySavedMemoryEchoes(...)
```

These functions must restore stable results without broadcasting Gameplay delegates, reopening Echo UI, replaying Story events, duplicating Journal entries, or playing NPC relocation effects.
