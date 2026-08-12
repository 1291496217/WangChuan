# 《忘川河畔》Week9 Day3 Progress

**Project:** WangChuan / 《忘川河畔》
**Engine:** Unreal Engine 5.8
**Development Branch:** `feature/ai-first-prototype`
**Week Theme:** Tutorial Dungeon Vertical Slice — From Controls to Land Temple
**Day Theme:** Five-Lantern Puzzle → Fragment03 → Boss Gate
**Date:** 2026-08-12
**Status:** Completed
**Human Puzzle Review:** PASS
**Second Technical Review:** PASS
**Commit / Push:** Pending separate Git step

---

# 1. Day3 Goal

Week9 Day3 completed the Tutorial Dungeon puzzle section.

The target loop was:

```text
Enter Lantern Puzzle
↓
Automatic five-lantern Preview
↓
Player observes
↓
Wrong input
→ Reset + full Replay

Correct sequence
→ persistent progress lighting
↓
Puzzle Complete
├─ Tutorial.Fragment03 becomes available
└─ Boss Gate opens
↓
Player may read Fragment03
↓
2 / 3 → 3 / 3
↓
Boss Arena becomes reachable
```

No Mini-Boss was implemented during Day3.

---

# 2. Starting Baseline

Day2 had already completed:

```text
Move / Look paused Instruction
Interact paused Instruction
CR01 Light / Heavy tutorial
CR01 Enemy Defeat → Fragment02
CR02 Lock-On tutorial
CR03 Dodge / Roll placeholder
```

The active map remained:

```text
/Game/WangChuan/Maps/TutorialDungeon_Prologue_v01
```

The Day3 implementation begins after CR03 at:

```text
TD_LanternPuzzle
```

and ends when the player can enter the empty Boss Arena.

---

# 3. Week6 Lantern System Reuse

Day3 did not create a second Tutorial-specific puzzle state machine.

It reused:

```text
ALanternPuzzlePiece
ALanternSequencePuzzle
BP_LanternPuzzlePiece
BP_LanternSequencePuzzle
```

and the existing Lantern visual / audio presentation.

The reused Week6 puzzle already provides:

```text
Preview
interaction lock during Preview
wrong-input reset
automatic full Replay
persistent correct-step lighting
Completed state
one-shot completion
timer-driven operation
No Tick
```

The Tutorial therefore integrates an already validated puzzle system rather than duplicating it.

---

# 4. Five Tutorial Lanterns

Five Lantern instances were configured:

```text
TutorialLantern_00
TutorialLantern_01
TutorialLantern_02
TutorialLantern_03
TutorialLantern_04
```

Piece IDs:

```text
0
1
2
3
4
```

All IDs and Actor references were reported unique and non-null.

Recorded locations:

```text
TutorialLantern_00:
(5620, -100, 80)

TutorialLantern_01:
(5450, -1200, 80)

TutorialLantern_02:
(5850, -1200, 80)

TutorialLantern_03:
(6250, -1200, 80)

TutorialLantern_04:
(6030, -90, 80)
```

---

# 5. Lantern Pitch Mapping

The Tutorial kept the established pitch mapping:

```text
Piece 0:
0.8

Piece 3:
0.9

Piece 2:
1.0

Piece 4:
1.1

Piece 1:
1.2
```

The existing Quiet Child Lantern tone was reused.

The puzzle remains understandable through its visual state even if audio presentation is later revised.

---

# 6. Puzzle Controller

Placed:

```text
Tutorial_LanternSequencePuzzle
```

Objective identity:

```text
Tutorial.LanternPuzzle01
```

The fixed correct sequence is:

```text
0 → 3 → 2 → 4 → 1
```

No randomization was added.

The Tutorial puzzle is intentionally deterministic.

---

# 7. Tutorial Preview Timing

Day3 used instance-level Tutorial timing:

```text
Initial Preview Delay:
1.0 s

Preview Light Duration:
1.0 s

Preview Gap Duration:
0.6 s

Reset Delay:
1.0 s
```

This is slightly more readable than the earlier baseline while leaving the global C++ defaults unchanged.

The developer's final human puzzle review accepted the resulting Preview readability and difficulty.

---

# 8. Activation Behavior

Recorded Activation Box:

```text
Center:
(6200, -1450, 100)

Extent:
(500, 250, 150)
```

The Trigger begins the Preview after the player reaches the Puzzle threshold from CR03, before they move deeply among the five Lanterns.

No paused Tutorial Instruction was added for the puzzle.

This preserves the Day2 design rule:

```text
Controls
→ explicit Tutorial Instruction

Puzzle rule
→ learned through Puzzle behavior itself
```

---

# 9. Preview State

Fresh Puzzle flow:

```text
Dormant
↓
Activation Box
↓
Previewing
```

During Preview:

```text
World remains unpaused
Lantern interaction disabled
one Lantern lights at a time
player E input does not submit a Puzzle step
```

Preview completion:

```text
all Lanterns off
↓
AwaitingInput
↓
all five Lantern interactions enabled
```

Codex runtime validation reported that interaction during Preview produced zero submitted input.

The developer subsequently passed the manual Preview test.

---

# 10. Wrong Input → Reset → Replay

Wrong input behavior:

```text
AwaitingInput
↓
wrong Lantern
↓
Resetting
↓
interaction disabled
↓
current sequence cleared
↓
all Lanterns extinguished
↓
1.0 s delay
↓
full five-step Preview automatically replays
↓
AwaitingInput
```

There is no:

```text
damage
time penalty
failure screen
level reload
```

The player is simply shown the full pattern again.

Human review:

```text
Wrong / Replay Readability:
PASS
```

---

# 11. Correct Progress

Correct sequence:

```text
0
→ 3
→ 2
→ 4
→ 1
```

Each correct Lantern remains lit.

Earlier successful Lanterns do not turn off while the player enters subsequent correct steps.

The completed visual therefore communicates progress directly:

```text
1 correct
→ 1 persistent light

3 correct
→ 3 persistent lights

5 correct
→ all five lights
```

Human review:

```text
Correct Progress Readability:
PASS
```

---

# 12. Puzzle Completed State

After the fifth correct input:

```text
all five Lanterns remain lit
interaction remains disabled
Puzzle state = Completed
CompleteObjective() fires once
```

Further calls / attempted input do not restart the sequence or produce another completion.

The Completed state remains stable for the rest of the PIE session.

---

# 13. Fragment03

Existing Tutorial Fragment:

```text
Actor:
TutorialFragment03

ID:
Tutorial.Fragment03

StartAvailable:
false
```

Fresh PIE:

```text
Fragment03:
Unavailable
```

Puzzle completion:

```text
SetFragmentAvailable(true)
```

The Puzzle does not auto-collect Fragment03.

Therefore immediately after Puzzle completion:

```text
Fragment03:
Available

Fragment03:
Uncollected

HUD:
2 / 3
```

This behavior was both recorded in the Result and visibly supported by the Day3 evidence.

---

# 14. Fragment03 Collection

The player may then interact normally:

```text
approach Fragment03
↓
E
↓
Tutorial Fragment UI
↓
read
↓
close
```

Only actual collection changes:

```text
2 / 3
→ 3 / 3
```

Fragment03 remains a Tutorial-only Fragment.

It does not enter:

```text
Formal Case
Memory Journal
RecordedMemoryEchoes
Story Encounter
SaveGame
AI Judgement
```

---

# 15. Tutorial Boss Gate

A lightweight Tutorial Gate was introduced.

New native class:

```text
AWCTutorialGate
```

Blueprint:

```text
/Game/WangChuan/Blueprints/Tutorial/BP_TutorialGate
```

Saved Actor:

```text
TutorialBossGate
```

The Gate uses a simple Prototype Cube panel appropriate for the current greybox.

---

# 16. Boss Gate Closed State

Fresh PIE:

```text
Gate:
Closed

Collision:
Query and Physics / BlockAll
```

Recorded panel bounds:

```text
100 × 400 × 400 cm
```

The closed Gate physically prevents progression into the Boss Arena.

The developer manually confirmed the closed Gate cannot be bypassed through the normal route.

---

# 17. Boss Gate Open State

Open offset:

```text
(0, 0, 500)
```

`OpenGate()`:

```text
moves Gate panel clear of doorway
disables collision
sets open state
```

The function is idempotent.

Repeated calls leave the Gate at the same open state rather than moving it repeatedly.

The developer manually confirmed that the opened Gate is traversable.

---

# 18. Tutorial Puzzle Completion Link

Created:

```text
ATutorialPuzzleCompletionLink
```

Blueprint:

```text
BP_TutorialPuzzleCompletionLink
```

Saved Actor:

```text
Tutorial_LanternPuzzle_CompletionLink
```

References:

```text
RequiredPuzzle:
Tutorial_LanternSequencePuzzle

RewardFragment:
TutorialFragment03

BossGate:
TutorialBossGate
```

The link subscribes to the existing Objective completion delegate.

The generic Lantern Puzzle class was not hardcoded to Tutorial rewards.

---

# 19. Puzzle Complete → Two Independent Results

The completion link performs:

```text
Puzzle Complete
├─ Fragment03 Available
└─ Boss Gate Open
```

This is a critical progression boundary.

The Gate does not wait for:

```text
Fragment03 interaction
Fragment03 collection
Fragment HUD = 3 / 3
```

to open.

---

# 20. Gate / Fragment Independence

Before Fragment03 was read, the Result recorded:

```text
Puzzle completed:
true

Fragment03 available:
true

Fragment03 collected:
false

Fragment count:
2 / 3

Gate open:
true

Gate collision:
disabled
```

The uploaded screenshot evidence also visibly shows the Boss Gate open while the HUD remains:

```text
Fragments 2 / 3
```

Result:

```text
Gate Opens Without Fragment03 Read:
PASS
```

This confirms that reading UI is not incorrectly used as a progression gate.

---

# 21. Screenshot Evidence Note

The majority of Day3 screenshot labels match the visible evidence.

One evidence-label mismatch was identified during the second technical review:

```text
13_Day3_GateOpen_FragmentUnread.png
```

actually shows the Fragment reading modal open and the HUD already at:

```text
3 / 3
```

Therefore screenshot 13 does not itself prove the "Fragment unread" state.

However, the intended independence is still directly supported by:

```text
11_Day3_Fragment03_Available.png
→ Fragment available at 2 / 3

12_Day3_BossGate_Open.png
→ Gate visibly open at 2 / 3
```

and by the Result's runtime assertions.

This is an evidence-file naming issue, not a gameplay blocker.

---

# 22. Boss Arena

Day3 intentionally leaves the Boss Arena empty.

Final:

```text
Boss count:
0
```

The existing:

```text
TD_BossSpawn
```

remains a development marker only.

Day3 proves only:

```text
Puzzle
→ Gate
→ Boss Arena reachable
```

Mini-Boss implementation remains deferred.

---

# 23. Fresh PIE Reset

A second fresh PIE session was used to validate runtime-only state.

Reset state:

```text
Puzzle:
Dormant / incomplete

Lanterns:
all off

Fragment03:
Unavailable / uncollected

Boss Gate:
Closed

Fragment HUD:
0 / 3
```

No Tutorial Puzzle persistence was introduced.

This matches the current Week9 design.

---

# 24. Story / Persistence Isolation

Day3 does not connect the puzzle to formal narrative persistence.

Confirmed:

```text
Tutorial Persistence Coordinator:
0

Story Encounters added:
0

Echo Relics added:
0

Journal changes:
0

SaveGame changes:
0

Ghost AI changes:
0

AI Judgement changes:
0

Real AI Calls:
0
```

Although `ALanternSequencePuzzle` derives from the existing objective architecture, it is used here only as a reusable runtime puzzle and completion-event source.

---

# 25. Puzzle / Boss Combat Isolation

Final Tutorial map state:

```text
Puzzle-area enemies:
0

Boss-area Boss count:
0

Puzzle Tutorial Instruction Triggers:
0
```

No Combat Gate or new enemy behavior was introduced for the Puzzle section.

---

# 26. Build Validation

Final build:

```text
WangChuanEditor
Development
Win64
```

Result:

```text
PASS
```

Recorded UnrealBuildTool execution:

```text
26.10 seconds
```

---

# 27. MapCheck

Final engine summary:

```text
0 Errors
0 Warnings
```

The logs also contained six duplicate-location warning lines involving three pairs of pre-existing greybox StaticMeshActors.

Day3 did not create or modify those Actors.

The final engine MapCheck summary nevertheless reported:

```text
0 Errors
0 Warnings
```

These pre-existing greybox-location notes are non-blocking for Day3.

---

# 28. Static Audit

Recorded:

```text
Lantern count:
5

Unique IDs:
PASS

Puzzle Controller count:
1

Lantern refs:
5 non-null / non-duplicate

Correct Sequence:
[0, 3, 2, 4, 1]

Gate count:
1

Completion Link count:
1

Fragment03 StartAvailable:
false

Puzzle enemy count:
0

Boss count:
0

Puzzle Instruction Trigger count:
0

Persistence Coordinator count:
0
```

Static audit:

```text
PASS
```

---

# 29. Codex Runtime Validation

Two fresh PIE sessions were used for the implementation audit.

Recorded runtime assertions:

```text
37 / 37 PASS
```

Covered areas included:

```text
fresh initial state
Preview
Preview interaction lock
World remains unpaused
AwaitingInput
wrong input
Resetting
automatic Replay
persistent correct lighting
completion
Fragment03 availability
Gate opening
Gate idempotency
Gate / Fragment independence
fresh PIE reset
```

---

# 30. Test-Only Lantern Helper Note

Day3 locally modified:

```text
LanternPuzzlePiece.h
LanternPuzzlePiece.cpp
```

only to add:

```text
TriggerLanternInteractionForTest()
```

for Codex runtime automation.

The Result states that this helper routes through the existing `Interact()` implementation and does not alter the actual Lantern Puzzle state machine.

The developer's final manual Puzzle validation passed without relying on this API as a player-facing feature.

Second technical review conclusion:

> This test-only entry point is not a Day3 gameplay blocker, but it should not automatically become part of the formal production commit.

Before Git staging:

```text
audit whether any formal game code / Blueprint depends on TriggerLanternInteractionForTest()
```

If the answer is:

```text
NO
```

then:

```text
do not stage the LanternPuzzlePiece test-only diff
```

and leave the existing production Lantern class unchanged in the commit.

If it is intentionally retained later, it should be explicitly justified as formal test infrastructure rather than silently added to the runtime API.

---

# 31. Human Puzzle Review

After Codex marked:

```text
READY FOR HUMAN PUZZLE REVIEW
```

the developer completed the Day3 manual test and reported the full Day3 Puzzle flow as passed.

The human review covered the intended player-facing checks:

```text
Fresh PIE initial state
No Puzzle Instruction popup
Activation timing
Five-Lantern spatial readability
Preview readability
E rejected during Preview
Interaction after Preview
Wrong Reset
Full Replay
Persistent correct progress
Completed state
Fragment03 availability
HUD remains 2 / 3 before collection
Boss Gate opens on completion
Gate does not wait for Fragment03 read
Closed Gate blocking
Open Gate traversal
Fragment03 reading
2 / 3 → 3 / 3
No formal Journal / Case contamination
Empty Boss Arena
Tutorial Puzzle difficulty
Fresh PIE reset
```

Human Puzzle Review:

```text
PASS
```

---

# 32. Second Technical Review

The uploaded Day3 Result Package was reviewed after the manual PASS.

Reviewed:

```text
Week9Day3_FiveLanternPuzzle_Result.md

16 screenshots

Lantern reuse architecture

five Lantern configuration

fixed sequence

Preview / Reset / Replay behavior

persistent correct-light behavior

Fragment03 completion binding

Boss Gate implementation

Puzzle Completion Link

Gate / Fragment independence

fresh PIE reset

Build / MapCheck

scope declarations
```

The technical review found no blocking architecture or progression issue.

Key progression evidence is internally consistent:

```text
Puzzle Complete
→ Fragment03 Available

Puzzle Complete
→ Boss Gate Open

HUD remains 2 / 3 before Fragment03 collection
```

Second Technical Review:

```text
PASS
```

---

# 33. Day3 Completion Summary

Completed:

- reused Week6 five-Lantern puzzle architecture;
- five Tutorial Lantern pieces;
- unique Piece IDs 0–4;
- fixed `0 → 3 → 2 → 4 → 1` sequence;
- Tutorial-readable Preview timing;
- Activation Box placement;
- interaction lock during Preview;
- wrong-input Reset;
- automatic full Replay;
- persistent correct-step lighting;
- one-shot Completed state;
- Tutorial-only Boss Gate;
- closed Gate collision;
- idempotent Gate opening;
- Tutorial Puzzle Completion Link;
- Puzzle Complete → Fragment03 Available;
- Puzzle Complete → Boss Gate Open;
- Gate independent from Fragment03 read;
- Fragment03 `2 / 3 → 3 / 3`;
- Boss Arena reachable;
- no Boss saved;
- fresh PIE state reset;
- no formal Story / Journal / Persistence integration;
- build PASS;
- MapCheck final summary PASS;
- human Puzzle review PASS;
- second technical review PASS.

---

# 34. Deferred

Next stages still include:

```text
Tutorial Mini-Boss
Boss combat validation
Boss Defeat
Exit Gate opening
Exit transition
Land Temple greybox / transition
Dodge / Roll implementation
CR03 real Dodge tutorial
final Fragment03 narrative text
Boss Gate visual polish / animation
```

The existing environment-lighting issue also remains:

```text
Dungeon ceiling deferred
until internal dungeon lighting is designed
```

---

# 35. Final Gate

```text
Codex Day3 Implementation Gate:
READY FOR HUMAN PUZZLE REVIEW

Human Puzzle Review:
PASS

Second Technical Review:
PASS
```

Therefore:

```text
WEEK9 DAY3:
COMPLETE
```

The Tutorial Dungeon can now proceed to the Mini-Boss / Boss Defeat stage.
