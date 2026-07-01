# Development Log — Week 2

**Project:** WangChuan / 《忘川河畔》  
**Engine:** Unreal Engine 5.4  
**Focus:** Player attack, ghost enemy combat prototype, animation-driven combat feedback, and code cleanup  
**Week Theme:** Combat MVP — from attack input to a maintainable two-way combat loop

---

## Week 2 Goal

Week 2 focused on building the first complete combat prototype for WangChuan. The original goal was to let the player attack an enemy, let the enemy receive damage, and create a basic but expandable combat loop.

By the end of Week 2, the system had grown beyond the original MVP:

- Player attack input and attack montage playback
- Player attack rhythm lock
- Box Trace based melee hit detection
- Ghost enemy health, death, and delayed destruction
- Ghost enemy Idle / Walk / Attack / Hit Reaction / Death animation states
- Enemy chase and attack behavior
- Real player health and defeated state
- Enemy attack damage driven by Anim Notify
- Hit Reaction and Hit Stun replacing simple color feedback
- Future-facing hit data preparation for weapon attacks
- Code cleanup and lightweight refactor for `AGhostEnemy` and `AWCCharacter`

---

## 6/25 — Day 1: Player Attack Input Prototype

### Goal

Create the first player attack input pipeline and verify that left mouse click can trigger attack logic in C++.

### Attack Input Setup

- Created `IA_Attack`.
- Set `IA_Attack` to Boolean type.
- Added `IA_Attack` to `IMC_Default`.
- Bound Left Mouse Button as the attack input.

### C++ AttackAction Integration

- Added `AttackAction` to `AWCCharacter`.
- Exposed `AttackAction` to Blueprint under the Input category.
- Declared and implemented `Attack()`.
- Used a Debug Message to confirm that the attack input was correctly triggered.

### Input Binding Optimization

Updated `SetupPlayerInputComponent()` so all input bindings check whether the corresponding Input Action is assigned before binding.

Input Actions with null checks:

- `MoveAction`
- `LookAction`
- `JumpAction`
- `InteractAction`
- `JournalAction`
- `AttackAction`

This prevents crashes or null pointer issues if an Input Action is not assigned in Blueprint.

### Result

Attack input successfully triggered from Left Mouse Button.

---

## 6/25 — Day 1 Advanced: Attack Anim Montage Prototype

### Goal

Connect the player attack input to an attack animation montage.

### Initial Animation Resource

- Imported `FreeAnimationLibrary` from Fab.
- Selected a temporary attack animation.
- Created `AM_LightAttack` from the animation.
- Added `LightAttackMontage` to `AWCCharacter`.
- Played the montage inside `Attack()` using `Montage_Play()`.
- Added simple null checks and Debug Message fallback.

### Blueprint Configuration

- Compiled C++.
- Assigned `AM_LightAttack` to `LightAttackMontage` in `BP_PlayerCharacter`.
- Confirmed that the player was using the correct Anim Blueprint.

### Animation Direction Issue

The initial `anim_Counter_Attack` animation played as a side-facing attack and did not work well as a forward light attack.

#### Investigation

- Initially suspected Mesh rotation or Blueprint rotation settings.
- Adjusted rotation, but the issue remained.
- Concluded that the animation itself was not suitable for a forward-facing light attack.

### Mixamo Replacement

- Downloaded a more suitable attack animation from Mixamo.
- Created a Mixamo folder under Animations.
- Imported the animation.
- Retargeted it to the Manny skeleton.
- Created a new montage, still named `AM_LightAttack`.
- Reassigned `LightAttackMontage` in `BP_PlayerCharacter`.

### Result

Player attack montage successfully played with a more suitable forward attack animation.

---

## 6/25 — Post-Week1: Character Rotation Fix

### Issue

When pressing `S`, the character moved backward while still facing forward, which looked like backward running.

### Cause

Movement input was working, but the character was not rotating toward movement direction.

### Fix

Updated `AWCCharacter` rotation settings:

```cpp
bUseControllerRotationPitch = false;
bUseControllerRotationYaw = false;
bUseControllerRotationRoll = false;
GetCharacterMovement()->bOrientRotationToMovement = true;
GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
```

### Result

- `W`: character runs forward.
- `S`: character turns and runs backward relative to camera direction.
- `A / D`: character turns toward movement direction.
- Mouse controls the camera only.

This is currently suitable for exploration mode. A future combat lock-on mode may require different rotation rules.

---

## 6/26 — Day 2: Ghost Enemy Health Prototype

### Goal

Create a basic ghost enemy actor with health, damage, and death logic.

### Ghost Enemy Class

- Created `AGhostEnemy` C++ class.
- Created `BP_GhostEnemy` as a Blueprint child.
- Placed `BP_GhostEnemy` in the map.

### Temporary Enemy Mesh

- Added `EnemyMesh` as a temporary `UStaticMeshComponent`.
- Configured a simple visual mesh in `BP_GhostEnemy`.
- Skeletal Mesh was intentionally postponed.

### Enemy Health

Added:

- `MaxHealth`
- `Health`

In `BeginPlay()`, initialized:

```cpp
Health = MaxHealth;
```

Blueprint changes to `MaxHealth` correctly affected runtime `Health`.

### Enemy Damage Logic

Implemented:

```cpp
TakeHit(float DamageAmount)
```

Behavior:

- Subtracts `DamageAmount` from `Health`.
- Displays current enemy health through Debug Message.
- Calls `Die()` when `Health <= 0`.

### Enemy Death Logic

Implemented `Die()`:

- Displays `Ghost Defeated` Debug Message.
- Calls `Destroy()`.

### Testing

Temporarily called `TakeHit()` from `BP_GhostEnemy` BeginPlay.

Tests:

- `TakeHit(25)` reduced health correctly.
- Multiple calls eventually reduced health to zero.
- Enemy destroyed correctly.
- Temporary test nodes were removed after testing.

### Result

Ghost enemy health and death prototype completed.

---

## 6/26 — Day 2 Advanced: Player Attack Box Trace

### Goal

Use Box Trace to detect enemies in front of the player during attack.

### Attack Detection Parameters

Added to `AWCCharacter`:

- `AttackDamage`
- `AttackRange`
- `AttackBoxHalfSize`
- `PerformAttackTrace()`

These parameters were exposed to `BP_PlayerCharacter` under the Combat category for tuning.

### Box Trace Attack Detection

Inside `Attack()`, called `PerformAttackTrace()`.

`PerformAttackTrace()` uses:

- `GetActorForwardVector()` as attack direction.
- `AttackRange` as trace distance.
- `AttackBoxHalfSize` as the box dimensions.
- `ActorsToIgnore` to ignore the player.
- Debug Draw to visualize the Box Trace.

### Enemy Hit Logic

When Box Trace hits an Actor:

- Displayed `Hit Actor` Debug Message.
- Cast the hit Actor to `AGhostEnemy`.
- If cast succeeded, called `TakeHit(AttackDamage)`.

### Attack Range Issue

Initial Box Trace was too large and too close to the player center. When the player stood close to an enemy while facing away, the enemy could still be hit.

### Fix

Moved the Box Trace start point forward from the player so the detection area better matched the attack direction.

### Tuned Parameters

Because the current attack animation is a short punch, the detection range was reduced.

Final test values:

- `AttackRange = 50`
- `AttackBoxHalfSize = FVector(10, 60, 70)`

### Result

Left mouse attack successfully hit the ghost enemy in front of the player and reduced enemy health.

---

## 6/26 — Day 2 Advanced Extra: Attack Rhythm Lock

### Goal

Prevent attack spamming and create a basic action-game attack rhythm.

### Attack State

Added:

- `bIsAttacking`
- `AttackDuration`
- `AttackTimerHandle`
- `EndAttack()`

### Attack Flow

`Attack()` now:

1. Checks `bIsAttacking`.
2. Returns if already attacking.
3. Sets `bIsAttacking = true`.
4. Plays the attack montage.
5. Calls `PerformAttackTrace()`.
6. Starts a Timer.
7. Calls `EndAttack()` after `AttackDuration`.

`EndAttack()` sets:

```cpp
bIsAttacking = false;
```

### Result

Attack rhythm lock worked correctly. Debug Messages were cleaned after testing.

---

## 6/27 — Day 3: Simple Enemy Hit Feedback

### Goal

Add simple feedback when the enemy is hit.

### Feedback Variables

Added to `AGhostEnemy`:

- `NormalColor`
- `HitColor`
- `HitFlashDuration`
- `KnockbackDistance`
- `DynamicMaterial`
- `HitFeedbackTimerHandle`

### Dynamic Material Setup

- Created `M_GhostEnemy` in the Materials folder.
- Added a Vector Parameter named `BaseColor`.
- Connected `BaseColor` to the material Base Color.
- Assigned `M_GhostEnemy` to `BP_GhostEnemy`.
- Created a Dynamic Material Instance in `BeginPlay()`.

### Hit Feedback Functions

Implemented:

- `ShowHitFeedback()`
- `ResetHitFeedback()`

Behavior:

- On hit, set `BaseColor` to `HitColor`.
- Start `HitFeedbackTimerHandle`.
- Restore `BaseColor` to `NormalColor` after `HitFlashDuration`.

### Knockback Feedback

Implemented `ApplyKnockback()`.

Original behavior:

- Calculates direction from player to enemy.
- Clears Z-axis influence.
- Normalizes direction.
- Moves enemy away from player by `KnockbackDistance`.

### TakeHit() Update

Enemy now:

- Subtracts health.
- Calls `ShowHitFeedback()`.
- Calls `ApplyKnockback()`.
- Calls `Die()` if health reaches zero.

### Die() Update

Cleared `HitFeedbackTimerHandle` before destroying the enemy to avoid Timer residue.

### Result

Enemy showed temporary color feedback and knockback when hit.

---

## 6/27 — Day 3 Advanced: Simple Ghost Enemy Behavior

### Goal

Give the ghost enemy simple chase and attack behavior.

### Tick Enabled

Set:

```cpp
PrimaryActorTick.bCanEverTick = true;
```

`Tick()` only calls:

```cpp
UpdateEnemyBehavior(DeltaTime);
```

### Behavior Parameters

Added:

- `ChaseRange`
- `AttackRange`
- `MoveSpeed`
- `AttackCooldown`
- `bCanAttackPlayer`
- `EnemyAttackCooldownTimerHandle`

### UpdateEnemyBehavior()

Each frame:

1. Finds the player Pawn.
2. Calculates distance to player.
3. If player is within `AttackRange`, calls `TryAttackPlayer()`.
4. Else if player is within `ChaseRange`, calls `MoveTowardPlayer()`.
5. Else enemy stays idle.

### MoveTowardPlayer()

- Calculates direction from enemy to player.
- Clears Z-axis.
- Normalizes direction.
- Moves using `MoveSpeed * DeltaTime` for frame-rate independence.
- Rotates enemy to face the player.

### TryAttackPlayer()

- Checks `bCanAttackPlayer`.
- If true, displays `Player Hit` Debug Message.
- Sets `bCanAttackPlayer = false`.
- Starts `EnemyAttackCooldownTimerHandle`.

### ResetEnemyAttack()

Restores:

```cpp
bCanAttackPlayer = true;
```

### Die() Update

Enemy death now also clears `EnemyAttackCooldownTimerHandle`.

### Result

Enemy can chase the player and trigger a simple attack message at close range.

---

## 6/27 — Current Combat Prototype State

By the end of Day 3 Advanced:

- Player has attack input.
- Left mouse plays attack montage.
- Attack rhythm is controlled by `bIsAttacking` and Timer.
- Player attack generates a Box Trace in front of the character.
- Box Trace can hit `BP_GhostEnemy`.
- Enemy has health, damage, and death logic.
- Enemy briefly changes color when hit.
- Enemy is lightly knocked back when hit.
- Enemy destroys when health reaches zero.
- Enemy can chase the player.
- Enemy can trigger `Player Hit` at close range.

---

## 6/27 — Day 3 Extra: Ghost Enemy Visual & Animation Upgrade

### Goal

Upgrade the enemy from a temporary Static Mesh target into a Skeletal Mesh enemy with basic Idle / Walk animation.

### Prepared Animation Resources

Downloaded and retargeted Mixamo animations:

- `Enemy_Idle1`
- `Enemy_Walk1`
- `Enemy_Death1`
- `Enemy_Attack1`

### Issue 1: Skeletal Mesh Direction Was Wrong

After replacing Static Mesh with Skeletal Mesh, the enemy could move toward the player, but the model appeared to walk sideways.

#### Cause

- C++ movement direction and Actor rotation logic were correct.
- The problem came from the Skeletal Mesh model's forward direction not matching Actor Forward.
- `EnemyMesh` was also originally the RootComponent, which made it difficult to adjust Mesh rotation independently in Blueprint.

#### Fix

- Added `SceneRoot` to `AGhostEnemy`.
- Set `SceneRoot` as RootComponent.
- Changed `EnemyMesh` to `USkeletalMeshComponent`.
- Attached `EnemyMesh` to `SceneRoot`.

### Issue 2: Old BP_GhostEnemy Did Not Refresh Correctly

After changing the C++ component structure, the old Blueprint displayed `EnemyMesh` incorrectly.

#### Cause

Unreal Blueprint can cache C++ component structure. Large changes to component type or root hierarchy may not refresh cleanly in an existing Blueprint.

#### Fix

- Created a new Blueprint child: `BP_GhostEnemy_New`.
- Reconfigured Skeletal Mesh, material, Anim Class, and enemy parameters.

### Issue 3: Original ABP_GhostEnemy / ABS_GhostEnemy Failed

#### Cause

Likely related to Skeleton mismatch, animation resource selection, AnimGraph cache, or Blueprint configuration.

#### Fix

- Created a new AnimBP: `ABP_GhostEnemy_Manny`.
- Used a compatible Manny / female UE skeleton.
- Rebuilt the AnimGraph.
- Used `Blend Poses by Bool` to switch Idle / Walk.
- In `Event Blueprint Update Animation`, read `GetIsMoving()` from GhostEnemy.

### Issue 4: bIsMoving Could Not Be Found in AnimBP

#### Cause

C++ bool variable visibility and Blueprint display naming were unreliable when accessed directly.

#### Fix

Added BlueprintPure Getter:

```cpp
GetIsMoving()
```

AnimBP reads movement through the Getter instead of directly accessing `bIsMoving`.

This pattern prepared future Getters:

- `GetIsDead()`
- `GetIsAttacking()`
- `GetIsHitReacting()`

### Issue 5: Idle Played Once and Stopped

#### Investigation

- C++ `bIsMoving` was correct.
- AnimBP `IsMoving` sync was correct.
- Blend Bool logic was correct.
- Issue was with the old Idle Sequence Player node.

#### Fix

- Deleted the old Idle Sequence Player node.
- Dragged `Enemy_Idle1` into AnimGraph again.
- Re-enabled `Loop Animation`.
- Reconnected to the Idle / Walk Blend.

### Result

- Enemy plays Idle when stopped.
- Enemy plays Walk when chasing.
- Enemy can switch back and forth correctly.
- `BP_GhostEnemy_New` and `ABP_GhostEnemy_Manny` became the current working enemy setup.

---

## 6/28 — Day 4: Enemy Death Animation

### Goal

Add enemy death animation and delayed destruction.

### Death State

Added:

- `bIsDead`
- `DeathDestroyDelay`
- `DeathTimerHandle`
- `GetIsDead()`
- `FinishDeath()`

### TakeHit() Update

- Early return if `bIsDead`.
- If `Health <= 0`, call `Die()`.
- If the last hit kills the enemy, do not play normal hit feedback.

### UpdateEnemyBehavior() Death Protection

At the beginning of behavior update:

- If enemy is dead, set `bIsMoving = false`.
- Return immediately.

Dead enemies no longer chase or attack.

### Die() Update

`Die()` now:

- Prevents repeated death logic.
- Sets `bIsDead = true`.
- Sets `bIsMoving = false`.
- Sets `bCanAttackPlayer = false`.
- Clears active Timers.
- Disables `EnemyMesh` collision.
- Starts `DeathTimerHandle`.

### FinishDeath()

- Clears `DeathTimerHandle`.
- Calls `Destroy()`.

### AnimBP Update

Added `IsDead` to `ABP_GhostEnemy_Manny`.

In Event Blueprint Update Animation:

- Get Owning Actor.
- Cast To GhostEnemy.
- From `As Ghost Enemy`, call `GetIsMoving()` and set `IsMoving`.
- Call `GetIsDead()` and set `IsDead`.

### AnimGraph Death Blend

Added a second `Blend Poses by Bool`:

- False Pose: Idle / Walk Locomotion.
- True Pose: `Enemy_Death1`.
- Active Value: `IsDead`.

### Result

Enemy now plays Death animation before being destroyed.

---

## 6/28 — Day 4 Advanced: Enemy Attack Animation

### Goal

Add enemy attack animation when the player enters `AttackRange`.

### Attack State

Added:

- `bIsAttacking`
- `EnemyAttackDuration`
- `EnemyAttackDurationTimerHandle`
- `GetIsAttacking()`
- `EndEnemyAttack()`

### Behavior Update

`UpdateEnemyBehavior()` now checks `bIsAttacking`.

If attacking:

- Set `bIsMoving = false`.
- Return.

### TryAttackPlayer() Update

Checks:

- `bIsDead`
- `bIsAttacking`
- `bCanAttackPlayer`

If valid:

- Sets `bCanAttackPlayer = false`.
- Sets `bIsAttacking = true`.
- Sets `bIsMoving = false`.
- Starts attack duration Timer.
- Starts attack cooldown Timer.

### EndEnemyAttack()

Sets:

```cpp
bIsAttacking = false;
```

### ResetEnemyAttack()

Restores:

```cpp
bCanAttackPlayer = true;
```

### Die() Update

Death clears attack state and attack Timers. Death remains highest priority.

### AnimBP Update

Added `IsAttacking` and reads it through `GetIsAttacking()`.

### AnimGraph Attack Blend

Animation priority:

```text
Death > Attack > Idle / Walk
```

Added Attack Blend:

- False Pose: Idle / Walk.
- True Pose: `Enemy_Attack1`.
- Active Value: `IsAttacking`.

### Issue: Attack Animation Did Not Replay Fully

When attacking multiple times, the attack animation did not always restart from the beginning.

#### Cause

`Blend Poses by Bool` did not reset the child animation, so the Sequence Player could continue from its previous playback position.

#### Fix

Enabled:

```text
Reset Child on Activation
```

on the Attack Blend node.

### Result

Enemy attacks now play correctly and restart from the first frame each time.

---

## 6/29 — Day 5: Player Health & Enemy Damage

### Goal

Upgrade enemy `Player Hit` from Debug Message into real player damage.

### Player Health System

Added to `AWCCharacter` under Combat:

- `MaxHealth`
- `Health`
- `bIsDead`
- `ReceiveDamage(float DamageAmount)`
- `GetHealth()`
- `GetIsDead()`
- `Die()`

### BeginPlay Initialization

Initialized:

```cpp
Health = MaxHealth;
bIsDead = false;
```

### ReceiveDamage()

Behavior:

- Return if player is already dead.
- Subtract damage from `Health`.
- Display current player health.
- Call `Die()` if `Health <= 0`.

### Player Die()

`Die()` now:

- Prevents repeated death logic.
- Sets `bIsDead = true`.
- Calls `GetCharacterMovement()->StopMovementImmediately()`.
- Gets the current PlayerController.
- Calls `DisableInput(PlayerController)`.
- Displays `Player Defeated`.

### Attack Death Protection

`Attack()` returns immediately if the player is dead.

### Enemy Damage Parameter

Added to `AGhostEnemy`:

- `EnemyAttackDamage`

### DealDamageToPlayer()

Implemented in `AGhostEnemy`.

Original behavior:

- Get PlayerController.
- Get Player Pawn.
- Cast to `AWCCharacter`.
- Call `ReceiveDamage(EnemyAttackDamage)`.

### TryAttackPlayer() Update

Enemy attack now enters attacking state and calls `DealDamageToPlayer()`.

### Player Death Stops Enemy

`UpdateEnemyBehavior()` checks whether the player is dead.

If player is dead:

- Set `bIsMoving = false`.
- Return.

### Result

Enemy can damage the player. Player can be defeated, stop moving, and lose input.

---

## 6/29 — Day 5 Advanced: Enemy Attack Anim Notify

### Goal

Move enemy damage timing from attack start to the actual hit frame in the attack animation.

### Anim Notify Concept

Anim Notify is an event placed on an animation timeline. It is useful for timing gameplay events such as:

- Attack hit frames
- Footsteps
- Sound effects
- Visual effects

### C++ Interface

Added to `AGhostEnemy`:

```cpp
OnEnemyAttackHit()
```

Marked as `BlueprintCallable`, allowing AnimBP to call it when the Notify fires.

### TryAttackPlayer() Adjustment

Removed immediate `DealDamageToPlayer()` from `TryAttackPlayer()`.

`TryAttackPlayer()` now only:

- Enters attack state.
- Starts attack duration Timer.
- Starts attack cooldown Timer.

### Enemy_Attack1 Notify

- Opened `Enemy_Attack1`.
- Added an Anim Notify near the actual hit frame.
- Named it `EnemyAttackHit`.

### AnimBP Notify Event

In `ABP_GhostEnemy_Manny` Event Graph:

- Added `Event AnimNotify_EnemyAttackHit`.
- Got Owning Actor.
- Cast to GhostEnemy.
- Called `OnEnemyAttackHit()` from `As Ghost Enemy`.

### OnEnemyAttackHit() Logic

Checks:

- Enemy is not dead.
- Enemy is currently attacking.
- Player exists.
- Player is not dead.
- Player is still within `AttackRange`.

If valid, calls:

```cpp
DealDamageToPlayer();
```

### Distance Protection

Added distance check to allow basic whiff behavior. If the player leaves `AttackRange` before the Notify frame, no damage is applied.

### Result

Enemy damage is now animation-timed instead of triggered immediately at attack start.

---

## 6/30 — Day 6: Enemy Hit Reaction Animation

### Goal

Replace simple color hit feedback with a real hit reaction animation and short Hit Stun.

### Hit Reaction State

Added:

- `bIsHitReacting`
- `HitReactionDuration`
- `HitReactionTimerHandle`
- `GetIsHitReacting()`
- `StartHitReaction()`
- `EndHitReaction()`

### TakeHit() Update

On hit:

- Subtract health.
- If dead, enter Death flow.
- If alive, call `StartHitReaction()`.
- Apply Knockback.
- Stop using the previous color flash as the main feedback.

### StartHitReaction()

Sets:

```cpp
bIsHitReacting = true;
bIsMoving = false;
bIsAttacking = false;
```

Also:

- Clears attack duration Timer.
- Starts Hit Reaction Timer.

This allows player hits to interrupt enemy attacks.

### EndHitReaction()

Sets:

```cpp
bIsHitReacting = false;
```

Enemy then returns to normal behavior checks.

### Behavior Protection

Updated:

- `UpdateEnemyBehavior()` returns while hit reacting.
- `TryAttackPlayer()` returns while hit reacting.
- `OnEnemyAttackHit()` returns while hit reacting.

This prevents an interrupted attack from still dealing damage through Anim Notify.

### Die() Update

Death clears Hit Reaction Timer. Death remains highest priority.

### AnimBP Update

Added `IsHitReacting`.

AnimBP reads it through:

```cpp
GetIsHitReacting()
```

### AnimGraph Hit Reaction Blend

Animation priority became:

```text
Death > Hit Reaction > Attack > Idle / Walk
```

Added Hit Reaction Blend:

- False Pose: Attack output.
- True Pose: `Enemy_Reaction_Hit1`.
- Active Value: `IsHitReacting`.

Settings:

- `Reset Child on Activation = true`
- `Enemy_Reaction_Hit1 Loop Animation = false`

### Result

Enemy now plays hit reaction animation, briefly stops behavior, and can have its attack interrupted.

---

## 6/30 — Day 6 Advanced: Future Attack Preparation

### Goal

Prepare the hit system for future weapon attacks, especially replacing the current punch with a sword attack.

### TakeHit() Interface Upgrade

Changed enemy hit interface from:

```cpp
TakeHit(float DamageAmount)
```

to:

```cpp
TakeHit(float DamageAmount, FVector HitDirection, float KnockbackStrength)
```

Meaning:

- `DamageAmount`: how much damage this hit deals.
- `HitDirection`: direction of knockback.
- `KnockbackStrength`: strength of knockback.

### ApplyKnockback() Upgrade

Changed `ApplyKnockback()` to receive:

- `KnockbackDirection`
- `KnockbackStrength`

Enemy no longer calculates knockback direction based only on player position. Attack logic now decides the direction.

### Player Attack Parameter

Added to `AWCCharacter`:

- `AttackKnockbackStrength`

### PerformAttackTrace() Update

When Box Trace hits `GhostEnemy`, player forward vector is used as `HitDirection`.

Current call structure:

```cpp
if (GhostEnemy)
{
    FVector HitDirection = GetActorForwardVector();

    GhostEnemy->TakeHit(
        AttackDamage,
        HitDirection,
        AttackKnockbackStrength
    );
}
```

### Result

Enemy damage, hit direction, and knockback strength are now passed together. This prepares the system for future attack types such as:

- Punch light attack
- Sword light attack
- Sword heavy attack
- Skill attack

---

## 7/1 — Day 7: Code Cleanup & Refactor

### Goal

Do not add new combat features. Clean and lightly refactor the completed Week 2 Combat MVP.

Primary focus:

- Remove duplicate logic.
- Improve code organization.
- Make `AGhostEnemy` and `AWCCharacter` easier to maintain.
- Prepare the project for Week 3 or additional polish.

---

## 7/1 — Day 7 Round 1: GhostEnemy Refactor

### Goal

Clean repeated player lookup, state checks, and Timer cleanup inside `AGhostEnemy`.

### Fixes

- Removed duplicated `bCanAttackPlayer` check in `TryAttackPlayer()`.
- Renamed `EnemyAttackDurationTimerHanlde` to `EnemyAttackDurationTimerHandle`.
- Renamed `HitReactionTimeHandle` to `HitReactionTimerHandle`.
- Removed unused `KnockbackDistance`, since knockback strength now comes from player attack data.

### Player Lookup Helper

Added:

```cpp
GetPlayerCharacter() const
```

This centralizes:

- Get PlayerController.
- Get Pawn.
- Cast to `AWCCharacter`.

### Player State Helper

Added:

```cpp
IsPlayerValidAndAlive() const
```

Used to check whether the player exists and is not defeated.

### Enemy Behavior State Helper

Added:

```cpp
CanUpdateBehavior() const
```

Returns false if enemy is:

- Dead
- Hit reacting
- Attacking

This preserves behavior priority:

```text
Dead > Hit Reaction > Attack > Move
```

### Enemy Attack State Helper

Added:

```cpp
CanStartAttack() const
```

Centralizes checks for:

- `bIsDead`
- `bIsHitReacting`
- `bIsAttacking`
- `bCanAttackPlayer`

### Timer Cleanup Helper

Added:

```cpp
ClearCombatTimers()
```

Centralizes clearing:

- `HitFeedbackTimerHandle`
- `HitReactionTimerHandle`
- `EnemyAttackCooldownTimerHandle`
- `EnemyAttackDurationTimerHandle`

### Simplified Functions

Updated:

- `UpdateEnemyBehavior()`
- `DealDamageToPlayer()`
- `OnEnemyAttackHit()`
- `Die()`

These now use the new helper functions instead of repeating player lookup and Timer cleanup.

### Test Result

All GhostEnemy behavior remained stable:

- Idle
- Walk / chase
- Attack
- Anim Notify damage
- Hit Reaction
- Death
- Player defeated stop behavior

---

## 7/1 — Day 7 Round 2: WCCharacter Refactor

### Goal

Clean player combat flow, attack montage playback, Box Trace hit handling, and death logic.

### Player Action Helper

Added:

```cpp
CanAct() const
```

Currently checks whether the player is dead. Future states such as Stun, Dialogue, or Cutscene can be added here.

### Montage Playback Helper

Added:

```cpp
PlayLightAttackMontage()
```

This moved montage null checks, AnimInstance retrieval, and `Montage_Play()` out of `Attack()`.

### Attack Timer Helper

Added:

```cpp
StartAttackTimer()
```

This centralizes starting `AttackTimerHandle` and calling `EndAttack()` after `AttackDuration`.

### Attack Hit Handling Helper

Added:

```cpp
HandleAttackHit(AActor* HitActor)
```

This separates Box Trace execution from what happens after an Actor is hit.

When the hit actor is a GhostEnemy:

- Calculate `HitDirection = GetActorForwardVector()`.
- Call `GhostEnemy->TakeHit(AttackDamage, HitDirection, AttackKnockbackStrength)`.

### Attack Debug Helper

Added:

```cpp
ShowAttackHitDebug(AActor* HitActor)
```

This isolates hit Debug Message logic from `PerformAttackTrace()`.

### PerformAttackTrace() Simplification

`PerformAttackTrace()` now focuses on:

1. Calculating Box Trace start and end.
2. Running the trace.
3. Returning if nothing is hit.
4. Passing the hit Actor to debug and hit handling helpers.

### ReceiveDamage() Improvement

Added Health clamp:

```cpp
if (Health < 0.0f)
{
    Health = 0.0f;
}
```

This prevents player health from displaying negative values.

### Die() Improvement

Player death now:

- Sets `bIsDead = true`.
- Sets `bIsAttacking = false`.
- Clears `AttackTimerHandle`.
- Stops movement immediately.
- Disables input.
- Displays `Player Defeated`.

### Test Result

All player behavior remained stable:

- Movement
- Look
- Jump
- Interaction
- Journal
- Attack montage
- Box Trace hit
- Enemy damage
- Enemy Hit Reaction
- Enemy Death
- Enemy attack damage
- Player defeated state

---

## Final Week 2 Combat System State

At the end of Week 2, the current combat prototype supports:

### Player

- Movement, camera look, jump, interaction, and journal.
- Left mouse attack input.
- Attack montage playback.
- Attack rhythm lock through `bIsAttacking` and Timer.
- Box Trace melee hit detection.
- Attack hit data: damage, hit direction, and knockback strength.
- Health and defeated state.
- Input disabled after defeat.

### Ghost Enemy

- Skeletal Mesh setup with `SceneRoot` + `EnemyMesh`.
- Idle / Walk / Attack / Hit Reaction / Death animation states.
- Chase behavior.
- Attack behavior with cooldown.
- Attack damage triggered by Anim Notify.
- Distance protection for missed attacks.
- Health and death.
- Delayed destruction after Death animation.
- Hit Reaction and Hit Stun.
- Knockback using attack-provided direction and strength.
- Behavior stops when player is defeated.

### Current Animation Priority

```text
Death > Hit Reaction > Attack > Idle / Walk
```

### Current Combat Loop

```text
Player approaches ghost
↓
Ghost idles
↓
Ghost detects player and walks toward player
↓
Ghost enters AttackRange
↓
Ghost plays Attack animation
↓
EnemyAttackHit Anim Notify triggers damage
↓
Player Health decreases
↓
Player attacks back
↓
Player attack montage plays
↓
Box Trace hits GhostEnemy
↓
Ghost receives Damage + HitDirection + KnockbackStrength
↓
Ghost plays Hit Reaction and enters Hit Stun
↓
Ghost can be interrupted
↓
Ghost Health reaches 0
↓
Ghost plays Death animation
↓
Ghost is destroyed after delay
```

---

## Week 2 Summary

Week 2 successfully transformed the project from an exploration prototype into a playable combat prototype.

The combat system started with a simple attack input and ended with a two-way combat MVP containing:

- Player attack
- Enemy health
- Enemy chase behavior
- Enemy attack animation
- Player health
- Animation-timed enemy damage
- Enemy hit reaction
- Enemy death animation
- Future-facing hit data preparation
- Refactored player and enemy combat code

The system is still intentionally small, but it is now much more professional than a raw prototype. The next step can either be Week 3 content development or a Week 2 polish track focused on presentation, UI, VFX, SFX, and recording readiness.

---

## Next Steps

Possible future directions:

1. **Combat Polish**
   - Replace remaining Debug Messages with UI, sound, and visual feedback.
   - Add camera shake on hit.
   - Add hit spark / ghost impact effect.
   - Add player hurt feedback.

2. **Weapon Preparation**
   - Replace punch animation with sword attack.
   - Add sword mesh or weapon socket.
   - Expand hit data into an `FHitData` struct when attack types increase.

3. **Enemy Polish**
   - Improve chase movement.
   - Add simple attack anticipation.
   - Add better spacing and attack recovery.

4. **Recording Preparation**
   - Clean Debug Draw / Debug Messages.
   - Build a small combat test arena.
   - Tune animation timing and enemy parameters.
   - Record Week 2 showcase video when feedback feels polished enough.
