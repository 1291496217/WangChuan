# 《忘川河畔》Week9 Day2 Progress

**Project:** WangChuan / 《忘川河畔》
**Engine:** Unreal Engine 5.8
**Development Branch:** `feature/ai-first-prototype`
**Week Theme:** Tutorial Dungeon Vertical Slice — From Controls to Land Temple
**Day Theme:** Three-Room Combat Tutorial Integration
**Original Week9 Plan:** Day3 scope, expanded to all three combat-teaching rooms
**Date:** 2026-08-11
**Status:** Completed
**Human Gameplay Review:** PASS
**Second Technical Review:** PASS
**Commit / Push:** Pending separate Git step

---

# 1. Day2 Goal

Week9 Day2 converted the three-room combat greybox into a functional tutorial sequence.

The main goal was to teach the current combat controls without forcing the player to read short-lived HUD messages while gameplay continued around them.

The new teaching rule is:

```text
Reach a new gameplay concept
↓
Open Tutorial Instruction
↓
Pause the World
↓
Player reads at their own pace
↓
E or Close
↓
Resume gameplay
↓
Immediately practice the concept
```

The three combat rooms now have distinct responsibilities:

```text
Combat Room 01
→ Light Attack / Heavy Attack / Light Combo

Combat Room 02
→ Lock-On / Strafe

Combat Room 03
→ Future Dodge / Roll placeholder
```

The player manually validated the actual combat behavior after Codex completed the implementation.

---

# 2. Starting Baseline

Before Day2, the Tutorial Dungeon already contained:

```text
Tutorial Dungeon linear route
Three sequential Combat Rooms
Downhill / Uphill combat transitions
Tutorial Fragment01 / 02 / 03
Tutorial Fragment reading UI
Fragment progress HUD
Move / Look transient hint
Interact transient hint
```

Combat-room spatial roles had already been established:

```text
CR01:
Light / Heavy

CR02:
Lock-On

CR03:
Future Dodge / Roll
```

Fragment02 remained:

```text
Tutorial.Fragment02
StartAvailable = false
```

and was positioned after CR01 near the bridgehead.

Day2's main integration task was therefore:

```text
Paused Tutorial Instruction system
+
CR01 basic enemy
+
Enemy Defeat → Fragment02
+
CR02 basic enemy
+
CR03 Dodge placeholder
```

---

# 3. Input Mapping Audit

The actual Enhanced Input configuration was audited before the Tutorial text was created.

Confirmed PC controls:

```text
W / A / S / D
→ Move

Mouse
→ Look

E
→ Interact

Left Mouse Button
→ Light Attack

Right Mouse Button
→ Heavy Attack

Middle Mouse Button
→ Lock-On

Space
→ Jump
```

Existing additional gamepad / arrow-key / touch mappings were not modified.

Two empty legacy entries associated with `IA_Interact` were observed in the mapping data, but the valid PC binding remains:

```text
E
```

No input asset was modified during Day2.

---

# 4. Unified Paused Tutorial Instruction System

Day2 replaced the previous transient control-hint approach with a dedicated Tutorial Instruction modal.

New native classes:

```text
UTutorialInstructionWidget
ATutorialInstructionTrigger
ATutorialEnemyDefeatFragmentLink
```

New Blueprint assets:

```text
/Game/WangChuan/UI/Tutorial/WBP_TutorialInstruction

/Game/WangChuan/Blueprints/Tutorial/BP_TutorialInstructionTrigger

/Game/WangChuan/Blueprints/Tutorial/BP_TutorialEnemyDefeatFragmentLink
```

The system remains Tutorial-specific and does not introduce a global tutorial manager or quest framework.

---

# 5. Tutorial Instruction Widget

Created:

```text
WBP_TutorialInstruction
```

Native base:

```text
UTutorialInstructionWidget
```

The intended Blueprint-authored hierarchy is:

```text
TutorialInstructionRoot [CanvasPanel]
├─ ScreenShade [Border]
└─ InstructionPanel [Border]
   └─ InstructionColumn [VerticalBox]
      ├─ InstructionTitle [TextBlock]
      ├─ InstructionBody [TextBlock]
      └─ CloseButton [Button]
         └─ CloseButtonLabel [TextBlock]
```

The runtime class uses `BindWidget` fields rather than constructing the complete Widget Tree dynamically.

The Instruction interface deliberately remains minimal:

```text
Title
Body
E / Close
```

No tutorial encyclopedia, tabs, quest list, or control-remapping UI was introduced.

---

# 6. World Pause Behavior

Opening a Tutorial Instruction performs the following sequence:

```text
validate player / modal state
↓
stop movement / jump
↓
exit Lock-On
↓
hide world interaction prompt
↓
record previous pause state
↓
SetPause(true)
↓
show cursor
↓
UI-focused input mode
↓
focus Tutorial Instruction widget
```

The implementation does not emulate pause through:

```text
Global Time Dilation
Custom Time Dilation
```

It uses the real pause path.

Runtime validation confirmed that while the Instruction is open:

```text
World Paused:
true

Cursor Visible:
true

Movement Ignored:
true

Look Ignored:
true
```

The developer's final PIE review also confirmed that enemies do not continue attacking while the Tutorial explanation is being read.

---

# 7. Pause Ownership

The Tutorial Instruction system tracks whether the world was already paused before it opened.

The implementation records concepts equivalent to:

```text
bWorldWasPausedBeforeInstruction
bInstructionOwnsPause
```

The close path only unpauses the game if the Tutorial Instruction itself created the pause.

This prevents the Tutorial UI from incorrectly releasing a pause owned by another system.

---

# 8. E Close While Paused

Paused gameplay input cannot be assumed to continue through the normal Character input path.

Therefore `UTutorialInstructionWidget` handles keyboard input directly.

The widget's key path:

```text
NativeOnKeyDown
→ E
→ common close path
→ FReply::Handled()
```

The Close Button uses the same common cleanup path.

A defensive early-return path also exists in Character interaction handling.

This prevents one E press from becoming:

```text
Close Instruction
+
Interact with world object
```

in the same input event.

---

# 9. Same-E Interaction Leak Protection

The Interaction tutorial was specifically tested near Fragment01.

Flow:

```text
Interaction Instruction opens
↓
Fragment progress = 0 / 3
↓
Player closes Instruction
↓
Progress remains 0 / 3
↓
Player must press E again
↓
Fragment01 interaction occurs
```

Result:

```text
Same E Does Not Collect Fragment:
PASS
```

This preserves clear separation between:

```text
learning the control
```

and:

```text
performing the control
```

---

# 10. Instruction Close / Cleanup

Both:

```text
E
```

and:

```text
Close Button
```

use the same Tutorial Instruction cleanup path.

Closing performs:

```text
remove widget
clear modal state
release owned pause
hide cursor
restore Game input mode
restore living-player gameplay control
```

Runtime validation confirmed:

```text
Instruction Closed:
PASS

World Resumed:
PASS

Player Control Restored:
PASS
```

---

# 11. Player Death Cleanup

The Tutorial Instruction modal was integrated with the existing death flow.

If the player dies while an Instruction is active:

```text
Instruction closes
owned pause is safely released
cursor is cleaned up
modal state is cleared
dead-player movement is not restored
```

Runtime death-cleanup validation:

```text
PASS
```

This preserves the existing defeated-state behavior.

---

# 12. Modal Exclusivity

Tutorial Instruction does not share the same modal state as Tutorial Fragment reading.

The new Instruction is rejected if incompatible UI is already active, including:

```text
Tutorial Fragment
Dialogue
Memory Echo
Journal
Checkpoint UI
```

Tutorial Fragment opening is also rejected while Tutorial Instruction is active.

This preserves the distinction:

```text
Tutorial Instruction
→ teaches controls

Tutorial Fragment
→ presents world residual memory
```

The two modals do not stack.

---

# 13. One-Shot Instruction State

Tutorial Instructions use runtime one-shot IDs.

IDs:

```text
Tutorial.Instruction.MoveLook
Tutorial.Instruction.Interact
Tutorial.Instruction.LightHeavy
Tutorial.Instruction.LockOn
Tutorial.Instruction.DodgePlaceholder
```

Runtime state uses a transient set equivalent to:

```text
ShownTutorialInstructionIDs
```

The state is:

```text
runtime only
not SaveGame
```

Instruction IDs are only marked as shown after the UI opens successfully.

Placed overlap Triggers also use one-shot behavior.

Returning to a previously completed Trigger does not repeatedly reopen the same Tutorial.

---

# 14. Tutorial Instruction Trigger

A generic no-Tick Actor was created:

```text
ATutorialInstructionTrigger
```

Its role:

```text
Pawn overlap
→ validate local AWCCharacter
→ request Tutorial Instruction
→ disable repeat Trigger after successful open
```

The Trigger contains data for:

```text
InstructionID
Title
Body
one-shot behavior
```

This allows CR01, CR02, and CR03 to share one implementation rather than creating room-specific tutorial classes.

---

# 15. Tutorial HUD Responsibility Simplification

Before Day2:

```text
WBP_TutorialHUD
→ Fragment Progress
→ timed Move / Look hint
→ timed Interact hint
```

After Day2:

```text
WBP_TutorialHUD
→ Fragment Progress only
```

The old transient hint path is no longer active in the player-facing Tutorial flow.

Recorded implementation:

```text
ShowTimedHint()
→ retained as compatibility no-op

TutorialHintText
→ collapsed

HintBackground
→ collapsed
```

The Bootstrap no longer launches the old Move / Look timed hint.

Fragment01 overlap no longer launches the old Interact timed hint.

Normal world interaction prompts remain available after the paused Instruction is closed.

---

# 16. Movement Instruction

Tutorial start now opens:

```text
Tutorial.Instruction.MoveLook
```

Content:

```text
Movement

WASD — Move
Mouse — Look
```

The Instruction pauses the world and remains until:

```text
E
or
Close
```

The developer manually confirmed:

```text
Pause:
PASS

E Close:
PASS

Control restoration:
PASS
```

---

# 17. Interaction Instruction

A Tutorial Trigger is placed before Fragment01.

Content:

```text
Interaction

E — Interact
```

The player flow is now:

```text
approach Fragment01
↓
Interaction Instruction
↓
Pause
↓
E / Close
↓
Resume
↓
normal [E] world prompt
↓
press E again
↓
read Fragment01
```

The previous timed Interact tutorial hint is disabled.

---

# 18. Combat Room 01 Instruction

CR01 now teaches the player's existing basic attacks.

Instruction ID:

```text
Tutorial.Instruction.LightHeavy
```

Content:

```text
Basic Attacks

Left Mouse Button — Light Attack
Right Mouse Button — Heavy Attack

Repeated Light Attacks form a combo.
```

The Trigger is positioned before the enemy becomes the intended active combat focus.

The Tutorial pauses before combat begins, allowing the player to read without being attacked.

---

# 19. CR01 Basic Enemy

CR01 contains one basic Tutorial enemy.

Final Actor:

```text
TutorialEnemy_CR01_Basic
```

Final Blueprint:

```text
/Game/WangChuan/Blueprints/Enemies/BP_GhostEnemy_New
```

AI Controller:

```text
WCGhostAIController
```

Final approximate position:

```text
(3080, -370, 100)
```

Facing:

```text
Yaw ≈ 180°
```

Tutorial-basic tuning retained:

```text
MaxHealth = 100
MoveSpeed = 150
```

Existing Sight / Nav AI tuning was not changed.

---

# 20. Initial Tutorial Enemy Configuration Problem

The first Day2 implementation used:

```text
BP_GhostEnemy
```

for both Tutorial enemies.

A later audit found that the current legacy Blueprint had:

```text
no Skeletal Mesh
no Animation Class
```

on its Enemy Mesh configuration.

This made it an unsuitable Tutorial gameplay enemy.

The project development baseline identified the currently working enemy setup as:

```text
BP_GhostEnemy_New
+
SKM_Quinn
+
ABP_GhostEnemy_Manny
```

The two Tutorial enemy instances were therefore corrected.

This was an important integration fix rather than a cosmetic change.

---

# 21. Tutorial Enemy Corrective Pass

Both Tutorial enemies were replaced with:

```text
BP_GhostEnemy_New
```

while preserving the intended room locations.

## CR01

```text
XY:
(3080, -370)

Z:
100
```

## CR02

```text
XY:
(3070, -4720)

Z:
-160
```

The capsule centers were aligned to the final thickened Tutorial floors.

Recorded runtime ground gaps:

```text
CR01:
0.025 cm

CR02:
0.0 cm
```

Both final enemies load:

```text
SKM_Quinn
ABP_GhostEnemy_Manny
```

and remain possessed by:

```text
WCGhostAIController
```

with Pawn collision enabled.

No Ghost AI parameter tuning was performed during this corrective pass.

---

# 22. CR01 Enemy Defeat → Fragment02

Day2 formally connects the first combat completion to Tutorial Fragment02.

A Tutorial-only link Actor was created:

```text
ATutorialEnemyDefeatFragmentLink
```

Saved Actor:

```text
Tutorial_CR01_DefeatToFragment02
```

References:

```text
RequiredEnemy:
TutorialEnemy_CR01_Basic

RewardFragment:
TutorialFragment02
```

Flow:

```text
BeginPlay
↓
validate references
↓
bind AGhostEnemy::OnEnemyDefeated
↓
CR01 enemy defeated
↓
SetFragmentAvailable(true)
↓
unbind one-shot link
```

No Tick is used.

---

# 23. Fragment02 Remains Tutorial-Only

Fragment02 final saved data remains:

```text
FragmentID:
Tutorial.Fragment02

StartAvailable:
false

Location:
approximately (3000, -1250, 0)
```

Enemy defeat does not:

```text
auto collect Fragment02
auto open Fragment UI
auto increase progress
write Journal
write SaveGame
trigger Story Objective
```

It only makes the existing Tutorial Fragment available.

The intended flow is:

```text
CR01 enemy dies
↓
Fragment02 appears
↓
player approaches
↓
E
↓
read
↓
Fragment progress increases
```

The developer manually confirmed this gameplay flow.

---

# 24. CR02 Lock-On Instruction

CR02 teaches the existing Lock-On system.

Instruction ID:

```text
Tutorial.Instruction.LockOn
```

Content:

```text
Lock-On

Middle Mouse Button — Lock On / Release

Move while locked on to circle the target.
```

The Trigger is positioned on the CR02 approach before the player is expected to begin the Lock-On exercise.

---

# 25. CR02 Basic Enemy

CR02 contains one basic Tutorial enemy.

Final Actor:

```text
TutorialEnemy_CR02_Basic
```

Final Blueprint:

```text
/Game/WangChuan/Blueprints/Enemies/BP_GhostEnemy_New
```

AI Controller:

```text
WCGhostAIController
```

Approximate final position:

```text
(3070, -4720, -160)
```

Facing:

```text
Yaw ≈ 90°
```

Tutorial-basic tuning:

```text
MaxHealth = 100
MoveSpeed = 150
```

No additional reward or progression binding is attached to the CR02 enemy.

Its defeat does not unlock:

```text
Fragment03
Puzzle
Boss
Exit
```

---

# 26. CR03 Dodge / Roll Placeholder

CR03 remains a future combat-teaching space.

Instruction ID:

```text
Tutorial.Instruction.DodgePlaceholder
```

Content explicitly states:

```text
Dodge / Roll

Space — Dodge / Roll (planned)

Current build:
Space is still Jump.
```

This intentionally avoids presenting an unimplemented action as a working mechanic.

Day2 did not create:

```text
IA_Dodge
Dodge binding
Roll montage
Jump replacement
fake roll
```

Final current input remains:

```text
Space = Jump
```

---

# 27. CR03 Enemy Policy

Final saved CR03 enemy count:

```text
0
```

This is intentional.

Because Dodge / Roll is not implemented, the player is not forced to complete a combat challenge that assumes a missing mechanic.

The existing future combat marker remains available for later development.

---

# 28. Puzzle Instruction Policy

No additional Tutorial popup was added for the Lantern Puzzle.

Reason:

```text
E Interaction
```

has already been taught before Fragment01.

The Puzzle is expected to teach itself through its presentation and interaction behavior rather than another control instruction.

Final:

```text
Puzzle Tutorial Instruction:
NONE
```

---

# 29. Existing Ghost AI Preserved

Day2 reused the existing validated Enemy AI stack.

No changes were made to:

```text
SightRadius
LoseSightRadius
PeripheralVision
Last Seen
Return Home
Leash
Damage Aggro
Pathfinding
Enemy Personality
```

Both Tutorial enemies use the existing:

```text
WCGhostAIController
```

The Day2 task did not introduce Behavior Tree, EQS, Hearing, Group Alert, or new AI architecture.

---

# 30. Build Validation

Final build:

```text
Target:
WangChuanEditor

Platform:
Win64

Configuration:
Development
```

Result:

```text
Succeeded
```

Build status:

```text
PASS
```

The final build was run after temporary editor-only hierarchy-generation helper code was removed.

No temporary hierarchy-generation helper remains in formal `Source/`.

---

# 31. MapCheck

Saved editor world:

```text
0 Errors
0 Warnings
```

PIE duplicated-world MapCheck separately reported:

```text
0 Errors
6 same-location warnings
```

The six PIE warnings refer to existing paired Boss Gate / Boss Arena frame pieces at shared coordinates.

They do not reference new Day2 Tutorial actors.

Saved-map MapCheck result:

```text
PASS
```

---

# 32. Static Configuration Audit

Recorded Day2 static audit:

```text
Actor Count:
142

Baseline:
135

Day2 additions:
4 instruction triggers
2 enemies
1 defeat link
```

Confirmed:

```text
Instruction assets exist:
YES

Placed trigger IDs unique:
YES

All placed triggers one-shot:
YES

CR01 Enemy:
1

CR02 Enemy:
1

CR03 Enemy:
0

Fragment02 StartAvailable:
false

Fragment02 link:
CR01 enemy → TutorialFragment02

Fragment03 StartAvailable:
false

Fragment03 link:
none

Puzzle instruction trigger:
0

Bootstrap instruction widget reference:
valid
```

---

# 33. Human Gameplay Validation

Codex intentionally left the core combat behaviors for developer validation.

The developer subsequently completed the full Day2 manual test and reported all items as passed.

Final human gameplay results:

```text
Lock-On:
PASS

Light Combo:
PASS

Heavy Attack:
PASS

Hit Reaction:
PASS

Player Damage:
PASS

Enemy Death:
PASS
```

The developer also reported that all Day2 manual Tutorial tests passed.

This closes the six `PENDING HUMAN VALIDATION` items from the Codex Result.

---

# 34. Final Tutorial Flow Through Day2

The current Tutorial opening flow is now:

```text
Spawn
↓
Movement Instruction
World Pause
↓
E / Close
↓
Fragment Progress 0 / 3
↓
Interaction Instruction
World Pause
↓
E / Close
↓
Fragment01
Read
↓
Progress 1 / 3
↓
CR01 Basic Attacks Instruction
World Pause
↓
E / Close
↓
Light Combo / Heavy practice
↓
CR01 Enemy defeated
↓
Fragment02 becomes available
↓
Fragment02 read
↓
Progress 2 / 3
↓
Downhill
↓
CR02 Lock-On Instruction
World Pause
↓
E / Close
↓
Lock-On / Strafe practice
↓
Uphill
↓
CR03 Dodge / Roll Placeholder
World Pause
↓
E / Close
↓
Space remains Jump
↓
Puzzle section
```

This provides a complete first-pass onboarding for the currently implemented movement, interaction, attack, and Lock-On mechanics.

---

# 35. Formal-System Isolation

Day2 did not connect Tutorial combat to the formal Story / Case systems.

Confirmed:

```text
Story Objective:
NONE

Story Encounter:
NONE

Echo Relic:
NONE

RecordedMemoryEchoes:
NONE

Memory Journal:
NONE

SaveGame / Persistence:
NONE

Formal Case:
NONE

AI Judge:
NONE

Real AI Calls:
0
```

Fragment02 remains a Tutorial reward rather than a formal Case Fragment.

---

# 36. Scope Protection

Confirmed not implemented:

```text
Dodge / Roll gameplay
Fragment03 Puzzle binding
Combat gates
room locking
Boss tutorial
Boss logic
Exit transition
Land Temple transition
Ghost AI tuning
new Enemy AI architecture
Persistence changes
AI Judgement changes
```

---

# 37. Result Package Evidence Note

The Result Package contains a strong written implementation and validation record, plus final Tutorial screenshots.

Two screenshot filenames do not fully match what the image visibly proves:

```text
12_Day2_Instruction_Widget_Hierarchy.png
```

shows the runtime Dodge / Roll Instruction rather than the Blueprint Designer hierarchy.

```text
05_Day2_Fragment02_Link_Config.png
```

shows the map / Actor placement view rather than a detailed reference-properties panel.

This is a Result-package evidence-labeling issue, not a gameplay blocker.

The developer's complete manual Day2 validation passed, and the Result's static configuration audit records the required references.

Future Result Packages should ensure that screenshot names accurately describe the visible evidence.

---

# 38. Worktree / Temporary Artifact Note

The Day2 Result records pre-existing and earlier uncommitted work in the repository.

It also records local Codex verification artifacts under:

```text
Saved/Codex/Week9Day2/
```

including audit / capture scripts and JSON files.

These are development verification artifacts.

They are not formal game assets and should not be committed to GitHub.

---

# 39. Second Technical Review

The uploaded Day2 Result Package was reviewed after the developer reported full manual PASS.

Reviewed:

```text
Week9Day2_ThreeRoomCombatTutorial_Result.md

15 screenshots

Tutorial Instruction architecture

Pause ownership

E close / input-consumption design

modal cleanup

CR01 / CR02 enemy configuration

Tutorial enemy corrective pass

Fragment02 defeat-link design

CR03 zero-enemy policy

Build

MapCheck

static configuration audit

scope-boundary declarations
```

The package is internally consistent on the core implementation.

The corrected Tutorial enemies use the current validated enemy Blueprint and preserve the existing AI baseline.

No blocking architecture or scope issue was identified.

Second technical review:

```text
PASS
```

---

# 40. Day2 Completion Summary

Completed:

- unified paused Tutorial Instruction system;
- Blueprint-editable Instruction Widget;
- real world pause;
- pause ownership protection;
- E close while paused;
- E input-consumption protection;
- Close Button common path;
- death-safe modal cleanup;
- modal exclusivity;
- one-shot Instruction IDs;
- generic Tutorial Instruction Trigger;
- HUD reduced to Fragment progress;
- Movement Instruction;
- Interaction Instruction;
- CR01 Basic Attacks Instruction;
- CR01 basic enemy;
- CR01 enemy correction to `BP_GhostEnemy_New`;
- Light Combo manual validation;
- Heavy Attack manual validation;
- Hit Reaction manual validation;
- Player Damage manual validation;
- Enemy Death manual validation;
- CR01 Enemy Defeat → Fragment02;
- Fragment02 `1 / 3 → 2 / 3` Tutorial flow;
- CR02 Lock-On Instruction;
- CR02 basic enemy;
- CR02 enemy correction to `BP_GhostEnemy_New`;
- Lock-On manual validation;
- CR03 Dodge / Roll placeholder;
- Space remains Jump;
- CR03 enemy count remains zero;
- no redundant Puzzle tutorial;
- final Editor build;
- final saved-world MapCheck;
- static actor / reference audit;
- complete human Day2 gameplay review;
- second technical review.

---

# 41. Deferred to Next Stage

The Tutorial Dungeon's next major gameplay section is the Lantern Puzzle.

Still deferred:

```text
Five-Lantern Puzzle integration
Fragment03 Puzzle Complete binding
Boss Gate opening
Tutorial Mini-Boss
Exit Gate
Land Temple transition
Dodge / Roll implementation
CR03 real Dodge tutorial
```

The known environment issue from the previous layout stage also remains:

```text
Dungeon ceiling not added yet
```

Reason:

```text
internal dungeon lighting design is not yet established
```

Future sequence:

```text
design dungeon lighting
→ validate interior readability
→ add ceiling / roof
```

---

# 42. Final Gate

```text
Codex Day2 Implementation Gate:
READY FOR HUMAN GAMEPLAY REVIEW

Human Gameplay Review:
PASS

Second Technical Review:
PASS
```

Therefore:

```text
WEEK9 DAY2:
COMPLETE
```

The Tutorial Dungeon now has a complete first-pass teaching loop for:

```text
Move
Look
Interact
Light Attack
Heavy Attack
Light Combo
Lock-On
```

with Dodge / Roll clearly reserved for future implementation.
