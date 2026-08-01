# 《忘川河畔》Week 7 — Day 3 Progress

**Project:** WangChuan / 《忘川河畔》  
**Active Project:** `WangChuan_UE58_Migration`  
**Internal Module:** `WangChuan`  
**Engine:** Unreal Engine 5.8  
**Date:** 2026-08-01  
**Week Theme:** Story Persistence, SaveGame & Resume Flow  
**Milestone:** Week 7 Day 3 / Original Day 5  
**Day Theme:** Full World Restore Coordination and Load Order

---

## Day Goal

Day 3 connected the independently verified persistence systems into one complete Save / Load / Restore flow.

Previously completed:

```text
Runtime World
→ stable state capture
→ SaveGame
→ disk
```

```text
Saved stable facts
→ silent Actor Restore interfaces
```

Day 3 completed:

```text
Disk Save
→ UWCGameInstance
→ AWCStoryPersistenceCoordinator
→ stable ID matching
→ ordered silent Restore
→ complete Story World
```

The project can now stop PIE, start a new PIE session, load the fixed Save Slot, and automatically reconstruct the saved Quiet Child Story state without replaying the events that originally produced it.

---

## Result Overview

Completed:

- Deferred next-tick automatic loading
- No-save New Game handling
- Current-World ID-to-Actor map construction
- Full SaveGame/World compatibility validation
- Story Encounter relationship accessors
- Story NPC Anchor prevalidation
- Ordered Objective restoration
- Ordered Encounter restoration
- Derived Echo Relic restoration
- Quiet Child Stage and Anchor restoration
- Memory Journal restoration
- Automatic State B restoration
- Continued Encounter 02 gameplay after State B restore
- Automatic State C restoration
- Repeated Load / Restore idempotency
- Full side-effect monitoring
- Manual code and runtime review

All planned tests passed.

---

# Modified Files

```text
StoryEncounter.h
StoryEncounter.cpp
WCStoryNPC.h
WCStoryNPC.cpp
WCStoryPersistenceCoordinator.h
WCStoryPersistenceCoordinator.cpp
```

Reused without modification:

```text
WCSaveTypes
WCGameSaveGame
WCGameInstance
StoryObjectiveBase
EnemyDefeatObjective
GhostEnemy
LanternSequencePuzzle
LanternPuzzlePiece
EchoRelic
WCCharacter
StoryAnchor
```

No `WangChuan.Build.cs` change was required.

---

# 1. Story Encounter Relationship Accessors

Added:

```cpp
AStoryObjectiveBase* GetStoryObjective() const;
AEchoRelic* GetEchoRelic() const;
```

The Coordinator now reads the actual runtime relationship:

```text
Encounter
→ configured Objective
→ configured Echo Relic
```

This avoids hardcoded persistence mappings. These pointers are used only inside the current World and are never serialized.

---

# 2. Story NPC Anchor Prevalidation

Added:

```cpp
bool HasStoryAnchorID(FName AnchorID) const;
```

The function validates saved Anchor IDs through the existing stable-ID search without modifying the NPC.

This allows the Coordinator to reject an invalid save before any Objective, Encounter, Relic, NPC, or Journal state is changed.

---

# 3. Deferred Automatic Restore

The Coordinator now supports:

```cpp
bAutoLoadAndRestoreOnBeginPlay
```

Restore flow:

```text
Coordinator BeginPlay
→ SetTimerForNextTick
→ level Actors finish BeginPlay
→ HandleDeferredAutoRestore()
→ Load and Restore
```

This avoids restoring before other Actors finish their own initialization, including:

- Enemy Objective Delegate binding
- Encounter Delegate binding
- Lantern Puzzle setup
- Lantern Dynamic Material creation
- Echo Relic interaction initialization

The deferred timer is cleared in `EndPlay()`.

---

# 4. No-Save New Game Handling

When `WangChuan_Save_01` does not exist, the Coordinator:

- Does not create a blank save
- Does not report an Error
- Does not modify the default World
- Does not call Restore interfaces

The authored map state remains the valid State A:

```text
Quiet Child at Stage 0 / Anchor 01
Required Enemy alive
Objectives incomplete
Encounters incomplete
Relics locked
Journal empty
```

---

# 5. Current-World Actor Maps

The Coordinator builds temporary maps for:

```cpp
TMap<FName, AWCStoryNPC*>
TMap<FName, AStoryObjectiveBase*>
TMap<FName, AStoryEncounter*>
TMap<FName, AEchoRelic*>
TMap<FName, AStoryAnchor*>
```

Expected counts:

```text
Story NPCs = 1
Objectives = 2
Encounters = 2
Echo Relics = 2
Story Anchors = 3
```

These maps exist only during Restore and are not stored by `UWCGameInstance` or `UWCGameSaveGame`.

---

# 6. Full SaveGame / World Prevalidation

Before any Actor is modified, the Coordinator validates:

- SaveGame validity
- Supported `SaveVersion`
- Save record counts versus current World counts
- Non-`None` IDs
- No duplicate saved IDs
- Every saved ID exists in the current World
- Every saved NPC Anchor exists
- Every saved Anchor belongs to the configured NPC
- Every Encounter has a valid Objective and Echo Relic
- No Echo Relic belongs to multiple Encounters
- Completed Encounter implies completed Objective
- Encounter completion agrees with Journal Echo presence
- Every Journal Echo corresponds to a current Echo Relic

Validation completes before the first call to `ApplySavedObjectiveState()`.

This prevents a half-restored World.

---

# 7. Disk Load Entry

Added:

```cpp
LoadAndRestoreWorldState()
```

The function:

```text
validates WCGameInstance
→ checks Slot existence
→ reloads SaveGame from disk
→ calls RestoreLoadedWorldState()
```

Each explicit Load reads the current disk Slot rather than relying on stale in-memory data.

Restore does not automatically save again.

---

# 8. Ordered Full World Restore

Added:

```cpp
RestoreLoadedWorldState()
```

Final dependency order:

```text
1. Objectives
2. Encounters
3. Echo Relics
4. Story NPCs
5. Journal
```

## Objectives

```cpp
ApplySavedObjectiveState()
```

This restores:

- Required Enemy persistent defeated presentation
- Lantern Puzzle completed or Dormant presentation

No Objective Delegate is broadcast.

## Encounters

```cpp
ApplySavedEncounterState()
```

Encounter restoration occurs after Objectives because it derives `bStoryObjectiveCompleted` from the restored Objective.

No Story Event, Relic unlock, or NPC relocation occurs.

## Echo Relics

Relic state is derived rather than read from a duplicated SaveGame field:

```text
Objective incomplete + Encounter incomplete
→ locked
```

```text
Objective complete + Encounter incomplete
→ Available
```

```text
Encounter complete
→ Activated
```

The result is applied through `ApplySavedRelicState()` without UI or Delegate side effects.

## Story NPCs

```cpp
ApplySavedStoryState(Stage, AnchorID)
```

Quiet Child is moved directly to the saved stable Anchor.

No `EventResolved`, `Relocating`, Niagara, or relocation timer is used.

## Journal

```cpp
ApplySavedMemoryEchoes()
```

The saved order is preserved without treating old Echoes as newly acquired.

---

# 9. Restore Re-entry Guard

The Coordinator tracks:

```cpp
bRestoreInProgress
bHasRestoredLoadedWorld
SuccessfulRestoreCount
```

Overlapping Restore requests are rejected.

After one Restore finishes, the same stable data may be applied again for idempotency testing.

---

# 10. State A Test — No Save

Passed.

Verified:

```text
Quiet Child:
Stage 0
Anchor 01

Required Enemy:
Alive

Enemy Objective:
Incomplete and playable

Lantern Puzzle:
Incomplete

Encounters:
Incomplete

Relics:
locked

Journal:
Empty
```

No Error was produced for the missing save file.

---

# 11. State B Automatic Restore

State B save:

```text
Quiet Child:
Stage 1
Anchor 02

Enemy Objective:
Completed

Encounter 01:
Completed

Bell Echo 01:
Recorded

Lantern Objective:
Incomplete

Encounter 02:
Incomplete
```

After restarting PIE, the Coordinator restored State B automatically on the deferred next tick.

Passed.

Verified:

- Required Enemy did not return as an active enemy
- Enemy collision and Tick were disabled
- Enemy death audio did not replay
- `OnEnemyDefeated` did not broadcast
- Enemy Objective was completed
- Encounter 01 was completed
- Bell Echo 01 was Activated and non-repeatable
- Quiet Child appeared directly at Anchor 02
- Story Stage was 1
- Story State was `Available`
- Stage 1 dialogue worked
- No Niagara played
- NPC did not enter `Relocating`
- Lantern Puzzle remained incomplete and playable
- Lantern Echo 02 remained `locked`
- Journal contained exactly Bell Echo 01

---

# 12. Continued Gameplay After State B

From the restored State B World, the player continued normally:

```text
enter Lantern area
→ Preview begins
→ complete five-lantern sequence
→ Lantern Echo becomes Available
→ read Echo 02
→ Encounter 02 completes
→ Quiet Child relocates normally to Anchor 03
```

Passed.

This verified that Restore preserved:

- Objective activation
- Activation Box behavior
- Lantern interaction Delegates
- Puzzle completion
- Encounter Delegate bindings
- Echo activation
- Normal NPC relocation
- Story Stage progression

A restored incomplete Encounter remained fully playable.

---

# 13. State C Automatic Restore

State C:

```text
Quiet Child:
Stage 2
Anchor 03

Enemy Objective:
Completed

Lantern Objective:
Completed

Encounter 01:
Completed

Encounter 02:
Completed

Bell Echo 01:
Activated

Lantern Echo 02:
Activated

Journal:
Echo 01 → Echo 02
```

Passed.

Verified:

- Required Enemy remained persistently defeated
- All five lanterns remained lit
- Lantern interaction was disabled
- Puzzle Activation Box was disabled
- Both Objectives remained completed
- Both Encounters remained completed
- Both Relics were Activated and non-interactable
- Quiet Child appeared directly at Anchor 03
- Story Stage was 2
- Stage 2 dialogue worked
- Journal order remained Echo 01 followed by Echo 02
- No Echo UI opened automatically
- No Journal UI opened automatically
- No NPC relocation VFX played
- No Story Event replayed

---

# 14. Repeated Load / Restore Test

Passed.

Repeated Restore did not cause:

- Duplicate Journal entries
- Enemy death replay
- Objective Delegate broadcast
- Echo Delegate broadcast
- NPC relocation
- Niagara
- Position drift
- Lantern Preview replay
- New timers
- New Story progression

`SuccessfulRestoreCount` increased only as diagnostic data; the World result remained unchanged.

---

# 15. Side-Effect Monitoring

Monitored Gameplay Delegates:

```text
AGhostEnemy::OnEnemyDefeated
AStoryObjectiveBase::OnObjectiveCompleted
AEchoRelic::OnEchoActivated
```

No unexpected Delegate broadcast occurred during automatic or repeated Restore.

Also verified absent:

- Enemy death audio
- Enemy death VFX
- Lantern Preview audio
- Echo UI
- Journal UI
- NPC Niagara
- NPC `Relocating`
- Story Event replay

---

# 16. Journal Verification

State B:

```text
RecordedMemoryEchoes.Num() = 1
QuietChild.BellEcho01
```

State C:

```text
RecordedMemoryEchoes.Num() = 2
QuietChild.BellEcho01
QuietChild.LanternEcho02
```

Saved order was preserved and repeated Restore did not duplicate records.

---

# Build and Runtime Verification

## Compilation

Passed:

```text
WangChuanEditor
Win64
Development
```

No new module dependency was required.

## Blueprint Verification

All related Story, Objective, Encounter, Relic, NPC, Enemy, Lantern, and Journal Blueprints compiled successfully.

## Runtime Log

No new:

- C++ error
- Blueprint compile error
- Blueprint Runtime Error
- `Accessed None`
- Array bounds error
- Timer error
- Delegate Ensure
- invalid persistence ID error
- partial Restore state

remained after final testing and manual review.

---

# Architecture Review

## Complete Persistence Flow

```text
Gameplay changes current World
→ Coordinator captures stable facts
→ UWCGameSaveGame stores records
→ UWCGameInstance writes fixed Slot
→ PIE / map session ends
→ new World initializes
→ Coordinator waits one tick
→ disk SaveGame loads
→ Save and World are validated
→ silent Restore APIs run in dependency order
→ stable Story World is reconstructed
```

## Source of Truth

Saved facts:

```text
NPC Stage and Anchor
Objective completion
Encounter completion
Journal Echo records
```

Derived during Restore:

```text
Required Enemy defeated presentation
Lantern Puzzle presentation
Encounter ObjectiveResolved state
Relic locked / Available / Activated state
NPC interaction and visibility
Runtime Journal array
```

## Final Load Order

```text
Objective
→ Encounter
→ Relic
→ Story NPC
→ Journal
```

The order is determined by dependency, not presentation.

## SaveGame Is State, Not History Replay

The SaveGame stores facts:

```text
Objective already completed
Encounter already completed
NPC already at Stage 2
Echo already recorded
```

It does not store commands such as:

```text
kill Enemy
complete Objective
activate Relic
move NPC
```

Restore reconstructs the result without replaying history.

---

# Scope Preserved

Day 3 did not implement:

- Player Checkpoint Actor
- Player position restore
- Auto Save after Encounter completion
- Save/Load menu
- New Game / Continue UI
- Multiple save slots
- Cloud save
- Async save
- Cross-map restore
- SaveGame migration
- Runtime rollback or Enemy revival
- Global Actor auto-serialization

The project continues to use one fixed Slot and one Story map.

---

# Final Result

Week 7 Day 3 / Original Day 5 is complete.

The project now has a working end-to-end Story persistence loop:

```text
Complete Story progress
→ Capture real World
→ Save to disk
→ stop PIE
→ start new PIE
→ automatically load
→ validate IDs and facts
→ silently restore the Story World
→ continue unfinished gameplay
```

Supported stable progress states:

```text
State B:
Encounter 01 complete
Encounter 02 still playable
```

```text
State C:
Encounter 01 and Encounter 02 complete
```

The next planned milestone is:

```text
Week 7 Day 4
→ Checkpoint and Resume Position
```

Expected next work:

- `AWCPlayerCheckpoint`
- Stable `CheckpointID`
- Checkpoint capture
- Safe player relocation after World Restore
- Velocity cleanup
- Ground-safe resume placement
- Optional manual Save / Continue flow

The core Story persistence architecture is now operational and verified.
