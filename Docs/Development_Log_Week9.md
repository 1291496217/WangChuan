# 《忘川河畔》Development Log — Week9

**Project:** WangChuan / 《忘川河畔》
**UE Project / Module:** `WangChuan`
**Engine:** Unreal Engine 5.8.x（Pre-Final 记录为 5.8.1）
**Development Branch:** `feature/ai-first-prototype`
**Development Period:** 2026-08-10 — 2026-08-16
**Week Theme:** Tutorial Dungeon Vertical Slice — From Controls to Land Temple
**中文主题:** 新手地牢完整闭环、土地庙正式入口与通用 Memory Maze Encounter v0.1
**Final Status:** Week9 核心开发完成；Final Regression 人工审核通过；Day7 完整录屏完成
**Known Non-Blocking Items:** Fragment02 / Fragment03 仍为占位文本；Dodge / Roll 尚未实现且不属于 Week9 范围
**AI Judge / API:** 本周未扩展 AI Judge；`Real AI Calls = 0`

---

# 1. Week9 Overview

Week9 的目标是把此前分散的移动、战斗、解谜、Fragment、Boss 与地图切换能力整合成一个可完整游玩的新手流程，并把玩家最终交到土地庙前的正式剧情入口。

最终玩家流程：

```text
Tutorial Start
→ Move / Look
→ Fragment01 / Interact
→ CR01 Light / Heavy / Combo
→ Fragment02
→ Downhill Bridge
→ CR02 Lock-On / Strafe
→ Short Side Alcove / Uphill Passage
→ CR03 Future Dodge / Roll Placeholder
→ Five-Lantern Puzzle
→ Fragment03
→ Boss Gate
→ Tutorial Mini-Boss
→ Exit Gate
→ Fade
→ OpenLevel
→ Land Temple Front
```

Week9 后半又进行了一次重要架构收束：

```text
Tutorial-specific progression glue
→ Memory Maze Encounter v0.1
```

使 CR01、Lantern Puzzle 与 Mini-Boss 三类不同关卡流程统一为：

```text
Condition
→ AWCMemoryMazeEncounter
→ Action
```

因此 Week9 Final Regression 验证的不再只是一次性 Tutorial 逻辑，而是未来正式 Memory Maze 可以继续复用的基础 Encounter 架构。

---

# 2. Stage Summary

| Stage | 核心主题 | 结果 |
|---|---|---|
| Day1 | Tutorial Dungeon 简化与空间骨架 | 完成，一条清晰主路线 + 一个短 Side Alcove |
| Day1 Advanced | Tutorial Fragment、HUD、最小操作引导 | 完成，3 个 Tutorial Fragment 与独立 UI |
| Day1 Advanced Modification | 三段战斗教学空间 | 完成，CR01 / CR02 / CR03 顺序化布局 |
| Day2 | 战斗教学整合 | 完成，暂停式 Instruction、CR01 / CR02 Enemy、Fragment02 |
| Day3 | 五灯谜题、Fragment03、Boss Gate | 完成，固定序列与独立奖励 / Gate 解锁 |
| Day3 Advanced | Tutorial Mini-Boss、Exit Gate | 完成，Boss Defeat → Exit Gate |
| Day3 Advanced Extra | Fade、OpenLevel、土地庙前灰盒 | 完成，Tutorial → Land Temple 跨地图闭环 |
| Pre-Final | Memory Maze Encounter v0.1 | 完成，通用 Encounter / Condition / Action 架构 |
| Pre-Final Corrections | Ownership、Runtime Cycle、Ghost Hit Recovery | 完成，代码级审核通过 |
| Day4–7 Learning | AI 3D / Blender 流程学习 | 完成一轮流程实验；结果尚未达到生产质量 |
| Final Regression / Day7 | 完整新玩家流程、回归、录屏 | 人工审核通过，录屏完成 |

---

# 3. Day1 — Tutorial Dungeon Spatial Foundation

## 3.1 独立 Tutorial Map

创建：

```text
/Game/WangChuan/Maps/TutorialDungeon_Prologue_v01
```

地图基于 Week8 已验证的：

```text
/Game/WangChuan/Maps/MemoryMaze_CombatFirst_Layout_v02
```

通过 Unreal Editor 资产复制方式建立，而不是直接文件系统复制 `.umap`。

Week8 原地图保留为独立回归基线，没有被 Tutorial 开发覆盖。

原基线记录：

```text
MemoryMaze_CombatFirst_Layout_v02
SHA-256:
F9B8DD2A8D51AA3A744269E6A1BE330C91E3AD4850EFF616B74165C34194DD11
```

Day1 前后保持一致。

## 3.2 路线简化

Week8 的正式 Memory Maze 原型强调：

```text
Branch
Loop
Cross Connector
Optional Combat Route
```

但新手地牢的设计目标改为：

```text
可读性
>
路线复杂度
```

因此 Tutorial 删除了：

- 双主路线分叉；
- Cross Connector；
- Optional Combat Loop；
- Ambush shortcut；
- 多房间 AI 压测结构；
- Week8 保存的 Ghost / TargetPoint 测试 Actor。

Day1 最终骨架：

```text
TD_Entrance
→ TD_Fragment01_Slot
→ TD_CombatRoom
→ TD_Fragment02_Slot
→ TD_ShortSideAlcove
→ TD_LanternPuzzle
→ TD_Fragment03_Slot
→ TD_BossGate
→ TD_BossArena
→ TD_ExitGate
```

核心规则：

```text
1 mandatory main route
+
1 short dead-end Side Alcove
```

## 3.3 NavMesh / Collision

Day1 重新整理了 Tutorial NavMesh，并修正了 NavMesh Bounds 误覆盖高处 / 非地面的情况。

最终人工 PIE 全路线可通行，无阻塞，Boss Arena 与 Exit 区域均可到达。

---

# 4. Day1 Advanced — Tutorial Fragment & Minimal Guidance

## 4.1 独立 Tutorial Fragment

没有继续扩展早期 `AMemoryFragment`，而是新建 Tutorial-only 系统：

```text
ATutorialMemoryFragment
UTutorialFragmentWidget
UTutorialHUDWidget
ATutorialDungeonBootstrap
```

原因是旧 `AMemoryFragment` 仍绑定：

```text
数字 Fragment ID
旧 CollectedFragments
Quiet Child 早期叙事
旧 Journal / Memory prototype
```

而 Week9 需要一个不进入正式 Case / Story / SaveGame 的轻量教学层。

三个稳定语义 ID：

```text
Tutorial.Fragment01
Tutorial.Fragment02
Tutorial.Fragment03
```

## 4.2 Availability

Fragment 支持显式：

```text
SetFragmentAvailable(bool)
```

Available：

```text
Visible
Overlap enabled
Can become CurrentInteractable
Can be collected
```

Unavailable：

```text
Hidden
Overlap disabled
Cannot become CurrentInteractable
No prompt
Cannot be collected
```

特别处理了：

```text
玩家已经站在奖励位置附近
→ Enemy / Puzzle 完成
→ Fragment 突然 Available
```

不要求玩家先离开再重新进入 Trigger。

## 4.3 初始 Fragment 配置

### Fragment01

```text
ID:
Tutorial.Fragment01

StartAvailable:
true

Text:
一阵遥远的铃声从黑暗深处传来。
```

### Fragment02

```text
ID:
Tutorial.Fragment02

StartAvailable:
false

Text:
CONTENT PLACEHOLDER
```

### Fragment03

```text
ID:
Tutorial.Fragment03

StartAvailable:
false

Text:
CONTENT PLACEHOLDER
```

Fragment02 / 03 的占位文本在 Week9 Final Regression 后仍未替换，但其完整 gameplay flow 已验证，不构成 Week9 阻塞。

## 4.4 Tutorial HUD / Bootstrap

Tutorial HUD 只显示：

```text
Fragments X / 3
```

进度：

```text
0 / 3
→ 1 / 3
→ 2 / 3
→ 3 / 3
```

`ATutorialDungeonBootstrap` 只负责当前 Tutorial Map：

```text
初始化 Tutorial session
创建 HUD
Fragment total = 3
```

没有引入全局 Quest Manager / Tutorial Manager。

---

# 5. Combat Teaching Layout Refinement

原单一 Combat Room 扩展为三段顺序式教学空间：

```text
CR01 — Light / Heavy
↓
Downhill Bridge
↓
CR02 — Lock-On
↓
Transition / Side Alcove
↓
Uphill Passage
↓
CR03 — Future Dodge / Roll
↓
Puzzle
```

仍坚持：

```text
More teaching rooms
≠
More route ambiguity
```

因此没有恢复 Week8 的 Branch / Loop 复杂度。

## 5.1 Codex 初版灰盒问题

Codex 初始版本虽然满足大体拓扑，但人工游玩发现：

- Bridgehead 存在 floor / wall seam；
- Downhill Bridge 边界不足；
- CR02 入口暴露；
- CR02 Floor 太薄；
- CR02 Exit / 转角存在明显缺口；
- Transition / Alcove 边缘不完整；
- Uphill Passage 一侧过度开放；
- CR03 入口包围不足；
- CR03 → Puzzle connector 太短；
- 旧 CR01 → Puzzle 封堵方式过度复杂。

因此本阶段形成一个重要工作经验：

> Codex 的“拓扑完成”不等于可玩的 Greybox 完成；空间任务必须经过开发者实际走图检查。

## 5.2 最终人工修正

完成：

- 更安全、较直的 Downhill Bridge；
- CR02 Entry / Exit containment；
- CR02 Floor 加厚；
- Side Alcove 移至 CR02→CR03 过渡段；
- Uphill Passage 支撑和侧墙修复；
- CR03 Entry 强化；
- CR03 → Puzzle Connector 延长；
- 旧 CR01 → Puzzle shortcut 使用更简单的物理墙体关闭；
- Recast NavMesh rebuild。

## 5.3 已知环境问题

Tutorial Dungeon 目前仍没有正式 Ceiling / Roof。

原因：

```text
内部照明方案尚未确定
```

当前决定：

```text
先设计 dungeon lighting
→ 再加 ceiling / roof
```

该问题不阻塞 Week9 gameplay。

---

# 6. Day2 — Combat Tutorial Integration

## 6.1 最终 PC Controls

```text
W / A / S / D
→ Move

Mouse
→ Look

E
→ Interact

LMB
→ Light Attack

RMB
→ Heavy Attack

MMB
→ Lock-On

Space
→ Jump
```

Week9 没有添加 Dodge Input。

---

## 6.2 Paused Tutorial Instruction

创建：

```text
UTutorialInstructionWidget
ATutorialInstructionTrigger
WBP_TutorialInstruction
BP_TutorialInstructionTrigger
```

教学规则改为：

```text
进入新机制区域
→ 打开 Tutorial Instruction
→ Pause World
→ 玩家自行阅读
→ E / Close
→ 恢复 Gameplay
→ 立即实践
```

而不是在战斗继续进行时显示短暂 HUD 文字。

Instruction 系统处理：

- 真正 `SetPause(true)`；
- Cursor / Input Mode；
- Movement / Look ignore；
- Lock-On 退出；
- Pause ownership；
- E 键在 Pause 状态下由 Widget 直接处理；
- Close Button 共用同一 cleanup path；
- Player Death 时安全清理 modal。

### Same-E Leak 修正

Fragment01 旁的 Interact 教学特别验证：

```text
E 关闭教学窗口
≠
同一个 E 同时收集 Fragment
```

关闭教学后必须再次按 E 才能真正交互。

---

## 6.3 CR01 — Basic Attacks

职责：

```text
LMB Light
RMB Heavy
Light Combo
```

放置 1 个基础 Enemy。

CR01 Enemy Defeat 后：

```text
Fragment02 Available
```

但：

```text
不自动 Collect
```

玩家仍需主动接近、E、阅读，HUD 才从：

```text
1 / 3
→ 2 / 3
```

---

## 6.4 CR02 — Lock-On

职责：

```text
MMB Lock-On
Strafe
Forward / Backward movement
```

放置 1 个基础 Enemy。

人工验证：

```text
Lock-On
Light Combo
Heavy
Hit Reaction
Player Damage
Enemy Death
```

均通过。

---

## 6.5 Enemy Blueprint 集成问题

初版使用：

```text
BP_GhostEnemy
```

后续审计发现该旧 Blueprint 当前：

```text
EnemyMesh:
No Skeletal Mesh
No Animation Class
```

因此 Tutorial 两个 Enemy 改为稳定基线：

```text
BP_GhostEnemy_New
+
SKM_Quinn
+
ABP_GhostEnemy_Manny
```

这是重要的可复用检查项：

> 新放置 Enemy 时不要因为名称更短 / 更旧就默认使用 `BP_GhostEnemy`；当前已验证 gameplay baseline 是 `BP_GhostEnemy_New`。

---

## 6.6 CR03 — Dodge / Roll Placeholder

CR03 只保留未来教学位置。

Instruction 明确告诉玩家：

```text
Dodge / Roll — planned
Current build:
Space is still Jump
```

Week9 没有实现：

```text
IA_Dodge
Roll Montage
I-Frame
Jump replacement
```

Final Regression 后仍保持这一状态。

这是明确的 Week9 scope decision，不是功能缺陷。

---

# 7. Day3 — Five-Lantern Puzzle → Fragment03 → Boss Gate

Week9 复用 Week6 已验证的：

```text
ALanternPuzzlePiece
ALanternSequencePuzzle
BP_LanternPuzzlePiece
BP_LanternSequencePuzzle
```

没有重新实现 Tutorial-only Puzzle state machine。

## 7.1 Final Puzzle

五灯：

```text
TutorialLantern_00
TutorialLantern_01
TutorialLantern_02
TutorialLantern_03
TutorialLantern_04
```

Piece ID：

```text
0 / 1 / 2 / 3 / 4
```

固定正确序列：

```text
0 → 3 → 2 → 4 → 1
```

Tutorial timing：

```text
Initial Preview Delay:
1.0 s

Preview Light Duration:
1.0 s

Preview Gap:
0.6 s

Reset Delay:
1.0 s
```

运行逻辑：

```text
Enter
→ Preview
→ interaction locked

Wrong
→ Reset
→ automatic full Replay

Correct
→ completed steps stay lit

5 correct
→ Puzzle Completed
```

---

## 7.2 Fragment03 / Boss Gate

Puzzle 完成产生两个**独立**结果：

```text
Puzzle Complete
├─ Fragment03 Available
└─ Boss Gate Open
```

关键规则：

```text
Boss Gate does NOT wait for Fragment03 read / collect
```

因此玩家可以：

```text
HUD = 2 / 3
→ 不读 Fragment03
→ 直接进入 Boss Arena
```

Fragment03 被收集后才：

```text
2 / 3
→ 3 / 3
```

---

## 7.3 Day3 Automation Note

Day3 曾为 Codex runtime automation 局部添加：

```text
TriggerLanternInteractionForTest()
```

到 `LanternPuzzlePiece`。

正式 gameplay 并不依赖该 helper。

该 diff 被定义为 test-only，不应被误认为正式 Puzzle 架构依赖。

---

# 8. Day3 Advanced — Tutorial Mini-Boss & Exit Gate

Tutorial Mini-Boss 不是新 Boss Framework，而是：

```text
已教学战斗机制的最终考试
```

复用：

```text
BP_GhostEnemy_New
→ AGhostEnemy
→ WCGhostAIController
```

没有增加：

```text
Boss Phase
Boss-specific AI Controller
Behavior Tree
EQS
特殊技能
召唤
Boss cinematic
```

## 8.1 Final Boss Parameters

```text
MaxHealth:
225

MoveSpeed:
160

Damage:
12
```

保持不变：

```text
Attack Cooldown
Sight tuning
global Leash / AI architecture
```

人工验证：

- Sight / Aggro；
- Lock-On；
- Light / Heavy；
- Hit Reaction；
- Player Damage；
- Health feel；
- Damage feel；
- Leash / ReturnHome；
- Player Death regression；
- Boss Death。

均通过。

---

## 8.2 Boss Defeat → Exit Gate

初版使用轻量：

```text
ATutorialBossEncounter
```

流程：

```text
TutorialMiniBoss
→ OnEnemyDefeated
→ ExitGate.OpenGate()
```

Boss 死亡：

```text
不会自动切图
```

而只是：

```text
Exit Gate Open
```

玩家需要自己走向出口。

---

# 9. Day3 Advanced Extra — Exit Transition & Land Temple Front

## 9.1 Final Transition

新增：

```text
ATutorialLevelTransitionTrigger
```

最终链路：

```text
Boss Defeat
→ Exit Gate Open
→ Player walks through Gate
→ Transition Trigger
→ Fade to Black
→ OpenLevel
→ LandTemple_Prologue_Greybox
```

Fade：

```text
0.75 s
```

使用轻量 Camera Fade，没有建立大型 Loading Framework。

---

## 9.2 Land Temple Front Greybox

创建：

```text
/Game/WangChuan/Maps/LandTemple_Prologue_Greybox
```

参考：

```text
土地庙前部.png
```

但只参考：

```text
布局
空间层级
中轴关系
```

没有复刻最终美术。

最终前部结构：

```text
Arrival
→ Main Approach
→ two-stage broad stairs
→ elevated Front Plaza
→ three-bay Shanmen / Front Gate
→ simple Temple Front massing
```

左右两侧均保留：

```text
future NPC / function / interaction area
```

目标地图为安全区：

```text
Enemy = 0
Puzzle = 0
Boss = 0
Formal Story = not started
```

---

## 9.3 Cross-Map Wrong Pawn Issue

第一次进入土地庙地图时错误生成：

```text
BP_ThirdPersonCharacter
```

而非：

```text
BP_PlayerCharacter
```

### Root Cause

新地图没有设置：

```text
World Settings
→ GameMode Override
```

因此继承 native `WangChuanGameMode` 的模板 Default Pawn。

### Fix

Land Temple World Settings 改为：

```text
BP_GameMode
```

有效 Pawn：

```text
BP_PlayerCharacter
```

Fresh PIE 重新验证正确。

### Future Rule

以后创建任何新 Gameplay Map，必须固定检查：

```text
World Settings
→ GameMode Override
→ Effective Default Pawn
→ Actual PIE Possessed Pawn
```

“地图能加载”不代表“生成了正确角色”。

---

# 10. Pre-Final — Memory Maze Encounter v0.1

完整 Tutorial 跑通后，发现 CR01、Puzzle、Boss 使用了多套 Tutorial-specific glue。

原模式：

```text
ATutorialEnemyDefeatFragmentLink
ATutorialPuzzleCompletionLink
ATutorialBossEncounter
```

本质都属于：

```text
某个条件完成
→ 推进关卡
```

因此 Final Regression 前完成通用化。

---

## 10.1 Core Architecture

最终：

```text
AWCMemoryMazeEncounter

Conditions
├─ AWCEnemyDefeatEncounterCondition
└─ AWCObjectiveCompleteEncounterCondition

Actions
├─ AWCOpenGateEncounterAction
└─ AWCRevealTutorialFragmentEncounterAction

Progression
└─ AWCProgressionGate
```

Encounter State：

```text
Dormant
Active
Completed
```

Completion Policy：

```text
All
Any
```

核心原则：

```text
Delegate-driven
No Tick
Idempotent
Blueprint-editable
World Partition serializable
```

---

## 10.2 Three Real Migrations

### CR01

```text
EnemyDefeatCondition
→ Encounter_Tutorial_CR01
→ Reveal Fragment02 Action
```

### Lantern Puzzle

```text
ObjectiveCompleteCondition
→ Encounter_Tutorial_LanternPuzzle
├─ Reveal Fragment03 Action
└─ Open Boss Gate Action
```

### Mini-Boss

```text
EnemyDefeatCondition
→ Encounter_Tutorial_MiniBoss
→ Open Exit Gate Action
```

稳定 Encounter IDs：

```text
Tutorial.CR01.BasicCombat
Tutorial.LanternPuzzle
Tutorial.MiniBoss
```

旧 Tutorial glue 当前地图 placed refs：

```text
0
```

源码 / Blueprint 类型可暂时作为 compatibility 保留，但正式 runtime 不再依赖。

---

## 10.3 Progression Gate

将原：

```text
AWCTutorialGate
```

通用逻辑下沉至：

```text
AWCProgressionGate
```

并保留：

```text
AWCTutorialGate : AWCProgressionGate
```

作为兼容层。

这样现有 `BP_TutorialGate` 不需要在 Week9 收尾阶段做高风险 reparent，同时未来正式 Memory Maze 可以直接依赖通用 Gate 类型。

---

# 11. Encounter Correctness Corrections

通用架构在代码级审核中又发现两个如果不处理就会影响未来复用的边界。

---

## 11.1 Exclusive Condition / Action Ownership

风险：

```text
Encounter A ─┐
             ├→ same Condition / Action
Encounter B ─┘
```

Condition / Action 都拥有可变 runtime state，因此共享实例会造成：

```text
Condition 被一个 Encounter Deactivate
→ 另一个 Encounter 丢失事件

Action 被 Encounter A 执行
→ bExecuted = true
→ Encounter B 的奖励 silent no-op
```

### Fix

Condition / Action 增加 runtime-only：

```text
OwningEncounter
```

支持：

```text
TryClaimOwnership()
ReleaseOwnership()
GetOwningEncounter()
```

规则：

```text
unowned
→ claim

same owner
→ idempotent success

different owner
→ reject
```

---

## 11.2 Transactional Claim / Rollback

Encounter 激活：

```text
claim Conditions
→ claim Actions
```

如果任一 claim 失败：

```text
rollback 本次已获得 ownership
→ Encounter 保持 Dormant
→ 不 Activate Condition
→ 不执行 Action
```

防止“半激活 Encounter”。

---

## 11.3 Runtime-Cycle Correctness

后续代码审核继续发现：

```text
Encounter A Completed
→ Condition satisfied = true
→ Action executed = true
→ A EndPlay releases owner

Encounter B reclaim same objects
```

如果不重置 runtime state，B 可能继承上一轮：

```text
Satisfied / Executed
```

造成：

```text
B instant complete
但 Action no-op
```

### Final Fix

**只有 genuine new-owner claim 时**重置：

Condition：

```text
bIsActive = false
bIsSatisfied = false
```

Action：

```text
bExecuted = false
```

Same-owner repeated claim：

```text
NO RESET
```

`ReleaseOwnership()`：

```text
只释放 Owner
不重置 runtime state
```

最终 lifecycle test：

```text
Completed A
→ EndPlay
→ owner null
→ B reclaim
→ fresh Condition / Action state
→ B 完成
→ Action 对 B 再执行 exactly once
```

通过。

---

# 12. Ghost Hit-Reaction Chase Recovery

Pre-Final 完整游玩时发现一个并非 Encounter-specific 的共享 AI Bug：

> Mini-Boss / 普通 Ghost 在某些受击路径状态后可能永久停止追击。

## 12.1 Root Cause

旧流程：

```text
Chasing
→ Hit Reaction
→ StopMovement
→ retry MoveTo once
```

但 PathFollowing 后续可能进入：

```text
Blocked
OffPath
Invalid
Idle
```

而 Chasing state 不会继续恢复路径。

结果：

```text
State = Chasing
但没有 active path
→ Enemy 原地站住
```

直接 Knockback 也可能把 Pawn 推向 NavMesh 边缘。

## 12.2 Fix

加入：

```text
Chasing path-liveness watchdog
```

仅在合理条件下恢复 MoveTo：

```text
not Dead
not HitReacting
not Attacking
valid target
target visible
outside AttackRange
path state requires recovery
```

Path：

```text
Blocked / OffPath / Invalid
→ throttled retry

Aborted
→ no immediate retry
```

另外：

```text
Leash / ReturnHome
优先于 Chase recovery

Knockback destination
→ ProjectPointToNavigation
```

保持：

```text
Global Sight tuning unchanged
Boss parameters unchanged
```

定向验证：

- Boss 连续受击后恢复追击；
- Boss 靠墙 Knockback 后恢复；
- Hit Reaction 期间玩家拉出 AttackRange 后恢复；
- CR01 普通 Enemy 连续受击后恢复；
- Leash 边缘受击后正确 ReturnHome。

均通过。

---

# 13. Day4–7 — AI 3D / Blender Learning

由于 Week9 原定核心 Gameplay 功能在前半段快速推进，Day4–7 的一部分时间用于学习 AI 3D 与 Blender 基础资产流程。

完成的学习 / 实验：

```text
角色 / 怪物原画
→ AI 高模生成（带材质）
→ AI 自动重拓扑低模
→ Blender 导入高模
→ 修复粘连 / 内部多余面
→ Sculpt 微调
→ QuadRemesher 智能拓扑
→ UV 展开
→ Shrinkwrap
→ High → Low Bake
```

实际以骷髅怪物作为测试对象：

- 生成一版 AI 高模（带材质）；
- 生成 AI 重拓扑低模；
- 在 Blender 中手动修复高模问题；
- 学习 QuadRemesher；
- 学习 Blender 智能 UV；
- 学习 Shrinkwrap；
- 学习 Normal / Roughness 等贴图烘焙工作流。

### 当前实验结论

手动流程目前：

```text
Topology cleanliness:
有所改善
```

但最终视觉 / 材质效果：

```text
暂时不如 Meshy 自带重拓扑 + 贴图生成结果
```

主要原因是本阶段 Blender / Bake 学习时间较短，经验不足。

因此本轮没有把手工 Blender 版本视为最终生产资产。

当前更合理的结论是：

```text
Meshy output
→ 当前可用资产基线

Blender manual retopo / UV / bake
→ 继续学习
→ 用于未来更高质量与可控性需求
```

该阶段的价值主要是建立完整 3D Pipeline 概念，而不是得到最终 Skeleton 成品。

---

# 14. Week9 Final Regression & Day7 Recording

Final Regression 已从“第一次玩”的角度完整人工运行。

验证主链：

```text
Start
→ Move / Look
→ Fragment01
→ CR01
→ Fragment02
→ CR02
→ CR03
→ Lantern Puzzle
→ Fragment03
→ Boss Gate
→ Mini-Boss
→ Exit Gate
→ Fade
→ Land Temple Front
```

Final Regression 人工结果：

```text
PASS
```

Day7：

```text
完整流程录屏:
COMPLETED
```

当前没有已报告的 Week9 blocker。

---

# 15. Final Known Deferred Items

以下内容明确不阻塞 Week9 完成。

## 15.1 Fragment02 / Fragment03 Narrative Content

当前：

```text
Fragment02:
CONTENT PLACEHOLDER

Fragment03:
CONTENT PLACEHOLDER
```

但其：

```text
Availability
Interaction
Modal UI
Collection
HUD progress
Encounter progression
```

已经全部验证。

后续只需要在 narrative/content pass 替换正式文本。

---

## 15.2 Dodge / Roll

当前：

```text
Space = Jump
```

CR03 仍是：

```text
Future Dodge / Roll teaching room
```

未实现：

```text
IA_Dodge
Roll
I-Frame
Dodge animation
```

这是明确的 Week9 out-of-scope feature。

---

## 15.3 Dungeon Ceiling / Lighting

当前地牢没有 Ceiling / Roof。

后续：

```text
Internal Lighting Design
→ Lighting Validation
→ Add Ceiling / Roof
```

---

## 15.4 Land Temple

本周只完成：

```text
front greybox
safe arrival
formal-story entry boundary
```

仍未完成：

```text
final architecture
materials
lighting polish
interior
NPC
formal dialogue
first Case
Land Temple Hub functions
```

---

## 15.5 Tutorial Completion Persistence

本周没有新增：

```text
bTutorialDungeonCompleted
Chapter Entry State
Tutorial Encounter SaveGame
```

Memory Maze Encounter v0.1 也是：

```text
Local Encounter Framework
```

而不是 global / persistent Encounter system。

---

# 16. Memory Maze Encounter v0.1 Boundary

当前 v0.1 使用 direct Actor references：

```text
Encounter
→ Condition
→ Enemy / Objective

Encounter
→ Action
→ Gate / Reward
```

因此未来正式 Memory Maze 建议：

```text
一个 Encounter 的 hard references
尽量保持在同一房间 / 邻近 progression 区域
```

暂时不要用 v0.1 建立跨整个大地图的长距离硬引用链。

未来如果出现真实需求：

```text
World Partition cell unload / reload
长期 backtracking
Encounter state persistence
跨房间远距离引用
```

再设计：

```text
v0.2 runtime-state ownership / registry / persistence
```

Week9 不提前增加 Subsystem / DataAsset / Procedural Encounter 系统。

---

# 17. Important Problems & Reusable Fixes

| 问题 | 原因 | 最终解决 | 后续规则 |
|---|---|---|---|
| Codex 初版三房 Greybox 可走但不可靠 | 自动结果更偏拓扑检查，缺少真实第三人称走图判断 | 开发者人工修 Bridge、CR02、Uphill、CR03、旧 shortcut | 场景布局必须人工 PIE 走图 |
| Tutorial Enemy 没 Mesh / Anim | 使用了旧 `BP_GhostEnemy` | 改为 `BP_GhostEnemy_New` | 当前 Enemy gameplay baseline 使用 `_New` |
| Tutorial modal 同一 E 可能泄漏到 World Interaction | Pause 下 Character 输入与 Widget 输入路径不同 | Widget 直接处理 E + common close + defensive guard | Modal 输入必须明确消费 |
| 新 Land Temple 生成错误 Pawn | Map 没有 GameMode Override | 设置 `BP_GameMode`，验证 `BP_PlayerCharacter` | 新地图必须检查 World Settings / Pawn |
| Puzzle / Boss / CR01 出现多套 Tutorial glue | 每个功能单独写 Link / Encounter | 抽象 `Condition → Encounter → Action` | 正式 Memory Maze 继续复用 v0.1 |
| 两个 Encounter 可共享同一 Condition / Action | stateful Actor 没有 owner | Exclusive runtime ownership | 一个实例同一时间只属于一个 Encounter |
| Encounter Claim 可能半成功 | 逐个绑定无事务保护 | Transactional claim + rollback | 激活前先完成完整依赖 claim |
| 新 Owner reclaim 继承旧 `Satisfied / Executed` | ownership release 没定义新 runtime cycle | genuine new-owner claim reset | same-owner 不 reset，新 owner 才 reset |
| Ghost 受击后偶发永久停追 | `Chasing` state 与 PathFollowing liveness 脱节 | chase path watchdog + Nav knockback projection | AI state 与 active path 都要验证 |
| Dungeon Roof 未添加 | Lighting 尚未设计 | 延后 | 先照明，再封顶 |
| 手工 Retopo / Bake 视觉下降 | Blender / Bake 经验不足 | Meshy 作为当前资产基线，继续学习 | 不因拓扑更干净就牺牲最终材质效果 |

---

# 18. Final Technical Architecture Snapshot

## Tutorial-only Systems

继续保持 Tutorial-specific：

```text
ATutorialMemoryFragment
UTutorialFragmentWidget
UTutorialHUDWidget
ATutorialDungeonBootstrap
UTutorialInstructionWidget
ATutorialInstructionTrigger
ATutorialLevelTransitionTrigger
```

这些职责本身就是教学 / Tutorial 边界，因此不强行通用化。

## Reusable Gameplay Systems

```text
AWCMemoryMazeEncounter
AWCEncounterConditionBase
AWCEnemyDefeatEncounterCondition
AWCObjectiveCompleteEncounterCondition

AWCEncounterActionBase
AWCOpenGateEncounterAction
AWCRevealTutorialFragmentEncounterAction

AWCProgressionGate
```

其中：

```text
AWCRevealTutorialFragmentEncounterAction
```

是 Tutorial-specific endpoint adapter；

Encounter Core 本身不依赖 Tutorial。

---

# 19. Scope Protection

Week9 明确没有扩展：

```text
Formal Case 01
AI Judge UE API
Moral Judgement UI
Disposition UI
AI-generated Case
Procedural Maze
Encounter Manager / Subsystem
SaveGame Encounter persistence
Behavior Tree / EQS
Boss phase system
Dodge / Roll
formal Land Temple story
```

AI 判官外部原型保持冻结，不因 Tutorial Dungeon 开发而扩张。

```text
Real AI Calls:
0
```

---

# 20. Week9 Final Completion Summary

Week9 最终完成：

- 独立 Tutorial Dungeon Map；
- 从 Week8 复杂迷宫中提取一条可读主路线；
- 一个短 Side Alcove；
- 三段 Combat Teaching 空间；
- Tutorial Fragment01 / 02 / 03；
- Fragment availability / collection / modal UI；
- Tutorial HUD `0 / 3 → 3 / 3`；
- Tutorial Bootstrap；
- Move / Look 教学；
- Interact 教学；
- Pause-based Tutorial Instruction；
- CR01 Light / Heavy / Combo；
- CR01 Enemy Defeat → Fragment02；
- CR02 Lock-On / Strafe；
- CR03 Future Dodge Placeholder；
- 五灯 Puzzle；
- Preview / Wrong / Replay / Correct / Complete；
- Fragment03 availability；
- Boss Gate；
- Tutorial Mini-Boss；
- Boss Defeat → Exit Gate；
- Exit Trigger；
- Fade；
- `OpenLevel()`；
- Land Temple Front Greybox；
- Correct Land Temple GameMode / PlayerStart；
- Memory Maze Encounter v0.1；
- Encounter Condition / Action；
- Progression Gate；
- CR01 / Puzzle / Boss 迁移；
- Exclusive ownership；
- Transactional claim / rollback；
- Runtime-cycle correctness；
- Ghost hit-reaction chase recovery；
- World Partition reference reload validation；
- Final Regression 人工通过；
- Day7 完整录屏；
- AI 3D / Blender 基础流程学习与骷髅资产实验。

---

# 21. Week9 Final Gate

```text
Tutorial Dungeon Spatial Flow:
PASS

Tutorial Fragment Flow:
PASS

Combat Tutorial:
PASS

Lantern Puzzle:
PASS

Mini-Boss:
PASS

Exit Gate:
PASS

Tutorial → Land Temple Transition:
PASS

Land Temple Safe Arrival:
PASS

Memory Maze Encounter v0.1:
PASS

Encounter Ownership / Runtime Cycle:
PASS

Ghost Hit-Recovery Regression:
PASS

Full Human Final Regression:
PASS

Day7 Recording:
COMPLETE
```

Known non-blocking items：

```text
Fragment02 / Fragment03:
PLACEHOLDER NARRATIVE CONTENT

Dodge / Roll:
NOT IMPLEMENTED
OUT OF WEEK9 SCOPE

Dungeon Ceiling:
DEFERRED UNTIL LIGHTING DESIGN
```

Therefore:

```text
WEEK9 CORE DEVELOPMENT:
COMPLETE
```

---

# 22. Next Development Direction

Week9 结束后，不建议继续向 Tutorial Dungeon 增加新功能。

下一阶段更合理的重点是：

```text
1. 冻结 Week9 Tutorial baseline；
2. 用 Memory Maze Encounter v0.1 开始正式 Memory Maze 内容设计；
3. 设计正式 Fragment / Case 内容，而不是继续使用 Tutorial placeholder；
4. 根据真正的 gameplay 需求再决定 Encounter v0.2；
5. 继续独立推进 AI 3D → Blender production pipeline；
6. 后续实现 Dodge / Roll 时再启用 CR03 正式教学。
```

Week9 的核心成果不是“做完了一张新手地图”，而是建立了：

```text
可完整游玩的 Tutorial Vertical Slice
+
可复用的 Memory Maze Encounter foundation
+
正式剧情入口 Land Temple Front
```

这三者共同构成后续正式内容开发的新基线。

---

# 23. Post-Week9 Repository Hygiene & Codex Scratch Policy

Week9 收尾后对项目 `Saved` 目录进行了一次单独审计，确认 Codex 在自动化执行、截图、静态审计和 PIE 验证过程中留下了较多一次性任务产物。

审计样本：

```text
Saved total:
约 135.7 MiB
841 个条目

Saved/Codex:
约 23.3 MiB
168 个实际文件
```

`Saved/Codex` 中主要包含：

```text
67 × Python scripts
54 × JSON audit / validation files
17 × PNG screenshots
18 × txt marker files
10 × lock files
少量 log / pyc
```

其中截图约占：

```text
22.4 MiB
```

并存在已经明确标记为：

```text
obsolete_screenshots
```

的旧证据图，以及多轮任务遗留的一次性 Python / JSON / lock 文件。

这类文件在：

```text
Codex 执行
→ 自动验证
→ Result Package 制作
→ Human / Technical Review
```

期间具有价值，但在任务已经：

```text
PASS
→ Progress recorded
→ Git committed
```

之后不应长期堆积在项目目录。

---

## 23.1 Unreal Generated Saved Data 与 Codex Scratch 的区别

`Saved` 中并非所有体积都属于 Codex 手动生成。

本次审计还观察到：

```text
Saved/Crashes:
约 94.3 MiB

Saved/Logs:
约 9.27 MiB
```

其中最大的一批 Crash Dump 来自 Week9 之前的旧日期，因此不能全部归因于 Codex。

但 Codex 高频：

```text
启动 Editor
运行 Python automation
PIE
Map reload
Commandlet / audit
```

会间接产生额外：

```text
UE Logs
Crash / Ensure data
```

因此以后应分别管理：

```text
Codex scratch
vs
UE generated diagnostic data
```

---

## 23.2 Default Preservation Rules

默认保留：

```text
Saved/Config/
Saved/SaveGames/
Saved/Autosaves/
```

原因：

```text
Saved/Config
→ Editor / per-project user settings

Saved/SaveGames
→ 实际 Gameplay persistence 测试数据

Saved/Autosaves
→ Editor 恢复价值
```

不要在普通 Codex cleanup 中自动删除。

---

## 23.3 Default Disposable / Reviewable Data

任务完成后通常可清理：

```text
Saved/Codex/<Task>/
obsolete screenshots
one-off Python scripts
temporary audit JSON
marker / lock files
temporary capture files
ShaderDebugInfo
old UE Logs
AutoScreenshot / tmp files
```

`Saved/Crashes`：

```text
先记录日期 / crash identity
→ 确认不再用于 active debugging
→ 再清理
```

不要为了磁盘整洁而删除仍在调查中的 crash evidence。

---

## 23.4 Codex Scratch Lifecycle

以后 Codex 使用：

```text
Saved/Codex/<TaskName>/
```

只能作为 temporary workspace。

推荐生命周期：

```text
Codex executes
↓
Saved/Codex/<TaskName>
temporary scripts / JSON / screenshots
↓
Result Package created
↓
Human Review
↓
Technical Review
↓
PASS
↓
formal source / assets committed
↓
delete task scratch
```

禁止形成：

```text
Week8 scratch
+
Week9 Day1 scratch
+
Day2 scratch
+
Day3 scratch
+
Pre-Final scratch
+
obsolete screenshots
→ indefinitely accumulated
```

---

## 23.5 Result Package Rule

在清理 `Saved/Codex/<TaskName>` 前：

```text
only required evidence
→ copy into Result Package
```

Result Package 应包含：

```text
final report
必要 screenshot
必要 source patch / source_review
必要 technical evidence
```

不应把全部中间脚本、全部重复截图和所有临时 JSON 原样复制过去。

---

## 23.6 One-Off Script Rule

一次性 Python 脚本应默认视为 disposable。

只有满足以下条件时才考虑保留为正式工具：

```text
跨任务可复用
命名稳定
输入 / 输出明确
不依赖某次地图临时 Actor 名
有实际维护价值
```

否则：

```text
task complete
→ delete
```

不要因为 Codex 曾经执行过一个脚本，就把它永久当成项目工具。

---

## 23.7 Git Hygiene Rule

永远不要 Commit：

```text
Saved/Codex/
Saved/Logs/
Saved/Crashes/
ResultPackage screenshots
technical_evidence
temporary patch review files
one-off automation scripts
```

除非未来某个文件被明确提升为正式、可维护的项目工具。

Week9 Git Prompt 已坚持：

```text
explicit per-path staging
```

而不是：

```text
git add .
git add -A
```

该规则后续继续保持。

---

## 23.8 Tutorial Dead-Code Cleanup After Encounter Revision

Week9 Pre-Final 已将多个早期 Tutorial-specific progression glue 迁移至：

```text
Condition
→ AWCMemoryMazeEncounter
→ Action
```

因此以下早期类型当前地图 placed runtime refs 已为：

```text
0
```

已知 cleanup candidates：

```text
ATutorialEnemyDefeatFragmentLink
ATutorialPuzzleCompletionLink
ATutorialBossEncounter
```

以及其对应 Blueprint wrappers。

此前为了降低 Pre-Final 风险，它们被暂时作为：

```text
unplaced compatibility code / assets
```

保留。

Week9 完成后可以进行正式 dead-code audit。

删除前必须同时验证：

```text
C++ references = 0
Blueprint asset references = 0
Map / ExternalActor references = 0
Serialized asset references = 0
```

不能仅凭：

```text
text search returns 0
```

就删除 Unreal 类。

---

## 23.9 Tutorial Code That Must Not Be Blindly Removed

以下系统仍属于 Week9 最终正式流程，应默认保留：

```text
ATutorialMemoryFragment
UTutorialFragmentWidget
UTutorialHUDWidget
ATutorialDungeonBootstrap

UTutorialInstructionWidget
ATutorialInstructionTrigger

ATutorialLevelTransitionTrigger
```

这些类虽然名称包含 `Tutorial`，但职责本身仍然是当前 Tutorial Vertical Slice 的有效功能，不属于 dead code。

---

## 23.10 AWCTutorialGate Compatibility Audit

当前架构：

```text
AWCProgressionGate
↑
AWCTutorialGate
↑
BP_TutorialGate
```

`AWCTutorialGate` 已经只承担 compatibility role。

未来 cleanup 可以审计：

```text
BP_TutorialGate
```

是否能够安全 reparent 到：

```text
AWCProgressionGate
```

且保持：

```text
Boss Gate
Exit Gate
OpenOffset
Collision
Transition Trigger gate reference
```

完全一致。

只有经过：

```text
Reference audit
→ Reparent
→ Save / Reload
→ Build
→ MapCheck
→ PIE regression
```

之后，才允许删除：

```text
WCTutorialGate.h/.cpp
```

否则继续保留 compatibility subclass。

---

## 23.11 Test-Only Runtime API Cleanup

此前 Day3 automation 曾增加：

```text
TriggerLanternInteractionForTest()
```

到 `LanternPuzzlePiece`。

正式 Tutorial / Encounter runtime 不依赖该 API。

如果该 test-only diff 当前仍在工作区或尚未正式进入需要维护的 test framework：

```text
audit references
→ remove from production runtime source
```

不要因为 Codex automation 曾使用过，就长期扩大 production gameplay API surface。

---

## 23.12 Post-Week9 Cleanup Principle

Week9 之后的 cleanup 目标不是：

```text
删除所有名字里有 Tutorial 的东西
```

而是：

```text
删除已经被 Revised / Encounter v0.1 替代、
且能够证明零正式引用的实现。
```

最终原则：

```text
Keep:
currently exercised gameplay architecture

Delete:
obsolete duplicated glue
temporary automation artifacts
unused compatibility code proven safe to remove

Preserve:
user settings
save data
autosaves
unresolved diagnostic evidence
```

这项 cleanup 应作为 Week9 完成后的独立 maintenance pass 执行，而不是混入新玩法开发。
