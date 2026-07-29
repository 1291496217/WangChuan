# Development Log — Week 6

**Project:** WangChuan / 《忘川河畔》  
**Engine:** Unreal Engine 5.8  
**Focus:** reusable Story Objectives, a lantern-sequence puzzle, a second narrative encounter, NPC relocation presentation, a formal Memory Journal, and combat/jump state corrections  
**Week Theme:** Extending the Narrative Foundation — one shared encounter pipeline for combat and non-combat objectives

---

## Week 6 Goal

Week 6 extended the narrative-gameplay foundation completed in Week 5 without replacing the existing interaction, dialogue, Echo Relic, Journal-recording, or NPC-stage systems.

The main architectural goal was to support two different condition types through one story flow:

```text
Combat Condition
→ Enemy Defeat Objective

Non-Combat Condition
→ Lantern Sequence Objective

Both
→ AStoryObjectiveBase
→ AStoryEncounter
→ Echo Relic Available
→ Player Reads the Complete Memory Echo
→ Encounter Completed
→ Story Event
→ NPC Relocation
→ New Story Stage
```

The central rule remained:

```text
Objective Condition Resolved
≠
Encounter Completed
```

Completing an Objective only makes its Echo Relic available. The Encounter completes only after the player fully reads and activates that Echo.

The week deliberately avoided:

- A global Quest Manager or Story Subsystem
- Gameplay Ability System integration
- DataTable-driven quest graphs
- Puzzle logic inside `AWCCharacter`
- Direct Lantern-to-NPC or Lantern-to-Echo references
- Separate C++ classes for every lantern or Echo Relic
- Tick-based puzzle, relocation, or objective polling
- Aerial combat, jump-cancel attacks, or a new input-buffer system
- SaveGame persistence for the Journal

---

## Week 6 Result Overview

By the end of Week 6, the project added or completed:

- `AStoryObjectiveBase` with activation, reset, and one-time completion broadcasting
- `ALanternPuzzlePiece`, `ELanternPuzzleState`, and `ALanternSequencePuzzle`
- Timer-driven Preview, persistent correct progress, and wrong-input reset/replay
- Encounter 02, Echo Relic 02, Anchor 03, Story Stage 2, and Stage 2 dialogue
- Emissive/Point Light/Pitch polish and a final five-lantern sequence
- `AEnemyDefeatObjective` and the final Objective-only `AStoryEncounter`
- Niagara-assisted, simplified NPC hide/teleport/reveal
- `UMemoryJournalWidget`, reusable entry rows, and formal J-key Journal assets
- Journal damage safety and modal-state cleanup
- Ground-only attacks plus Combat Idle/Lock-On airborne animation corrections
- Full regression testing and a final showcase recording

---

## Day 1 — Story Objective Foundation

### Goal

Create a reusable Story Objective base and allow Story Encounters to receive either a combat condition or a non-combat Objective while preserving the Week 5 Echo gate.

---

### `AStoryObjectiveBase`

Created:

```text
Source/WangChuan/StoryObjectiveBase.h
Source/WangChuan/StoryObjectiveBase.cpp
```

The Actor is intentionally lightweight and does not Tick.

Added:

- `ObjectiveID`
- `bIsActive`
- `bIsCompleted`
- `OnObjectiveCompleted`
- `ActivateObjective()`
- `CompleteObjective()`
- `ResetObjective()`
- `GetIsObjectiveActive()`
- `GetIsObjectiveComplete()`

Behavior:

```text
Inactive Objective
→ cannot complete

Active Objective
→ CompleteObjective()
→ bIsCompleted = true
→ bIsActive = false
→ OnObjectiveCompleted broadcasts once
```

`ResetObjective()` restores the full Objective to its initial state.

---

### Initial `AStoryEncounter` Objective Support

The first Week 6 implementation extended `AStoryEncounter` with an optional `StoryObjective` path while temporarily retaining the Week 5 `RequiredEnemy` path.

Initial configuration rule:

```text
RequiredEnemy XOR StoryObjective
```

Added:

- `StoryObjective`
- `bStoryObjectiveCompleted`
- `HandleStoryObjectiveCompleted()`
- `IsEncounterConditionResolved()`
- `UnlockEchoRelicFromResolvedCondition()`

Both condition paths used one Echo-unlock entry point:

```text
Required Enemy defeated
→ bRequiredEnemyDefeated = true
→ UnlockEchoRelicFromResolvedCondition()

Story Objective completed
→ bStoryObjectiveCompleted = true
→ UnlockEchoRelicFromResolvedCondition()
```

Neither path completed the Encounter, moved the NPC, modified Story Stage, opened the Echo UI, or wrote to the Journal.

---

### Configuration Protection

The initial dual-path implementation validated:

- Both condition sources configured — Error, bind neither
- Neither source configured — Warning
- Exactly one source configured — bind that source

Delegates were bound in `BeginPlay()` and removed in `EndPlay()`.

The test Objective verified:

- Activation
- One-time completion
- Repeated `CompleteObjective()` rejection
- Story Encounter event receipt
- Echo Relic unlock
- Encounter remaining incomplete until Echo activation

---

### Destroyed Enemy State Fix

An integration issue appeared when the Required Enemy was destroyed before the player finished reading its Echo.

The old condition check depended on the Enemy Actor still being valid. The Actor could already be destroyed even though the Encounter had correctly recorded:

```text
bRequiredEnemyDefeated = true
```

`IsEncounterConditionResolved()` was corrected to read the stored result:

```text
bRequiredEnemyDefeated
or
bStoryObjectiveCompleted
```

The Encounter no longer loses completed progress when the original condition Actor is destroyed.

---

### Day 1 Result

At the end of Day 1:

- A reusable Objective base existed
- Objective completion and Encounter completion remained separate
- Objective completion broadcasting was one-time
- Combat and non-combat conditions could use the same Echo gate
- Invalid dual-condition configuration failed clearly
- Destroyed Enemy Actors no longer erased recorded progress
- Encounter 01 remained functional
- Development Editor / Win64 built successfully

---

## Day 2 — Lantern Puzzle Piece

### Goal

Create one reusable, independently testable lantern Actor that could later be coordinated by a sequence-puzzle Objective.

---

### `ALanternPuzzlePiece`

Created:

```text
Source/WangChuan/LanternPuzzlePiece.h
Source/WangChuan/LanternPuzzlePiece.cpp
```

The class:

- Inherits from `AActor`
- Implements `IInteractable`
- Does not Tick
- Owns only one lantern's presentation and interaction state

Component structure:

```text
SceneRoot
├─ LanternMesh
├─ InteractionSphere
└─ LanternLight
```

The Mesh does not block the player. The Query Only interaction sphere overlaps Pawn. The Point Light begins disabled and is controlled by C++.

---

### Lantern Identity and Feedback

Added:

- `PieceID`
- `bInteractionEnabled`
- `bIsLit`
- `bEnableInteractionOnBeginPlay`
- `InteractionPrompt`
- `LanternTone`
- `FeedbackDuration`
- `ToneVolumeMultiplier`
- `TonePitchMultiplier`
- `LanternFeedbackTimerHandle`

Added functions:

- `SetLanternLit()`
- `GetIsLanternLit()`
- `PlayLanternFeedback()`
- `PlayLanternFeedbackForDuration()`
- `FinishLanternFeedback()`
- `PlayLanternTone()`
- `SetInteractionEnabled()`
- `GetIsInteractionEnabled()`

The same source sound can be reused with per-instance Pitch and Volume differences.

---

### Interaction Delegate

Created:

```cpp
FOnLanternInteractedSignature
OnLanternInteracted
```

Interaction flow:

```text
Player presses E
→ validate interaction and player state
→ play local light/sound feedback
→ broadcast this Lantern Actor
```

The Piece does not store or evaluate the correct sequence. The future Controller reads:

```cpp
InteractedLantern->GetPieceID()
```

This delegate-based design replaced a direct `PuzzleOwner` dependency and kept each lantern independently reusable.

---

### Prompt and Timer Safety

Disabling a lantern:

- Clears it from `CurrentInteractable`
- Hides its Prompt
- Rejects E input
- Keeps overlap detection available

Re-enabling it refreshes the Prompt if the player is still inside its sphere and does not overwrite another current Interactable.

`SetLanternLit(true)` clears old temporary feedback timers. This prevents a previous timer from extinguishing a lantern that the Puzzle Controller needs to keep lit.

---

### Day 2 Assets and Tests

Created:

```text
BP_LanternPuzzlePiece
```

Three initial level instances used Piece IDs `0`, `1`, and `2`.

Verified:

- Independent interaction
- Prompt enter/exit cleanup
- Point Light on/off
- Timed feedback
- Repeated input timer refresh
- Safe null sound
- Positional placeholder sound
- Disable/enable behavior
- Prompt restoration without leaving the sphere
- No complete sequence logic inside a Piece

---

## Day 3 — Lantern Sequence Logic and Preview

### Goal

Connect the independent lanterns into a reusable, non-combat Story Objective.

The initial formal sequence was:

```text
0 → 2 → 1
```

---

### `ELanternPuzzleState`

Created:

- `Dormant`
- `Previewing`
- `AwaitingInput`
- `Resetting`
- `Completed`

One Enum prevents contradictory combinations of multiple state Booleans.

---

### `ALanternSequencePuzzle`

Created:

```text
Source/WangChuan/LanternSequencePuzzle.h
Source/WangChuan/LanternSequencePuzzle.cpp
```

The class inherits `AStoryObjectiveBase`, owns an `ActivationBox`, and does not Tick.

It is responsible for:

- Lantern Actor references
- Correct sequence data
- Current correct input
- Configuration validation
- Objective activation
- Puzzle state
- Preview timing
- Interaction enable/disable
- Correct/wrong evaluation
- Reset and automatic replay
- Objective completion
- Timer and delegate cleanup

It does not reference Echo Relics, Story NPCs, Story Stages, the Journal, or dialogue.

---

### Activation and Preview

First entry into the Activation Box starts the Objective and disables the trigger from starting it again.

Preview flow:

```text
Dormant
→ ActivateObjective()
→ Previewing
→ play one PieceID at a time
→ wait PreviewLightDuration + PreviewGapDuration
→ finish Preview
→ extinguish all pieces
→ AwaitingInput
→ enable lantern interaction
```

Timers, rather than Tick or a simultaneous loop, control every step.

---

### Correct and Wrong Input

Input uses:

```text
InputIndex = CurrentPlayerInput.Num()
SubmittedPieceID = InteractedLantern->GetPieceID()
ExpectedPieceID = CorrectSequence[InputIndex]
```

Correct input:

- Appends the Piece ID
- Converts temporary feedback to persistent light
- Advances visible progress
- Completes when the input length reaches the sequence length

Wrong input:

```text
AwaitingInput
→ Resetting
→ disable all interaction
→ clear Prompt
→ clear input
→ extinguish all lanterns
→ wait ResetDelay
→ replay the full Preview from step one
```

No failure penalty or level re-entry is required.

---

### Completion and Reset

Full completion:

- Clears Preview and Reset timers
- Permanently disables lantern interaction
- Keeps all completed lanterns lit
- Sets Puzzle State to `Completed`
- Calls the base `CompleteObjective()`

The base class guarantees one completion broadcast.

`ResetObjective()` performs a full Story Objective reset. It remains distinct from the per-attempt wrong-input reset.

---

### Dynamic Delegate Binding Fix

The first Activation Box test compiled but failed at runtime with:

```text
Unable to bind delegate to ' OnPuzzleAreaEntered'
```

The dynamic delegate macro argument had been split between `::` and the function name. Macro stringification preserved leading whitespace and reflection searched for the wrong UFUNCTION name.

All dynamic delegate targets were changed to continuous expressions:

```cpp
&ALanternSequencePuzzle::OnPuzzleAreaEntered
&ALanternSequencePuzzle::HandleLanternInteracted
```

Runtime delegates are now bound with `AddUniqueDynamic` in `BeginPlay()` and removed in `EndPlay()`.

This fixed both Activation Box startup and Lantern interaction delivery.

---

### Day 3 Result

Verified:

- Activation Box startup
- Timer-driven Preview
- Preview input lockout
- Prompt restoration
- Persistent correct-step lighting
- First-step and mid-sequence errors
- Full reset and automatic replay
- One-time Objective completion
- Completed-state interaction lockout
- Invalid configuration rejection
- Timer and delegate lifecycle cleanup
- No new delegate Ensure

---

## Day 4 — Encounter 02 and Echo Relic 02

### Goal

Connect the Lantern Sequence Objective to the existing narrative pipeline while proving:

```text
Lantern Puzzle Completed
≠
Encounter 02 Completed
```

---

### Echo Relic 02

Created:

```text
Content/WangChuan/Blueprints/Story/Relics/BP_EchoRelic_Lanterns.uasset
```

Level Actor:

```text
QuietChild_LanternEcho_02
```

The Actor reuses `AEchoRelic`, `IInteractable`, the existing Memory Echo UI, Prompt refresh, Journal recording, and duplicate protection.

Configured data:

```text
EchoID = QuietChild.LanternEcho02
Title = The Lights That Answered

Locked Prompt = [E] Examine
Available Prompt = [E] Listen
Locked Text = The embers give no answer.
```

Its Echo ID is distinct from `QuietChild.BellEcho01`, allowing both records to coexist in the Journal.

---

### Encounter 02

Created:

```text
BP_QuietChild_Encounter_02
```

Initial Day 4 configuration:

```text
EncounterID = QuietChild.Encounter02
RequiredEnemy = None
StoryObjective = QuietChild_LanternPuzzle_01
EchoRelic = QuietChild_LanternEcho_02
StoryNPC = QuietChild
NextStoryAnchor = None
NextStoryStage = 2
CompletionStoryEventID = QuietChild.LanternPuzzleEchoActivated
```

Anchor 03 was intentionally deferred to Day 5.

---

### Encounter 02 Flow

```text
Lantern Objective completes
→ Encounter records bStoryObjectiveCompleted
→ Echo Relic 02: Locked → Available
→ Encounter remains incomplete

Player fully reads Echo 02
→ Journal records QuietChild.LanternEcho02
→ Echo Relic 02: Available → Activated
→ OnEchoActivated
→ Encounter 02 completes
```

Waiting after Puzzle completion did not move the NPC, modify Story Stage, write to the Journal, or auto-open the Echo.

Starting but cancelling the Echo also left the Encounter incomplete.

---

### Debug and Unlock Accuracy

Encounter debug output was generalized to display both condition types:

- Required Enemy defeated
- Story Objective completed
- Echo activated
- Encounter completed
- Configured next Story Stage

`UnlockEchoRelicFromResolvedCondition()` now checks the return value of `UnlockRelic()`. Repeated unlock attempts produce an accurate warning rather than reporting a second successful unlock.

---

### Day 4 Result

The project proved that both Encounter types could share the same second half:

```text
Encounter 01:
Required Enemy → Echo → Encounter Complete

Encounter 02:
Lantern Objective → Echo → Encounter Complete
```

Encounter 01 regression, Encounter 02 completion, Prompt refresh, Journal recording, duplicate protection, PIE startup/exit, and Development Editor / Win64 build all passed.

---

## Day 5 — Anchor 03, Story Stage 2, and the Final Encounter 02 Flow

### Goal

Complete the second narrative loop:

```text
Read Echo 02
→ Encounter 02 Completed
→ Story Event
→ Quiet Child relocates from Anchor 02 to Anchor 03
→ CurrentStoryStage = 2
→ StoryState = Available
→ Stage 2 Dialogue
```

---

### Anchor 03

Added level Actor:

```text
QuietChild_Anchor_03
AnchorID = QuietChild.Anchor03
```

The Anchor was positioned after the Lantern/Echo area with valid ground height, discoverable facing, and no overlap with the puzzle or relic interaction volumes.

The existing model-forward correction remained:

```text
MeshFacingYawOffset = 90
NPCMesh Relative Rotation = (0, 0, 0)
```

Only Anchor rotation was used to control the new visual facing.

---

### Story Data Expansion

The Quiet Child level instance was expanded to:

```text
StoryAnchors[0] = QuietChild_Anchor_01
StoryAnchors[1] = QuietChild_Anchor_02
StoryAnchors[2] = QuietChild_Anchor_03
```

`DialogueByStage` was expanded to three entries without changing Stage 0 or Stage 1 data.

Stage 2 dialogue:

```text
Little Girl:
You came again.

Little Girl:
The lights answered you.

Little Girl:
I think... someone once answered me too.
```

The sequence does not start a third formal Objective.

---

### Final Encounter 02 Configuration

```text
EncounterID = QuietChild.Encounter02
StoryObjective = QuietChild_LanternPuzzle_01
EchoRelic = QuietChild_LanternEcho_02
StoryNPC = QuietChild
NextStoryAnchor = QuietChild_Anchor_03
NextStoryStage = 2
CompletionStoryEventID = QuietChild.LanternPuzzleEchoActivated
```

Final NPC state flow:

```text
Available at Stage 1
→ EventResolved
→ Relocating
→ Available at Stage 2
```

`PendingStoryStage` remains `2` during relocation. `CurrentStoryStage` changes only after the relocation completes.

---

### Old-Location and New-Location Safety

At Anchor 02:

- Prompt is explicitly cleared
- `CurrentInteractable` is cleared when it points to the NPC
- Collision and overlap interaction are disabled
- The old location cannot be remotely interacted with

At Anchor 03:

- Mesh visibility is restored
- Collision and overlap events are restored
- Prompt enter/exit works
- Stage 2 dialogue opens
- Prompt recovers after dialogue closure

Null Anchor, invalid Stage, empty Stage 2 dialogue, cancellation, and repeat-completion cases all failed safely.

---

### Day 5 Result

The complete two-Encounter flow passed:

```text
Stage 0 Dialogue
→ Enemy Objective
→ Echo 01
→ Anchor 02 / Stage 1
→ Lantern Objective
→ wrong input and automatic replay
→ correct completion
→ Echo 02
→ Anchor 03 / Stage 2
→ Stage 2 Dialogue
```

Build, Save All, PIE, Git commit, and GitHub push were completed for the Day 5 milestone.

---

## Day 6 — Lantern Puzzle Readability, Emissive Feedback, and Full Regression

### Goal

Improve the three-lantern version's readability without relying on development Debug, then run a full Story, UI, and combat regression.

The target player experience was:

```text
enter the area
→ notice and read the Preview
→ distinguish the three lanterns
→ see persistent correct progress
→ understand wrong-input reset and replay
→ complete the sequence without reading the Output Log
```

---

### Final Day 6 Timing and Audio

The three-lantern milestone used:

```text
Initial Preview Delay = 1.0 s
Preview Light Duration = 0.8 s
Preview Gap Duration = 0.5 s
Reset Delay = 1.0 s
```

Pitch mapping:

```text
Piece 0 = 0.8
Piece 2 = 1.0
Piece 1 = 1.2
```

For the sequence `0 → 2 → 1`, the audible order became:

```text
Low → Medium → High
```

The same source sound remained reusable, while Pitch supplied a clear identity independent of world position. The timing provided a readable initial pause, visible light duration, distinct dark gaps, and a perceptible but short wrong-input reset.

These timing values were retained by the later five-lantern configuration. Advanced 7 subsequently changed the formal Piece count, sequence, and complete Pitch mapping.

---

### Emissive Material Integration

Created:

```text
M_LanternGlow
MI_LanternGlow
```

Material parameters:

- `GlowColor`
- `GlowStrength`

Each Lantern Blueprint creates and stores a Dynamic Material Instance:

```text
LanternMID
```

`BP_OnLanternLitChanged()` synchronizes presentation:

```text
SetLanternLit(true)
→ Point Light visible
→ GlowStrength = LitEmissiveStrength

SetLanternLit(false)
→ Point Light hidden
→ GlowStrength = 0
```

This made Preview steps, correct progress, reset, and completed state readable even with placeholder geometry.

---

### BeginPlay Order Fix

Initial PIE startup produced:

```text
LanternMID is invalid
```

`ALanternSequencePuzzle::BeginPlay()` called `SetAllLanternsLit(false)` before every Lantern Blueprint was guaranteed to have run its own `BeginPlay()` and created `LanternMID`.

Actor `BeginPlay()` order is not guaranteed, so the Puzzle Controller had introduced a hidden initialization dependency.

The redundant call was removed from:

```text
Source/WangChuan/LanternSequencePuzzle.cpp
```

Final ownership:

```text
Lantern Piece BeginPlay
→ create MID
→ initialize its own unlit state

Puzzle Controller BeginPlay
→ validate configuration
→ disable interaction

StartPuzzle()
→ control all formal puzzle lighting
```

The fix did not change the state machine, Preview, reset, correct input, or completion logic. It only removed premature presentation initialization.

---

### Readability and Boundary Tests

Verified without player-facing Debug:

- Preview order and dark gaps were readable
- Point Light and Emissive changed together
- Correct lanterns remained lit as visible progress
- First-, second-, and third-step errors reset all progress
- Reset Delay led into a complete automatic replay
- Preview rejected E input
- Fast correct input completed once
- Fast repeated input correctly became a wrong next step
- Leaving during Preview did not stop the sequence
- Leaving after partial progress preserved that progress
- Returning after completion did not restart the puzzle
- With `LanternTone = None`, the visual-only puzzle remained completable
- Stage 1 dialogue provided guidance without revealing the answer

The no-Debug full flow passed:

```text
Stage 1 Dialogue
→ Preview
→ intentional wrong input
→ reset and replay
→ correct completion
→ Echo Relic 02
→ Encounter 02
→ Anchor 03
→ Stage 2 Dialogue
```

Dialogue, both Memory Echoes, Journal records, both Encounters, light/heavy combat, combo buffering, hit traces, enemy/player death, Lock-On, and strafe movement also passed regression.

Development Editor / Win64 build, PIE startup/exit, Save All, Git commit, and GitHub push completed successfully.

---

## Day 7 — Final Showcase and Week 6 Wrap-Up

The complete two-Encounter flow was run once more before recording. Lantern Preview and feedback, reset/replay, both Echo/Encounter paths, NPC relocation, Stage 2 dialogue, and clean PIE startup/exit remained valid.

A Week 6 showcase recording was produced covering the Puzzle, Echo, Encounter, relocation, and final dialogue.

---

## Advanced Work — Final Consolidation

### Advanced 1 — NPC Relocation Visual Feedback

The initial visual pass added a staged relocation presentation to `AWCStoryNPC`:

```text
clear interaction
→ Relocating
→ short delay
→ spawn old-location Niagara
→ Mesh transition
→ hide and teleport
→ reveal at the new Anchor
```

Created:

```text
/Game/WangChuan/Story/NPC/Effects/NS_QuietChild_Relocation
```

The one-shot pale cyan-white Sprite Burst is spawned in world space at the old location plus `(0,0,80)`, has no collision or Gameplay Light, and auto-destroys.

The initial implementation also created two Quiet Child-specific Masked Fade materials using `FadeOpacity` and `DitherTemporalAA`. Visibility tuning changed:

```text
Fade duration: 0.80 → 1.20 seconds
Particle count: 24 → 40
Lifetime: 1.1 → 1.5 seconds
Sprite size: 8 → 14
```

The Fade materials were an intermediate implementation and were removed by Advanced 5.

---

### Advanced 4 — Enemy Defeat Objective Wrapper

Created:

```text
Source/WangChuan/EnemyDefeatObjective.h
Source/WangChuan/EnemyDefeatObjective.cpp
/Game/WangChuan/Story/Objectives/BP_EnemyDefeatObjective_QuietChild_01
```

`AEnemyDefeatObjective` converts one configured `AGhostEnemy::OnEnemyDefeated` event into the common `AStoryObjectiveBase` interface.

It supports:

- Objective activation
- Already-dead Enemy recovery
- Exact Enemy instance filtering
- `AddUniqueDynamic` binding
- `EndPlay()` unbinding
- Base-class one-time completion protection

It does not unlock Echoes, complete Encounters, send Story Events, or relocate NPCs.

Encounter 01 was migrated from a direct Enemy condition to this Objective wrapper.

---

### Advanced 5 — Final Encounter Cleanup and Relocation Simplification

After evaluating the Fade implementation on the placeholder Mixamo character, the final relocation system was simplified.

Removed:

- Fade MIDs
- Fade parameters
- Fade update timers
- Two dedicated Fade material assets

Restored:

- Original Mixamo materials

Preserved:

- `NS_QuietChild_Relocation`
- Old-location observation
- Explicit hide
- Teleport
- Delayed reveal

Final timing:

```text
RelocationStartDelay = 0.35 seconds
RelocationVFXObservationDuration = 0.75 seconds
RelocationRevealDelay = 0.50 seconds
RelocationVFXOffset = (0, 0, 80)
```

Missing Niagara produces a warning but does not block relocation.

The same cleanup removed the Legacy `RequiredEnemy` branch from `AStoryEncounter`. The final architecture is:

```text
Gameplay Condition
→ Specific AStoryObjectiveBase subclass
→ AStoryEncounter
→ Echo Relic
→ Story progression
```

Final condition ownership:

```text
Encounter 01 → AEnemyDefeatObjective
Encounter 02 → ALanternSequencePuzzle
```

`AStoryEncounter` no longer contains a Ghost Enemy dependency or a direct Enemy delegate path.

---

### Advanced 6 — Formal Memory Journal and Combat Safety

Created:

```text
Source/WangChuan/MemoryJournalWidget.h/.cpp
Source/WangChuan/MemoryJournalEntryWidget.h/.cpp
Content/WangChuan/UI/Story/WBP_MemoryJournal.uasset
Content/WangChuan/UI/Story/WBP_MemoryJournalEntry.uasset
```

The J-key debug text was replaced with a formal UMG Journal.

The Journal:

- Reads `AWCCharacter::RecordedMemoryEchoes`
- Reuses `FMemoryEchoData`
- Preserves recording order
- Uses reusable selectable entry rows
- Displays title, Echo text, and player resonance text
- Selects the first entry by default
- Provides an empty state
- Preserves `EchoID` duplicate protection
- Closes through J or its Close button

Open state uses `GameAndUI`, shows the cursor, focuses the Journal, stops movement, and blocks gameplay actions without pausing the world.

The combat-balance correction prevents Journal invulnerability:

```text
In combat or locked on
→ Journal cannot open

Journal already open and player receives damage
→ close Journal
→ restore safe control state
→ apply damage normally
```

Dialogue, Memory Echo, Journal, and death states remain mutually safe. Dead players cannot open the Journal, and death cleanup does not restore movement or look input.

---

### Advanced 7 — Final Five-Lantern Puzzle

The formal puzzle was expanded from three lanterns to five without introducing five hardcoded handlers.

Added level instances:

```text
QuietChild_Lantern_03 — PieceID 3, Pitch 0.9
QuietChild_Lantern_04 — PieceID 4, Pitch 1.1
```

Final sequence:

```text
0 → 3 → 2 → 4 → 1
```

Final Pitch order:

```text
0.8 → 0.9 → 1.0 → 1.1 → 1.2
```

The core continues to use:

- `LanternPieces`
- `CorrectSequence`
- Each Actor's `PieceID`
- `CurrentPlayerInput.Num()`
- `CorrectSequence.Num()`

Fixed three-lantern warnings and comments were removed. Validation now covers empty arrays, null references, duplicate Actors, duplicate Piece IDs, empty sequences, and sequence IDs that do not exist.

The five lanterns use a shallow arc. Interaction Sphere radius was reduced:

```text
170 cm → 110 cm
```

This prevents adjacent interaction overlap while preserving stable single-lantern prompts.

The new instances reuse the existing Blueprint, `MI_LanternGlow`, Point Light, MID behavior, and lantern sound. Preview and reset timing retain the Day 6 values; only the formal Piece count, sequence, spatial layout, and Pitch mapping changed.

---

### Advanced 8 — Attack, Jump, Combat Idle, and Lock-On Corrections

Light and heavy attacks are now ground-only.

Added shared checks:

- `IsAnyAttackActive()`
- `CanStartGroundAttack()`
- `CanStartJump()`
- `CancelActiveAttackForAirborneTransition()`

Final rules:

```text
Grounded and not jumping
→ light/heavy attack may start

Attack active
→ jump is rejected

Airborne
→ light/heavy attack is rejected

Rejected inputs
→ are not buffered
```

Existing grounded light-combo buffering remains unchanged.

`OnMovementModeChanged()` cancels an active attack when movement enters `Falling`. Cancellation stops the Montage and Hit Stop, clears attack/combo timers, clears buffered input, resets combo state, and invalidates the current attack.

Hit Notify and trace paths revalidate:

- Player can act and is alive
- An attack is active
- Player is grounded
- A jump has not already been submitted

This prevents stale airborne Notifies from dealing damage.

Animation-facing getters were separated from persistent gameplay state:

```text
GetIsInCombat()
GetIsLockedOn()
```

They report true to the AnimBP only while grounded. Internal combat, lock-on, and target state remain active in the air, so Jump/Falling animation takes priority and grounded Combat Idle or Lock-On Strafe returns naturally after landing.

No AnimBP binary edit or Enhanced Input remapping was required.

---

## Key Problems and Solutions

### 1. Objective Completion Could Be Lost After an Enemy Actor Was Destroyed

**Cause:** completion was re-evaluated through Actor validity.

**Solution:** store and read the Encounter's resolved Boolean state.

---

### 2. Dynamic Delegate Binding Compiled but Failed at Runtime

**Cause:** splitting `&ClassName::FunctionName` across whitespace caused Unreal's dynamic-delegate macro to stringify a function name with a leading space.

**Solution:** keep the expression continuous, bind in `BeginPlay()`, and unbind in `EndPlay()`.

---

### 3. A Correct Lantern Could Be Extinguished by an Old Feedback Timer

**Solution:** persistent `SetLanternLit(true)` clears the temporary feedback timer before preserving correct progress.

---

### 4. Wrong Input Could Allow Additional Input or Multiple Reset Timers

**Solution:** immediately enter `Resetting`, disable every Piece, clear Prompt/input/light state, and use one reset timer before replay.

---

### 5. Two Encounter Condition Sources Created Ambiguous Ownership

**Initial protection:** enforce `RequiredEnemy XOR StoryObjective`.

**Final solution:** wrap Enemy defeat in `AEnemyDefeatObjective` and remove the direct `RequiredEnemy` branch from `AStoryEncounter`.

---

### 6. NPC Fade and Relocation Particles Were Too Subtle

The first pass increased Fade time and Niagara density, lifetime, and size.

The final pass retained the improved Niagara but removed placeholder-character Fade materials and runtime MID/timer complexity.

---

### 7. Opening the Journal During Combat Could Prevent Damage

**Solution:** reject Journal opening while in combat or locked on. If damage arrives while it is already open, close it first and then apply damage normally.

---

### 8. Combat Idle or Lock-On Strafe Overrode Jump Animation

**Cause:** AnimBP-facing combat/lock-on getters remained true while airborne.

**Solution:** keep internal gameplay state but expose those animation Booleans only while grounded.

---

### 9. Attacks and Jump Could Start in Incompatible States

**Solution:** centralize ground/jump validation at input entry, cancel attacks on movement-mode change, and revalidate in Notify/trace paths.

---

### 10. A Fixed Three-Lantern Assumption Limited the Puzzle

**Solution:** remove fixed-count warnings and keep Preview, input, completion, reset, and validation driven by arrays and Piece IDs.

---

## Final Week 6 State and Validation

The final implementation combines the work above as:

```text
Enemy Defeat Objective ─┐
                        ├→ AStoryEncounter
Five-Lantern Objective ─┘
→ Echo activation
→ ordered Journal record
→ Story Event
→ Niagara-assisted hide/teleport/reveal
→ new NPC Stage and dialogue
```

Final-state clarifications:

- `AStoryEncounter` is Objective-only; its temporary direct `RequiredEnemy` path was removed.
- Encounter 01 uses `AEnemyDefeatObjective`; Encounter 02 uses `ALanternSequencePuzzle`.
- The formal puzzle has five array-driven Pieces, synchronized Point Light/Emissive feedback, and sequence `0 → 3 → 2 → 4 → 1`.
- NPC relocation retains Niagara and staged timing but no longer uses Fade materials, MIDs, or a Fade timer.
- The Journal is a runtime UMG interface with ordered entries, duplicate protection, modal-state safety, and no combat invulnerability.
- Player attacks are ground-only; airborne animation takes priority over Combat Idle and Lock-On Strafe while internal combat/target state is preserved.

All manual boundary and regression tests performed after the Week 6 tasks passed, including:

- Objective, Encounter, already-dead Enemy, wrong-Enemy, and duplicate-event cases
- Three-lantern Day 6 readability tests and the final five-lantern Preview/error matrix
- Visual-only, no-sound, fast-input, leave/return, and completed-state Puzzle cases
- Both Echoes, Journal entry states, damage closure, dialogue/Echo/death exclusion
- NPC relocation, missing-VFX fallback, Prompts, Anchors, and Story Stages
- Attack/jump ordering, Combat Idle jump, Lock-On jump/landing, ledge fall, combo, damage, and death
- Asset/map reload and PIE startup/exit

Full C++ builds succeeded. Final logs contained no related `Accessed None`, Blueprint Runtime Error, delegate/timer error, fatal error, or crash.

---

## Known Limitations and Future Polish

### Story Persistence

Story Stage, Encounter state, Puzzle completion, and Journal Echo records are not yet restored through SaveGame.

---

### Journal Scope

The current Journal is a focused Memory Echo prototype. It does not include:

- Tabs or categories
- Search
- Quest tracking
- Map or inventory pages
- Save persistence
- Final localization/layout polish

---

### NPC Presentation

Quiet Child still uses a placeholder Mixamo character. Final character art may require a new material/presentation pass.

Relocation currently uses Niagara plus hide/teleport/reveal and does not include:

- Walking or NavMesh movement
- Relocation audio
- A final cinematic transition
- A production character-specific dissolve

---

### Lantern Presentation

The gameplay puzzle is complete, but future art polish may still replace:

- Placeholder lantern geometry
- Prototype scene lighting
- Current sound mix
- Simple completion presentation

The array-driven gameplay core does not require changes for those replacements.

---

### Combat Scope

Week 6 intentionally does not add:

- Aerial attacks
- Jump-cancel attacks
- Buffered rejected jump/attack inputs
- New air-combo or landing-combo states

The final policy is deliberately ground-only.

---

## Week 6 Summary

Week 6 proved that the Week 5 narrative foundation can support both combat and non-combat story conditions without a large quest framework.

The final architecture is:

```text
Gameplay-specific Objective
→ common AStoryObjectiveBase completion
→ AStoryEncounter unlocks an Echo
→ player completes the Memory Echo
→ Journal records the memory
→ Encounter sends a Story Event
→ NPC relocates
→ Story Stage and dialogue advance
```

The Lantern Sequence Puzzle provides the first complete non-combat Objective, while `AEnemyDefeatObjective` converts the existing combat condition into the same interface.

Week 6 also converted the debug Journal into a player-facing UMG feature, improved NPC relocation readability without retaining placeholder Fade complexity, expanded the formal puzzle to five lanterns, and corrected combat/jump animation boundaries.

The resulting systems remain event-driven, timer-driven, array-driven, and intentionally lightweight. The full two-Encounter Quiet Child flow passed regression testing and was captured in the Week 6 showcase recording.
