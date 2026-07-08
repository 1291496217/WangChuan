# Development Log — Week 3

**Project:** WangChuan / 《忘川河畔》  
**Engine:** Unreal Engine 5.4  
**Focus:** Combat feedback polish, player/enemy UI, audio/VFX feedback, player hit/death feedback, Huangquan Road showcase area, and recording-ready integration  
**Week Theme:** Combat Polish & Showcase Foundation — from Combat MVP to a readable, recordable gameplay slice

---

## Week 3 Goal

Week 3 focused on polishing the Week 2 combat MVP into a clearer and more showcase-ready gameplay demo.

The goal was not to add large new systems. Instead, the focus was to make the existing combat loop easier to read, easier to feel, and suitable for a short gameplay recording.

By the end of Week 3, the project added:

- Toggleable attack debug draw
- Player Health UI and Defeated UI
- Enemy head health bar
- Enemy UI polish for death, distance, and screen overlap
- Player and enemy combat sound feedback
- Attack hit VFX
- Player hit flash and camera shake
- Player hurt sound and death ragdoll
- Huangquan Road greybox combat showcase area
- Enemy ground snapping for uneven terrain
- A Week 3 showcase recording
- GitHub update for the Week 3 milestone

The Week 3 priority remained:

```text
Completion > Playability > Extensibility > Visual Quality
```

---

## 7/2 — Day 1: Debug Cleanup & Combat UI Foundation

### Goal

Start Week 3 by reducing debug-only presentation and replacing key combat feedback with simple player-facing UI.

The goal was to make the combat system cleaner for testing and recording without deleting useful debug tools.

---

### Attack Debug Toggle

Added a debug toggle to `AWCCharacter`:

```cpp
bShowAttackDebug
```

This controls whether the player attack Box Trace debug draw should be visible.

Behavior:

- If `bShowAttackDebug == true`, the Box Trace is displayed.
- If `bShowAttackDebug == false`, the Box Trace is hidden.
- `ShowAttackHitDebug()` also checks `bShowAttackDebug` before displaying hit actor debug text.

This allows the project to keep useful development tools while producing clean showcase recordings.

---

### Player Health UI

Created a simple UMG widget:

```text
WBP_PlayerHUD
```

Initial UI elements:

- Player Health label
- Player Health Progress Bar
- Defeated Text, hidden by default

Added UI support to `AWCCharacter`:

- `PlayerHUDClass`
- `PlayerHUDWidget`
- Widget creation in `BeginPlay()`
- `AddToViewport()` for runtime display

Added:

```cpp
GetHealthPercent()
```

This returns `Health / MaxHealth` and is used by the player health Progress Bar.

---

### Defeated UI

Connected the Defeated Text visibility to the player's dead state.

Behavior:

- Player alive: Defeated text hidden
- Player dead: Defeated text visible

This replaced the earlier reliance on Debug Message for player defeat feedback.

---

### Player Health Debug Cleanup

Since the player health bar now displayed the player's health state, the old `Player Health` Debug Message in `ReceiveDamage()` was disabled.

Core damage logic remained unchanged:

- Damage subtraction
- Health clamp
- Death check
- `Die()` call when `Health <= 0`

---

## 7/2 — Day 1 Advanced: Enemy Head Health Bar

### Goal

Replace enemy health Debug Message with an in-world enemy head health bar.

The goal was to let players directly see enemy health changes during combat.

---

### Enemy Widget Component

Added to `AGhostEnemy`:

```cpp
UWidgetComponent* HealthWidgetComponent
```

The widget component is attached to the enemy and displays a UMG health bar above the enemy.

Added:

```cpp
GetHealthPercent()
```

This gives the enemy health bar a `0.0` to `1.0` value.

---

### Initial Issue: Wrong `Get Owning Actor` Node

The first attempt tried to use `Get Owning Actor` inside `WBP_EnemyHealthBar`.

Problem:

- The node found in Blueprint was the `AnimInstance` version of `Get Owning Actor`.
- Its target expected an animation instance.
- `WBP_EnemyHealthBar` is a `UserWidget`, not an `AnimInstance`.
- This caused the compile error:

```text
self is not a AnimInstance
```

### Cause

The health bar logic was conceptually correct, but the Blueprint node came from the wrong context.

The widget should not try to guess which actor owns it through an unrelated node.

---

### EnemyHealthBarWidget C++ Class

Created a dedicated C++ widget class:

```text
EnemyHealthBarWidget
```

Parent class:

```cpp
UUserWidget
```

It stores:

```cpp
AGhostEnemy* EnemyOwner
```

Added functions:

```cpp
SetEnemyOwner(AGhostEnemy* NewEnemyOwner)
GetEnemyHealthPercent()
```

`AGhostEnemy::BeginPlay()` now:

1. Gets the widget from `HealthWidgetComponent`
2. Casts it to `UEnemyHealthBarWidget`
3. Calls `SetEnemyOwner(this)`

`WBP_EnemyHealthBar` was reparented to `EnemyHealthBarWidget`.

Its Progress Bar binding simply calls:

```text
Get Enemy Health Percent
```

This is more stable and clearer than trying to find an owner from inside the widget.

---

### Enemy Health Debug Cleanup

The old `Ghost Hit! Health` Debug Message in `AGhostEnemy::TakeHit()` was disabled.

Enemy health is now shown through the head health bar instead.

---

## 7/2 — Day 1 Advanced Extra: Enemy UI Polish

### Goal

Polish the enemy head health bar so it looks less like a debug widget and more like gameplay UI.

---

### Problem 1: UI Size Did Not Change With Distance

The health bar initially used Screen Space.

Screen Space keeps UI readable but does not naturally scale with world distance.

### Fix

Changed the widget component to World Space.

Result:

- Enemy health bar appears as part of the world
- It becomes smaller when far away
- It becomes larger when close
- It fits better as a head health bar

---

### Problem 2: UI Overlap With Player View

In close third-person combat, the enemy health bar could overlap the player or enemy body.

### Fix

Adjusted:

- Relative Location
- Draw Size
- Relative Scale

The health bar was moved higher and scaled down so it stays readable without dominating the screen.

---

### Problem 3: Health Bar Stayed After Enemy Death

Because enemy death uses a delayed destroy flow, the health widget stayed visible while the death animation played.

### Fix

In `AGhostEnemy::Die()`, the widget is hidden immediately:

```cpp
HealthWidgetComponent->SetVisibility(false)
```

Enemy death still keeps the original flow:

```text
Die()
↓
Death animation / death state
↓
DeathTimerHandle
↓
FinishDeath()
↓
Destroy()
```

---

### Blueprint Override Note

After changing component defaults in C++, `BP_GhostEnemy_New` still needed to be checked.

Existing Blueprint children may keep old component override values, so the widget size, location, and display mode were reconfigured in `BP_GhostEnemy_New`.

---

### Result

- Enemy head health bar displays correctly
- Health bar scales naturally with distance
- Health bar no longer noticeably blocks close combat
- Health bar hides immediately when enemy dies
- Existing enemy damage, hit reaction, knockback, death, and delayed destroy logic remained stable

---

## 7/3 — Day 2: Combat Sound Feedback

### Goal

Add core combat sound feedback so the player can understand hit, whiff, and death events without relying on debug messages.

The sound direction remained restrained and atmospheric to avoid making the game feel cheap or overly loud.

---

### Audio Asset Pipeline

Established a simple audio production pipeline:

```text
Stable Audio AI
↓
mp3 generation
↓
Audacity cleanup / trimming
↓
wav export
↓
Unreal Engine import
```

Imported audio into:

```text
Content/WangChuan/Audio/Combat
```

Initial sound assets included:

- `SFX_Player_Attack_Hit`
- `SFX_Player_Attack_Whiff`
- `SFX_EvilGhost_Death`
- `SFX_Player_Death`

---

### Player Combat Audio

Added to `AWCCharacter`:

- `AttackHitSound`
- `AttackWhiffSound`
- `PlayerDeathSound`

Used:

```cpp
UGameplayStatics::PlaySoundAtLocation()
```

for attack sounds.

Used:

```cpp
UGameplayStatics::PlaySound2D()
```

for player death sound, since player death should always be audible as a global failure feedback.

---

### Attack Hit / Whiff Logic

`HandleAttackHit()` was changed from `void` to `bool`.

Behavior:

- Returns `true` if the hit actor is a valid `AGhostEnemy`
- Returns `false` if no valid enemy is hit

`PerformAttackTrace()` now uses this return value:

- Valid enemy hit: play attack hit sound
- Non-enemy hit or no hit: play attack whiff sound

This avoids playing a strong hit sound when the player only hits a wall, rock, or other non-enemy actor.

---

### Enemy Death Sound

Added to `AGhostEnemy`:

```cpp
EvilGhostDeathSound
```

Played inside `Die()` after the `bIsDead` guard, so it only triggers once.

---

### Result

- Player attack hit sound works
- Player attack whiff sound works
- Evil ghost death sound works
- Player death sound works
- Combat feedback no longer relies only on visual debug text

---

## 7/3 — Day 2 Advanced: Evil Ghost Combat Audio

### Goal

Add audio feedback for the evil ghost's own combat actions while keeping sound timing consistent with gameplay logic.

---

### Added Evil Ghost Audio

Added to `AGhostEnemy`:

- `EvilGhostAttackHitSound01`
- `EvilGhostAttackHitSound02`
- `EvilGhostAttackWhiffSound`
- `EvilGhostHurtSound`

Added helper functions:

- `PlayEvilGhostAttackHitSound()`
- `PlayEvilGhostAttackWhiffSound()`
- `PlayEvilGhostHurtSound()`

The attack hit sound randomly selects between two hit sounds to reduce repetition.

---

### Anim Notify Timing

Enemy attack sound was not placed in `TryAttackPlayer()`.

Reason:

- `TryAttackPlayer()` only means the attack has started.
- The real hit frame is controlled by the attack animation's Anim Notify.
- Playing hit audio at attack start would create incorrect feedback if the player moved away before the hit frame.

Instead, enemy attack hit and whiff audio were placed in:

```cpp
OnEnemyAttackHit()
```

This is the same function called by the attack Anim Notify.

---

### Hit / Whiff Logic

`OnEnemyAttackHit()` now checks whether the player is still within `AttackRange` when the Anim Notify fires.

Behavior:

- Player still in range: play attack hit sound and call `DealDamageToPlayer()`
- Player out of range: play attack whiff sound and do not deal damage

This makes enemy audio match the actual gameplay result.

---

### Enemy Hurt Sound

`EvilGhostHurtSound` plays when the enemy is hit but not killed.

If the hit kills the enemy:

- Do not play normal hurt sound
- Only play death sound

This avoids stacking hurt and death sounds on the final hit.

---

### Result

- Enemy attack hit sound matches actual damage timing
- Enemy whiff sound plays when the player escapes before the hit frame
- Enemy hurt sound plays on non-lethal hits
- Enemy death sound remains distinct
- Combat audio feedback became clearer and less repetitive

---

## 7/4 — Day 3: Simple Hit VFX

### Goal

Add a simple visual effect when the player attack successfully hits an enemy.

The goal was to make hit confirmation visible, not to build a full VFX system.

---

### Initial Plan

The original plan was to use Starter Content particles as placeholder hit feedback.

After testing, a better placeholder was imported:

```text
Realistic Starter VFX Pack
```

A blood spray effect was selected as the temporary hit VFX.

This fit the current direction better because the evil ghost is treated as a physical enemy, not a purely ghost-like target.

---

### Player Hit VFX Variables

Added to `AWCCharacter`:

- `AttackHitEffect`
- `AttackHitEffectScale`

Added:

```cpp
PlayAttackHitEffect(const FHitResult& HitResult)
```

This function uses:

- `HitResult.ImpactPoint` as the preferred spawn location
- `HitResult.Location` as fallback
- `UGameplayStatics::SpawnEmitterAtLocation()`

---

### HandleAttackHit Update

`HandleAttackHit()` now receives:

```cpp
const FHitResult& HitResult
```

The hit effect only spawns when the hit actor is confirmed to be an `AGhostEnemy`.

This prevents hit VFX from appearing when the player hits non-enemy objects.

---

### Result

- Player attack hit spawns a short hit VFX
- Whiff attacks do not spawn VFX
- Non-enemy hits do not spawn VFX
- Hit VFX works together with hit sound, enemy health bar, hit reaction, and knockback

---

## 7/5 — Day 4: Player Hit Flash

### Goal

Add a simple screen flash when the player is actually damaged by an enemy.

This provides immediate visual feedback without adding player hit reaction animations or new combat states.

---

### Player Hit Flash Widget

Created:

```text
WBP_PlayerHitFlash
```

Widget setup:

- Fullscreen Image
- Initial opacity set to 0
- Short animation: transparent → red flash → transparent

The flash is intentionally short and subtle.

---

### UMG Animation Note

When adding animation to Image Color and Opacity, Unreal first creates a Section.

The Section must be selected before keyframes can be added.

For this implementation, the Section type was set to:

```text
Absolute
```

This was necessary before setting Color and Opacity keyframes.

---

### PlayerHitFlashWidget C++ Class

Created:

```text
PlayerHitFlashWidget
```

Parent:

```cpp
UUserWidget
```

Declared:

```cpp
BlueprintImplementableEvent PlayHitFlash()
```

`WBP_PlayerHitFlash` was reparented to `PlayerHitFlashWidget` and implements the event by playing the hit flash animation.

This allows C++ to trigger the widget while Blueprint controls the animation.

---

### Player Hit Feedback Entry Point

Added to `AWCCharacter`:

```cpp
PlayPlayerHitFeedback()
```

This function calls the hit flash widget.

It is triggered from:

```cpp
ReceiveDamage()
```

Because `ReceiveDamage()` only runs when the player actually takes damage, enemy whiff attacks do not trigger the flash.

---

### Result

- Player damage triggers a short red flash
- Enemy whiff does not trigger red flash
- Player death still displays Defeated UI correctly
- No player animation or input state was changed

---

## 7/5 — Day 4 Advanced: Player Hit Camera Shake

### Goal

Add a small camera shake when the player is hit.

The goal was to add impact without making the camera uncomfortable.

---

### Camera Shake Blueprint

Created:

```text
BP_PlayerHitCameraShake
```

Parent:

```text
CameraShakeBase
```

The shake was kept short and low intensity.

---

### C++ Integration

Added to `AWCCharacter`:

```cpp
HitCameraShakeClass
```

`PlayPlayerHitFeedback()` now:

1. Plays the red flash
2. Starts the camera shake through `PlayerCameraManager`

The camera shake is also triggered only through `ReceiveDamage()`.

---

### Result

- Player hit causes slight camera shake
- Enemy whiff does not shake the camera
- Player death flow remains stable

---

## 7/5 — Day 4 Extra: Player Hurt Sound

### Goal

Add player hurt sound to complete the ordinary player hit feedback loop.

---

### Player Hurt Sound

Added to `AWCCharacter`:

```cpp
PlayerHurtSound
```

This is played through:

```cpp
UGameplayStatics::PlaySound2D()
```

inside `PlayPlayerHitFeedback()`.

---

### Death Sound Separation

`ReceiveDamage()` was adjusted so that normal hurt feedback only plays if the player survives the hit.

Behavior:

- Non-lethal damage: play hurt sound, red flash, camera shake
- Lethal damage: call `Die()` and play player death sound only

This avoids stacking hurt and death sounds on the final hit.

---

### Result

- Player ordinary hit sound works
- Player death sound remains clean
- Ordinary hit and death feedback are clearly separated

---

## 7/5 — Day 4 Extra Plus: Player Death Ragdoll Prototype

### Goal

Add a simple death ragdoll prototype to make player defeat more visible.

This was treated as a prototype, not a final death animation system.

---

### Ragdoll Setup

Checked that the player skeletal mesh has a usable Physics Asset.

Updated `AWCCharacter::Die()`:

- Stop movement
- Disable movement
- Disable player input
- Disable capsule collision
- Set mesh collision profile to `Ragdoll`
- Enable physics simulation on the mesh
- Wake all rigid bodies

Death feedback now includes:

- Player death sound
- Defeated UI
- Input disabled
- Ragdoll fall

---

### Result

- Player enters ragdoll on death
- Defeated UI appears
- Player input remains disabled
- Enemy stops acting after player death
- No major flying, spinning, or ground penetration issue occurred during testing

---

## 7/6 — Day 5: Huangquan Road Combat Test Area

### Goal

Create a small combat test area that can be reused as the beginning of the Huangquan Road section.

The goal was to avoid a disconnected arena and instead build a playable greybox area that supports both combat testing and future level expansion.

---

### Initial Area Structure

Built an initial Huangquan Road greybox segment with:

- Player start /荒地苏醒处
- Main road section
- Broken tablet / interactable stone as a route marker
- Left side branch path
- Small combat clearing
- Two placed enemies for testing

The first version established the basic route, but the combat clearing still looked too rectangular and test-platform-like.

---

### Result

- A playable combat test route existed
- Player could move from start area into the combat clearing
- Enemy placement supported combat testing
- The area still needed more natural boundaries and stronger Huangquan Road identity

---

## 7/7 — Day 6: Huangquan Road Greybox Polish

### Goal

Polish the Day 5 greybox so the area feels more like a reusable Huangquan Road starting segment.

---

### Broken Edge Pass

The edges of the combat clearing and other map regions were broken up using placeholder UE blockout meshes.

This reduced the feeling of a rectangular test platform and made the area feel more like a fragmented underworld road.

---

### Visual Guidance

Added a large slanted block monument visible from the player start.

Current role:

- Strong visual landmark
- Main route guidance
- Placeholder for a future giant dead tree, broken tablet, stone gate, or other Huangquan Road silhouette

---

### Branch Path and Return Loop

Improved the left side path:

- Added a route marker
- Added a memory fragment placeholder
- Built a small return path back to the main combat area

This created a simple exploration loop:

```text
Main path
↓
Side branch
↓
Small discovery / reward
↓
Return to main route
```

---

### Far Silhouette

Built a simple high platform silhouette in the distance.

Purpose:

- Suggest future route continuation
- Add background structure
- Avoid the feeling that the level ends abruptly

---

### Grave Mound Placeholders

Added simple grave mound placeholders using UE meshes.

The grave mounds were given a soil-like color to distinguish them from the grey blockout.

They help suggest a Huangquan Road /荒冢 atmosphere even before final art assets are available.

---

### Scale Adjustment

Reduced the overall map space to make the starting area more compact.

This improved:

- Player pacing
- Showcase recording flow
- Combat encounter timing
- Initial area readability

---

### Result

The area now contains:

- Player wake-up area
- Main route
- Route marker
- Side branch
- Memory fragment placeholder
- Return loop
- Combat clearing
- Enemy placement
- Far platform silhouette
- Grave mound placeholders
- Broken greybox boundaries

The greybox now feels more like a Huangquan Road starting area rather than a generic combat arena.

---

## 7/8 — Day 7: Integration, Tuning & Showcase Recording

### Goal

Integrate the Week 3 systems, fix issues found during full-area testing, tune gameplay values, and record the Week 3 showcase.

---

### Enemy Grounding Issue

During testing in the Huangquan Road greybox, enemies sometimes floated when moving across height differences such as slopes or down ramps.

### Cause

`AGhostEnemy` still uses simple manual movement:

```text
Direction.Z = 0
SetActorLocation(NewLocation)
```

This moves the enemy only on the XY plane and does not automatically follow the ground height.

Since `AGhostEnemy` is not currently using `CharacterMovementComponent`, it does not naturally handle slopes, gravity, or floor snapping.

---

### SnapToGround Fix

Added a lightweight ground snapping solution.

New logic:

1. Trace downward from above the enemy
2. Detect the ground below
3. Adjust enemy Z location to the ground hit point plus an offset

Added tuning parameters:

- `GroundTraceStartHeight`
- `GroundTraceEndDepth`
- `GroundOffset`

`MoveTowardPlayer()` now calls `SnapToGround()` after moving.

This keeps the current simple enemy movement system while fixing the most visible floating issue in the greybox.

---

### Final Tuning

Adjusted values after testing:

- Enemy movement over uneven terrain
- Ground trace height/depth/offset
- Combat pacing inside the greybox
- Enemy placement and encounter flow
- Recording-readiness of UI, VFX, audio, and death feedback

---

### Showcase Recording

Recorded the Week 3 showcase in the Huangquan Road greybox area.

The recording demonstrates:

- Player moving through the greybox Huangquan Road area
- Enemy encounter
- Enemy chase and attack
- Player health UI
- Player hit feedback
- Player attack hit sound and VFX
- Enemy health bar
- Enemy hit reaction
- Enemy death
- Player defeat
- Player ragdoll death

The full recording is about 90 seconds and serves as a development showcase record.

A shorter 30–45 second version can later be edited for portfolio presentation.

---

### GitHub Update

After completing integration and recording, the Week 3 version was committed and uploaded to GitHub.

This created a clear version-control checkpoint for the Combat Polish & Showcase Foundation milestone.

---

## Final Week 3 System State

At the end of Week 3, the project supports:

### Combat Debug

- Player attack Box Trace debug draw can be toggled with `bShowAttackDebug`
- Attack hit debug messages can be hidden for recording
- Player/enemy health debug messages have been replaced by UI

### Player UI and Feedback

- Player Health Bar
- Defeated UI
- Player hit flash
- Player hit camera shake
- Player hurt sound
- Player death sound
- Player death ragdoll

### Enemy UI and Feedback

- Enemy head health bar
- Enemy health percent binding through `EnemyHealthBarWidget`
- Enemy UI distance/scale polish
- Enemy health bar hides immediately on death
- Enemy hurt sound
- Enemy attack hit sound
- Enemy attack whiff sound
- Enemy death sound

### Combat Audio

- Player attack hit sound
- Player attack whiff sound
- Player hurt sound
- Player death sound
- Enemy attack hit sounds with simple random variation
- Enemy attack whiff sound
- Enemy hurt sound
- Enemy death sound
- Stable Audio AI → Audacity → wav → UE import pipeline

### Combat VFX

- Player attack hit VFX
- Hit VFX spawns only on valid enemy hits
- Non-enemy hits and whiffs do not spawn hit effects

### Enemy Movement

- Original simple chase behavior retained
- Added `SnapToGround()` to reduce floating over uneven greybox terrain
- Long-term AI movement can later be upgraded to CharacterMovement / NavMesh if needed

### Level / Showcase Area

- Huangquan Road greybox combat segment
- Player wake-up area
- Main route
- Side branch
- Memory fragment placeholder
- Return loop
- Combat clearing
- Grave mound placeholders
- Far silhouette / high platform
- Broken boundaries for a less arena-like shape

---

## Week 3 Completion Review

### Minimum Success Standard

All minimum Week 3 goals were completed:

- Box Trace Debug Draw toggle
- Player Health UI
- Player Defeated UI
- At least 2 basic sound effects
- At least 1 hit visual feedback
- Small combat test area
- Simple combat showcase recording

### Ideal Success Standard

All ideal Week 3 goals were completed:

- Attack hit sound
- Attack whiff sound
- Enemy hurt sound
- Enemy death sound
- Player hurt sound
- Player Health Bar
- Defeated Screen
- Hit VFX
- Player hit flash
- Camera Shake
- Combat test area
- GitHub update
- Week 3 showcase video

---

## Week 3 Summary

Week 3 successfully transformed the Week 2 Combat MVP from a technically functional prototype into a readable and recordable gameplay showcase segment.

The combat system now communicates through:

- UI
- Animation
- Sound
- VFX
- Camera feedback
- Ragdoll death
- Greybox level context

The project now has a playable Huangquan Road combat slice that can serve as a foundation for future Week 4 work and later portfolio presentation.
