# 《忘川河畔》Week 7 — Day 1 Progress

**Project:** WangChuan / 《忘川河畔》  
**Active Project:** `WangChuan_UE58_Migration`  
**Internal Module:** `WangChuan`  
**Engine:** Unreal Engine 5.8  
**Date:** 2026-07-30  
**Week Theme:** Story Persistence, SaveGame & Resume Flow  
**Day 1 Theme:** Persistence Audit and Stable IDs

---

## Day 1 Goal

Before creating a `USaveGame` class or writing data to disk, Day 1 focused on identifying the stable story facts that should persist across sessions and establishing reliable semantic IDs for the existing narrative systems.

The main goals were:

- Audit the ownership and lifetime of the current Story state
- Separate Runtime, Persistent, Transitional, and Derived state
- Confirm the future SaveGame source-of-truth fields
- Add a stable ID to `AWCStoryNPC`
- Expose safe read-only ID accessors for existing Story Actors
- Verify that all current Story IDs are non-empty and unique within their own type
- Preserve all existing Week 4–6 gameplay behavior
- Establish the Week 7 Save Slot and Save Version conventions without writing to disk yet

Day 1 intentionally did **not** create:

- `UWCGameSaveGame`
- `UWCGameInstance`
- `AWCStoryPersistenceCoordinator`
- Disk Save / Load operations
- Restore APIs
- Checkpoint Actors
- Save or Load UI

---

## Confirmed Starting Condition

The project currently has no custom `UGameInstance` class.

This allows Week 7 to introduce a new, purpose-built class later:

```cpp
UWCGameInstance : public UGameInstance
```

No compatibility work or migration from an older custom GameInstance is required.

---

# Completed Code Changes

## 1. `AWCStoryNPC` Stable Identity

Added a stable semantic ID field:

```cpp
FName StoryNPCID = NAME_None;
```

Added a read-only accessor:

```cpp
FName GetStoryNPCID() const;
```

The ID is intended for SaveGame and world-state restoration matching.

It is separate from:

```text
NPCDisplayName
Actor Name
Actor Label
Actor Pointer
```

The formal Quiet Child ID was configured as:

```text
QuietChild
```

This value identifies the narrative entity and should remain unchanged even if the Blueprint name, Actor Label, displayed character name, Mesh, localization, or level placement changes later.

---

## 2. `AStoryObjectiveBase` Identity Access

The existing `ObjectiveID` was preserved.

Added a public read-only accessor:

```cpp
FName GetObjectiveID() const;
```

This allows the future Persistence Coordinator to collect Objective state without exposing internal Objective fields or relying on Actor names.

Final Objective IDs:

```text
QuietChild.EnemyDefeat01
QuietChild.LanternPuzzle01
```

---

## 3. `AStoryEncounter` Identity Access and Safe Default

The existing `EncounterID` was preserved.

Added a public read-only accessor:

```cpp
FName GetEncounterID() const;
```

The default value was changed from a text-like placeholder to the real empty `FName` value:

```cpp
NAME_None
```

This ensures that future validation using `IsNone()` correctly identifies an unconfigured Encounter.

Final Encounter IDs:

```text
QuietChild.Encounter01
QuietChild.Encounter02
```

---

## 4. `AEchoRelic` Identity Access

The existing Echo identity remains stored in:

```cpp
MemoryEchoData.EchoID
```

Added a direct read-only accessor:

```cpp
FName GetEchoID() const;
```

This avoids copying the full `FMemoryEchoData` structure when the persistence system only needs the identity key.

Final Echo IDs:

```text
QuietChild.BellEcho01
QuietChild.LanternEcho02
```

---

## 5. `AStoryAnchor` Existing Identity Support Confirmed

No code change was required.

The class already provides:

```cpp
FName GetAnchorID() const;
```

Final Anchor IDs:

```text
QuietChild.Anchor01
QuietChild.Anchor02
QuietChild.Anchor03
```

---

# Final Stable ID Registry

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

Validation result:

- All IDs are non-`None`
- No duplicate ID exists within the same Actor type
- IDs do not depend on Actor Labels
- IDs do not depend on generated Actor Names
- IDs do not depend on runtime Actor Pointers
- IDs describe narrative meaning rather than current presentation

---

# Persistence Audit

## 1. `AWCStoryNPC`

### Persistent Source-of-Truth Data

```text
StoryNPCID
CurrentStoryStage
Current Anchor ID
```

These values answer:

- Which Story NPC is being restored?
- Which stable narrative stage has been reached?
- Which stable story location should the NPC occupy?

### Runtime or Transitional Data — Not Saved

```text
StoryState = Relocating
StoryState = EventResolved
PendingStoryStage
PendingRelocationAnchor Actor Pointer
Relocation timers
Current Niagara playback
Current interaction overlap
Current dialogue line
```

Loading should restore the NPC directly into a stable state:

```text
Saved Anchor
Saved Story Stage
StoryState = Available
```

It should not continue a partially completed relocation animation or timer.

---

## 2. `AStoryObjectiveBase`

### Persistent Source-of-Truth Data

```text
ObjectiveID
bIsCompleted
```

### Runtime Data — Not Saved

```text
bIsActive
```

An incomplete Objective may safely restart from its initial state after loading.

A completed Objective should be restored silently without calling the normal gameplay completion path.

---

## 3. `AEnemyDefeatObjective`

### Persistent Fact

```text
Objective Completed
```

### Derived or Runtime Data — Not Saved Separately

```text
bRequiredEnemyDefeated
RequiredEnemy Actor Pointer
Enemy death animation state
Enemy destruction timer
```

The completed Objective is the stable fact.

The required enemy's restored defeated/removed presentation can later be derived from that fact.

---

## 4. `ALanternSequencePuzzle`

### Persistent Fact

```text
Lantern Objective Completed
```

### Transitional Data — Not Saved

```text
Puzzle Preview Index
Current player input sequence
Reset timer
Preview timer
Current Puzzle State while Previewing or Resetting
Temporary individual lantern feedback
```

Restore rules established for later implementation:

```text
Objective incomplete
→ Puzzle returns to its initial Dormant state
→ Activation Box remains usable
→ Lanterns begin unlit

Objective completed
→ Puzzle restores as Completed
→ All lanterns show completed presentation
→ Lantern interaction remains disabled
→ Activation Box remains disabled
```

The individual lantern states are derived presentation, not separate persistent facts.

---

## 5. `AStoryEncounter`

### Persistent Source-of-Truth Data

```text
EncounterID
bEncounterCompleted
```

### Derived or Configuration Data — Not Saved Separately

```text
bStoryObjectiveCompleted
StoryObjective Actor Pointer
EchoRelic Actor Pointer
StoryNPC Actor Pointer
NextStoryAnchor Actor Pointer
NextStoryStage configuration
CompletionStoryEventID configuration
```

`bStoryObjectiveCompleted` can be derived from the linked Objective's restored completion state.

Level-configured references and progression targets remain part of the map and Blueprint configuration rather than the SaveGame payload.

---

## 6. `AEchoRelic`

### Identity

```text
EchoID
```

### Derived State

The Relic state can be reconstructed from Objective and Encounter facts:

```text
Objective incomplete
→ Relic Locked

Objective complete and Encounter incomplete
→ Relic Available

Encounter complete
→ Relic Activated
```

Therefore, the first SaveGame version does not need to independently save `EEchoRelicState`.

Saving all three states separately could create contradictions, such as:

```text
Objective incomplete
Encounter complete
Relic Locked
```

The persistence design instead saves the core facts and derives the Relic presentation.

### Transitional Data — Not Saved

```text
bActivationInProgress
Current Memory Echo UI page
Current interaction prompt state
```

---

## 7. `AWCCharacter` and Memory Journal

### Persistent Data

```text
RecordedMemoryEchoes
```

The first SaveGame version is expected to store the full `FMemoryEchoData` entries because the project currently has no separate global Echo Data Registry.

This preserves:

- `EchoID`
- Title
- Echo text
- Player resonance text
- Journal order

### Runtime Data — Not Saved

```text
Dialogue current line
Memory Echo current page
Journal open state
Journal selected entry
CurrentInteractable Actor Pointer
Lock-On Target Actor Pointer
Current Combo Index
Input Buffer state
Current attack Montage position
Velocity
```

---

# State Classification Summary

## Persistent Facts

```text
Quiet Child Story Stage
Quiet Child Anchor ID
Objective completion
Encounter completion
Recorded Memory Echoes
Current Checkpoint ID — planned and optional
Save Version
```

## Transitional State

```text
NPC Relocating
NPC EventResolved
Puzzle Previewing
Puzzle Resetting
Echo activation in progress
Dialogue or Journal UI progress
Attack, Combo, Hit Stop, Lock-On, and animation progress
```

## Derived State

```text
Echo Relic Locked / Available / Activated
Lantern completed visual state
Enemy Objective defeated presentation
Encounter's cached Objective-completed flag
NPC stable Available state after restoration
```

## Level Configuration

```text
Story Anchor references
Next Story Stage
Completion Story Event ID
Correct lantern sequence
DialogueByStage
Echo content configured on Relics
```

Level configuration remains in the map and Blueprint assets and is not duplicated into SaveGame.

---

# Stable ID Understanding

## Actor Pointer

An Actor Pointer only references one runtime Actor instance in the current World.

Stopping PIE or unloading the World destroys that Actor. A later session creates a new Actor instance at a different runtime address.

Therefore, Actor Pointers cannot serve as cross-session SaveGame identity.

## Actor Name

Generated Actor Names may depend on spawn order, duplication, PIE World context, or runtime creation.

They identify an implementation instance, not a narrative concept.

## Actor Label

Actor Labels are editor-facing organizational names and can be renamed for developer convenience.

Changing an Outliner label should not invalidate an existing save.

## Stable Semantic ID

A stable semantic ID describes what the content means:

```text
QuietChild
QuietChild.Encounter02
QuietChild.Anchor03
```

It remains valid when presentation or implementation details change.

---

# Planned Persistence Constants

The following Week 7 conventions were established:

```text
Save Slot Name = WangChuan_Save_01
User Index = 0
Save Version = 1
```

No disk data was written on Day 1.

---

# Build and Validation Results

## C++ Build

Configuration:

```text
Target: WangChuanEditor
Platform: Win64
Configuration: Development
```

Result:

```text
Build succeeded
```

## Blueprint Compilation

```text
7 related Blueprints compiled successfully
```

## ID Validation

```text
All five ID categories checked
All IDs are non-None
No duplicate ID exists within the same category
```

## Runtime Validation

```text
PIE started successfully
No new C++ Warning/Error caused by the Day 1 changes
No new Blueprint Warning/Error caused by the Day 1 changes
No Blueprint Runtime Error
No Accessed None
```

The Day 1 identity additions did not introduce new gameplay side effects.

---

# Day 1 Completion Assessment

Day 1 completion standard:

```text
PASS
```

The project can now clearly answer:

- Which data represents persistent facts
- Which data represents temporary gameplay or presentation state
- Which data can be derived during restoration
- Which Actor owns each piece of state
- Why stable semantic IDs are required
- Why Actor Pointers, generated Actor Names, and Actor Labels are unsuitable as long-term SaveGame keys

The project is ready to proceed to:

```text
Week 7 Day 2 — SaveGame Data and GameInstance API
```

---

# Day 1 Advanced Status

The optional Day 1 Advanced section has **not started yet**.

If continued on the same development day, the next controlled scope is the original Day 2 task:

```text
Create UWCGameSaveGame
Create the saved-state structs
Create UWCGameInstance
Configure the Game Instance Class
Implement one-slot disk Create / Save / Load APIs
Verify round-trip persistence with isolated test data
```

This Advanced section should still avoid connecting every Story Actor immediately. Full world-state capture remains a later step.
