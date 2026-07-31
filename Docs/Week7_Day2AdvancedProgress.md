# 《忘川河畔》Week 7 — Day 2 Advanced Progress

**Project:** WangChuan / 《忘川河畔》  
**Active Project:** `WangChuan_UE58_Migration`  
**Internal Module:** `WangChuan`  
**Engine:** Unreal Engine 5.8  
**Date:** 2026-07-31  
**Week Theme:** Story Persistence, SaveGame & Resume Flow  
**Milestone:** Week 7 Day 2 Advanced / Original Day 4  
**Day Theme:** Silent Restore Interfaces

---

## Day Goal

Day 2 Advanced established explicit, side-effect-free Restore interfaces for the current Story systems.

The central architectural goal was to separate:

```text
Normal Gameplay Transition
```

from:

```text
SaveGame State Restoration
```

Normal Gameplay functions may broadcast Delegates, play audio and VFX, open UI, unlock Relics, send Story Events, and relocate NPCs. Restore functions must instead apply already-established facts directly:

```text
Apply saved stable facts
→ restore final Actor state
→ do not replay the event that originally produced that state
```

---

# Result Overview

Completed:

- Silent Objective restoration
- Objective subclass restoration hooks
- Persistent defeated-state restoration for the required enemy
- Silent Enemy Defeat Objective restoration
- Silent Lantern Puzzle restoration
- Silent Encounter restoration
- Silent Echo Relic restoration
- Silent Quiet Child Stage and Anchor restoration
- Silent Memory Journal restoration
- Restore State B testing
- Restore State C testing
- Gameplay Delegate side-effect monitoring
- NPC VFX and relocation side-effect testing
- Repeated-restore idempotency testing
- Full manual code review

All planned tests passed.

---

# Modified Files

```text
StoryObjectiveBase.h / .cpp
EnemyDefeatObjective.h / .cpp
GhostEnemy.h / .cpp
LanternSequencePuzzle.h / .cpp
StoryEncounter.h / .cpp
EchoRelic.h / .cpp
WCStoryNPC.h / .cpp
WCCharacter.h / .cpp
```

The following orchestration files were intentionally not modified:

```text
WCSaveTypes
WCGameSaveGame
WCGameInstance
WCStoryPersistenceCoordinator
```

This preserved the separation between:

```text
Actor-level Restore capability
```

and:

```text
Coordinator-level ID matching and load order
```

---

# 1. Base Objective Restore Interface

Added to `AStoryObjectiveBase`:

```cpp
ApplySavedObjectiveState(bool bSavedCompleted)
```

and a protected subclass hook:

```cpp
OnSavedObjectiveStateApplied()
```

The Base restore path directly applies:

```cpp
bIsCompleted = bSavedCompleted;
bIsActive = false;
```

It does not call:

```cpp
CompleteObjective()
ResetObjective()
ActivateObjective()
```

and does not broadcast:

```cpp
OnObjectiveCompleted
```

After the stable Base state is assigned, subclasses restore their own derived presentation through `OnSavedObjectiveStateApplied()`.

## Gameplay and Restore Difference

Normal Gameplay:

```text
Objective becomes active
→ condition is met
→ CompleteObjective()
→ OnObjectiveCompleted broadcasts
→ Encounter reacts
```

Restore:

```text
Save says Objective was completed
→ ApplySavedObjectiveState(true)
→ set completed state directly
→ restore subclass presentation
→ no Delegate broadcast
```

---

# 2. Persistent Enemy Defeated State

Added to `AGhostEnemy`:

```cpp
ApplyPersistentDefeatedState()
```

The function restores the Enemy as already defeated by:

- Clearing combat timers
- Clearing the death timer
- Setting Health to zero
- Setting stable dead-state flags
- Hiding the health bar
- Disabling collision
- Disabling Tick
- Hiding the Actor

It does not call:

```cpp
Die()
```

and therefore does not:

- Play death audio
- Play death VFX
- Replay the death animation
- Broadcast `OnEnemyDefeated`
- Start a destruction timer
- Destroy the Actor

## Why the Enemy Is Hidden Instead of Destroyed

The restored Enemy remains as a disabled Actor instance:

```text
RequiredEnemy pointer remains valid
Repeated Restore calls remain safe
Debug inspection remains possible
No gameplay collision or AI remains
```

Destroying the Enemy during Restore would make repeated application and Objective reference validation less predictable.

---

# 3. Enemy Defeat Objective Restore

`AEnemyDefeatObjective` overrides:

```cpp
OnSavedObjectiveStateApplied()
```

## Completed Restore

```text
bRequiredEnemyDefeated = true
Enemy Delegate unbound
Required Enemy restored as persistently defeated
Objective remains inactive
```

No Enemy or Objective Delegate is broadcast.

## Incomplete Restore

```text
bRequiredEnemyDefeated = false
configuration revalidated
Enemy Delegate restored
Objective becomes active directly
```

The Restore path does not call `ActivateObjective()` when doing so could immediately evaluate and broadcast completion.

If the saved Objective is incomplete but the current Enemy is already dead, the contradiction is logged instead of silently completing the Objective through the Gameplay path.

---

# 4. Lantern Puzzle Restore

`ALanternSequencePuzzle` overrides:

```cpp
OnSavedObjectiveStateApplied()
```

Restore always clears:

- Preview timers
- Reset timers
- Current player input
- Current Preview index
- Lantern interaction

It never attempts to resume a partially running puzzle state.

## Completed Puzzle Restore

```text
PuzzleState = Completed
all five lanterns lit
lantern interaction disabled
ActivationBox disabled
partial input cleared
all puzzle timers cleared
```

The Restore path does not call `CompleteObjective()`.

No Preview, reset, sound sequence, or completion Delegate is replayed.

## Incomplete Puzzle Restore

```text
PuzzleState = Dormant
all lanterns unlit
lantern interaction disabled
ActivationBox enabled
partial input cleared
all timers cleared
```

The Preview is not automatically started during Restore. The player can begin the puzzle normally through the intended Activation Box after loading.

## Persistent Fact and Derived Presentation

The SaveGame stores only:

```text
QuietChild.LanternPuzzle01
Completed = True / False
```

The following are derived during Restore:

```text
PuzzleState
ActivationBox collision
lantern interaction
all-lantern completion lighting
```

No separate five-lantern Boolean array was added to the SaveGame.

---

# 5. Encounter Restore

Added to `AStoryEncounter`:

```cpp
ApplySavedEncounterState(bool bSavedCompleted)
```

The function directly restores:

```cpp
bEncounterCompleted
```

and derives:

```cpp
bStoryObjectiveCompleted
```

from the configured Objective's already-restored state.

This establishes the required future load order:

```text
restore Objectives first
→ restore Encounters second
```

## Encounter Restore Does Not Call

```cpp
HandleStoryObjectiveCompleted()
UnlockEchoRelicFromResolvedCondition()
HandleEchoRelicActivated()
RecieveStoryEvent()
RelocateToStoryAnchor()
```

It therefore does not:

- Unlock a Relic as a new event
- Send a Story Event
- Complete an Encounter twice
- Move Quiet Child
- Play NPC relocation presentation

If a saved Encounter is completed while its configured Objective is incomplete, the inconsistency is reported rather than hidden.

---

# 6. Echo Relic Restore

Added to `AEchoRelic`:

```cpp
ApplySavedRelicState(EEchoRelicState SavedState)
```

The Restore function:

- Clears any previous activation-in-progress state
- Clears stale player interaction
- Sets `RelicState` directly
- Restores collision and Prompt for `locked` and `Available`
- Disables interaction for `Activated`

It does not call:

```cpp
UnlockRelic()
ConfirmEchoRead()
```

and therefore does not:

- Open Memory Echo UI
- Record a Journal entry
- Broadcast `OnEchoActivated`
- Complete an Encounter
- Display a new-unlock event as if it just occurred

## Relic State Derivation

The SaveGame does not independently store `EEchoRelicState`.

Future Coordinator logic will derive it:

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

The Relic Restore interface applies that derived result silently.

---

# 7. Quiet Child Story Restore

Added to `AWCStoryNPC`:

```cpp
ApplySavedStoryState(
    int32 SavedStoryStage,
    FName SavedAnchorID
)
```

The function validates:

- Story Stage is non-negative
- Anchor ID is not `None`
- A matching Anchor exists in `StoryAnchors`
- Dialogue is not actively using the NPC

Only after validation succeeds does it modify the World.

## Restore Behavior

The function:

- Clears relocation timers
- Clears pending Story Stage and pending Anchor
- Clears stale player interaction
- Disables NPC interaction during application
- Finds the target Anchor by stable `AnchorID`
- Teleports directly to its Transform
- Applies `MeshFacingYawOffset`
- Sets `CurrentStoryStage`
- Sets `StoryState = Available`
- Clears runtime Story-event debug state
- Restores visibility, collision, and interaction

It does not call:

```cpp
RecieveStoryEvent()
RelocateToStoryAnchor()
```

and does not:

- Enter `EventResolved`
- Enter `Relocating`
- Play relocation Niagara
- Wait for relocation timers
- Replay a Story Event

## Stable Anchor ID

Restore searches by:

```text
QuietChild.Anchor01
QuietChild.Anchor02
QuietChild.Anchor03
```

rather than relying on a saved array index. Reordering `StoryAnchors` will not invalidate a save as long as semantic Anchor IDs remain stable.

---

# 8. Memory Journal Restore

Added to `AWCCharacter`:

```cpp
ApplySavedMemoryEchoes(
    const TArray<FMemoryEchoData>& SavedEchoes
)
```

The function restores the runtime Journal array directly.

It:

- Rejects replacement while incompatible Story UI is active
- Closes the Journal if necessary
- Clears current records
- Preserves saved order
- Skips `None` Echo IDs
- Skips duplicate Echo IDs
- Adds validated records directly

It does not call:

```cpp
RecordMemoryEcho()
```

and therefore does not:

- Treat old records as newly acquired
- Open the Journal
- Open Memory Echo UI
- Activate a Relic
- Complete an Encounter
- Add duplicate entries on repeated Restore

---

# 9. Manual Restore State B

State B represented:

```text
Encounter 01 completed
Encounter 02 not completed
```

Expected stable result:

```text
Enemy Objective completed
Required Enemy persistently defeated

Encounter 01 completed
Bell Echo 01 Activated

Lantern Objective incomplete
Encounter 02 incomplete
Lantern Echo 02 locked

Quiet Child:
Stage 1
Anchor 02
StoryState Available

Journal:
Bell Echo 01
```

## State B Result

Passed.

Verified:

- Required Enemy hidden and non-colliding
- Enemy health UI hidden
- No death audio or death event replay
- Quiet Child moved directly to Anchor 02
- Stage 1 applied
- Stage 1 dialogue remained usable
- No Niagara relocation
- No `Relocating` state
- Bell Echo was non-interactable as Activated
- Lantern Echo retained its locked interaction
- Lantern Puzzle returned to incomplete Dormant state
- Journal contained exactly one Echo
- Encounter 02 remained playable

---

# 10. Manual Restore State C

State C represented:

```text
Encounter 01 completed
Encounter 02 completed
```

Expected stable result:

```text
Enemy Objective completed
Lantern Objective completed

Encounter 01 completed
Encounter 02 completed

Bell Echo 01 Activated
Lantern Echo 02 Activated

Quiet Child:
Stage 2
Anchor 03
StoryState Available

Journal:
Bell Echo 01
Lantern Echo 02
```

## State C Result

Passed.

Verified:

- Required Enemy remained persistently defeated
- All five lanterns remained lit
- Lantern interaction was disabled
- Puzzle Activation Box was disabled
- Both Encounters were complete
- Both Relics were non-interactable
- Quiet Child moved directly to Anchor 03
- Stage 2 dialogue remained usable
- Journal contained two Echoes in saved order
- No relocation VFX, audio, Story Event, or UI replay occurred

---

# 11. Delegate Side-Effect Tests

Restore testing monitored:

```text
AGhostEnemy::OnEnemyDefeated
AStoryObjectiveBase::OnObjectiveCompleted
AEchoRelic::OnEchoActivated
```

Applying Restore State B and State C did not produce unexpected Delegate broadcasts.

This confirmed that Restore paths did not accidentally call:

```cpp
Die()
CompleteObjective()
ConfirmEchoRead()
```

and did not re-enter Encounter Gameplay handlers.

---

# 12. Idempotency Tests

The same Restore state was applied more than once:

```text
Apply State B
→ Apply State B again
```

```text
Apply State C
→ Apply State C again
```

Passed.

Repeated application did not cause:

- Duplicate Journal entries
- Duplicate Delegate broadcasts
- Enemy death replay
- Puzzle completion replay
- Relic activation replay
- NPC relocation VFX
- Additional timers
- Position drift
- Repeated Story Events
- Prompt duplication

Journal counts remained:

```text
State B → 1 Echo
State C → 2 Echoes
```

The second application produced the same final state as the first.

---

# 13. Test Direction Boundary

Testing focused on:

```text
fresh default World
→ Restore State B
```

```text
fresh default World
→ Restore State C
```

and the valid forward application:

```text
Restore State B
→ Restore State C
```

The milestone did not implement a runtime rollback such as:

```text
State C
→ State A
→ revive previously defeated Enemy
```

The current Save/Load design restores a fresh level instance into saved progress. It is not a time-reversal system.

---

# Build and Runtime Verification

## Compilation

Passed:

```text
WangChuanEditor
Win64
Development
```

All related declarations and definitions were consistent. No new module dependency was required.

## Blueprint Verification

Relevant Story, Enemy, Objective, Puzzle, Relic, NPC, and Encounter Blueprints compiled successfully after the C++ changes.

## Runtime Verification

Passed:

- State B application
- State C application
- Repeated State B application
- Repeated State C application
- Required Enemy silent defeat
- Completed and incomplete Lantern presentation
- Relic interaction states
- Quiet Child Stage 1 and Stage 2 dialogue
- Memory Journal count and ordering
- Delegate side-effect monitoring
- NPC VFX and relocation-state monitoring

No new:

- Blueprint Runtime Error
- `Accessed None`
- Array bounds error
- Delegate Ensure
- Timer error
- unintended Story progression
- duplicate Journal record

remained after testing and manual review.

---

# Architecture Review

## Gameplay Entry and Restore Entry

| System | Normal Gameplay | Silent Restore |
|---|---|---|
| Enemy | `Die()` | `ApplyPersistentDefeatedState()` |
| Objective | `CompleteObjective()` | `ApplySavedObjectiveState()` |
| Enemy Objective | Enemy defeat handler | `OnSavedObjectiveStateApplied()` |
| Lantern Puzzle | Preview/input/completion flow | `OnSavedObjectiveStateApplied()` |
| Encounter | Objective/Echo Delegate handlers | `ApplySavedEncounterState()` |
| Relic | `UnlockRelic()` / `ConfirmEchoRead()` | `ApplySavedRelicState()` |
| Story NPC | `RecieveStoryEvent()` / relocation | `ApplySavedStoryState()` |
| Journal | `RecordMemoryEcho()` | `ApplySavedMemoryEchoes()` |

## Persistent Facts

The SaveGame still stores only:

```text
StoryNPCID / StoryStage / AnchorID
ObjectiveID / Completed
EncounterID / Completed
Recorded Memory Echoes
```

Day 2 Advanced did not add duplicated SaveGame fields.

## Derived Runtime State

Restored from persistent facts:

```text
Enemy hidden defeated presentation
Lantern Puzzle Completed/Dormant state
Lantern lights and interaction
Encounter ObjectiveResolved state
Echo Relic state
Quiet Child visibility, interaction and facing
Runtime Journal array
```

## Side Effects Explicitly Avoided

Restore does not replay:

```text
Enemy death
Objective completion
Echo activation
Encounter completion
Story Event
NPC relocation
Niagara
audio
Memory Echo UI
Journal acquisition
```

---

# Scope Preserved

Day 2 Advanced did not implement:

- Coordinator-driven Restore
- Save ID to Actor Map construction
- automatic `LoadSavedGame()`
- delayed post-`BeginPlay` restore
- full load-order orchestration
- player checkpoint restore
- automatic save after Encounter completion
- New Game / Continue UI
- multi-slot support
- cross-map persistence
- SaveGame version migration
- enemy revival / runtime rollback

---

# Final Result

Week 7 Day 2 Advanced / Original Day 4 is complete.

The project now has two independently verified halves:

```text
World Capture
→ stable facts written to SaveGame
```

and:

```text
Silent Actor Restore
→ saved facts applied without gameplay side effects
```

The next milestone can safely connect them:

```text
Week 7 Day 3
→ Full World Restore Coordination and Load Order
```

Expected Coordinator flow:

```text
Load SaveGame
→ wait until level Actors finish initialization
→ validate and map stable IDs
→ restore Objectives
→ restore Encounters
→ derive and restore Relics
→ restore Story NPC
→ restore Journal
→ verify final World consistency
```

That phase must continue to avoid assumptions about cross-Actor `BeginPlay()` order and must apply Restore in an explicit dependency order.
