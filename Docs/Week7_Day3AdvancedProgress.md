# 《忘川河畔》Week 7 — Day 3 Advanced Progress

**Project:** WangChuan / 《忘川河畔》  
**Active Project:** `WangChuan_UE58_Migration`  
**Internal Module:** `WangChuan`  
**Engine:** Unreal Engine 5.8  
**Date:** 2026-08-01  
**Week Theme:** Story Persistence, SaveGame & Resume Flow  
**Milestone:** Week 7 Day 3 Advanced / Original Day 6  
**Day Theme:** Player Checkpoint and Resume Position

---

## Day Goal

Day 3 Advanced extended the completed Story persistence pipeline with a stable player resume-location system.

Completed flow:

```text
Player activates stable Checkpoint
→ AWCCharacter stores Runtime CheckpointID
→ Capture writes CurrentCheckpointID
→ SaveGame is written to disk
→ New PIE restores Story World
→ Player is restored last to a safe Checkpoint Transform
```

The SaveGame stores a semantic Checkpoint ID rather than a raw player Transform.

---

# Result Overview

Completed:

- `AWCPlayerCheckpoint`
- Stable Checkpoint IDs
- Exactly-one-default Checkpoint validation
- Runtime Checkpoint ownership on `AWCCharacter`
- Checkpoint activation without automatic disk save
- Safe ground Trace and Capsule-height placement
- Player yaw, velocity, and MovementMode restoration
- Checkpoint inclusion in World ID validation and Actor maps
- Default Runtime Checkpoint initialization for New Game
- Checkpoint capture into `CurrentCheckpointID`
- Save rejection for missing or invalid Checkpoints
- Prevalidation of Checkpoint and resume Transform
- Player restoration as the final Restore phase
- State A, State B, and State C resume tests
- Invalid ID and invalid-ground tests
- Repeated Load idempotency tests
- Full manual review

All tests passed.

---

# Files

## Created

```text
WCPlayerCheckpoint.h
WCPlayerCheckpoint.cpp
```

## Modified

```text
WCCharacter.h
WCCharacter.cpp
WCStoryPersistenceCoordinator.h
WCStoryPersistenceCoordinator.cpp
```

No SaveGame schema or `Build.cs` change was required because `CurrentCheckpointID` already existed.

---

# 1. `AWCPlayerCheckpoint`

Created:

```cpp
AWCPlayerCheckpoint : public AActor
```

Components:

```text
SceneRoot
ActivationBox
ResumeArrow
```

Responsibilities:

- Own a stable `CheckpointID`
- Mark the map's default Checkpoint
- Detect player overlap
- Validate a safe resume point
- Update the player's Runtime Checkpoint
- Build a resume Transform

It does not save to disk, capture Story state, or automatically load.

---

# 2. Stable IDs

Configured IDs:

```text
WangChuan.Checkpoint.Start
WangChuan.Checkpoint.AfterEncounter01
WangChuan.Checkpoint.AfterEncounter02
```

The persistent fact is the ID. The physical location remains current-map presentation data.

This allows Checkpoint locations to be adjusted later without changing the semantic meaning of an existing save.

---

# 3. Default Checkpoint

The map requires exactly one:

```text
bIsDefaultCheckpoint = true
```

Current default:

```text
WangChuan.Checkpoint.Start
```

Validation now checks:

- At least one Checkpoint exists
- IDs are non-`None`
- IDs are unique
- Exactly one default exists

---

# 4. Runtime and Persistent Ownership

Runtime owner:

```cpp
AWCCharacter::CurrentCheckpointID
```

Persistent owner:

```cpp
UWCGameSaveGame::CurrentCheckpointID
```

Meaning:

```text
Runtime ID
→ most recently activated Checkpoint in this Session

SaveGame ID
→ Checkpoint active at the last successful save
```

---

# 5. Activation Does Not Save

Overlap flow:

```text
validate Checkpoint
→ validate resume Transform
→ update Player Runtime Checkpoint
→ show optional feedback
```

It does not call:

```cpp
CaptureAndSaveWorldState()
SaveCurrentGame()
SaveGameToSlot()
```

Therefore:

```text
Checkpoint Activated
≠
Game Saved
```

Disk ownership remains in `UWCGameInstance`, while capture coordination remains in `AWCStoryPersistenceCoordinator`.

---

# 6. Safe Resume Transform

`ResumeArrow` represents the intended player feet location and yaw.

The Checkpoint:

```text
traces to ground
→ reads scaled Capsule Half Height
→ places Character center above ground
→ adds clearance
→ keeps yaw
→ removes Pitch and Roll
```

Activation and Restore fail safely when no valid ground is found.

---

# 7. Player Restore API

Added to `AWCCharacter`:

```cpp
SetCurrentCheckpointID()
GetCurrentCheckpointID()
ApplySavedCheckpointState()
```

`ApplySavedCheckpointState()`:

- Stops jumping
- Consumes movement input
- Stops movement before teleport
- Teleports with `TeleportPhysics`
- Stops movement again
- Restores `MOVE_Walking`
- Aligns Controller yaw
- Updates Runtime Checkpoint ID

It does not restore Health, revive the player, restore attacks, restore Lock-On, or act as an in-combat Quick Load.

---

# 8. Capture Integration

`CaptureAndSaveWorldState()` now reads:

```cpp
Player->GetCurrentCheckpointID()
```

Capture is rejected when:

- Runtime ID is `None`
- The ID does not exist in the current World
- Checkpoint validation fails

The valid value is written to:

```cpp
SaveData->CurrentCheckpointID
```

The previous forced `NAME_None` assignment was removed.

---

# 9. Restore Prevalidation

Before modifying any Story Actor, the Coordinator validates:

```text
CurrentCheckpointID is non-None
Checkpoint exists in the current World
Checkpoint Actor is valid
safe resume Transform can be built
```

The resume Transform is prepared before Objective restoration starts.

An invalid Checkpoint therefore cannot produce a partially restored World.

---

# 10. Final Restore Order

Final order:

```text
1. Objectives
2. Encounters
3. Echo Relics
4. Story NPCs
5. Memory Journal
6. Player Checkpoint
```

The player is restored last because moving the player may evaluate overlaps, Prompts, Puzzle Triggers, Relic ranges, or NPC interaction ranges.

The World must already be in its final restored state before the player enters it.

---

# 11. Test Results

## State A

```text
Story State A
Checkpoint Start
```

Passed:

- Player resumed at Start
- Yaw matched ResumeArrow
- Velocity was zero
- MovementMode was Walking
- Player did not float or clip into ground

## State B

```text
Encounter 01 completed
Checkpoint AfterEncounter01
```

Passed:

- Required Enemy remained defeated
- Quiet Child restored to Stage 1 / Anchor 02
- Bell Echo remained Activated
- Journal contained Echo 01
- Player resumed at Checkpoint 02
- Encounter 02 remained playable

## State C

```text
Encounter 01 and 02 completed
Checkpoint AfterEncounter02
```

Passed:

- Five lanterns remained completed
- Both Relics remained Activated
- Quiet Child restored to Stage 2 / Anchor 03
- Journal order remained Echo 01 → Echo 02
- Player resumed at Checkpoint 03

## Invalid Tests

Passed:

- Invalid Checkpoint ID stopped Restore before World mutation
- Invalid ground stopped Restore before World mutation
- Repeated Load produced no position drift or duplicate Story state

---

# 12. `AddDynamic` Callback Binding Issue

## Symptom

The Checkpoint overlap callback compiled but failed to bind at runtime.

Problematic formatting split the callback after `::`:

```cpp
&AWCPlayerCheckpoint::
    HandleActivationBoxBeginOverlap
```

## Cause

`AddDynamic` uses Unreal macro stringification and reflection for the callback name.

The line break and indentation after `::` caused the generated function-name string to include leading whitespace, so runtime reflection could not find the intended callback.

## Fix

The full member function pointer was kept contiguous:

```cpp
ActivationBox->OnComponentBeginOverlap.AddDynamic(
    this,
    &AWCPlayerCheckpoint::HandleActivationBoxBeginOverlap
);
```

## Rule

For reflected Unreal dynamic Delegate macros, do not split:

```cpp
&ClassName::FunctionName
```

between `::` and the function name.

This rule also applies when using macros such as:

```text
AddDynamic
AddUniqueDynamic
BindDynamic
```

The corrected binding passed all overlap tests.

---

# 13. Current Checkpoint Usability Limitation

The current version does not require the player to enter a later Checkpoint in order to save at all.

New Game initializes:

```text
WangChuan.Checkpoint.Start
```

so manual save always has at least one valid resume ID.

However, advancing the saved resume location depends on activating a later Checkpoint.

Example:

```text
Player completes Encounter 01
→ misses Checkpoint 02
→ manually saves
→ Story State B is saved
→ CurrentCheckpointID is still Start
→ reload restores State B but places player at Start
```

Therefore, the system is not dependent on Checkpoint overlap for saving, but it is dependent on overlap for updating the resume location.

---

# 14. Invisible Checkpoint Problem

A purely invisible Trigger can be missed because the player may:

- Walk around its edge
- Use an alternate route
- Turn back before crossing it
- Receive no clear confirmation that resume progress changed

This is a legitimate player-experience problem.

A technically valid invisible volume is not automatically a readable game system.

---

# 15. Recommended Demo Solution

Use a hybrid approach.

## Mandatory Route Volumes

Place ActivationBoxes across unavoidable traversal spaces:

```text
doorways
bridges
narrow exits
post-Encounter main-path chokepoints
```

The boxes should be wider than the playable route.

## Subtle Feedback

On activation, provide restrained feedback:

```text
brief “Memory Anchored” text
short soft sound
small visual pulse
temporary checkpoint/save icon
```

## Story-Driven Activation

For major milestones, update the Runtime Checkpoint automatically when the Story reaches a stable state:

```text
Encounter 01 completed
+ Quiet Child relocation finished
→ activate AfterEncounter01
```

```text
Encounter 02 completed
+ Quiet Child relocation finished
→ activate AfterEncounter02
```

The Checkpoint Actor still provides the resume Transform. Only activation becomes reliable and Story-driven.

## Optional Auto Save

The next phase may trigger a Coordinator save after a stable Story/Checkpoint transition.

It must not save while Quiet Child is still `EventResolved` or `Relocating`.

---

# Recommended Final Rule

```text
Start:
default Runtime Checkpoint

Encounter 01 stable completion:
automatically activate Checkpoint 02

Encounter 02 stable completion:
automatically activate Checkpoint 03

Physical Trigger:
backup route activation

Player feedback:
brief and visible

Disk Save:
Coordinator-owned
```

This preserves the architecture and prevents hidden Checkpoints from being silently missed.

---

# Build and Runtime Verification

Passed:

```text
WangChuanEditor
Win64
Development
```

No new:

- C++ compile error
- Blueprint Runtime Error
- `Accessed None`
- array error
- timer error
- duplicate ID error
- ground Trace error
- Delegate/VFX/UI side effect

remained after the callback correction and manual review.

---

# Final Result

Week 7 Day 3 Advanced / Original Day 6 is complete.

The persistence pipeline now includes safe player resume position:

```text
Story progress
+ Runtime Checkpoint
→ complete snapshot
→ disk save
→ new PIE
→ ordered Story Restore
→ Journal Restore
→ player restored last
```

Supported states:

```text
State A → Start
State B → AfterEncounter01
State C → AfterEncounter02
```

The next milestone is:

```text
Week 7 Day 4 / Original Day 7
→ stable Auto Save triggers
→ Story-driven Checkpoint activation
→ Checkpoint feedback
→ regression testing
→ Week 7 wrap-up
```
