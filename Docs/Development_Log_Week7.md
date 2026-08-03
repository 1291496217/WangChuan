# 《忘川河畔》Week 7 Progress

**Project:** WangChuan / 《忘川河畔》  
**Active Project:** `WangChuan_UE58_Migration`  
**Internal Module:** `WangChuan`  
**Engine:** Unreal Engine 5.8  
**Development Period:** 2026-07-30 — 2026-08-02  
**Week Theme:** Story Persistence, SaveGame, Resume Flow & Visible Rest Points  
**Status:** Complete through Day 4

---

## Week 7 Goal

Week 7 focused on building a complete persistence loop for the existing Quiet Child story flow:

```text
Runtime Story World
→ stable-state capture
→ SaveGame
→ disk
→ new PIE / new World
→ ordered silent restoration
→ continue unfinished gameplay
```

The implementation was then extended with player resume locations and redesigned into a visible, player-operated rest-point system:

```text
Visible Soul Rest Point
→ press E
→ validate stable World
→ manually save
→ unlock destination
→ same-map Fast Travel
```

The final system deliberately does **not** use Auto Save.

---

# Final Completion Summary

Completed:

- Persistent-state audit and stable semantic IDs
- SaveGame data structures and Save Version validation
- Custom GameInstance disk Save / Load API
- Complete Story World capture
- Transitional-state save rejection
- Side-effect-free Actor Restore interfaces
- Ordered full-World restoration
- Automatic load after World initialization
- Player Checkpoint and safe resume placement
- Visible Soul Rest Points
- Manual save through interaction
- Persistent unlocked rest-point list
- Same-map Fast Travel
- Transactional save and rollback behavior
- State A / B / C cross-PIE verification
- Combat, Story, UI, and Persistence regression testing

---

# 1. Stable Persistence Identity

Week 7 first established stable semantic IDs rather than relying on Actor pointers, generated names, or editor labels.

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

## Echo Relics

```text
QuietChild.BellEcho01
QuietChild.LanternEcho02
```

## Story Anchors

```text
QuietChild.Anchor01
QuietChild.Anchor02
QuietChild.Anchor03
```

## Player Checkpoints

```text
WangChuan.Checkpoint.Start
WangChuan.Checkpoint.AfterEncounter01
WangChuan.Checkpoint.AfterEncounter02
```

All IDs were validated as non-`None` and unique within their Actor type.

Stable IDs describe narrative meaning and remain independent from:

- Actor memory address
- Runtime spawn order
- Actor Name
- Outliner Label
- Blueprint display name
- Current visual presentation

---

# 2. Persistence State Model

Week 7 separated state into four categories.

## Persistent Facts

Saved to disk:

```text
Save Version
Current Checkpoint ID
Unlocked Checkpoint IDs

Quiet Child Story Stage
Quiet Child Anchor ID

Objective completion
Encounter completion

Recorded Memory Echoes
```

## Transitional State

Not saved:

```text
NPC EventResolved / Relocating
Pending Story Stage
Relocation timers and Niagara

Lantern Preview / Resetting
Current puzzle input
Preview and reset timers

Echo activation in progress
Dialogue / Echo / Journal page state

Attack, Combo, Hit React, Lock-On
Velocity and animation progress
```

## Derived State

Reconstructed during load:

```text
Enemy defeated presentation
Lantern completed presentation
Encounter Objective-resolved cache
Echo Relic locked / Available / Activated
NPC stable Available state
Rest-point light, material, VFX, Prompt, and menu state
```

## Level Configuration

Remains in map and Blueprint assets:

```text
Story Actor references
Story Anchor references
Next Story Stage
Completion Story Event
Correct lantern sequence
Dialogue content
Echo content
Checkpoint resume transforms
```

---

# 3. SaveGame and GameInstance Foundation

Created:

```text
WCSaveTypes.h / .cpp
WCGameSaveGame.h / .cpp
WCGameInstance.h / .cpp
```

## Save Data Structures

Implemented:

```cpp
FWCSavedStoryNPCState
FWCSavedObjectiveState
FWCSavedEncounterState
```

Final `UWCGameSaveGame` stores:

```text
SaveVersion
CurrentCheckpointID
UnlockedCheckpointIDs
StoryNPCStates
ObjectiveStates
EncounterStates
RecordedMemoryEchoes
```

The final format is:

```cpp
CurrentSaveVersion = 2;
```

Version 2 added persistent unlocked rest-point IDs. No Version 1 → Version 2 migration framework was added; incompatible development saves are rejected safely.

---

## `UWCGameInstance`

Configured as the project GameInstance.

Fixed slot:

```text
Slot Name: WangChuan_Save_01
User Index: 0
```

Implemented:

```cpp
CreateNewSave()
HasSavedGame()
SaveCurrentGame()
LoadSavedGame()
DeleteSavedGame()
GetLoadedSaveData()
```

Design boundary:

```text
UWCGameSaveGame
→ serializable data only

UWCGameInstance
→ disk access and in-memory save ownership
```

`CreateNewSave()` does not automatically overwrite disk data. `LoadSavedGame()` validates a temporary object before replacing the currently loaded save. Deletion is idempotent.

Cross-PIE Save / Load / Delete tests passed.

---

# 4. Story World Capture

Created:

```text
WCStoryPersistenceCoordinator.h / .cpp
```

The Coordinator is placed once in `Prototype_Map` and does not Tick.

It validates and scans:

```text
Story NPCs
Objectives
Encounters
Echo Relics
Story Anchors
Player Checkpoints
Player Character
```

## Captured Story Snapshot

```text
NPC ID / Stage / Anchor
Objective ID / Completed
Encounter ID / Completed
Journal Echo records in existing order
Current Checkpoint ID
Unlocked Checkpoint IDs
```

Capture uses local candidate arrays before mutating the loaded SaveGame:

```text
build complete candidate snapshot
→ validate every record
→ replace SaveGame arrays
→ write disk once
```

Repeated saves replace the previous snapshot rather than appending duplicates.

---

## Transitional-State Protection

Capture rejects unstable Quiet Child states:

```text
EventResolved
Relocating
```

This prevents inconsistent snapshots such as:

```text
Encounter completed
but
NPC still at the previous Stage / Anchor
```

A rejected capture leaves the previous disk save unchanged.

---

# 5. Silent Restore Interfaces

Normal Gameplay transitions and SaveGame restoration were separated explicitly.

| System | Gameplay Path | Restore Path |
|---|---|---|
| Enemy | `Die()` | `ApplyPersistentDefeatedState()` |
| Objective | `CompleteObjective()` | `ApplySavedObjectiveState()` |
| Lantern Puzzle | Preview / Input / Completion | Objective Restore Hook |
| Encounter | Objective / Echo Delegates | `ApplySavedEncounterState()` |
| Echo Relic | Unlock / Confirm Read | `ApplySavedRelicState()` |
| Story NPC | Story Event / Relocation | `ApplySavedStoryState()` |
| Journal | `RecordMemoryEcho()` | `ApplySavedMemoryEchoes()` |
| Player | Normal movement | `ApplySavedCheckpointState()` |

Restore functions apply already-established facts directly.

They do not replay:

```text
Enemy death
Objective completion
Echo activation
Encounter completion
Story Events
NPC relocation
Niagara
audio
Journal acquisition
Memory Echo UI
```

---

## Enemy and Enemy Objective

A completed Enemy Objective restores the required enemy as:

```text
Health = 0
dead flags set
collision disabled
Tick disabled
health bar hidden
Actor hidden
```

The Actor is retained instead of destroyed so references remain valid and repeated Restore remains safe.

---

## Five-Lantern Puzzle

Incomplete restore:

```text
PuzzleState = Dormant
all lanterns unlit
ActivationBox enabled
partial input and timers cleared
```

Completed restore:

```text
PuzzleState = Completed
all five lanterns lit
interaction disabled
ActivationBox disabled
timers cleared
```

Partial Preview or input progress is never persisted.

---

## Echo Relics

Relic state is derived rather than saved separately:

```text
Objective incomplete
+ Encounter incomplete
→ locked
```

```text
Objective complete
+ Encounter incomplete
→ Available
```

```text
Encounter complete
→ Activated
```

This avoids contradictory duplicated data.

---

## Quiet Child

Restore uses:

```text
StoryNPCID
StoryStage
AnchorID
```

The NPC is teleported directly to the saved Anchor, applies `MeshFacingYawOffset`, and enters a stable `Available` state.

Restore does not use:

```text
RecieveStoryEvent()
RelocateToStoryAnchor()
```

and does not replay relocation VFX or timers.

---

## Journal

Saved Echo records replace the runtime Journal array directly.

Restore preserves order, skips invalid/duplicate IDs, and does not call `RecordMemoryEcho()`.

---

# 6. Ordered Full-World Restore

The Coordinator automatically checks the fixed Save Slot after all level Actors finish `BeginPlay()`:

```text
Coordinator BeginPlay
→ SetTimerForNextTick
→ level Actors finish initialization
→ load SaveGame
→ validate World and save
→ ordered silent Restore
```

No-save startup is treated as a valid New Game state.

The final Restore order is:

```text
1. Objectives
2. Encounters
3. Echo Relics
4. Story NPCs
5. Memory Journal
6. Checkpoint Runtime State and Presentation
7. Player Location
```

The order follows data dependencies:

- Encounter reads restored Objective state.
- Relic state derives from Objective and Encounter.
- Checkpoint presentation derives from unlocked IDs.
- Player moves last so overlap checks evaluate against the final restored World.

---

## Full Prevalidation

Before the first Actor is modified, Restore validates:

- Save Version
- Save / World record counts
- Non-`None` and unique IDs
- Every saved Actor exists in the current map
- NPC Anchor exists and belongs to that NPC
- Encounter has valid Objective and Relic relationships
- Completed Encounter implies completed Objective
- Journal Echo presence agrees with Encounter completion
- Checkpoint IDs exist
- Current Checkpoint is unlocked
- Resume Transform can be built safely

Invalid data stops Restore before World mutation, preventing a partially restored level.

---

# 7. Player Checkpoint and Safe Resume

Created:

```text
WCPlayerCheckpoint.h / .cpp
```

Initial Checkpoint support added:

```text
stable CheckpointID
exactly one default Checkpoint
ActivationBox
ResumeArrow
safe ground Trace
player Capsule-height adjustment
resume yaw
```

The SaveGame stores a semantic Checkpoint ID rather than a raw Transform.

The current map resolves that ID to the latest designed resume location.

---

## Safe Placement

`ResumeArrow` represents the intended player feet position and horizontal facing direction.

Resume placement:

```text
Trace to ground
→ ground impact position
→ add scaled Capsule Half Height
→ add small clearance
→ clear Pitch and Roll
→ apply yaw
```

Player Restore also:

```text
stops jumping
consumes movement input
clears velocity
teleports with TeleportPhysics
restores MOVE_Walking
aligns Controller yaw
```

Invalid ground or an invalid Checkpoint stops Restore before Story mutation.

---

# 8. Visible Soul Rest Points

The original invisible Checkpoint prototype was redesigned into visible, interactable rest points.

Created:

```text
WCCheckpointMenuWidget.h / .cpp
```

Blueprint assets:

```text
/Game/WangChuan/Blueprints/Checkpoint/BP_SoulRestPoint
/Game/WangChuan/UI/Checkpoint/WBP_CheckpointMenu
```

Modified assets include:

```text
BP_PlayerCharacter
Prototype_Map
```

The three native Checkpoint instances were replaced with `BP_SoulRestPoint`.

---

## Player-Facing Role

The final rest point provides:

```text
Manual Save
Resume Location
Same-Map Fast Travel
```

It does not currently:

```text
restore Health
refill resources
respawn enemies
reset puzzles
open upgrades
open equipment
```

---

## Visual Presentation

`BP_SoulRestPoint` uses visible Mesh, light, glow, and first-unlock feedback.

Presentation is derived from:

```text
UnlockedCheckpointIDs.Contains(CheckpointID)
```

and is not serialized as separate light or VFX data.

The three rest points retain stable IDs while display names and visual treatment remain editable.

---

# 9. Manual Save Interaction

`AWCPlayerCheckpoint` now implements `IInteractable`.

Final interaction flow:

```text
enter range
→ Prompt appears
→ press E
→ validate player and World stability
→ request Coordinator SaveAtCheckpoint()
→ unlock and save
→ update presentation
→ open rest-point menu
```

Overlap itself does not:

```text
unlock
save
change CurrentCheckpointID
open UI
move player
```

This removes hidden save behavior and makes saving an explicit player decision.

---

## Use Restrictions

A rest point cannot be used while the player is:

```text
dead
attacking
in combat
locked on
falling
in dialogue
viewing a Memory Echo
reading the Journal
already in the rest-point menu
```

---

## Transactional Save

`SaveAtCheckpoint()` applies candidate runtime Checkpoint state, then attempts a complete World capture.

```text
candidate Current / Unlocked state
→ Capture and Save
→ keep on success
→ roll back on failure
```

If Quiet Child is relocating or another validation fails:

```text
runtime Checkpoint state rolls back
presentation rolls back
menu does not open
old disk save remains unchanged
```

No Auto Save was added.

---

# 10. Unlocked Rest Points and Save Version 2

Runtime player state:

```text
CurrentCheckpointID
UnlockedCheckpointIDs
```

Persistent SaveGame state:

```text
CurrentCheckpointID
UnlockedCheckpointIDs
```

Rules:

- Unlocked list must not be empty.
- IDs must be non-`None` and unique.
- Every ID must exist in the current map.
- Current Checkpoint must be included in the unlocked list.

New Game initializes Start as the runtime current/unlocked point but does not write a disk save until the player uses the rest point.

---

# 11. Checkpoint Menu

`WBP_CheckpointMenu` displays ordered destination options:

```text
CheckpointID
DisplayName
Unlocked
Current
TravelOrder
```

Destination states:

```text
Undiscovered
→ disabled

Current
→ disabled

Unlocked non-current
→ Travel enabled
```

Opening the menu:

```text
stops movement
hides interaction Prompt
shows cursor
uses Game and UI input
ignores movement and look
```

Closing restores input and the nearby Prompt when appropriate. Player death also closes the menu.

---

# 12. Same-Map Fast Travel

Fast Travel is allowed only to an unlocked, non-current Checkpoint in the same map.

Flow:

```text
validate source and target
→ build safe transforms
→ close menu and clear old interaction
→ move player to target
→ update CurrentCheckpointID
→ explicitly save
→ refresh target interaction
```

Travel does not:

```text
reload the map
restore Story Actors again
respawn the enemy
reset lanterns
change Journal progress
```

If saving after travel fails, the player and current Checkpoint return to the source point.

The post-travel save is considered manual because it results from an explicit player-selected Travel action.

---

# 13. Final Rest-Point Configuration

## Start

```text
ID:
WangChuan.Checkpoint.Start

Default:
true

TravelOrder:
0
```

## After Encounter 01

```text
ID:
WangChuan.Checkpoint.AfterEncounter01

Default:
false

TravelOrder:
1
```

## After Encounter 02

```text
ID:
WangChuan.Checkpoint.AfterEncounter02

Default:
false

TravelOrder:
2
```

Exactly one Default Checkpoint is required.

---

# 14. Verified Story States

## State A — New Game / Initial Save

```text
Quiet Child:
Stage 0 / Anchor01

Objectives:
Incomplete

Encounters:
Incomplete

Journal:
Empty

Rest Points:
Start unlocked/current
```

Verified:

- No-save startup is safe.
- First E interaction creates the Version 2 save.
- Player resumes safely at Start.

---

## State B — Encounter 01 Completed

```text
Quiet Child:
Stage 1 / Anchor02

Enemy Objective:
Completed

Encounter 01:
Completed

Bell Echo:
Activated and recorded

Lantern Objective / Encounter 02:
Incomplete

Rest Points:
Start
AfterEncounter01
```

Verified:

- Required enemy stays defeated after reload.
- No death audio or Delegate replays.
- Stage 1 dialogue works.
- Five-lantern Encounter remains playable.
- Player can travel between unlocked points.

---

## State C — Encounter 02 Completed

```text
Quiet Child:
Stage 2 / Anchor03

Both Objectives:
Completed

Both Encounters:
Completed

Five Lanterns:
Completed presentation

Both Echo Relics:
Activated

Journal:
Echo01 → Echo02

Rest Points:
All three unlocked
```

Verified:

- State C restores across PIE sessions.
- All three rest points restore their visual state.
- Travel does not alter Story progress.
- Exit and reload resumes at the most recently selected travel point.

---

# 15. Testing and Verification

## Build

Passed:

```text
WangChuanEditor
Win64
Development
```

Relevant C++ and Blueprint assets compiled successfully.

---

## Persistence

Passed:

- New Game with no save
- Fixed-slot Save / Load / Delete
- Cross-PIE disk persistence
- Complete snapshot replacement
- Transitional save rejection
- Version 2 validation
- State A / B / C restore
- Continued gameplay after State B restore
- Repeated Save / Load idempotency
- Checkpoint unlock and presentation restore
- Fast Travel persistence
- Invalid ID and invalid-ground rejection
- Save and travel rollback

---

## Side-Effect Safety

Restore did not unexpectedly broadcast:

```text
OnEnemyDefeated
OnObjectiveCompleted
OnEchoActivated
```

Restore did not replay:

```text
Enemy death
Lantern Preview
Echo UI
Journal UI
Story Events
NPC Niagara
NPC Relocating
```

Repeated Restore did not create:

```text
duplicate Journal records
position drift
new timers
repeated Story progress
```

---

## Full Regression

Passed:

### Combat

```text
Light Combo
Heavy Attack
Lock-On
Enemy damage and death
Player damage and death
```

### Story

```text
Stage 0 dialogue
Encounter 01
Bell Echo
Stage 1
Five-lantern puzzle
Lantern Echo
Stage 2
```

### UI

```text
Dialogue
Memory Echo
Journal
Checkpoint Menu
Cursor and input cleanup
Player death menu cleanup
```

No remaining project-related:

```text
Accessed None
Blueprint Runtime Error
array bounds error
Timer error
Delegate Ensure
unexpected Auto Save
Journal duplication
```

was found after final testing.

---

# 16. Problems and Corrections

## Visual Studio IntelliSense False Errors

### Symptom

UnrealBuildTool compiled successfully, while Visual Studio showed many false errors in engine/template headers.

### Resolution

- Closed Visual Studio
- Reset `.vs` cache
- Regenerated UE5.8 project files
- Added minimal `WCSaveTypes.cpp`
- Rebuilt through UnrealBuildTool
- Used Build-only Error List filtering

Final C++ build remained clean.

---

## Malformed Save Summary Code

A copied summary block contained duplicated invalid text.

The malformed fragment was manually identified, corrected, compiled, and re-tested.

This reinforced the workflow:

```text
generated suggestion
→ manual syntax review
→ compile
→ runtime validation
→ acceptance
```

---

## `AddDynamic` Callback Binding Failure

### Cause

The callback pointer was split after `::`:

```cpp
&AWCPlayerCheckpoint::
    HandleActivationBoxBeginOverlap
```

`AddDynamic` uses macro stringification/reflection, and the line break introduced whitespace into the generated function name, causing runtime binding failure.

### Rule

Keep the full reflected member function pointer contiguous:

```cpp
&AWCPlayerCheckpoint::HandleActivationBoxBeginOverlap
```

The same rule applies to:

```text
AddDynamic
AddUniqueDynamic
BindDynamic
```

Both Checkpoint overlap callbacks passed after correction.

---

# 17. Final Architecture

```text
UWCGameSaveGame
→ serialized persistent facts

UWCGameInstance
→ fixed-slot disk gateway

AWCStoryPersistenceCoordinator
→ World validation
→ capture
→ ordered restore
→ rest-point save
→ Fast Travel

Story Actors
→ own Gameplay state
→ expose silent Restore APIs

AWCCharacter
→ runtime Journal
→ runtime Checkpoint progress
→ player UI/modal state

AWCPlayerCheckpoint / BP_SoulRestPoint
→ visible interaction
→ resume Transform
→ derived presentation
```

The complete final flow is:

```text
Player progresses Story
→ uses visible rest point
→ Coordinator validates stable World
→ Version 2 snapshot saved
→ new PIE initializes
→ SaveGame loads after BeginPlay
→ complete Save/World prevalidation
→ ordered silent Story Restore
→ rest-point presentation restored
→ player resumes last
→ unfinished gameplay remains playable
```

---

# 18. Scope Not Implemented

Week 7 intentionally did not add:

- Auto Save
- Multiple save slots
- Save migration framework
- Main menu New Game / Continue
- Cross-map travel
- Async / cloud save
- Player Health persistence
- Combat Quick Load
- Enemy respawn on rest
- Consumable refill
- Upgrade or equipment menu
- Generic automatic Actor serialization
- Runtime rollback / time-reversal system

---

# Week 7 Schedule Adjustment

Due to the restructuring of the game's development direction, the originally planned:

```text
Week 7 Day 5
Week 7 Day 6
Week 7 Day 7
```

are cancelled.

Week 7 development therefore concludes after Day 4 with the completed persistence, visible rest-point, manual-save, resume, and same-map travel systems.

Development will move directly into **Week 8 on 2026-08-03**.
