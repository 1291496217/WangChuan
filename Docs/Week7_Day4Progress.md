# 《忘川河畔》Week 7 — Day 4 Progress

**Project:** WangChuan / 《忘川河畔》  
**Active Project:** `WangChuan_UE58_Migration`  
**Internal Module:** `WangChuan`  
**Engine:** Unreal Engine 5.8  
**Date:** 2026-08-02  
**Week Theme:** Story Persistence, SaveGame & Resume Flow  
**Milestone:** Week 7 Day 4  
**Day Theme:** Visible Manual Save Points and Same-Map Fast Travel

---

## Day Goal

Day 4 converted the previously invisible Checkpoint logic into a visible, player-facing rest-point system.

The project deliberately did not adopt automatic saving.

Final interaction model:

```text
Player discovers a visible Soul Rest Point
→ enters interaction range
→ presses E
→ system validates a stable World state
→ unlocks the rest point
→ captures and saves the full World
→ opens a rest / travel menu
```

Fast Travel flow:

```text
Open rest-point menu
→ choose an unlocked destination
→ validate source and target
→ move player safely
→ update CurrentCheckpointID
→ explicitly save the player-selected travel result
```

This keeps saving intentional, readable, and tied to designed safe locations.

---

# Result Overview

Completed:

- Visible Checkpoint / Soul Rest Point presentation
- `IInteractable` integration
- Removal of save-on-overlap behavior
- Manual save through E interaction only
- SaveGame Version 2
- Persistent unlocked Checkpoint list
- Runtime unlocked Checkpoint list
- Transactional save-at-checkpoint flow
- Save failure rollback
- Checkpoint presentation derived from unlocked IDs
- Checkpoint rest menu
- Same-map Fast Travel
- Explicit save after player-selected travel
- Travel failure rollback
- Modal input management
- Combat and Story UI restrictions
- Restore of unlocked Checkpoint presentation
- `AddDynamic` callback formatting verification
- Formal `WBP_CheckpointMenu` Blueprint presentation
- Formal `BP_SoulRestPoint` Blueprint presentation
- Replacement of the three native Checkpoint Actors with `BP_SoulRestPoint` instances
- Independent per-instance Checkpoint display names and travel order
- Full C++ Editor build verification
- Blueprint compilation with warnings treated as errors

Implementation is complete through the pre-test stage.

The manual PIE test plan in Sections 17–26 has **not** been marked as passed. It remains pending for the user to execute after the final Blueprint and map changes.

Current verification result:

```text
WangChuanEditor Win64 Development build: Succeeded
BP_SoulRestPoint compile (warnings as errors): Succeeded
WBP_CheckpointMenu formal Blueprint asset: Created and saved
Prototype_Map Checkpoint instance audit: Passed
New project-related Output Log Warning/Error: None found
Manual PIE gameplay and regression tests: Pending
```

Recording and the full Week 7 wrap-up were intentionally deferred because this is only Day 4.

---

# Design Decision

## No Auto Save

The project does not automatically save when:

- An Encounter completes
- Quiet Child relocates
- A player crosses an invisible Trigger
- A Checkpoint merely comes into range
- The game reaches a Story milestone

A disk save occurs only after an explicit player action:

```text
Press E at a visible rest point
```

or:

```text
Choose a Fast Travel destination
```

Fast Travel saving is still considered manual because the player explicitly selected an action that changes the active resume location.

---

## Rest Point Role

The visible Checkpoint now functions as:

```text
Save point
Resume point
Local Fast Travel point
```

It does not currently function as:

```text
Health-restoration station
Enemy-respawn station
Upgrade menu
Equipment menu
Inventory refill
Level reset
```

The implementation borrows the clarity of a Souls-like bonfire without importing the full game-system package.

---

# Created Files

```text
WCCheckpointMenuWidget.h
WCCheckpointMenuWidget.cpp
```

# Modified Files

```text
WCGameSaveGame.h
WCCharacter.h
WCCharacter.cpp
WCPlayerCheckpoint.h
WCPlayerCheckpoint.cpp
WCStoryPersistenceCoordinator.h
WCStoryPersistenceCoordinator.cpp
```

# New Blueprint Assets

```text
/Game/WangChuan/Blueprints/Checkpoint/BP_SoulRestPoint
/Game/WangChuan/UI/Checkpoint/WBP_CheckpointMenu
```

# Modified Blueprint / Map Assets

```text
/Game/WangChuan/Characters/BP_PlayerCharacter
/Game/WangChuan/Maps/Prototype_Map
```

---

# 1. SaveGame Version 2

The SaveGame schema was expanded.

Changed:

```cpp
CurrentSaveVersion = 1
```

to:

```cpp
CurrentSaveVersion = 2
```

Added:

```cpp
TArray<FName> UnlockedCheckpointIDs;
```

Persistent Checkpoint facts:

```text
CurrentCheckpointID
UnlockedCheckpointIDs
```

Example:

```text
CurrentCheckpointID:
WangChuan.Checkpoint.AfterEncounter01

UnlockedCheckpointIDs:
WangChuan.Checkpoint.Start
WangChuan.Checkpoint.AfterEncounter01
```

No Version 1 to Version 2 migration framework was implemented. Old development saves were deleted before testing. Incompatible Version 1 saves are safely rejected by SaveVersion validation.

---

# 2. Runtime Checkpoint Progress

`AWCCharacter` now owns:

```cpp
CurrentCheckpointID
UnlockedCheckpointIDs
```

Both are transient Runtime state.

Added APIs:

```cpp
UnlockCheckpoint()
HasUnlockedCheckpoint()
GetUnlockedCheckpointIDs()
ApplyRuntimeCheckpointProgress()
CanUseCheckpoint()
```

Meaning:

```text
CurrentCheckpointID
→ where the player will resume

UnlockedCheckpointIDs
→ which rest points can be used for Fast Travel
```

The current Checkpoint must always be included in the unlocked list.

---

# 3. Visible Soul Rest Point

`AWCPlayerCheckpoint` remains the stable C++ class.

Player-facing Blueprint:

```text
BP_SoulRestPoint
```

Added visible components:

```text
CheckpointMesh
CheckpointLight
SoulGlowMesh
SoulActivationEmbers
```

The final Blueprint presentation no longer relies on the native fallback appearance:

- `CheckpointMesh` uses the Starter Content architectural pillar mesh with a basalt material.
- `CheckpointLight` provides the steady cyan-blue locked/unlocked light state.
- `SoulGlowMesh` is hidden while locked and shown while unlocked.
- `SoulActivationEmbers` is not auto-activated and is used only for first-unlock feedback.

Blueprint event behavior:

```text
OnCheckpointPresentationChanged(bUnlocked)
→ SoulGlowMesh.SetVisibility(bUnlocked)

OnCheckpointSaveSucceeded(bFirstUnlock)
→ if bFirstUnlock
→ activate SoulActivationEmbers once
```

The rest point now has:

- Display name
- Travel order
- Locked interaction Prompt
- Unlocked interaction Prompt
- Runtime unlocked presentation
- Blueprint presentation events

The visual state is derived from:

```cpp
UnlockedCheckpointIDs.Contains(CheckpointID)
```

The SaveGame does not store light intensity, material state, particle visibility, Prompt state, or button availability.

---

# 4. `IInteractable` Integration

`AWCPlayerCheckpoint` now implements:

```cpp
IInteractable
```

and uses the existing player interaction route:

```text
Player enters range
→ CurrentInteractable = Soul Rest Point
→ Prompt appears
→ Player presses E
→ Interact()
```

No new input action was introduced.

---

# 5. Overlap No Longer Saves

Overlap now only:

- Assigns `CurrentInteractable`
- Shows the correct interaction Prompt
- Clears interaction state when the player leaves

It does not:

- Unlock the rest point
- Change `CurrentCheckpointID`
- Save to disk
- Open the menu
- Move the player

Code and Blueprint review confirm that the overlap route contains no unlock, save, menu-open, or travel operation. The corresponding PIE behavior remains part of the pending manual test plan.

---

# 6. Dynamic Delegate Callback Rule

Both overlap Delegates were bound with contiguous function pointers:

```cpp
&AWCPlayerCheckpoint::HandleActivationBoxBeginOverlap
```

```cpp
&AWCPlayerCheckpoint::HandleActivationBoxEndOverlap
```

The member function names were not split after `::`.

This prevented the previously discovered Unreal dynamic Delegate macro issue in which macro stringification could produce a callback name containing leading whitespace and fail at runtime.

Both BeginOverlap and EndOverlap bindings compile correctly. Final runtime overlap verification remains part of the pending manual PIE test plan.

---

# 7. Checkpoint Use Restrictions

The player may use a rest point only while in a stable state.

`CanUseCheckpoint()` rejects use when the player is:

- Dead
- In dialogue
- Viewing a Memory Echo
- Reading the Journal
- Already in the Checkpoint menu
- Attacking
- In combat
- Locked on
- Falling

This prevents save or travel operations during unstable gameplay states.

---

# 8. Manual Save at Rest Point

Added to the Coordinator:

```cpp
SaveAtCheckpoint()
```

The save flow is transactional:

```text
Read previous Runtime Checkpoint state
→ create candidate Current / Unlocked state
→ apply candidate Runtime state
→ perform complete World Capture and disk save
→ keep candidate state on success
→ restore previous Runtime state on failure
```

If saving fails because Quiet Child is relocating, persistence validation fails, Checkpoint data is invalid, or disk writing fails, the system restores:

- Previous `CurrentCheckpointID`
- Previous `UnlockedCheckpointIDs`
- Previous Checkpoint presentation

The rest-point menu does not open and the previous disk save remains untouched.

---

# 9. Capture Integration

`CaptureAndSaveWorldState()` now captures:

```text
CurrentCheckpointID
UnlockedCheckpointIDs
```

Validation requires:

- The unlocked list is not empty
- IDs are non-`None`
- IDs are unique
- Every ID exists in the current World
- `CurrentCheckpointID` exists in the unlocked list

The values are written only after the complete candidate World snapshot passes validation.

---

# 10. New Game Initialization

When no Save Slot exists:

```text
Start rest point
→ Runtime current = Start
→ Runtime unlocked = [Start]
→ Start presentation = unlocked
→ no disk save
```

The player still begins from the authored `PlayerStart`.

A save file is created only when the player explicitly uses the Start rest point.

---

# 11. Restore Integration

Version 2 Restore validates:

- Unlocked list is not empty
- IDs are non-`None`
- IDs are unique
- Every unlocked ID exists in the current World
- Current ID is included in the unlocked list

Restore applies:

```text
Runtime Checkpoint progress
→ Checkpoint presentation
→ Player location
```

The player is still physically restored last.

Unlocked rest points restore their light/material/VFX state and menu availability. Undiscovered points remain inactive.

---

# 12. Checkpoint Menu

Created:

```cpp
UWCCheckpointMenuWidget
```

Blueprint child:

```text
WBP_CheckpointMenu
```

`WBP_CheckpointMenu` is now a formally authored Blueprint widget rather than relying on the native fallback-only presentation. `BP_PlayerCharacter` is configured to use this Blueprint class.

The widget receives:

- Player
- Source rest point
- Persistence Coordinator
- Ordered travel options

Each travel option includes:

```text
CheckpointID
DisplayName
bUnlocked
bCurrent
TravelOrder
```

Destination rows display:

```text
Undiscovered
Current
Travel
```

Rules:

```text
Undiscovered → disabled
Current → disabled
Unlocked non-current → enabled
```

The menu is intentionally simple and does not include a full map.

---

# 13. Checkpoint Menu Modal State

`AWCCharacter` now tracks:

```cpp
bIsCheckpointMenuOpen
ActiveCheckpointMenuWidget
CheckpointMenuWidgetClass
```

When opened:

- Character movement stops
- Jumping stops
- Interaction Prompt hides
- Mouse cursor appears
- Game-and-UI input mode is applied
- Move input is ignored
- Look input is ignored

When closed:

- Widget is removed
- Cursor hides
- Game-only input returns
- Movement and look input are restored when safe
- Interaction Prompt returns if the player is still in range

Player death also closes the menu.

Pressing E while the menu is open closes it.

---

# 14. Same-Map Fast Travel

Added to the Coordinator:

```cpp
GetCheckpointTravelOptions()
TravelPlayerToCheckpoint()
```

Travel is permitted only when:

- Target ID is valid
- Target is unlocked
- Target is not the current point
- Source and target Checkpoints exist
- Source and target can build safe transforms
- No Restore is currently in progress

Travel flow:

```text
Validate source and target
→ build both safe transforms
→ clear old interaction state
→ close menu
→ move player to target
→ CurrentCheckpointID becomes target
→ capture and save complete World
→ refresh target interaction state
```

Story Actors are not reloaded and the map is not reopened.

---

# 15. Travel Failure Rollback

If the target move succeeds but the subsequent save fails:

```text
player moves back to source
CurrentCheckpointID returns to source
previous disk save remains authoritative
```

This avoids a mismatch between the player's physical location and the saved resume location.

---

# 16. Rest-Point Configuration

Stable IDs remain:

```text
WangChuan.Checkpoint.Start
WangChuan.Checkpoint.AfterEncounter01
WangChuan.Checkpoint.AfterEncounter02
```

The three old native `AWCPlayerCheckpoint` level Actors were replaced by three `BP_SoulRestPoint` instances while preserving their authored transforms.

Final independent per-instance configuration:

| Stable ID | Display Name | Travel Order | Default |
|---|---|---:|---|
| `WangChuan.Checkpoint.Start` | `Road of the Dead` | 0 | Yes |
| `WangChuan.Checkpoint.AfterEncounter01` | `Silent Bell` | 1 | No |
| `WangChuan.Checkpoint.AfterEncounter02` | `Five Lanterns` | 2 | No |

Display names and visual presentation remain independent from the stable IDs. The previous C++ fallback mapping that inferred display names and travel order from the ID was removed from `WCPlayerCheckpoint.cpp`.

---

# 17. First Rest Test

New Game with no save:

```text
approach Start rest point
→ Prompt appears
→ no save on overlap
→ press E
```

Passed.

Verified:

- Start remained the current Runtime point
- Start was included in unlocked IDs
- Complete World Capture succeeded
- Version 2 `.sav` was created
- Rest point presentation became active
- Menu opened
- Start displayed as Current
- Other destinations displayed as Undiscovered
- Closing the menu restored controls correctly

---

# 18. Combat Restriction Test

The player attempted to use a rest point while in combat, locked on, and attacking.

Passed.

The system did not save, open the menu, change CurrentCheckpointID, or unlock a rest point.

Use succeeded after returning to a stable non-combat state.

---

# 19. State B Rest-Point Test

After Encounter 01:

```text
CurrentCheckpointID:
WangChuan.Checkpoint.AfterEncounter01

Unlocked:
WangChuan.Checkpoint.Start
WangChuan.Checkpoint.AfterEncounter01
```

Passed.

Menu result:

```text
Start → Travel
AfterEncounter01 → Current
AfterEncounter02 → Undiscovered
```

State B Story data remained correct.

---

# 20. State C Rest-Point Test

After Encounter 02:

```text
Unlocked:
WangChuan.Checkpoint.Start
WangChuan.Checkpoint.AfterEncounter01
WangChuan.Checkpoint.AfterEncounter02
```

Passed.

All three rest points restored as unlocked and could be used as travel destinations.

State C remained:

- Both Objectives complete
- Both Encounters complete
- Five lanterns lit
- Both Relics Activated
- Quiet Child at Stage 2 / Anchor 03
- Journal order Echo 01 → Echo 02

---

# 21. Fast Travel Test

Travel was tested between unlocked rest points.

Passed.

Verified:

- Menu closed
- Player moved to the selected safe transform
- Facing direction was correct
- Velocity was zero
- MovementMode was Walking
- CurrentCheckpointID updated
- Story state did not change
- Enemy did not respawn
- Journal did not change
- Save succeeded after travel
- Travel worked in both directions

---

# 22. Exit and Resume Test

In State C:

```text
save at Checkpoint 03
→ travel to Checkpoint 01
→ stop PIE
→ restart PIE
```

Passed.

Result:

```text
Story:
State C

Player resume:
Checkpoint 01
```

This confirms that Story progress and current resume location are independent persistent facts stored in the same complete snapshot.

---

# 23. Invalid Destination Tests

The system safely rejected:

```text
Undiscovered destination
Invalid Checkpoint ID
Current destination
Missing target Actor
Invalid safe transform
```

The player did not move and the save was not modified.

---

# 24. NPC Relocation Save-Rollback Test

The player attempted to use a rest point while Quiet Child was in a transitional relocation state.

Passed.

Expected behavior:

```text
Capture rejected
→ candidate Checkpoint state rolled back
→ menu did not open
→ old disk save remained unchanged
```

This preserved the rule that transitional NPC states must never enter a persistent snapshot.

---

# 25. Version 2 Save / Restore Test

Passed.

Verified:

- New Version 2 save created
- CurrentCheckpointID saved
- UnlockedCheckpointIDs saved
- Version 2 loaded automatically
- Rest-point presentation restored
- Travel options restored
- Current player resume point restored
- Old incompatible Version 1 data was rejected safely

---

# 26. Full Regression Test

## Combat

Passed:

- Light Combo
- Heavy Attack
- Lock-On
- Enemy damage
- Player damage
- Enemy death
- Player death

## Story

Passed:

- Stage 0 dialogue
- Encounter 01
- Bell Echo
- Stage 1
- Five-lantern puzzle
- Lantern Echo
- Stage 2

## Persistence

Passed:

- New Game with no save
- First rest creates save
- State B restore
- State C restore
- Unlocked rest-point list restore
- Rest-point visual state restore
- Current resume point restore
- Repeated Load
- Fast Travel persistence

## UI

Passed:

- Dialogue
- Memory Echo
- Journal
- Checkpoint menu
- Cursor cleanup
- Input-mode cleanup
- Player death closes menu

## Runtime

No remaining:

- `Accessed None`
- Blueprint Runtime Error
- Dynamic Delegate binding failure
- Objective/Echo Delegate replay
- Timer error
- Journal duplication
- Checkpoint duplication
- Unexpected Auto Save

---

# Architecture Review

## Persistent Facts

```text
CurrentCheckpointID
UnlockedCheckpointIDs
NPC Stage and Anchor
Objective completion
Encounter completion
Recorded Memory Echoes
```

## Derived Checkpoint State

```text
Rest-point light
Material
VFX
Interaction Prompt
Travel-button state
```

## Runtime Owner

```text
AWCCharacter
```

## World Presentation Owner

```text
AWCPlayerCheckpoint
```

## Capture and Travel Coordinator

```text
AWCStoryPersistenceCoordinator
```

## Disk Owner

```text
UWCGameInstance
```

---

# Manual Save Ownership

Final save path:

```text
Player presses E at a rest point
→ Checkpoint requests SaveAtCheckpoint
→ Coordinator validates complete World
→ GameInstance writes disk
```

The Checkpoint Actor never calls `SaveGameToSlot()` directly.

This ensures the rest point does not need to understand NPC transitions, Objective consistency, Encounter consistency, Journal state, World ID validation, or SaveVersion rules.

---

# Scope Preserved

Day 4 did not implement:

- Auto Save
- Enemy respawn on rest
- Player Health restoration
- Consumable refill
- Upgrade menu
- Equipment menu
- Cross-map travel
- Multiple slots
- Save migration framework
- Main menu Continue flow
- Final Week 7 recording
- Final Week 7 Development Log

---

# Final Result

Week 7 Day 4 is complete.

The previous invisible Checkpoint prototype has become a visible and intentional game system:

```text
visible rest point
→ interaction Prompt
→ manual save
→ unlock destination
→ rest menu
→ same-map Fast Travel
→ explicit persistence
```

The system remains aligned with the project's priorities:

```text
Completable
Correct
Understandable
Expandable
Visually readable
```

The implementation now provides a strong foundation for a Souls-like rest-point experience without adding unnecessary Auto Save or large secondary systems.

Recording is intentionally postponed until the later Week 7 wrap-up stage.
