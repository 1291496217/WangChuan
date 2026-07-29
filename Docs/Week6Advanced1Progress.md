# Week6 Advanced 1 Progress — NPC Relocation Visual Polish

## 1. Advanced 1 Goal

为 Quiet Child 的既有 Story Relocation 增加可观察但克制的离场表现：旧位置先出现一次性淡青白光点，NPC Mesh 平滑淡出，淡出完成后才传送到下一个 Story Anchor。Gameplay 状态与交互在视觉延迟开始前立即进入 Relocating/禁用状态。

## 2. Previous Relocation Behavior

原流程在 `RelocateToStoryAnchor()` 中立即清理交互、隐藏 Actor、传送到目标 Anchor，然后只等待 `RelocationRevealDelay` 再调用 `FinishRelocation()`。玩家无法观察旧位置的离场过程。

## 3. New Relocation Sequence

```text
RelocateToStoryAnchor()
→ Immediately clear CurrentInteractable / prompt
→ StoryState = Relocating
→ Disable collision, interaction sphere, and overlap generation
→ Wait RelocationStartDelay
→ Spawn Niagara at the old world location
→ Fade FadeOpacity from 1.0 to 0.0
→ Hide Actor only after fade completes
→ Teleport to the pending Story Anchor
→ Restore FadeOpacity to 1.0 while hidden
→ Wait the existing RelocationRevealDelay
→ FinishRelocation()
→ Update CurrentStoryStage and clear PendingStoryStage
→ StoryState = Available
→ Show Actor and restore interaction
```

## 4. Files Modified

- `Source/WangChuan/WCStoryNPC.h`
- `Source/WangChuan/WCStoryNPC.cpp`
- `Source/WangChuan/WangChuan.Build.cs`
- `Content/WangChuan/Blueprints/Story/NPC/BP_QuietChild.uasset`
- `Docs/Week6Advanced1Progress.md`

`WangChuan.uproject` was temporarily changed to run an editor-only asset setup script, then restored. No Python/editor-scripting plugin requirement remains in the final project diff.

## 5. Assets Created

- `/Game/WangChuan/Story/NPC/Effects/NS_QuietChild_Relocation`
- `/Game/WangChuan/Story/NPC/Materials/M_QuietChild_HighLimbs_Fade`
- `/Game/WangChuan/Story/NPC/Materials/M_QuietChild_Joints_Fade`

The two materials are Quiet Child-specific duplicates. The original shared Mixamo materials were not modified.

## 6. Niagara Implementation

`NS_QuietChild_Relocation` is a one-shot Niagara system based on a simple sprite burst:

- Particle count: `40`
- Particle lifetime: `1.5 seconds`
- Uniform sprite size: `14.0`
- Color: sRGB hex `DDF7F4FF` (pale cyan-white)
- Motion: gentle positive-Z Gravity Force, configured as `25.0`
- Fade: the template's `Scale Color` particle-update module
- Loop: `False` / emitter state `Once`
- Collision: none
- Smoke, explosion, gameplay light, and large bloom: none
- Spawn location: old NPC actor world location plus `(0, 0, 80)`
- Auto destroy: enabled by `SpawnSystemAtLocation`

The system is spawned in world space and is not attached to the NPC, so it remains at the old Anchor after the Actor teleports.

## 7. Fade Material Implementation

Both visible Quiet Child skeletal mesh slots use dedicated duplicated materials. Each material:

- Preserves the original Base Color, Normal, Roughness, and existing graph.
- Uses `Masked` blend mode.
- Adds scalar parameter `FadeOpacity`, default `1.0`.
- Routes `FadeOpacity` through Engine material function `DitherTemporalAA`.
- Connects the dither result to `Opacity Mask`.

All visible slots therefore use the same fade parameter and fade together.

## 8. C++ / Blueprint Implementation

`AWCStoryNPC` now:

- Creates and caches all required MIDs once in `BeginPlay()`.
- Validates every visible material slot before enabling the fade path.
- Uses short-lived `FTimerManager` timers; no permanent Actor Tick was added.
- Spawns the configured `UNiagaraSystem` at the old world location.
- Uses smoothstep interpolation for the opacity transition.
- Revalidates the pending Anchor before fade and before teleport.
- Clears start/fade/reveal timers in `EndPlay()`.
- Blocks relocation while the state or any relocation timer is active.
- Restores visibility, opacity, and a reasonable prior state if relocation must abort.

`BP_QuietChild` configures the Niagara asset, both dedicated fade materials, and the final timing/VFX defaults. `WangChuan.Build.cs` adds only the `Niagara` module dependency.

## 9. Timing Parameters

Final configured values:

- `RelocationStartDelay = 0.35 seconds`
- `RelocationFadeDuration = 1.20 seconds`
- `RelocationFadeUpdateInterval = 0.03 seconds`
- `RelocationRevealDelay = 0.50 seconds` (existing behavior retained)
- Old-location observation/fade time before teleport: approximately `1.55 seconds`
- `RelocationVFXOffset = (0, 0, 80)`
- Fade parameter name: `FadeOpacity`
- Niagara asset: `/Game/WangChuan/Story/NPC/Effects/NS_QuietChild_Relocation`

### Visibility Adjustment Record

PIE visual feedback identified two presentation problems:

1. **NPC Mesh fade was almost impossible to notice.**
   - Previous value: `RelocationFadeDuration = 0.80 seconds`
   - Adjusted value: `RelocationFadeDuration = 1.20 seconds`
   - Reason: the longer smoothstep transition gives the player enough time to read the dithered silhouette changing before teleport.
   - Scope: the C++ default and `BP_QuietChild` class default were both updated.

2. **The light particles were visible but too subtle.**
   - Particle count: `24 → 40`
   - Lifetime: `1.1 → 1.5 seconds`
   - Uniform sprite size: `8.0 → 14.0`
   - Reason: increasing density, screen coverage, and persistence improves recognition while retaining the same pale color, one-shot behavior, no collision, and no gameplay light.

No Story State, Anchor, Interaction, or Encounter sequencing was changed by this visibility pass.

## 10. Interaction Cleanup

At relocation request time—before the observation delay—the code:

- Calls the existing `ClearPlayerInteractionIfNeeded()`.
- Sets `StoryState = Relocating`.
- Calls the existing `SetStoryNPCInteractionEnabled(false)`.
- Disables Actor collision, the interaction sphere, and overlap generation through the existing helper.

The mesh remains visually present during the delay/fade, but it cannot be interacted with.

## 11. Story State and Stage Preservation

The existing architecture remains authoritative:

- Encounter/Story Event code still decides when relocation starts.
- `RelocateToStoryAnchor()` still receives the target Anchor and next stage.
- `PendingStoryStage` is assigned before the visual sequence.
- Niagara and materials never modify Story state or stages.
- `FinishRelocation()` remains responsible for committing `CurrentStoryStage`, clearing `PendingStoryStage`, restoring `Available`, and re-enabling interaction.
- No Encounter 01/02 reference is hard-coded into `AWCStoryNPC`.

## 12. Errors Encountered

### Problem A

The first Unreal Python material-generation attempt failed because `unreal.MaterialExpressionDitherTemporalAA` was not exposed in the UE 5.8 Python API.

### Problem B

The next attempt failed because `unreal.KismetEditorUtilities` was not exposed in this editor Python environment.

### Problem C

The Niagara module picker could not be operated reliably enough to add an extra Shape Location module. The final effect therefore uses a compact torso-centered burst at the old world position plus the configured Z offset, rather than claiming a randomized body-volume distribution.

### Problem D

During the visibility adjustment, Niagara initially reported that `NS_QuietChild_Relocation.uasset` could not be saved.

## 13. Root Cause and Fix

### A — Dither expression

- Cause: the engine material node is not a directly constructible Python class in this build.
- Fix: created `MaterialExpressionMaterialFunctionCall` and assigned `/Engine/Functions/Engine_MaterialFunctions02/Utility/DitherTemporalAA`.
- Why: this uses the engine's supported material function while producing the requested dithered opacity mask.
- Verification: both material assets saved, compiled, and were assigned to the two `BP_QuietChild` mesh slots.

### B — Blueprint compile API

- Cause: the expected Kismet utility wrapper is absent.
- Fix: used `unreal.BlueprintEditorLibrary.compile_blueprint`.
- Why: it is the exposed UE 5.8 editor API for this environment.
- Verification: `BP_QuietChild` compiled, saved, passed asset validation, and contains references to both fade materials and the Niagara system.

### C — Spawn distribution

- Cause: popup/window coordinate translation in the detached Niagara editor was unreliable.
- Fix: retained the stable one-shot Simple Sprite Burst centered at `ActorLocation + (0,0,80)`.
- Why: it satisfies the small, old-location light-point effect without risking a corrupt or partially configured Niagara stack.
- Verification: Niagara compiled and saved with the intended count, lifetime, color, sprite size, upward force, and one-shot state.

### D — Niagara save conflict

- Cause: two Unreal Editor processes had the same project open, so the second process held a conflicting project/package session.
- Fix: closed the unused editor instance without saving its untouched map, then retried the Niagara save.
- Why: this released the conflicting session while preserving the active Niagara edits and avoiding an unintended map save.
- Verification: retry succeeded; the Niagara asset timestamp and size changed, and the adjusted asset reopened through the subsequent editor/PIE session.

## 14. Encounter 01 Test

Status: **Implemented but not editor-verified**.

Required manual test:

1. Defeat the required enemy and make Echo Relic 01 available.
2. Stand inside the Quiet Child Anchor 01 interaction sphere.
3. Read Echo 01 to completion.
4. Verify the prompt clears immediately and dialogue cannot reopen during the delay/fade.
5. Verify the old-location particle burst and smooth fade are visible.
6. Verify teleport occurs only after fade completion.
7. Verify the particles remain at Anchor 01.
8. Verify the fully visible NPC appears at Anchor 02.
9. Verify `CurrentStoryStage == 1`, `StoryState == Available`, and Stage 1 dialogue works.

## 15. Encounter 02 Test

Status: **Implemented but not editor-verified**.

Required manual test:

1. Complete the Lantern Puzzle and make Echo Relic 02 available.
2. Stand inside the Quiet Child Anchor 02 interaction sphere.
3. Read Echo 02 to completion.
4. Repeat the prompt, no-interaction, old-location VFX, fade-before-teleport, and full-visibility checks.
5. Verify the NPC appears at Anchor 03.
6. Verify `CurrentStoryStage == 2`, `StoryState == Available`, and Stage 2 dialogue works.

## 16. Missing VFX Fallback Test

Status: **Implemented but not editor-verified**.

The null branch logs one warning per NPC instance and continues into the fade/teleport/reveal sequence. Manual test: temporarily set `RelocationVFXSystem = None`, complete either encounter, verify one warning and successful fade/relocation, then restore `NS_QuietChild_Relocation`.

## 17. Missing Material Fallback Test

Status: **Implemented but not editor-verified**.

The initialization path first validates every visible slot. If any slot lacks `FadeOpacity`, it logs a warning, creates no partial MID set, waits the configured fade duration, then uses the safe hide/teleport/reveal path. Manual test: temporarily assign one original non-fade material, complete a relocation, verify no Accessed None/permanent hiding, then restore both dedicated materials.

## 18. Regression Test

Status: **Implemented but not fully editor-verified**.

A normal PIE startup/exit smoke test passed. Full Stage 0/1/2 dialogue, Encounter 01/02, Echo UI, Journal, lantern puzzle, combat, death, lock-on, and player-death regression still requires the existing Week6 manual regression route.

## 19. Build Result

Target: `WangChuanEditor`  
Platform: `Win64`  
Configuration: `Development Editor`

Result: **Succeeded**

Final clean-project build after the visibility adjustment completed with:

```text
Result: Succeeded
Total execution time: 50.75 seconds
```

Earlier full and incremental compiles also completed successfully.

## 20. PIE Result

Status: **PIE smoke test passed; encounter paths not fully editor-verified**.

`Prototype_Map` entered PIE successfully, created the PIE world, ran, and exited normally. Log inspection found no:

- `Accessed None`
- Blueprint Runtime Error
- Timer Error
- Delegate Error
- crash/fatal error

The smoke-test log records PIE startup and `BeginTearingDown` normally.

The PIE startup/exit smoke test was rerun after the `1.20` second fade and stronger Niagara parameters were saved.

## 21. Final Architecture Conclusion

The visual polish is inserted only between `RelocateToStoryAnchor()` and the real teleport. Gameplay remains owned by the existing Story Encounter/NPC state flow. Niagara is presentation-only, materials are Quiet Child-specific, Anchor selection remains data-driven, and `FinishRelocation()` still commits the Story Stage and restores interaction. The implementation is timer-scoped, null-safe, duplicate-guarded, and does not add Tick, a new framework, or a new story path.

## 22. Git Status or Suggested Commit

Pre-existing untracked `Content/WangChuan/Audio/` and `Tools/` content was preserved and not modified by this task.

Suggested commit:

```text
Week6 Advanced 1 - Polish NPC relocation visuals
```
