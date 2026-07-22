# Development Log — Week 5

**Project:** WangChuan / 《忘川河畔》  
**Engine:** Unreal Engine 5.4 → Unreal Engine 5.8  
**Focus:** Story NPC foundation, reusable dialogue UI, NPC relocation, event-driven story encounters, Echo Relics, Memory Echoes, UE5.8 migration, and Codex MCP integration  
**Week Theme:** Narrative Gameplay Foundation — from isolated interaction prototypes to a complete story-driven gameplay loop

---

## Week 5 Goal

Week 5 focused on building the first reusable narrative-gameplay foundation on top of the combat, interaction, UI, and lock-on systems completed during Weeks 1–4.

The original plan was to create a lightweight NPC and encounter pipeline for the first story theme:

```text
The Quiet Child / 安静的孩子
```

The intended flow was:

```text
Meet Little Girl
→ Read Stage 0 Dialogue
→ Explore the encounter branch
→ Defeat the required enemy
→ Unlock a story relic
→ Read a Memory Echo
→ Complete the encounter
→ Relocate Little Girl
→ Read Stage 1 Dialogue
```

The week deliberately avoided building a traditional quest framework. The implementation remained focused on one complete, reusable, and testable story loop rather than introducing a large manager or subsystem.

The Week 5 priority remained:

```text
Completion > Playability > Extensibility > Visual Quality
```

The week intentionally avoided:

- A global Quest Manager or Quest Subsystem
- Dialogue Graphs or branching dialogue trees
- Gameplay Ability System integration
- Behavior Trees or AI Perception rewrites
- Companion AI or NPC navigation
- A complete SaveGame implementation
- A large production Journal UI
- Complex cinematic, dissolve, or teleport effects

The final development time was also used for two controlled extensions:

- Migrating the project from Unreal Engine 5.4 to Unreal Engine 5.8
- Connecting the UE5.8 built-in Unreal MCP server to Codex

---

## Week 5 Result Overview

By the end of Week 5, the project added:

- `StoryTypes.h`
- `EStoryNPCState`
- `FDialogueLine`
- `FDialogueSequence`
- `EEchoRelicState`
- `FMemoryEchoData`
- `AWCStoryNPC`
- `BP_QuietChild`
- `UDialogueWidget`
- `WBP_Dialogue`
- Reusable Conversation Mode in `AWCCharacter`
- `AStoryAnchor`
- `QuietChild_Anchor_01`
- `QuietChild_Anchor_02`
- Story-stage-based NPC relocation
- `MeshFacingYawOffset`
- `FOnGhostEnemyDefeatedSignature`
- `AGhostEnemy::OnEnemyDefeated`
- `AStoryEncounter`
- `BP_QuietChild_Encounter_01`
- Required-enemy instance filtering
- `AEchoRelic`
- `BP_EchoRelic_Bell`
- `FOnEchoRelicActivatedSignature`
- `UMemoryEchoWidget`
- `WBP_MemoryEcho`
- Memory Echo input mode and UI lifecycle
- Runtime Memory Echo Journal records with `EchoID` de-duplication
- `AWCStoryNPC::ReceiveStoryEvent()`
- The complete Quiet Child Encounter 01 gameplay loop
- A successful Unreal Engine 5.4 → 5.8 migration
- UE5.8 Epic Unreal MCP activation
- Codex MCP handshake, tool discovery, and live level read validation
- A final full-flow recording

---

## Day 1 — Story NPC Foundation

### Goal

Create a reusable Story NPC foundation while continuing to use the existing Week 1 interaction system.

The Story NPC layer needed to:

- Work through `IInteractable`
- Reuse `AWCCharacter::CurrentInteractable`
- Reuse `ShowInteractionPrompt()` and `HideInteractionPrompt()`
- Store Story Stage separately from current NPC behavior state
- Read dialogue data from Blueprint
- Avoid embedding Little Girl-specific story logic in the player class

---

### `StoryTypes.h`

Created:

```text
StoryTypes.h
```

The file centralizes lightweight story-related types shared by multiple narrative classes.

No Story Manager, Quest Subsystem, or Dialogue DataAsset was created.

---

### `EStoryNPCState`

Created:

```cpp
EStoryNPCState
```

The enum is exposed as `BlueprintType` and includes:

- `Dormant`
- `Available`
- `WaitingForEvent`
- `EventResolved`
- `Relocating`
- `ChapterComplete`

The system separates two concepts:

```text
CurrentStoryStage
= which narrative stage the NPC has reached

StoryState
= what the NPC is currently doing inside that stage
```

This separation became important later when the NPC temporarily entered `EventResolved` and `Relocating` while advancing from Stage 0 to Stage 1.

---

### `FDialogueLine`

Created:

```cpp
FDialogueLine
```

Fields:

- `SpeakerName` — `FText`
- `DialogueText` — `FText`

`DialogueText` supports multiline editing in Blueprint.

Keeping `SpeakerName` on each line allows a single sequence to support multiple speakers later without changing the base data model.

---

### `FDialogueSequence`

Created:

```cpp
FDialogueSequence
```

The structure contains:

```cpp
TArray<FDialogueLine> Lines
```

Each sequence represents the ordered dialogue for one Story Stage.

The first implementation used:

```text
DialogueByStage[0]
= first encounter dialogue
```

The same array was later expanded for Stage 1.

---

### `AWCStoryNPC`

Created:

```cpp
AWCStoryNPC
```

The class:

- Inherits from `AActor`
- Implements `IInteractable`
- Serves as a reusable C++ base for lightweight story NPCs
- Does not contain Little Girl-specific enemy, relic, or encounter logic
- Does not use Tick

Components:

- `SceneRoot`
- `NPCMesh` — `USkeletalMeshComponent`
- `InteractionSphere` — `USphereComponent`

The interaction sphere:

- Uses Query Only collision
- Ignores unrelated channels
- Overlaps Pawn
- Uses a default radius of approximately 200 cm
- Does not block player movement

---

### Story NPC Identity and State

Added:

- `NPCDisplayName`
- `InteractionPrompt`
- `CurrentStoryStage`
- `StoryState`
- `DialogueByStage`

Initial defaults:

```text
CurrentStoryStage = 0
StoryState = Available
InteractionPrompt = [E] Approach
```

`NPCDisplayName` uses `FText` for UI display.

`InteractionPrompt` remains an `FString` to match the existing `IInteractable::GetInteractionPrompt()` interface.

---

### Dialogue Accessors

Added:

```cpp
GetCurrentDialogueSequence()
GetNPCDisplayName()
```

`GetCurrentDialogueSequence()` uses `IsValidIndex()` before accessing `DialogueByStage`.

Invalid or missing dialogue configuration returns an empty sequence instead of risking an array access error.

---

### Existing Interaction Integration

`AWCStoryNPC` implements:

```cpp
Interact()
GetInteractionPrompt()
```

`OnPlayerEnter()`:

- Casts the overlapping Actor to `AWCCharacter`
- Assigns the NPC to `Player->CurrentInteractable`
- Shows the NPC interaction prompt

`OnPlayerExit()`:

- Clears the interaction only when `CurrentInteractable == this`
- Hides the prompt

The ownership check prevents one overlapping interactable from clearing another interactable's prompt.

---

### Temporary Dialogue Debug

The Day 1 version of `Interact()` temporarily displayed all current dialogue lines through an on-screen debug message.

This verified the complete data path:

```text
BP_QuietChild DialogueByStage
→ AWCStoryNPC
→ CurrentStoryStage
→ FDialogueSequence
→ FDialogueLine
→ On-screen output
```

If the current Story Stage had no dialogue data, the system displayed a configuration warning instead of crashing.

The temporary debug display was removed after the formal Dialogue UI was completed.

---

### `BP_QuietChild`

Created:

```text
BP_QuietChild
```

Parent class:

```text
AWCStoryNPC
```

The story theme remains:

```text
The Quiet Child / 安静的孩子
```

This is the encounter or story-theme title, not the NPC's personal name.

The temporary NPC display name is:

```text
Little Girl
```

`DialogueByStage[0]` was configured with short English test dialogue designed to provide direction without presenting a conventional explicit quest objective.

---

### Day 1 Result

At the end of Day 1:

- A reusable Story NPC base class existed
- Story Stage and Story State were separated
- Blueprint dialogue data could be read safely by C++
- `BP_QuietChild` could participate in the existing interaction system
- The old `InteractionStone` workflow remained functional
- Missing dialogue data was handled safely
- No large quest or story-management framework was introduced

---

## Day 2 — Dialogue UI and Conversation Mode

### Goal

Replace the Day 1 debug output with a reusable Dialogue UI and add a dedicated Conversation Mode to `AWCCharacter`.

The system needed to:

- Display speaker and dialogue text
- Advance through multiple lines
- Support Continue and Close buttons
- Support E-key progression
- Block gameplay input while dialogue is active
- Restore all input correctly when dialogue ends
- Close safely if the player dies

---

### Build Dependencies

Confirmed and added the required UI dependencies in `WangChuan.Build.cs`:

- `UMG`
- `Slate`
- `SlateCore`

These support Widget creation, focus management, and input-mode changes.

---

### `UDialogueWidget`

Created:

```cpp
UDialogueWidget
```

Parent class:

```text
UUserWidget
```

Responsibilities:

- Store the active dialogue sequence
- Store the current line index
- Store the owning Story NPC
- Store the owning player
- Display the current line
- Advance to the next line
- Request dialogue closure

The Widget does not:

- Progress Story Stages
- Evaluate quest conditions
- Control enemies or relics
- Own story encounter state

---

### Dialogue Widget Bindings

Added `BindWidget` references:

- `SpeakerNameText`
- `DialogueText`
- `ContinueButton`
- `ContinueButtonLabel`
- `CloseButton`

`NativeConstruct()` uses `AddUniqueDynamic` to bind button events once.

The Widget Blueprint Event Graph does not duplicate the same button handlers.

---

### Dialogue Runtime Data

Added:

- `ActiveDialogueSequence`
- `StoryNPCOwner`
- `PlayerOwner`
- `CurrentLineIndex`

`StartDialogue()`:

- Stores the sequence and Actor references
- Resets the line index to `0`
- Displays the first line
- Requests closure if the sequence is empty

`DisplayCurrentLine()`:

- Checks the current index
- Displays the configured speaker name and dialogue text
- Falls back to `NPCDisplayName` when `SpeakerName` is empty
- Changes the Continue label to `Close` on the final line

`AdvanceDialogue()`:

- Advances one line at a time
- Closes after the final line
- Avoids out-of-range access

`RequestCloseDialogue()` delegates final UI and player-state cleanup to `AWCCharacter::EndDialogue()`.

---

### `AWCCharacter` Dialogue API

Added:

```cpp
StartDialogue()
EndDialogue()
GetIsInDialogue()
```

State and references:

- `bIsInDialogue`
- `DialogueWidgetClass`
- `ActiveDialogueWidget`
- `ActiveDialogueNPC`

`StartDialogue()` validates:

- Player is alive
- No dialogue is already active
- NPC reference is valid
- Dialogue contains lines
- `DialogueWidgetClass` is assigned
- Widget creation succeeds

Only after all validation passes does the player enter Conversation Mode.

---

### Conversation Mode

When dialogue starts:

- `bIsInDialogue` becomes `true`
- Current movement stops
- Character velocity is cleared
- Jumping stops
- Interaction prompt is hidden
- Combat state is exited
- Lock-On is cleared
- The Dialogue Widget is added to the viewport
- Mouse cursor becomes visible

Input mode:

```text
FInputModeGameAndUI
```

`GameAndUI` was used instead of `UIOnly` so the existing Enhanced Input E action could continue reaching `AWCCharacter::Interact()` and advance dialogue.

The player controller ignores move and look input while dialogue is active.

---

### Input Context Changes

`AWCCharacter::Interact()` now has separate behavior:

```text
Dialogue active
→ E advances the active Dialogue Widget

Normal gameplay
→ E interacts with CurrentInteractable
```

This prevents every E press from creating another Widget.

`CanAct()` was extended to reject actions during dialogue.

The dialogue state blocks:

- Movement
- Mouse look
- Jump
- Light Attack
- Heavy Attack
- Lock-On
- Journal display
- Combo and attack-buffer actions that use `CanAct()`

Jump input was routed through `HandleJumpStarted()` and `HandleJumpCompleted()` rather than binding directly to `ACharacter::Jump`, allowing the same action-state validation to apply.

---

### Dialogue Closure and Prompt Recovery

`EndDialogue()`:

- Removes the Widget
- Clears active Widget and NPC references
- Resets `bIsInDialogue`
- Hides the cursor
- Restores `GameOnly` input mode
- Restores movement and look input when the player is alive

The previous NPC reference is retained long enough to restore the interaction prompt when:

```text
Player is still inside the NPC interaction range
AND
CurrentInteractable still points to that NPC
```

This prevents `[E] Approach` from disappearing permanently after a conversation.

---

### Player Death Integration

Player death closes any active dialogue.

The sequence ensures:

- Dialogue UI is removed
- Mouse cursor is hidden
- Dialogue state does not remain active
- Input is not restored after `bIsDead` becomes true
- Existing Disable Input, movement disable, ragdoll, and Defeated UI behavior continues normally

---

### `WBP_Dialogue`

Created:

```text
Content/WangChuan/UI/Story/WBP_Dialogue
```

Parent class:

```text
UDialogueWidget
```

The Widget includes:

- A dark dialogue background
- Speaker name text
- Wrapped dialogue text
- `ContinueSizeBox`
- `ContinueButton`
- `ContinueButtonLabel`
- `CloseSizeBox`
- `CloseButton`

The Size Boxes control button dimensions without changing the required C++ Widget names.

The UI is positioned near the lower portion of the screen and remains intentionally simple.

---

### Day 2 Result

At the end of Day 2:

- Dialogue debug output was replaced by a reusable formal UI
- Multi-line dialogue could be advanced by button or E key
- Dialogue could be closed normally or early
- Conversation Mode safely blocked gameplay actions
- Lock-On and combat state were cleared when dialogue began
- Full player control returned after closure
- Prompt recovery worked
- Player death safely closed the UI
- Existing interaction, combat, health, feedback, and lock-on systems remained compatible

---

## Day 3 — Story Anchor and NPC Relocation Prototype

### Goal

Create a lightweight location-based Story NPC progression system without implementing walking, NavMesh movement, AI Move To, or companion behavior.

The selected approach was:

```text
Hide NPC
→ Disable interaction
→ Teleport to Story Anchor
→ Wait for a short reveal delay
→ Apply new Story Stage
→ Show NPC
→ Restore interaction
```

---

### `AStoryAnchor`

Created:

```cpp
AStoryAnchor
```

Components:

- `SceneRoot`
- `FacingArrow`

The Actor:

- Does not Tick
- Does not participate in collision
- Hides its arrow during gameplay
- Stores location and rotation only
- Does not move NPCs itself
- Does not progress story state
- Does not process dialogue, enemies, or relics

Added:

```cpp
AnchorID
GetAnchorTransform()
GetAnchorID()
```

The relocation system copies Anchor Location and Rotation but does not copy Scale.

---

### Quiet Child Anchors

Placed:

```text
QuietChild_Anchor_01
QuietChild_Anchor_02
```

Configured IDs:

```text
QuietChild.Anchor01
QuietChild.Anchor02
```

Anchor 01 represents Little Girl's first location.

Anchor 02 represents the later story location after Encounter completion.

`FacingArrow` indicates the final intended visual facing direction.

---

### Story Anchor References

Added to `AWCStoryNPC`:

```cpp
TArray<AStoryAnchor*> StoryAnchors
```

The array uses `EditInstanceOnly` because it references Actors placed in the current level.

The map instance of `BP_QuietChild` was configured with:

```text
StoryAnchors[0] = QuietChild_Anchor_01
StoryAnchors[1] = QuietChild_Anchor_02
```

These references cannot be assigned as level Actors in the Blueprint Class Defaults.

---

### Relocation State and Data

Added:

- `RelocationRevealDelay`
- `PendingStoryStage`
- `RelocationTimerHandle`

`PendingStoryStage` remains separate until relocation completes, preventing the NPC from exposing Stage 1 dialogue while still hidden or transitioning.

Timers are cleared before and after relocation to prevent duplicate reveal callbacks.

---

### Relocation API

Added:

```cpp
RelocateToStoryAnchor()
RelocateToStoryAnchorByIndex()
FinishRelocation()
SetStoryNPCInteractionEnabled()
ClearPlayerInteractionIfNeeded()
```

`RelocateToStoryAnchor()` rejects:

- Null target Anchor
- Negative Story Stage
- A second request while already `Relocating`
- Relocation while the NPC owns an active dialogue

A missing dialogue sequence for the destination stage generates a warning but does not block location testing.

`RelocateToStoryAnchorByIndex()` validates the array index and reuses the main relocation function.

---

### Interaction and State Protection

At relocation start:

- `StoryState` becomes `Relocating`
- Existing player interaction with the NPC is cleared
- Prompt is hidden
- Actor collision is disabled
- Sphere overlap events are disabled
- Sphere collision becomes `NoCollision`
- NPC becomes hidden

While `StoryState != Available`:

- `GetInteractionPrompt()` returns an empty prompt
- `Interact()` rejects interaction
- `OnPlayerEnter()` does not assign the NPC as `CurrentInteractable`

At relocation completion:

- `CurrentStoryStage` receives `PendingStoryStage`
- `PendingStoryStage` returns to `INDEX_NONE`
- NPC becomes visible
- Collision and overlap events are restored
- `StoryState` returns to `Available`

---

### Stage 0 and Stage 1 Dialogue

`DialogueByStage` was expanded to include:

```text
DialogueByStage[0]
= Anchor 01 / first meeting

DialogueByStage[1]
= Anchor 02 / later encounter dialogue
```

The final Story Stage changes only when `FinishRelocation()` executes.

The same `WBP_Dialogue` is reused for both stages.

---

### Temporary Relocation Test Controls

Temporary Level Blueprint input was used to validate bidirectional relocation:

```text
R
→ AnchorIndex 1
→ Story Stage 1

T
→ AnchorIndex 0
→ Story Stage 0
```

These nodes were development-only and were removed or disconnected before formal Story Encounter integration.

---

### Problem: NPC Facing Did Not Match the Anchor

Initial relocation correctly updated:

- Location
- Story State
- Current Story Stage
- Interaction availability

However, Little Girl's visible face direction did not align with the Anchor's `FacingArrow`.

The offset remained consistent across different Anchor rotations, indicating a model-forward mismatch rather than an individual Anchor configuration problem.

---

### Cause

Unreal Actor forward direction:

```text
+X
```

Quiet Child model visual forward direction:

```text
+Y
```

The visual Mesh therefore differed from Actor forward by approximately 90 degrees.

The original relocation code applied the Anchor rotation directly to the Actor but did not compensate for the model's visual-forward axis.

---

### Solution: `MeshFacingYawOffset`

Added to `AWCStoryNPC`:

```cpp
float MeshFacingYawOffset = 0.0f;
```

The destination Actor rotation is now calculated as:

```cpp
FRotator TargetActorRotation = TargetTransform.Rotator();

TargetActorRotation.Yaw =
    FRotator::NormalizeAxis(
        TargetActorRotation.Yaw - MeshFacingYawOffset
    );
```

`BP_QuietChild` configuration:

```text
MeshFacingYawOffset = 90
NPCMesh Relative Rotation = (0, 0, 0)
```

Example:

```text
Anchor Yaw = 240°
Actor Yaw = 240° - 90° = 150°
Model visual offset = +90°
Final visible facing = 240°
```

This keeps the responsibilities clean:

```text
Story Anchor
= intended world-facing direction

Story NPC
= model-specific visual-forward compensation
```

The same Anchor can therefore be reused by NPC models with different forward-axis conventions.

---

### Day 3 Result

At the end of Day 3:

- Story Anchors were reusable and editor-readable
- Little Girl could relocate in both directions
- Story Stage changed only after relocation completed
- Old prompts and interaction references were cleaned safely
- New-location interaction restored correctly
- Dialogue could not be invalidated by relocation during Conversation Mode
- Invalid index, null Anchor, and repeated relocation requests were safely rejected
- Actor/mesh forward mismatch was solved through `MeshFacingYawOffset = 90`
- No NPC walking or navigation system was required

---

## Day 4 — Enemy Event and Story Encounter

### Goal

Connect the existing enemy-death flow to narrative state through an event-driven Story Encounter.

The architecture needed to preserve clear ownership:

```text
AGhostEnemy
→ broadcasts its own death

AStoryEncounter
→ listens to one configured Required Enemy
→ stores encounter progress

AWCStoryNPC
→ remains independent of enemy implementation
```

Enemy death was not allowed to complete the entire encounter or relocate Little Girl immediately.

---

### Ghost Enemy Defeated Delegate

Created:

```cpp
FOnGhostEnemyDefeatedSignature
```

Delegate parameter:

```cpp
AGhostEnemy* DefeatedEnemy
```

Added to `AGhostEnemy`:

```cpp
OnEnemyDefeated
```

The event is `BlueprintAssignable` and can be bound from C++ or Blueprint.

`AGhostEnemy` does not know which systems are listening.

---

### Death Broadcast Timing

`AGhostEnemy::Die()` gained an internal repeated-call guard:

```cpp
if (bIsDead)
{
    return;
}
```

The event is broadcast once after the enemy has entered a stable death state:

- `bIsDead` is true
- Movement and attack behavior are stopped
- Combat timers are cleared
- Health Widget is hidden
- Collision is disabled

Then:

```cpp
OnEnemyDefeated.Broadcast(this);
```

The normal death-delay flow continues afterward.

The event is not broadcast from:

- `TakeHit()`
- `FinishDeath()`
- The death timer
- Blueprint animation logic

This preserves one authoritative event source.

---

### Existing Enemy Death Compatibility

Adding the delegate did not replace the existing death lifecycle.

The enemy still:

- Stops behavior
- Hides the health bar
- Disables collision
- Plays the existing death state or animation
- Waits for `DeathDestroyDelay`
- Executes `FinishDeath()`
- Destroys normally

The narrative event is an additional notification, not a new death implementation.

---

### `AStoryEncounter`

Created:

```cpp
AStoryEncounter
```

The Actor:

- Does not Tick
- Does not collide
- Has a `SceneRoot`
- Exists as an invisible level coordinator
- Does not implement player interaction

Added identity:

```text
EncounterID = QuietChild.Encounter01
```

---

### Encounter References

Added `EditInstanceOnly` references:

- `RequiredEnemy` — `AGhostEnemy*`
- `StoryNPC` — `AWCStoryNPC*`
- `NextStoryAnchor` — `AStoryAnchor*`

The current level instance was configured with:

```text
RequiredEnemy = designated BP_GhostEnemy_New instance
StoryNPC = BP_QuietChild level instance
NextStoryAnchor = QuietChild_Anchor_02
```

The Required Enemy uses the existing enemy class and combat implementation; no special enemy subclass was needed.

---

### Encounter Progress State

Added:

```cpp
bRequiredEnemyDefeated
bEncounterCompleted
```

The states intentionally represent separate milestones:

```text
Before required enemy death:
false / false

After required enemy death:
true / false

After later Echo activation:
true / true
```

Added Blueprint-pure getters:

```cpp
GetIsRequiredEnemyDefeated()
GetIsEncounterCompleted()
```

State can be read but is not exposed for arbitrary Blueprint writes.

---

### Event Binding and Lifetime

`BeginPlay()`:

- Validates `RequiredEnemy`
- Binds with `AddUniqueDynamic`
- Validates future Story NPC and Anchor references
- Displays optional debug messages

`EndPlay()`:

- Removes the dynamic binding when the enemy is still valid
- Calls `Super::EndPlay()` afterward

The Encounter does not poll enemy state every frame.

---

### Required Enemy Filtering

Added:

```cpp
HandleRequiredEnemyDefeated(AGhostEnemy* DefeatedEnemy)
```

The handler rejects:

- Null enemy references
- Enemies other than the configured `RequiredEnemy`
- Repeated processing after `bRequiredEnemyDefeated` is true

This creates two levels of repeat protection:

```text
Enemy bIsDead
→ prevents repeated broadcast

Encounter bRequiredEnemyDefeated
→ prevents repeated story processing
```

Ordinary enemies can die normally without progressing the encounter.

---

### Encounter Completion Boundary

Required Enemy death only performs:

```text
bRequiredEnemyDefeated = true
```

It does not:

- Set `bEncounterCompleted`
- Move Little Girl
- Change `CurrentStoryStage`
- Activate a relic automatically
- Display Memory Echo UI
- Record Journal content

This preserves player agency and prevents the combat result from skipping the later investigation step.

---

### `BP_QuietChild_Encounter_01`

Created:

```text
BP_QuietChild_Encounter_01
```

Parent class:

```text
AStoryEncounter
```

The level also includes one clearly designated Required Enemy and at least one ordinary enemy for instance-filtering validation.

Temporary Day 3 R/T relocation controls were removed before formal encounter testing.

---

### Day 4 Result

At the end of Day 4:

- Combat and story were connected through a formal event
- Enemy death remained independent of Story Encounter existence
- Only the configured enemy progressed the encounter
- Required Enemy death produced `true / false`, not full completion
- Enemy Actor destruction did not erase stored encounter progress
- Story NPC and Anchor references were prepared but not used prematurely
- Existing combat, lock-on, dialogue, interaction, and enemy death behavior remained stable

---

## Day 5 — Echo Relic and Memory Echo

### Goal

Complete the first full narrative gameplay loop by adding a world relic, two-page Memory Echo UI, Journal record, Story Event delivery, Encounter completion, and NPC relocation.

The completed flow became:

```text
Meet Little Girl at Anchor 01
→ Read Stage 0 Dialogue
→ Find the silent bell
→ Defeat the Required Enemy
→ Bell becomes Available
→ Read the Memory Echo
→ Record the Echo
→ Complete the Encounter
→ Send Story Event to Little Girl
→ Relocate Little Girl to Anchor 02
→ Read Stage 1 Dialogue
```

---

### `EEchoRelicState`

Added to `StoryTypes.h`:

```cpp
EEchoRelicState
```

States:

- `Locked`
- `Available`
- `Activated`

Lifecycle:

```text
Locked
→ Available
→ Activated
```

`Locked` means the story condition is not yet satisfied.

`Available` means the player may investigate the Memory Echo.

`Activated` means the Echo has been fully read and confirmed.

---

### `FMemoryEchoData`

Added:

```cpp
FMemoryEchoData
```

Fields:

- `EchoID` — `FName`
- `Title` — `FText`
- `EchoText` — `FText`
- `PlayerResonanceText` — `FText`

`EchoID` is the unique identifier used for Journal de-duplication.

Story content remains configurable in Blueprint rather than hardcoded in `AWCCharacter` or the Widget.

---

### `AEchoRelic`

Created:

```cpp
AEchoRelic
```

The Actor:

- Inherits from `AActor`
- Implements `IInteractable`
- Does not Tick
- Uses `RelicMesh` for visuals
- Uses `InteractionSphere` for player interaction

The first prototype disables collision on the temporary relic Mesh and uses a Query Only sphere that overlaps Pawn.

---

### Relic Interaction States

Configured prompts:

```text
Locked:
[E] Examine

Available:
[E] Listen

Activated:
no prompt
```

Locked interaction displays a short message such as:

```text
The bell is silent.
```

It does not open the Memory Echo or progress story state.

---

### Relic State API

Added:

```cpp
UnlockRelic()
ConfirmEchoRead()
CancelEchoRead()
GetRelicState()
GetMemoryEchoData()
```

`UnlockRelic()`:

- Only succeeds from `Locked`
- Changes state to `Available`
- Refreshes the prompt if the player is already overlapping
- Does not automatically open UI or complete the Encounter

`ConfirmEchoRead()`:

- Requires `Available`
- Requires an active reading session
- Changes state to `Activated`
- Clears interaction references
- Disables the interaction sphere
- Broadcasts the activation event

`CancelEchoRead()`:

- Leaves the relic `Available`
- Clears the in-progress state
- Allows the player to investigate again

---

### Prompt Refresh While Overlapping

Added:

```cpp
RefreshPromptForOverlappingPlayer()
```

If the Required Enemy dies while the player is already inside the relic sphere, the prompt changes from:

```text
[E] Examine
```

to:

```text
[E] Listen
```

without requiring the player to leave and re-enter the volume.

The function does not overwrite a different active interactable.

---

### Relic Activation Event

Created:

```cpp
FOnEchoRelicActivatedSignature
```

Added:

```cpp
OnEchoActivated
```

The event is broadcast only after the player fully reads and confirms the Memory Echo.

The relic does not directly:

- Complete the Encounter
- Move the NPC
- Change Story Stage
- Access a Story Anchor

---

### Activation-in-Progress Protection

Added:

```cpp
bActivationInProgress
```

The flag prevents repeated interaction from creating multiple Memory Echo Widgets during one reading session.

It is cleared after:

- Successful completion
- Player death
- An aborted UI session

Widget or configuration failure leaves the relic in `Available` rather than incorrectly advancing story state.

---

### `UMemoryEchoWidget`

Created:

```cpp
UMemoryEchoWidget
```

Parent class:

```text
UUserWidget
```

The Widget handles a fixed two-page structure:

```text
Page 0
→ EchoText

Page 1
→ PlayerResonanceText
```

Bindings:

- `EchoTitleText`
- `EchoBodyText`
- `EchoContinueButton`
- `EchoContinueButtonLabel`

Runtime data:

- `ActiveEchoData`
- `PlayerOwner`
- `CurrentPageIndex`

Button events use `AddUniqueDynamic` to avoid duplicate progression.

---

### Memory Echo Page Flow

`StartMemoryEcho()`:

- Stores the Echo data
- Stores the player reference
- Resets to Page 0
- Displays the first page

`DisplayCurrentPage()`:

- Keeps the title visible
- Shows `EchoText` on Page 0
- Shows `PlayerResonanceText` on Page 1
- Uses `Continue` on Page 0
- Uses `Close` on Page 1

`AdvanceMemoryEcho()`:

- Advances from Page 0 to Page 1
- Requests completion after Page 1

The Widget asks the player to close the Echo rather than directly changing Relic or Encounter state.

---

### `WBP_MemoryEcho`

Created:

```text
Content/WangChuan/UI/Story/WBP_MemoryEcho
```

Parent class:

```text
UMemoryEchoWidget
```

The UI contains:

- Echo background/border
- Echo title
- Wrapped body text
- Continue/Close button
- Size Box-based button sizing

It remains visually separate from `WBP_Dialogue` while using the same Story UI directory.

---

### `AWCCharacter` Memory Echo Mode

Added:

```cpp
StartMemoryEcho()
EndMemoryEcho()
GetIsViewingMemoryEcho()
RecordMemoryEcho()
HasRecordedMemoryEcho()
```

State and data:

- `MemoryEchoWidgetClass`
- `ActiveMemoryEchoWidget`
- `ActiveEchoRelic`
- `ActiveMemoryEchoData`
- `bIsViewingMemoryEcho`
- `RecordedMemoryEchoes`

`StartMemoryEcho()` validates:

- Player is alive
- Dialogue is not active
- Another Memory Echo is not active
- Player is not attacking
- Source relic is valid
- `EchoID` is valid
- Widget class is assigned
- Player controller exists
- Widget creation succeeds

Only after validation does Memory Echo Mode begin.

---

### Memory Echo Input Mode

When an Echo starts:

- Lock-On is cleared
- Combat state is exited
- Movement stops
- Velocity is cleared
- Jumping stops
- Interaction prompt is hidden
- Widget is added to the viewport
- Cursor is shown
- Move and look input are ignored

Input mode:

```text
FInputModeGameAndUI
```

This preserves E-key progression while still allowing UI button clicks.

`AWCCharacter::Interact()` prioritizes Memory Echo progression before Dialogue progression and normal world interaction.

`CanAct()` was extended to reject gameplay actions while `bIsViewingMemoryEcho` is true.

---

### Memory Echo Completion and Cancellation

`EndMemoryEcho(bool bCompleted)`:

- Stores the previous Relic and Echo data before clearing references
- Removes the Widget
- Clears Memory Echo state
- Restores `GameOnly` input mode
- Restores move/look input when the player is alive

Successful completion:

```text
RecordMemoryEcho()
→ ConfirmEchoRead()
→ OnEchoActivated
```

Cancelled or interrupted reading:

```text
CancelEchoRead()
→ Relic remains Available
→ Encounter remains incomplete
```

---

### Player Death During Memory Echo

`AWCCharacter::Die()` closes an active Memory Echo using:

```cpp
EndMemoryEcho(false)
```

This ensures:

- UI closes
- Cursor hides
- Journal is not updated
- Relic remains `Available`
- `bActivationInProgress` clears
- `OnEchoActivated` is not broadcast
- Encounter remains incomplete
- Little Girl does not relocate
- Death input is not accidentally restored

---

### Memory Echo Journal Record

`RecordedMemoryEchoes` stores runtime records.

`HasRecordedMemoryEcho(EchoID)` checks unique IDs.

`RecordMemoryEcho()` rejects:

- `EchoID == None`
- An EchoID already present in the array

The existing debug Journal was extended rather than replaced.

It retains existing Memory Fragment information and appends:

```text
Memory Echoes
[Echo Title]
Echo Text
```

`PlayerResonanceText` is not required in the first Journal implementation, keeping the temporary output readable.

No SaveGame persistence was added.

---

### `AWCStoryNPC::ReceiveStoryEvent()`

Added:

```cpp
ReceiveStoryEvent(FName EventID)
GetLastReceivedStoryEvent()
```

Added state:

```cpp
LastReceivedStoryEvent
```

A valid Story Event:

- Stores the Event ID
- Moves Story State to `EventResolved` when appropriate
- Leaves relocation responsibility to the Encounter

Little Girl does not need to know whether the event originated from an enemy, relic, or another future story source.

---

### Story Encounter Echo Integration

Added to `AStoryEncounter`:

- `EchoRelic`
- `CompletionStoryEventID`
- `NextStoryStage`
- `HandleEchoRelicActivated()`

Current configuration:

```text
EchoRelic = QuietChild_BellEcho_01
CompletionStoryEventID = QuietChild.BellEchoActivated
NextStoryStage = 1
```

`BeginPlay()` binds both:

- Required Enemy defeat event
- Echo Relic activation event

`EndPlay()` removes both bindings.

---

### Required Enemy Handler Extension

Required Enemy death now performs:

```text
bRequiredEnemyDefeated = true
→ EchoRelic->UnlockRelic()
```

It still does not complete the Encounter or relocate Little Girl.

---

### Encounter Completion Handler

`HandleEchoRelicActivated()` validates:

- Encounter is not already complete
- Required Enemy has been defeated
- Activated relic is valid
- Activated relic matches the configured `EchoRelic`

After validation:

```text
bEncounterCompleted = true
→ StoryNPC->ReceiveStoryEvent()
→ StoryNPC->RelocateToStoryAnchor()
→ NextStoryStage = 1
```

State transition for Little Girl:

```text
Available
→ EventResolved
→ Relocating
→ Available at Stage 1
```

---

### `BP_EchoRelic_Bell`

Created:

```text
BP_EchoRelic_Bell
```

Parent class:

```text
AEchoRelic
```

Level Actor label:

```text
QuietChild_BellEcho_01
```

Configured example data:

```text
EchoID = QuietChild.BellEcho01
Title = The Silent Bell
```

The initial visual uses a temporary relic or bell-like Mesh. Gameplay functionality was prioritized over final asset quality.

---

### Day 5 Result

At the end of Day 5:

- The bell could be examined before combat but remained silent
- Ordinary enemy death did not unlock it
- Required Enemy death unlocked it
- Memory Echo UI supported two pages and both button/E-key input
- Reading completion recorded the Echo once
- Invalid ID and missing Widget configuration failed safely
- Player death cancelled the Echo safely
- Activated relics could not be reused
- Encounter completion could not repeat
- Little Girl received a semantic Story Event
- Little Girl relocated to Anchor 02 with correct facing
- Stage 1 dialogue became available
- The first complete narrative gameplay loop was fully playable

---

## Day 6 Extension — Unreal Engine 5.8 Migration and Codex MCP

### Goal

Use the remaining Week 5 time to perform a controlled engine migration and prepare the project for Codex-assisted Unreal development.

The migration was treated as an isolated extension rather than part of the Story Encounter implementation.

---

### Migration Environment

Validated environment:

```text
Visual Studio 2022 17.14.36
Windows 11 SDK 10.0.26100.7705
Unreal Engine 5.8
```

UE5.8 installation path:

```text
C:\Program Files\Epic Games\UE_5.8
```

Before migration:

- Git working tree was clean
- A migration branch was prepared
- A frozen UE5.4 backup was created
- UE5.4 remained installed as a fallback

---

### Independent UE5.8 Project Copy

Created a separate development copy:

```text
WangChuan_UE58_Migration
```

The internal project and module name remained:

```text
WangChuan
```

This avoided unnecessary renaming of:

- `.uproject`
- Modules
- Targets
- API macros
- Blueprint parent references

Old generated data was cleared:

- `.vs`
- `Binaries`
- `Intermediate`
- `Saved`
- `DerivedDataCache`
- Old Solution files

Project files were regenerated for UE5.8.

---

### UE5.8 Build and Project Validation

The project compiled successfully with:

```text
Development Editor
Win64
```

The UE5.8 Editor opened successfully.

Validation included:

- C++ modules
- Key Blueprints
- `Prototype_Map`
- Combat
- Lock-On
- Dialogue UI
- Memory Echo UI
- Story Encounter references
- NPC relocation
- The complete Week 5 gameplay loop

No migration issue remained that blocked continued development.

---

### Official Unreal MCP

Enabled the UE5.8 built-in Epic experimental plugin:

```text
ModelContextProtocol
```

The required Editor Toolsets were loaded.

MCP server:

```text
http://127.0.0.1:8000/mcp
```

Codex configuration was added and enabled.

The Unreal Editor must remain open, with the MCP service running, while Codex uses Unreal tools.

---

### MCP Protocol Validation

Codex successfully:

- Negotiated MCP protocol version `2025-06-18`
- Obtained a valid Session ID
- Enumerated `list_toolsets`
- Enumerated `describe_toolset`
- Enumerated `call_tool`
- Discovered more than 50 Unreal Toolsets

---

### Live Unreal Read Validation

Through MCP, Codex successfully read:

```text
/Game/WangChuan/Maps/Prototype_Map
```

Returned Actors included:

- `WorldSettings`
- Multiple `StaticMeshActor` instances
- `BP_InteractionStone`
- `BP_GhostEnemy_New`
- Other existing level Actors

The validation was read-only:

- No Actor was created
- No Actor was deleted
- No asset was saved or modified

---

### MCP Isolation Validation

A separate test project was also used:

```text
UE58MCPValidation
```

The plugin and Editor Toolsets loaded correctly.

The validation project could not bind port `8000` because the active WangChuan Editor already owned it.

This was correctly identified as expected port occupancy, not an MCP configuration failure.

---

### Codex Session Loading Note

A Codex task started before MCP configuration does not dynamically gain the Unreal tool connection.

Correct workflow:

```text
Start WangChuan UE5.8 Editor
→ Confirm MCP server is running
→ Start a new Codex task from the project context
```

A full Codex application restart is only needed if a new task still does not load the configured MCP server.

---

### Day 6 Result

At the end of Day 6:

- The project had a validated UE5.8 development copy
- Existing gameplay systems remained functional
- UE5.4 fallback data remained protected
- Epic Unreal MCP was connected to Codex
- Protocol handshake and Toolset discovery succeeded
- Codex could read the active Unreal level through MCP
- The new formal development project became `WangChuan_UE58_Migration`

---

## Day 7 — Full Flow Validation and Recording

### Goal

Perform a final Week 5 integration pass, record the full Quiet Child Encounter 01 flow, and resolve any remaining UE5.8 project-opening issue.

---

### Full Gameplay Validation

The final run covered:

```text
Start in Prototype_Map
→ Meet Little Girl at Anchor 01
→ Read Stage 0 Dialogue
→ Enter the encounter branch
→ Fight the Required Enemy
→ Receive enemy-defeat event
→ Unlock Bell Echo Relic
→ Read both Memory Echo pages
→ Record Journal Echo
→ Complete Story Encounter
→ Relocate Little Girl to Anchor 02
→ Read Stage 1 Dialogue
```

The integration pass also confirmed continued operation of:

- Player movement and camera
- Light Combo
- Heavy Attack
- Lock-On
- Eight-direction strafe movement
- Player Health UI
- Enemy Health Bar
- Hit VFX and combat audio
- Player damage and death
- Enemy damage and death
- Interaction prompts
- Dialogue input mode
- Memory Echo input mode
- Story Anchor references
- Story Encounter references

---

### Final Recording

A complete Week 5 gameplay recording was created.

The recording includes:

- Initial NPC interaction
- Stage 0 dialogue
- Branch exploration
- Required Enemy combat
- Enemy defeat and relic unlock
- Memory Echo UI
- Encounter completion
- NPC relocation
- Stage 1 dialogue

This recording serves as the Week 5 milestone record.

---

### Problem: Unknown Engine Version

`WangChuan.uproject` was written with an invalid local Engine Association GUID:

```json
"EngineAssociation": "{398E4C2E-40AD-5AE1-AB1A-36B1A6856C32}"
```

The local GUID no longer mapped to a valid Unreal installation, causing Windows and Unreal project tools to report:

```text
Unknown Engine Version
```

---

### Solution: Explicit UE5.8 Association

Changed the project descriptor to:

```json
"EngineAssociation": "5.8"
```

Then:

- Regenerated Visual Studio project files
- Rebuilt the project using `Development Editor / Win64`
- Reopened the project through UE5.8

The project was again recognized and launched correctly.

The active project remains:

```text
WangChuan_UE58_Migration
```

with the internal project name:

```text
WangChuan
```

---

### Day 7 Result

At the end of Day 7:

- Quiet Child Encounter 01 was validated from beginning to end
- All major Week 1–5 systems remained compatible
- The Week 5 result was recorded
- The invalid Engine Association GUID was identified and replaced with `5.8`
- Project files regenerated and compiled successfully
- UE5.8 and Unreal MCP remained functional
- Week 5 was considered complete

---

## Key Problems and Solutions

### 1. NPC Actor Rotation Was Correct but the Visible Model Faced the Wrong Direction

**Problem:**

Little Girl reached the correct Anchor position and Actor rotation, but the visible face direction did not match `FacingArrow`.

**Cause:**

```text
Unreal Actor forward = +X
Quiet Child model visual forward = +Y
```

The model contained a 90-degree visual-forward offset.

**Solution:**

Add model-specific compensation to `AWCStoryNPC`:

```cpp
MeshFacingYawOffset
```

Calculate:

```text
Actor Yaw = Anchor Yaw - MeshFacingYawOffset
```

Configure:

```text
BP_QuietChild MeshFacingYawOffset = 90
NPCMesh Relative Rotation = (0, 0, 0)
```

This preserves generic Anchor data while keeping model-specific orientation inside the NPC.

---

### 2. Relocation Could Leave an Interaction Prompt at the Old Position

**Risk:**

Disabling collision does not guarantee that the expected EndOverlap cleanup will always perform the desired interaction-state reset.

**Solution:**

Before relocation:

```cpp
ClearPlayerInteractionIfNeeded()
```

Explicitly clear `CurrentInteractable` and hide the prompt when it points to the relocating NPC.

This prevents remote interaction with the NPC's old location.

---

### 3. Dialogue and Relocation Could Conflict

**Risk:**

Relocating an NPC while it owns an active dialogue could remove or hide the current conversation source.

**Solution:**

Reject relocation until the dialogue is closed.

The NPC remains in place while Conversation Mode is active.

---

### 4. Enemy Death Could Broadcast or Process More Than Once

**Risk:**

Repeated calls to `Die()` or repeated story handling could duplicate encounter progression.

**Solution:**

Use two independent guards:

```text
AGhostEnemy::bIsDead
AStoryEncounter::bRequiredEnemyDefeated
```

Broadcast only from `Die()` after stable death-state cleanup.

Do not broadcast from `TakeHit()` or `FinishDeath()`.

---

### 5. Enemy Death Must Not Skip the Relic Investigation

**Risk:**

Treating enemy defeat as full Encounter completion would immediately move Little Girl and bypass the player's investigation of the bell.

**Solution:**

Separate progress state:

```text
Required Enemy Defeated
≠
Encounter Completed
```

Enemy death produces:

```text
bRequiredEnemyDefeated = true
bEncounterCompleted = false
```

Only full Memory Echo completion changes `bEncounterCompleted` to true.

---

### 6. Prompt Needed to Update While the Player Was Already Inside the Relic Range

**Problem:**

If the Required Enemy died while the player overlapped the bell, the old `[E] Examine` prompt could remain until the player left and re-entered.

**Solution:**

Call:

```cpp
RefreshPromptForOverlappingPlayer()
```

when `UnlockRelic()` changes the relic from `Locked` to `Available`.

---

### 7. Repeated E Input Could Create More Than One Memory Echo Session

**Risk:**

Multiple interactions during an active reading session could create duplicate Widgets or duplicate story completion.

**Solution:**

Use:

```cpp
bActivationInProgress
```

and reject additional interaction until the session completes or is cancelled.

Also disable the relic interaction sphere after final activation.

---

### 8. Player Death During Dialogue or Memory Echo Could Leave UI or Input State Behind

**Risk:**

A fatal hit during UI mode could leave the cursor visible, preserve a Widget, restore input incorrectly, or complete the story unintentionally.

**Solution:**

Dialogue death path:

```cpp
EndDialogue()
```

Memory Echo death path:

```cpp
EndMemoryEcho(false)
```

Both cleanup paths avoid restoring control after `bIsDead` is true.

The cancelled Memory Echo does not record Journal data or activate the relic.

---

### 9. MCP Validation Project Could Not Bind Port 8000

**Observation:**

The isolated validation project reported a port binding failure.

**Cause:**

The active WangChuan Editor was already correctly serving MCP on:

```text
127.0.0.1:8000
```

**Resolution:**

Treat the message as expected single-port ownership rather than a configuration failure.

Only one Editor process should own the configured MCP port at a time.

---

### 10. `.uproject` Displayed Unknown Engine Version

**Problem:**

`EngineAssociation` contained a stale local GUID:

```json
"{398E4C2E-40AD-5AE1-AB1A-36B1A6856C32}"
```

**Solution:**

Replace it with:

```json
"5.8"
```

Regenerate project files and rebuild the Editor target.

---

## Final Week 5 System State

### Story Data Architecture

The project now supports:

- `StoryTypes.h`
- `EStoryNPCState`
- `FDialogueLine`
- `FDialogueSequence`
- `EEchoRelicState`
- `FMemoryEchoData`
- Story Stage separate from current Story State
- Blueprint-configurable dialogue and Echo content
- Named Story Event IDs

---

### Story NPC

`AWCStoryNPC` now supports:

- Existing `IInteractable` integration
- NPC display identity
- Interaction prompts
- Stage-indexed dialogue
- `Available`, `EventResolved`, and `Relocating` flow
- Story Anchor references
- Hidden teleport relocation
- Delayed reveal
- Old-position interaction cleanup
- Destination-stage assignment
- Model-specific facing compensation
- Semantic Story Event reception

`BP_QuietChild` currently uses:

```text
NPCDisplayName = Little Girl
MeshFacingYawOffset = 90
```

---

### Dialogue System

The project now supports:

- `UDialogueWidget`
- `WBP_Dialogue`
- Speaker name display
- Wrapped dialogue text
- Multi-line progression
- Continue and Close buttons
- E-key progression
- Final-line Close label
- Early closure
- Conversation input mode
- Lock-On and combat-state cleanup
- Prompt restoration
- Player-death cleanup

---

### Story Anchors and Relocation

The project now supports:

- `AStoryAnchor`
- Editor-facing arrows
- Anchor identity
- World Transform access
- `QuietChild_Anchor_01`
- `QuietChild_Anchor_02`
- Index-based and direct-Anchor relocation
- Invalid index and null-reference protection
- Repeated relocation protection
- Story Stage assignment after reveal
- Correct visual facing at destination

---

### Enemy Story Events

`AGhostEnemy` now supports:

- `OnEnemyDefeated`
- One-time death broadcast
- Stable death-state broadcast timing
- Existing death-delay compatibility
- Independent narrative listeners

The enemy does not reference Story Encounter, Story NPC, Anchor, or Relic classes.

---

### Story Encounter

`AStoryEncounter` now supports:

- Encounter identity
- Required Enemy reference
- Story NPC reference
- Next Story Anchor reference
- Echo Relic reference
- Required-enemy state
- Encounter-completion state
- Dynamic event binding and unbinding
- Required-enemy instance filtering
- Completion Story Event ID
- Destination Story Stage
- NPC event delivery
- NPC relocation after completion

Current encounter:

```text
BP_QuietChild_Encounter_01
EncounterID = QuietChild.Encounter01
CompletionStoryEventID = QuietChild.BellEchoActivated
NextStoryStage = 1
```

---

### Echo Relic

The project now supports:

- `AEchoRelic`
- `BP_EchoRelic_Bell`
- Locked/Available/Activated states
- Separate Examine and Listen prompts
- Locked inspection text
- Runtime unlock
- Overlap prompt refresh
- Activation-in-progress protection
- Activation event broadcast
- Interaction disable after completion
- Cancellation and retry

Current Echo:

```text
QuietChild_BellEcho_01
EchoID = QuietChild.BellEcho01
Title = The Silent Bell
```

---

### Memory Echo and Journal

The project now supports:

- `UMemoryEchoWidget`
- `WBP_MemoryEcho`
- Echo title
- Echo memory text
- Player resonance text
- Two-page input flow
- E-key and button progression
- Dedicated Memory Echo input mode
- Player-death cancellation
- Runtime Journal recording
- `EchoID` de-duplication
- Existing Memory Fragment Journal compatibility

The Journal remains a debug-oriented implementation and is not persisted through SaveGame.

---

### UE5.8 Development Environment

The active development project is now:

```text
WangChuan_UE58_Migration
```

The internal project name remains:

```text
WangChuan
```

Current engine association:

```json
"EngineAssociation": "5.8"
```

The original UE5.4 project and frozen backup remain fallback references and should not receive new development changes.

---

### Codex MCP

The project now supports:

- UE5.8 Epic `ModelContextProtocol` plugin
- Local MCP server on `127.0.0.1:8000/mcp`
- Codex MCP configuration
- MCP `2025-06-18` handshake
- Session creation
- Toolset discovery
- More than 50 Unreal Toolsets
- Current-level and Actor-list reads

Initial validation remained read-only.

Formal future MCP write operations should first be tested in a sandbox map or clean Git state.

---

### Existing Week 1–4 Compatibility

The following existing systems remained functional through Week 5 and the UE5.8 migration:

- Enhanced Input movement and interaction
- InteractionStone
- Memory Fragments
- Player Health UI
- Defeated UI
- Light Combo
- Heavy Attack
- Root Motion attacks
- Player attack Anim Notifies
- Combo Window
- Input Buffer
- Hit Stop
- Hit/whiff audio
- Hit VFX
- Enemy Health Bar
- Enemy Hit Reaction
- Enemy attack and death behavior
- Player hurt feedback
- Player death and ragdoll
- Lock-On targeting
- Lock-On attack facing
- Combat Idle
- Eight-direction lock-on locomotion
- Backward movement with Blend Space Smoothing Time set to `0`

---

## Week 5 Completion Review

### Original Core Goals

The following Week 5 goals were completed:

- Reusable Story NPC base
- Story Stage and Story State separation
- Blueprint-configurable dialogue
- Formal Dialogue UI
- Conversation Mode
- Story Anchors
- NPC relocation
- Destination-stage dialogue
- Enemy defeat event
- Story Encounter coordinator
- Required Enemy filtering
- Echo Relic
- Memory Echo UI
- Journal Echo record
- Story Event delivery
- Encounter completion
- NPC relocation to the next story position
- Complete Quiet Child Encounter 01 integration
- Final full-flow recording

---

### Extension Goals

The following controlled extension goals were also completed:

- UE5.4 project freeze and backup
- Independent UE5.8 migration project
- UE5.8 compile and Blueprint validation
- Complete gameplay-loop regression on UE5.8
- Epic Unreal MCP activation
- Codex MCP configuration
- MCP handshake and Toolset discovery
- Live Unreal level read validation
- Engine Association repair

---

## Known Limitations and Future Polish

### Dialogue Presentation

Current Dialogue UI intentionally does not include:

- Branching choices
- Portraits
- Typewriter animation
- Voice playback
- Dialogue history
- Conditional dialogue graphs

The current linear sequence is sufficient for the first encounter.

---

### Story NPC Movement

Relocation is still:

```text
Hide
→ Teleport
→ Reveal
```

It does not include:

- Walking
- NavMesh movement
- AI Move To
- Companion following
- Dynamic face-player behavior
- Dissolve effects
- Relocation particles
- Relocation audio

This remains an intentional scope decision.

---

### Story Management

The project does not currently include:

- A global Quest Manager
- Story Subsystem
- DataTable-driven quest definitions
- Gameplay Tag-based story state
- Multi-encounter persistence
- SaveGame story restoration

The current level-instance references are appropriate for the first vertical-slice encounter but may require data-driven expansion if the number of encounters grows significantly.

---

### Journal

The Journal still uses a debug-oriented display.

Future work may add:

- Formal Journal Widget
- Tabs or categories
- Scrollable Echo entries
- Separate Memory Fragment and Memory Echo pages
- SaveGame persistence
- Localization-ready content layout

---

### Echo Relic Presentation

The current bell relic prioritizes function over final presentation.

Future polish may include:

- Final bell model
- Material-state feedback
- Locked/Available visual difference
- Bell sound
- Memory Echo opening animation
- Environmental VFX

---

### MCP Safety

The Unreal MCP plugin is experimental.

Future Codex-assisted Unreal work should follow:

```text
Clean Git state
→ Sandbox or copied map
→ Small scoped tool request
→ Verify Unreal result
→ Test gameplay
→ Commit only after review
```

Batch deletion, mass renaming, full-project redirector operations, and broad asset saves should not be delegated without a rollback point.

---

## Week 5 Summary

Week 5 transformed the project from a combat-focused prototype with basic interactions into a small but complete narrative gameplay slice.

The project now supports a full loop in which:

```text
Dialogue provides context
→ Exploration leads to a combat condition
→ Enemy death changes world state
→ A relic becomes interactable
→ A Memory Echo provides narrative information
→ The Journal records the result
→ A Story Encounter completes
→ An NPC receives a Story Event
→ The NPC changes location and dialogue stage
```

The architecture remains deliberately lightweight:

- Story NPCs own their state and dialogue data
- The player owns UI modes and input restoration
- Enemies broadcast their own death
- Relics broadcast their own activation
- Story Encounters coordinate references and progression
- Story Anchors store world placement and intended visual facing

The week also established the project's new technical baseline:

```text
Unreal Engine 5.8
+ Visual Studio 2022
+ Epic Unreal MCP
+ Codex tool access
```

The active development project is now `WangChuan_UE58_Migration`, while the UE5.4 project and frozen backup remain preserved as fallback references.

Week 5 completed all planned narrative-system goals, completed the Quiet Child Encounter 01 integration, passed full regression testing, and produced a final gameplay recording.
