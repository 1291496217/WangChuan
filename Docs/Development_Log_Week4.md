# Development Log — Week 4

**Project:** WangChuan / 《忘川河畔》  
**Engine:** Unreal Engine 5.4  
**Focus:** Unarmed combat expansion, animation-driven attacks, combo input, hit feedback, lock-on targeting, and lock-on locomotion  
**Week Theme:** Unarmed Combat Expansion — from a single light attack to a structured third-person combat prototype

---

## Week 4 Goal

Week 4 focused on expanding the Week 3 combat slice into a more complete unarmed combat prototype.

The original goal was to upgrade the player from a single left-click attack into a system with:

- A four-hit light combo
- A separate heavy attack
- Root Motion attack movement
- Animation-timed hit detection
- Structured attack data
- Combat Idle
- Clearer light/heavy impact hierarchy
- A recordable combat result

The core Week 4 goals were completed ahead of schedule. The remaining development time was then used for controlled advanced extensions:

- Combo Window and Input Buffer
- Light Combo finisher feedback
- Local Hit Stop
- Stronger Heavy Attack feedback
- Lock-On targeting
- Lock-On Strafe Movement
- Eight-direction lock-on locomotion

The Week 4 priority remained:

```text
Completion > Playability > Extensibility > Visual Quality
```

The week intentionally avoided weapon systems, inventory, parry, dodge, full enemy AI rewrites, Behavior Trees, GAS, and large narrative expansion.

---

## Week 4 Result Overview

By the end of Week 4, the project added:

- `AM_Player_LightCombo`
- `AM_Player_HeavyAttack`
- Root Motion from Montages Only
- Right-click Heavy Attack
- `FPlayerAttackData`
- Unified current-attack trace logic
- Player attack Anim Notify timing
- Four-section Light Combo
- Independent data for each combo hit
- C++ Montage section stopping
- Combat Idle
- Combo Window
- Input Buffer
- Light Combo finisher feedback
- Local player/enemy Hit Stop
- Stronger Heavy Attack sound, VFX, shake, and Hit Stop
- Lock-On target selection
- Automatic lock break conditions
- Lock-On attack facing
- Lock-On Strafe Movement
- Eight-direction Strafe Blend Space
- Final Week 4 showcase recording

---

## 7/9 — Day 1: Animation Audit, Root Motion, Heavy Attack, and Attack Data

### Goal

Start the Unarmed Combat Expansion by validating the required animation assets before building the combo system.

The initial work focused on three animation groups:

- Four-hit light combo
- Heavy side kick
- Combat Idle

The goal was to confirm animation quality, Root Motion behavior, Montage compatibility, collision stability, and suitability for later combat integration.

---

### Light Combo Animation Audit

The selected light combo animation was:

```text
H2H_PunchCombo02
```

The animation contains four distinct attacks:

1. First punch
2. Second punch
3. Rising hook / uppercut-style strike
4. Large finishing swing

The animation also contains visible forward body movement rather than remaining fully in place.

Initial review confirmed:

- The animation plays correctly on the current player skeleton
- The four attacks are visually distinguishable
- The forward stepping motion supports a grounded unarmed-combat feel
- The animation is suitable for splitting into Montage Sections
- The asset is usable as the Week 4 light combo foundation

---

### Light Combo Montage

Created:

```text
AM_Player_LightCombo
```

The Montage was temporarily assigned as the current `LightAttackMontage` and tested through the existing left-click attack pipeline.

The initial in-game result showed:

- Natural forward movement during the attack
- Better physical weight than the previous stationary punch
- No immediate playback failure
- No severe Mesh/Capsule separation

---

### Root Motion Initial Review

Enabled:

```text
EnableRootMotion
```

on the light combo Animation Sequence.

The animation preview displayed a clear Root Motion trajectory. The forward displacement generally matched the character's stepping and punching motion.

Initial observations:

- Capsule movement followed the attack reasonably well
- The Mesh remained aligned with the Capsule
- Basic wall tests did not show obvious collision tunneling
- The camera moved slightly while following the character
- The camera movement added some weight, but required comfort monitoring

At this stage, Root Motion was considered a usable candidate rather than immediately accepted or rejected.

---

### Root Motion from Montages Only

The player AnimBP Root Motion Mode was changed to:

```text
Root Motion from Montages Only
```

This preserved normal `CharacterMovement` locomotion while allowing attack Montages to drive attack displacement.

The following tests were completed:

- Flat-ground attacks
- Repeated attacks
- Direct wall contact
- Angled wall contact
- Uphill and downhill movement
- Uneven terrain
- Close-range enemy contact
- Combat-area boundary behavior
- Camera comfort
- Capsule and Mesh stability

The Heavy Attack Montage was also temporarily tested through the existing attack input.

Result:

- Light Combo Root Motion was acceptable
- Heavy Attack Root Motion was acceptable
- Capsule behavior remained stable
- No obvious wall penetration was observed
- Camera motion remained tolerable
- In-place fallback was not required for the current Week 4 implementation

Final Week 4 direction:

```text
Use Root Motion from Montages Only
```

while continuing to monitor camera comfort and close-range collision behavior.

---

### Heavy Attack Input

Created:

```text
IA_HeavyAttack
```

Configuration:

- Value Type: Boolean
- Bound in `IMC_Default`
- Input: Right Mouse Button

Added `HeavyAttackAction` to `AWCCharacter` and bound it with:

```cpp
ETriggerEvent::Started
```

to:

```cpp
HeavyAttack()
```

---

### Heavy Attack Montage

Created and assigned:

```text
AM_Player_HeavyAttack
```

The animation is a turning side kick with a short forward step.

`HeavyAttack()` initially followed the same broad state protection as the light attack:

- Check `CanAct()`
- Reject input while `bIsAttacking`
- Set `bIsAttacking = true`
- Play the Heavy Attack Montage
- Start the attack duration timer
- Return to the normal action state through `EndAttack()`

---

### Heavy Attack Prototype Parameters

The final tested Heavy Attack values were:

```text
Damage             = 50
Range              = 100
BoxHalfSize        = 50, 50, 50
Duration           = 1.5
KnockbackStrength  = 250
```

The larger range accounts for the kick's forward step.

The cubic Box Trace was chosen to provide a practical waist-level hit region without extending excessively upward.

The Heavy Attack was intentionally differentiated from light attacks through:

- Higher damage
- Longer duration
- Larger effective range
- Stronger knockback
- Slower commitment
- More pronounced animation anticipation

---

### Initial Heavy Attack Timing Problem

The first Heavy Attack implementation called its trace immediately when the input function started.

This caused the enemy to react and receive knockback while the player was still beginning the turn, before the foot visually reached the target.

This was accepted temporarily because player attack Anim Notify timing had not yet been implemented.

The issue was scheduled for correction through an animation-timed hit event.

---

### `FPlayerAttackData`

To avoid continuing with separate loose variables for every attack type, a shared attack structure was created:

```cpp
USTRUCT(BlueprintType)
struct FPlayerAttackData
```

The structure contains:

- `Damage`
- `Range`
- `BoxHalfSize`
- `Duration`
- `KnockbackStrength`

Added:

```cpp
LightAttackData
HeavyAttackData
CurrentAttackData
```

`CurrentAttackData` stores the parameters for the attack currently being executed.

This provided a cleaner foundation for:

- Light attacks
- Heavy attacks
- Combo-specific attacks
- Anim Notify hit timing
- Future attack-type expansion

---

### Unified Attack Trace

Created:

```cpp
PerformCurrentAttackTrace()
```

The function reads from `CurrentAttackData` rather than separate light/heavy variables.

It controls:

- Box Trace range
- Box dimensions
- Damage
- Knockback
- Hit/whiff result
- Hit sound
- Hit VFX
- Enemy `TakeHit()` calls

The trace continued to use:

```cpp
GetActorForwardVector()
```

as the attack direction, with the start position offset forward from the player.

---

### Attack Timer Refactor

Updated:

```cpp
StartAttackTimer(float Duration)
```

Light and Heavy Attacks can now use their own durations through the same timer helper.

This removed the need for one shared hardcoded attack duration.

---

### Legacy Attack Cleanup

After successful testing, the old duplicated attack variables and trace paths were removed or retired.

The active attack pipeline became:

```text
Attack input
→ Set CurrentAttackData
→ Play the correct Montage
→ Trigger the unified attack trace
→ End through the shared attack timer
```

At the end of Day 1, the project had:

- A validated Root Motion solution
- A functional Heavy Attack
- A shared attack-data structure
- A unified trace path
- Stable Week 3 combat feedback compatibility

---

## 7/10 — Day 2: Anim Notify Hits, Four-Hit Combo, Section Control, and Combat Idle

### Goal

Move player attack damage from the moment of button input to the actual visual hit frame, then build the first functional four-hit light combo.

---

### Player Attack Anim Notify

Added to `AWCCharacter`:

```cpp
OnPlayerAttackHitNotify()
```

The function was exposed to Blueprint so the player AnimBP could call it from Montage Anim Notify events.

The notify entry point checks:

- `CanAct()`
- `bIsAttacking`
- Whether the current attack hit was already processed

---

### Duplicate-Hit Protection

Added:

```cpp
bHasProcessedAttackHit
```

Behavior:

- Reset to `false` when an attack begins
- Set to `true` after the attack trace is processed
- Reset in `EndAttack()`
- Reset during player death cleanup

This prevents one attack from applying damage more than once because of duplicate or repeated Notify events.

---

### Attack Timing Update

Removed the immediate `PerformCurrentAttackTrace()` call from both:

```cpp
Attack()
HeavyAttack()
```

The updated flow became:

```text
Input
→ Set CurrentAttackData
→ Play Montage
→ Wait for PlayerAttackHit Anim Notify
→ OnPlayerAttackHitNotify()
→ PerformCurrentAttackTrace()
```

This corrected the Heavy Attack's early knockback problem.

The enemy now receives damage when the side kick visually reaches the target rather than when the turn begins.

---

### AnimBP Notify Connection

In the player AnimBP Event Graph:

```text
Event AnimNotify_PlayerAttackHit
→ Try Get Pawn Owner
→ Cast to WCCharacter
→ OnPlayerAttackHitNotify
```

After this change:

- Hit Sound follows the actual impact frame
- Whiff Sound follows the attack frame
- Hit VFX appears at the actual impact moment
- Enemy Hit Reaction begins at the correct time
- Debug Box Trace appears at the Notify frame
- Death-state protection prevents stale Notify damage

---

### Light Combo Sections

The light combo Montage was split into:

```text
Light_1
Light_2
Light_3
Light_4
```

Each section corresponds to one attack in the combo animation.

All four sections use the same Notify name:

```text
PlayerAttackHit
```

The active attack's data is selected in C++ through `CurrentAttackData`.

---

### Combo State

Added:

- `MaxLightComboIndex`
- `CurrentLightComboIndex`
- `ComboResetTime`
- `ComboResetTimerHandle`
- `LightComboAttackData`

The combo index follows:

```text
0 → Light_1
1 → Light_2
2 → Light_3
3 → Light_4
```

Each combo hit can have independent:

- Damage
- Range
- Box size
- Duration
- Knockback strength

The fourth hit was configured as the strongest light-combo hit.

---

### Combo Helper Functions

Added helpers for combo organization:

```cpp
ResetLightCombo()
GetLightComboSectionName()
GetLightComboAttackData()
AdvanceLightCombo()
```

Responsibilities:

- Reset the next attack to `Light_1`
- Convert combo index into Montage Section name
- Return combo-specific attack data
- Advance the combo index
- Fall back to `LightAttackData` if the array is not configured correctly

---

### Section-Based Montage Playback

Updated:

```cpp
PlayLightAttackMontage(FName SectionName)
```

The function:

1. Plays `AM_Player_LightCombo`
2. Jumps to the requested Montage Section

This allowed individual clicks to select `Light_1`, `Light_2`, `Light_3`, or `Light_4`.

---

### Montage Section Playback Problem

Initial testing produced the following result:

- First click started at `Light_1` and played through the entire combo
- Second click started at `Light_2` and continued through the remaining combo
- Third click started at `Light_3` and continued through `Light_4`
- Fourth click played only `Light_4`

The combo index and `Montage_JumpToSection()` were working correctly.

The actual issue was that Montage Sections determine where playback starts but do not automatically stop playback at the next section boundary.

---

### C++ Montage Stop Fix

The final solution used C++ to stop the active attack Montage.

Added current Montage tracking:

```cpp
CurrentAttackMontage
```

Added:

```cpp
StopCurrentAttackMontage()
```

Light and Heavy Attacks store their active Montage. `EndAttack()` calls the stop helper after the current attack's configured duration.

Result:

- `Light_1` stops after its own attack duration
- `Light_2` stops after its own attack duration
- `Light_3` stops after its own attack duration
- `Light_4` plays as the final strike
- The next click after `Light_4` returns to `Light_1`

The Combo Reset Timer was also corrected so `ComboResetTime` is passed as the timer duration.

---

### Combat Idle Motivation

After individual Section stopping worked, a new visual problem became obvious.

The combo animation uses a raised boxing stance, while the normal exploration Idle has the arms hanging naturally.

Returning directly from a short punch Section to the normal Idle caused a visible pose snap.

The issue was most noticeable after `Light_3`, where the character's body rises through the hook/uppercut motion and then immediately returns to an upright Idle.

---

### Combat State

Added:

```cpp
bIsInCombat
CombatIdleDuration
CombatIdleTimerHandle
```

Added helpers:

```cpp
GetIsInCombat()
EnterCombatState()
ExitCombatState()
```

Behavior:

- Light Attack enters or refreshes combat state
- Heavy Attack enters or refreshes combat state
- The player stays in Combat Idle for a short period after attacking
- The timer eventually returns the player to exploration Idle
- Death clears the combat state and timer

---

### AnimBP Combat Idle

Added:

```text
H2H_Idle
```

as the Combat Idle animation.

The AnimBP reads `GetIsInCombat()` from the `As WCCharacter` output of the cast.

This was an important Blueprint detail: calling the getter without using the cast character reference did not read the correct player instance and did not update the state.

The initial Combat Idle condition was:

```text
IsInCombat AND GroundSpeed < 10
```

The AnimGraph structure became:

```text
Main States / Combat Idle
→ DefaultSlot
→ Control Rig
→ Output Pose
```

The Montage Slot remained after the locomotion/idle selection so attack Montages could override the underlying pose.

---

### Day 2 Result

At the end of Day 2:

- Light and Heavy Attacks dealt damage at their visual hit frames
- Four separate light attacks could be selected in order
- Each combo hit used independent attack data
- Heavy Attack reset the light combo
- Combo timeout returned to `Light_1`
- Individual Montage Sections stopped correctly
- Combat Idle reduced the visual gap between attacks and normal Idle
- Existing audio, VFX, health bars, enemy reactions, death, and ragdoll systems remained stable

Known visual limitation:

- The original combo animation was designed as one continuous sequence
- Separating it into individual attacks still creates some pose discontinuity
- `Light_3` remains the clearest example
- The issue was accepted for the current prototype rather than expanding into a complex animation-transition system

---

## 7/11 — Day 3: Integration Test and Showcase Recording

### Goal

Use the day as a Week 4 integration checkpoint rather than adding another large system.

By this point, most original Week 4 targets had already been completed.

---

### Integrated Systems Reviewed

The following systems were tested together:

- Root Motion from Montages Only
- Four-hit light combo
- Heavy Attack
- `FPlayerAttackData`
- Combo-specific attack data
- Player attack Anim Notify
- Montage Section stopping
- Combat Idle
- Hit and whiff audio
- Hit VFX
- Enemy health bar
- Enemy Hit Reaction
- Enemy death
- Player health UI
- Player hit feedback
- Player death
- Player ragdoll
- Combat state cleanup

---

### Heavy Attack State

The Heavy Attack remained configured as:

```text
Damage             = 50
Range              = 100
BoxHalfSize        = 50, 50, 50
Duration           = 1.5
KnockbackStrength  = 250
```

It remained clearly differentiated from the light combo through commitment, damage, range, and knockback.

---

### Root Motion State

The Root Motion solution continued to pass practical testing.

Observed result:

- Attacks move the player naturally
- The Capsule follows the attack
- Wall behavior remains acceptable
- Close-range enemy interaction remains usable
- Slight camera movement remains present but acceptable

---

### First Week 4 Recording

A development recording was created to document the current state.

The recording included:

- Light Combo
- Heavy Attack
- Attack hit timing
- Enemy Hit Reaction
- Combat Idle
- Basic combat loop
- Existing Week 3 UI and feedback

The system was considered stable enough to move from the original Week 4 requirements into optional advanced extensions.

---

## 7/12 — Day 4: Combo Window, Input Buffer, and Finisher Feedback

### Goal

Improve combo input responsiveness so the player does not need to wait for each attack to fully end before pressing the next input.

---

### Combo Window State

Added:

```cpp
bCanBufferLightAttack
bHasBufferedLightAttack
ComboWindowOpenRatio
ComboWindowOpenTimerHandle
ComboWindowCloseTimerHandle
```

Definitions:

- `bCanBufferLightAttack`: the current attack is in a valid input window
- `bHasBufferedLightAttack`: the player entered the next light attack during that window
- `ComboWindowOpenRatio`: when the window opens relative to attack duration

The initial tested ratio was:

```text
0.55
```

---

### Combo Window Functions

Added:

```cpp
OpenLightComboWindow()
CloseLightComboWindow()
ClearLightComboBuffer()
ConsumeBufferedLightAttack()
```

The timer-based prototype opens the window during the later part of the current attack.

The system intentionally remained timer-based rather than moving the window to Anim Notifies because the current result already felt acceptable.

---

### Buffered Input Flow

The updated light attack flow became:

```text
Current attack is playing
→ Combo Window opens
→ Player presses Light Attack
→ Store buffered input
→ Current attack ends
→ Clear current attack state
→ Consume buffered input
→ Start the next combo section
```

Inputs entered before the window opens are ignored.

This avoids turning early button mashing into a fully automatic combo.

---

### `EndAttack()` Integration

`EndAttack()` now:

- Stops the current Montage
- Clears `bIsAttacking`
- Resets hit-processing state
- Closes and clears window timers
- Checks for a buffered light attack
- Starts the next combo hit when appropriate
- Otherwise returns to Combat Idle

The order is important because `bIsAttacking` must be cleared before the buffered call to `Attack()`.

---

### Buffer Cleanup

The buffer is cleared during:

- Combo reset
- Heavy Attack
- Player death
- Other attack-state resets

This prevents delayed or stale light attacks from firing after the combat state has changed.

---

### Combo Window Result

Testing confirmed:

- Normal click-after-completion input still works
- Late input can be buffered
- `Light_1` connects to `Light_2`
- `Light_2` connects to `Light_3`
- `Light_3` connects to `Light_4`
- Early input is rejected
- No input means the combo stops
- Heavy Attack clears the buffer
- Player death clears the buffer
- Existing Notify, audio, VFX, and Combat Idle behavior remains stable

The combo changed from a sequence that required strict post-animation timing into a more forgiving action-game input flow.

---

### Light Combo Finisher State

Added:

```cpp
bIsCurrentAttackFinisher
```

The value is set when:

```cpp
ComboIndex == MaxLightComboIndex - 1
```

This identifies `Light_4` as the light-combo finisher.

Heavy Attack, attack end, and death cleanup prevent the finisher flag from leaking into other attacks.

---

### Finisher Feedback Resources

Added support for:

- `ComboFinisherHitSound`
- `ComboFinisherHitEffect`
- `ComboFinisherCameraShakeClass`

The initial finisher sound used an available heavier impact sound as a placeholder.

The finisher effect used a larger blood-spray effect than ordinary light attacks.

---

### Finisher Feedback Logic

Added:

```cpp
PlayComboFinisherFeedback()
```

On a valid `Light_4` enemy hit:

- Play the heavier finisher sound
- Spawn the stronger hit effect
- Trigger a short camera shake
- Avoid also playing the normal hit sound

On a whiff:

- Play only the normal whiff sound
- Do not trigger finisher VFX
- Do not trigger finisher camera shake

Input-buffered transitions into `Light_4` correctly preserve the finisher state.

---

## 7/13 — Day 5: Local Hit Stop and Heavy Attack Feedback

### Goal

Add a clearer impact hierarchy between normal light hits, the fourth light-combo hit, and the Heavy Attack.

---

### Hit Stop Design Decision

Global Time Dilation was not used.

The project already depends on multiple timers:

- Attack timers
- Combo reset
- Combo Window
- Combat Idle
- Enemy attack timing
- Enemy hit reaction
- Enemy death
- UI and feedback timing

Changing global time could interfere with those systems.

Instead, Hit Stop was implemented locally through:

```cpp
CustomTimeDilation
```

applied to:

- The player
- The enemy that was hit

This leaves the rest of the world and most gameplay timers unaffected.

---

### Hit Stop Parameters

Added:

- `bEnableHitStop`
- `LightHitStopDuration`
- `FinisherHitStopDuration`
- `HeavyHitStopDuration`
- `HitStopTimeDilation`
- `HitStopTimerHandle`
- Weak target reference for the affected enemy

The three attack tiers use different stop durations:

```text
Normal Light < Light Finisher < Heavy Attack
```

---

### Hit Stop Functions

Added:

```cpp
ApplyHitStop(AActor* HitActor, float Duration)
EndHitStop()
```

`ApplyHitStop()`:

- Checks whether Hit Stop is enabled
- Validates the target and duration
- Ends any previous Hit Stop
- Lowers player Custom Time Dilation
- Lowers target Custom Time Dilation
- Starts the restore timer

`EndHitStop()`:

- Restores player dilation to `1.0`
- Restores the target if it is still valid
- Clears the weak target
- Clears the Hit Stop timer

Player death also restores Hit Stop state.

---

### Heavy Attack Type State

Added:

```cpp
bIsCurrentAttackHeavy
```

Behavior:

- Light Attack sets it to `false`
- Heavy Attack sets it to `true`
- Heavy Attack clears the light-finisher flag
- Attack end and player death clear the Heavy state

Feedback priority became:

```text
Heavy Attack > Light Combo Finisher > Normal Light Attack
```

---

### Heavy Attack Feedback Resources

Added support for:

- `HeavyAttackHitSound`
- `HeavyAttackHitEffect`
- `HeavyAttackCameraShakeClass`

Added:

```cpp
PlayHeavyAttackHitFeedback()
```

A valid Heavy Attack hit now triggers:

- Heavier impact sound
- Stronger hit effect
- Stronger but restrained camera shake
- The longest Hit Stop tier
- Existing high knockback

Fallback behavior uses ordinary hit feedback if a dedicated resource is not assigned.

---

### Sound Duplication Protection

The ordinary hit-sound branch was updated so it does not also play when:

- `bIsCurrentAttackFinisher == true`
- `bIsCurrentAttackHeavy == true`

This prevents layered feedback from becoming muddy.

---

### Day 5 Result

Testing confirmed:

- Normal light hits have subtle Hit Stop
- `Light_4` has stronger Hit Stop
- Heavy Attack has the strongest Hit Stop
- Finisher and Heavy sound do not duplicate the normal hit sound
- Whiffs do not trigger Hit Stop
- Whiffs do not trigger strong VFX or camera shake
- Enemy death remains stable
- Combo Window and Input Buffer remain stable
- Combat Idle remains stable
- Player death does not leave Custom Time Dilation active

---

## 7/14 — Day 6: Lock-On Prototype v1

### Goal

Use the remaining Week 4 time for a controlled lock-on prototype that improves attack direction and third-person combat readability.

The first version intentionally excluded:

- Target switching
- Lock-On UI
- Dedicated lock-on locomotion
- Boss camera behavior
- Full Souls-like targeting features

---

### Lock-On Input

Created:

```text
IA_LockOn
```

The Boolean action was added to `IMC_Default` and bound through `LockOnAction`.

`SetupPlayerInputComponent()` binds it to:

```cpp
ToggleLockOn()
```

---

### Lock-On State

Added:

```cpp
bIsLockedOn
CurrentLockOnTarget
LockOnRadius
LockOnBreakDistance
LockOnRotationInterpSpeed
LockOnMinCameraDot
```

These values control:

- Whether the player is locked on
- Which enemy is targeted
- Target search range
- Automatic break distance
- Rotation speed
- Minimum forward-camera alignment

---

### Target Selection

Added:

```cpp
FindBestLockOnTarget()
```

The initial prototype uses:

```cpp
UGameplayStatics::GetAllActorsOfClass()
```

to find `AGhostEnemy` actors.

Candidates are filtered by:

- Valid actor state
- Enemy alive state
- Distance inside `LockOnRadius`
- Camera-forward Dot Product
- Basic combined distance and camera alignment score

This favors enemies that are both relatively close and near the camera's forward direction.

---

### Lock-On and Unlock

Added:

```cpp
LockOnToTarget()
UnlockTarget()
```

On lock:

- Set `CurrentLockOnTarget`
- Set `bIsLockedOn = true`
- Enable controller-yaw character rotation
- Disable orient-to-movement rotation

On unlock:

- Clear the target
- Set `bIsLockedOn = false`
- Restore exploration rotation settings

---

### Continuous Lock-On Update

Added:

```cpp
UpdateLockOn(float DeltaTime)
```

Called from `Tick()`.

The update:

- Validates the target
- Unlocks if the target is invalid
- Unlocks if the target dies
- Unlocks if the distance exceeds `LockOnBreakDistance`
- Calculates the horizontal direction to the target
- Uses `RInterpTo` for player rotation
- Synchronizes Controller Yaw with the target

The prototype locks Yaw only.

Pitch was intentionally excluded to reduce close-range camera instability and vertical shaking.

---

### Attack Facing

Added optional instant attack-facing behavior:

```cpp
FaceLockOnTargetInstantly()
```

Before Light or Heavy Attack begins, the player can immediately face the active target.

This improves:

- Box Trace direction
- Root Motion attack direction
- Heavy side-kick alignment
- Close-range hit consistency

---

### Automatic Cleanup

Lock-On is cleared when:

- The player manually toggles it off
- The target becomes invalid
- The target dies
- The target moves beyond break distance
- The player dies

---

### Day 6 Result

Testing confirmed:

- Nearby enemies can be locked
- Repeated input unlocks
- No target does not cause a crash
- Player and camera face the target
- Light Combo direction becomes more stable
- Heavy Attack alignment improves
- Target death unlocks correctly
- Excessive distance unlocks correctly
- Player death clears lock state

The remaining weakness was locomotion: the player faced the enemy, but still lacked dedicated lock-on movement animations.

---

## 7/15 — Day 7: Lock-On Strafe Movement and Eight-Direction Locomotion

### Goal

Add dedicated movement presentation while locked on so the player can face the target while moving forward, backward, and sideways.

---

### Lock-On Getter

Added:

```cpp
GetIsLockedOn()
```

as a BlueprintPure getter for the player AnimBP.

As with `GetIsInCombat()`, the getter must be called from the `As WCCharacter` cast output to read the correct character instance.

---

### AnimBP Lock-On Variables

Added:

- `IsLockedOn`
- `MovementDirection`
- `ShouldUseLockOnMovement`

`Calculate Direction` uses:

- Player Velocity
- Player Actor Rotation

Typical results:

```text
0      = Forward
90     = Right
-90    = Left
180    = Backward
-180   = Backward
```

The lock-on movement condition is:

```text
IsLockedOn AND GroundSpeed > 10
```

---

### Combat Idle Condition Update

The previous Combat Idle condition only considered `IsInCombat`.

Lock-On should also place the player in a ready stance even before the first attack.

The AnimBP condition was updated to:

```text
ShouldUseCombatIdle =
(IsInCombat OR IsLockedOn) AND GroundSpeed < 10
```

Result:

- Attack state + stationary → Combat Idle
- Lock-On state + stationary → Combat Idle
- Lock-On state + moving → Strafe locomotion
- Exploration state → Normal locomotion/Idle

---

### Initial Four-Direction Strafe Blend Space

Created:

```text
BS_LockOn_Strafe
```

Type:

```text
1D Blend Space
```

Axis:

```text
Movement Direction
-180 to 180
```

Initial samples:

```text
-180 / 180 = Running Backward
-90        = Left Strafe
0          = Forward Run / Jog
90         = Right Strafe
```

The AnimGraph combines:

```text
Main States
→ Combat Idle selection
→ Lock-On movement selection
→ DefaultSlot
→ Control Rig
→ Output Pose
```

Keeping `DefaultSlot` after locomotion selection allows attack Montages to override Strafe animation correctly.

---

### Lock-On Movement Speed

Added:

```cpp
NormalWalkSpeed
LockOnWalkSpeed
```

`BeginPlay()` records the normal movement speed.

On lock:

```text
MaxWalkSpeed = LockOnWalkSpeed
```

On unlock:

```text
MaxWalkSpeed = NormalWalkSpeed
```

This gives the lock-on state a more controlled combat pace.

---

### Movement Input

`Move()` remained unchanged.

Because the lock-on update turns Controller Yaw toward the target:

- `W` moves toward the target
- `S` moves away while facing the target
- `A` strafes left
- `D` strafes right

This avoided unnecessary input-system changes.

---

### Four-Direction Result

The initial version successfully supported:

- Forward movement
- Backward running
- Left Strafe
- Right Strafe
- Combat Idle when stationary
- Normal locomotion after unlock
- Attack Montage override

However, diagonal movement looked unnatural.

With only four samples:

- `W + A`
- `W + D`
- `S + A`
- `S + D`

had to blend directly between forward/backward and side animations.

This produced visible foot sliding, torso twisting, and inconsistent step rhythm.

---

### Diagonal Animation Expansion

Prepared four in-place diagonal animations:

- Forward Left
- Forward Right
- Backward Left
- Backward Right

The Blend Space was expanded to eight directions:

```text
-180 = Running Backward
-135 = Backward Left
-90  = Left Strafe
-45  = Forward Left
0    = Forward Run / Jog
45   = Forward Right
90   = Right Strafe
135  = Backward Right
180  = Running Backward
```

The same backward animation was placed at both axis ends to support the `-180 / 180` boundary.

Result:

- Diagonal direction selection became correct
- Movement looked more natural than four-direction blending
- The player could circle the target with a more complete combat-locomotion presentation

---

### Running Backward Jitter

A new issue appeared during continuous backward movement.

Observed behavior:

- The character occasionally twisted briefly left or right
- The body returned to Running Backward immediately afterward
- The movement system itself remained functional

Cause:

- Pure backward direction sits near the `-180 / 180` boundary
- Small direction fluctuations crossed the Blend Space boundary
- Axis smoothing interpolated through unintended directional samples
- This produced a brief sideways animation blend and visible body rotation

---

### Smoothing Fix

The final solution was:

```text
BS_LockOn_Strafe Smoothing Time = 0
```

Removing extra axis smoothing stopped the unwanted interpolation.

Result:

- Running Backward jitter disappeared
- Backward movement remained stable
- Eight-direction input became more direct
- Direction transitions were less smoothed but remained acceptable in testing

This was accepted as the more stable project-specific tradeoff.

---

### Final Combat Integration Test

The final Week 4 integration test covered:

- Normal exploration locomotion
- Lock-On and unlock
- Lock-On Combat Idle
- Eight-direction Strafe Movement
- Light Combo from movement
- Heavy Attack from movement
- Root Motion attack facing
- Attack Anim Notify
- Combo Window
- Input Buffer
- Light finisher feedback
- Heavy feedback
- Local Hit Stop
- Hit and whiff audio
- Hit VFX
- Camera shake
- Enemy health bar
- Enemy Hit Reaction
- Enemy death
- Player death and cleanup

All major systems remained functional together.

---

### Final Week 4 Recording

A final showcase recording was completed.

The recording documents:

- Four-hit Light Combo
- Heavy Attack
- Combat Idle
- Root Motion
- Animation-timed hit detection
- Combo Window and Input Buffer
- Light-combo finisher feedback
- Heavy Attack feedback
- Hit Stop
- Lock-On
- Lock-On Strafe Movement
- Eight-direction locomotion
- Enemy and player combat feedback

This recording serves as the Week 4 development milestone record.

---

## Final Week 4 System State

At the end of Week 4, the project supports:

### Attack Animation and Movement

- `AM_Player_LightCombo`
- `AM_Player_HeavyAttack`
- Root Motion from Montages Only
- Root Motion-based forward attack movement
- Stable Capsule follow behavior in current tests
- Attack Montage override through `DefaultSlot`

### Attack Data Architecture

- `FPlayerAttackData`
- `LightAttackData`
- `HeavyAttackData`
- `CurrentAttackData`
- `LightComboAttackData`
- Shared `PerformCurrentAttackTrace()`
- Attack-specific damage, range, Box size, duration, and knockback

### Light Combo

- `Light_1`
- `Light_2`
- `Light_3`
- `Light_4`
- Independent attack data per hit
- Combo index progression
- Combo timeout reset
- Heavy Attack combo reset
- C++ Montage stopping
- Player attack Anim Notify
- Duplicate-hit protection
- Combo Window
- Input Buffer
- Stronger fourth-hit feedback

### Heavy Attack

- Right Mouse Button input
- Side-kick Montage
- Independent Heavy Attack data
- Real hit-frame Notify
- Stronger damage
- Larger range
- Stronger knockback
- Dedicated hit sound
- Dedicated hit effect
- Dedicated camera shake
- Strongest Hit Stop tier

### Combat Feedback

- Normal light-hit feedback
- Light finisher feedback
- Heavy Attack feedback
- Hit Sound
- Whiff Sound
- Hit VFX
- Camera Shake
- Local player/enemy Hit Stop
- Feedback priority protection
- No strong feedback on whiff

### Combat State and Animation

- `bIsInCombat`
- Timed Combat Idle
- `GetIsInCombat()`
- Combat Idle after attacks
- Combat Idle while locked and stationary
- Normal locomotion while not in combat
- Attack Montage overlay over locomotion states

### Lock-On

- `IA_LockOn`
- Target search
- Distance/camera-based target selection
- Player facing
- Controller yaw alignment
- Automatic break distance
- Automatic unlock on target death
- Automatic unlock on player death
- Attack-facing correction
- Improved Light and Heavy attack direction

### Lock-On Locomotion

- `GetIsLockedOn()`
- `BS_LockOn_Strafe`
- Forward movement
- Backward movement
- Left and Right Strafe
- Forward Left / Forward Right
- Backward Left / Backward Right
- Lock-On movement speed
- Exploration speed restoration
- Stable backward movement with Smoothing Time set to `0`

### Existing Week 3 Compatibility

The following Week 3 systems remained functional:

- Player Health UI
- Defeated UI
- Enemy head health bar
- Player hurt feedback
- Player death sound
- Player ragdoll
- Enemy hurt sound
- Enemy attack sound
- Enemy death sound
- Hit VFX
- Enemy Hit Reaction
- Enemy Hit Stun
- Enemy death flow
- Huangquan Road combat test area

---

## Week 4 Completion Review

### Original Core Goals

All original Week 4 goals were completed:

- Animation audit
- Root Motion / in-place decision
- Heavy Attack prototype
- Structured attack data
- Player attack Anim Notify
- Four-hit Light Combo
- Combat Idle
- Parameter tuning
- Integration testing
- Showcase recording

### Advanced Extension Goals

The additional Week 4 extensions were also completed:

- Combo Window
- Input Buffer
- Light Combo finisher feedback
- Local Hit Stop
- Heavy Attack strong feedback
- Lock-On prototype
- Lock-On Strafe Movement
- Eight-direction lock-on locomotion
- Final integrated recording

---

## Known Limitations and Future Polish

### Combo Animation Transitions

The source light-combo animation was authored as one continuous sequence.

When individual sections stop and return to Combat Idle:

- Some transitions remain abrupt
- `Light_3` has the clearest pose discontinuity
- Combat Idle improves the result but does not fully reconstruct missing recovery animation

Possible future improvements:

- Better attack-specific recovery animations
- Montage blend tuning
- Dedicated transition animations
- A different combo animation set designed for separate inputs

---

### Combo Window Timing

The current Combo Window uses timers and `ComboWindowOpenRatio`.

It works well enough for the prototype, but future polish could use dedicated Anim Notifies for:

- Open Combo Window
- Close Combo Window
- Attack recovery
- Cancel windows

The timer-based system remains the preferred current solution because it is stable and appropriately scoped.

---

### Root Motion Camera Comfort

Root Motion attack movement currently feels grounded and passes collision tests.

A slight camera movement remains visible during attacks.

Future polish may include:

- Camera lag tuning
- Spring Arm tuning
- Attack-specific camera behavior
- Additional long-session comfort testing

---

### Lock-On Scope

The current Lock-On is intentionally a prototype.

It does not yet include:

- Target switching
- Lock-On indicator UI
- Line-of-sight obstruction handling
- Soft lock / aim assist
- Boss-specific camera behavior
- Vertical camera tracking
- Multiple target cycling

---

### Lock-On Locomotion

Eight-direction animation greatly improved diagonal movement.

Remaining possible issues include:

- Different animation step rates
- Minor foot sliding
- Asset-to-asset stance inconsistency
- More direct direction changes because Smoothing Time is `0`

Future polish could use:

- Animation Rate Scale tuning
- Stride Warping
- Orientation Warping
- Distance Matching
- More consistent animation resources
- A full 2D speed/direction Blend Space
- Motion Matching

These are not required for the current gameplay prototype.

---

## Week 4 Summary

Week 4 transformed the Week 3 player combat system from a single light attack into a structured and presentation-ready unarmed combat prototype.

The system now combines:

- Root Motion
- Four-hit Light Combo
- Heavy Attack
- Data-driven attack parameters
- Anim Notify hit timing
- Combo Window
- Input Buffer
- Combat Idle
- Finisher feedback
- Heavy feedback
- Hit Stop
- Lock-On
- Eight-direction Strafe locomotion

The week also produced multiple integration tests and a final showcase recording.

The resulting combat slice is still intentionally smaller than a complete commercial action system, but it now demonstrates a clear Gameplay Programmer progression:

```text
Single attack
→ Structured attack data
→ Animation-timed combat
→ Four-hit combo
→ Buffered input
→ Layered impact feedback
→ Lock-On targeting
→ Lock-On combat locomotion
```

Week 4 completed both its original Unarmed Combat Expansion target and its controlled advanced extensions while keeping the existing Week 1–3 systems stable.
