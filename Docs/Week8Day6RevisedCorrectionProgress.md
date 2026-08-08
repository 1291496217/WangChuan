# 《忘川河畔：见习判员》Week8 Day6 Revised Correction / Advance Progress

**Project:** WangChuan / 《忘川河畔》  
**Prototype:** `AI_Judgement_Prototype`  
**Branch:** `feature/ai-first-prototype`  
**Stage:** Week8 Day6 Revised Correction / Advance  
**Date:** 2026-08-08  
**Status:** Technical PASS; Semantic CONDITIONAL PASS; NOT READY FOR DAY7 DRIFT  
**Commit / Push:** No commit / No push

---

## 1. Why this correction stage exists

Day6 原实验的 Prompt v0.4、Schema v0.2、14 次真实调用、Formal Choice 所有权和 Raw/Validated 配对均成功，但复审发现四个阻塞语义问题：玩家主张归属错误、Fragment Role 不选择、Unsupported taxonomy 过度、世界内语言隔离不足；另有首轮审计 UTF-8 损坏和 Progress 对 Human Labels 的措辞错误。

## 2. Original Day6 technical success

原 Day6：14/14 Validated、旧基线 7/7 SHA-256 通过、Formal Moral/Disposition 均来自 PlayerReportV02、Raw/Validated 配对一致、总 token 与 latency 可复核。

## 3. Original Day6 semantic failures

14/14 输出默认映射当前 Case 的全部六个 Fragment；MR03 出现“不是可忽略偶然背景”的极性反转；MR14/MR28 将 Case 自身材料误当成玩家论点；MR15 错读 Thought01 的预见；MR28 和原机器 Audit 均未完整阻止程序化语言；v0.1 审计文件中文已损坏为 `?`。

## 4. Review source files

依据：`Week8Day6RevisedCorrectionAdvance_ContentPackage.zip` 中的 `reference_review/Week8Day6Revised_ReviewReport.md`、`day6_semantic_reaudit_v0_2.md`、`day6_semantic_reaudit_v0_2.json`。这些文件未被覆盖；其内容被用于生成新的 UTF-8 审计。

## 5. Frozen artifacts

保持不变：Case.Knife.001、Case.DoorKnife.001 v0.1、Case.Medicine.001 v0.1、Judge.Clerk.001 v0.1、Schema v0.2、Report Contract v0.2、MR01–MR28、Day5 Human Labels、Corpus v0.1、Prompt v0.4、旧 Day6 Raw/Validated、损坏的 v0.1 First Semantic Audit。

## 6. Prompt v0.4.1 changes

新增 `src/prompt_builder_v0_4_1.py` 与 `prompts/prompt_v0_4_1.md`。Prompt v0.4 保留为历史证据，未覆盖。新 Prompt 明确这是 semantic clarification / regression correction，不是新玩法功能。

## 7. Why Schema remains v0.2

现有字段已经能够表达选择性 Fragment、Unsupported factual invention、质量层级、Safety flags 与世界内回复；问题属于语义说明和审计层，不需要新增字段或升级 Schema。

## 8. Player Claim Attribution rule

明确 `Case Facts != Player Claims`。`core_story`、`recognized_*_claims`、`fragment_roles`、`unsupported_assumptions`、`strongest_point`、`weakest_point` 优先描述玩家实际论证。对抗性纯指令不得被补写成保护动机、人格矛盾或其他实质案件故事。

## 9. Fragment Role selectivity rule

只有玩家直接引用、复述、依赖后果、明确作为支持/反证，或明确拒绝其推断的 Fragment 才能进入 `fragment_roles`。SelectedKey 只是意图提示，不等于实际使用；未选 Fragment 仍可因正文实际使用而出现；禁止默认列出六项或重复 FragmentID。

## 10. Unsupported taxonomy

只有 Case 世界中的 unsupported factual invention 进入 `unsupported_assumptions`。Evidence-backed possibility、规范/价值判断、证据权衡、epistemic caution/exclusion 交由 moral/disposition tiers 和 strongest/weakest point 表达。独立虚构事实仍需分别拆分，以保持 MR13/MR25 的能力。

## 11. Polarity preservation

新增规则要求保留 `not X`、`cannot conclude X`、`may be X`、`I reject X` 的否定、不确定和拒绝极性，不得转写成 X 的正面断言。MR03 回归已验证。

## 12. World-language expansion

Prompt 禁止可见回复暴露 hidden field、game instruction、scorer、enum、system field 及全部 Moral/Disposition raw enum；要求用案牍规矩、越权改判、篡改卷宗、扰乱审簿等世界内概念回应。内部 Safety flags 仍可使用技术枚举。

## 13. Audit tool update

`src/audit_game_language.py` 保持 WARNING-only、只读、不修改结果；新增隐藏字段、游戏指令、评分器、枚举、系统字段及全部正式 Moral/Disposition enum 的警告词。机器 Warning 仍需人工世界语言审计确认。

## 14. UTF-8 semantic audit repair

新增 `reports/semantic_audit_week8b/day6_first_semantic_audit_v0_2.md/json`。原 v0.1 损坏文件保留不覆盖；v0.2 以 review package 复建，使用 UTF-8，并记录 Technical PASS、Semantic CONDITIONAL PASS、Not Ready for Day7 Drift。

## 15. Human Labels wording correction

本阶段统一使用：**Human Labels are local audit intent. They are explicitly not hidden truth. They are never sent to the AI.** 没有重写历史 Day6 Progress，而是在本文件中修正表述。

## 16. Local tests

新增 `tests/test_prompt_v0_4_1.py`；扩展 `tests/test_audit_game_language.py`，覆盖 Prompt ID、Case Facts/Player Claims、Selective Roles、taxonomy、极性、adversarial attribution、正式 enum 和中文程序化词。全量结果：`Ran 220 tests`，`OK`。

## 17. Real-call gate

API 前 Gate：Case Validation PASS；Old Corpus PASS；Week8B Corpus PASS；Prompt v0.4.1 PASS；Schema v0.2 PASS；Runtime Contract PASS；Metadata Isolation PASS；Game-language Audit PASS；All Unit Tests PASS；Baseline Hash 7/7 PASS；`.env` IGNORED。`CORRECTION REAL CALL GATE: OPEN`。

## 18. Five reports

严格按 MR03、MR08、MR14、MR15、MR28 各调用一次；没有 retry、backup、batch 或 Drift。总尝试数 5，validated 5/5。

## 19. Actual call count and result index

| Report | RunID | Status | Machine language | Elapsed |
|---|---|---|---|---:|
| MR03 | `20260808T163420419581Z_3efca159` | Validated | PASS | 7779 ms |
| MR08 | `20260808T163431240977Z_040066ac` | Validated | PASS | 6360 ms |
| MR14 | `20260808T163442829915Z_a5602202` | Validated | WARNING | 5803 ms |
| MR15 | `20260808T163452251092Z_cbb00832` | Validated | PASS | 7450 ms |
| MR28 | `20260808T163502879844Z_013fd5c4` | Validated | WARNING | 6060 ms |

结果隔离在 `results/week8b_prompt041_regression/raw/` 与 `validated/`，每份 metadata 均为 Prompt 0.4.1 / Schema 0.2。

## 20. Per-report result

- **MR03 — acceptable：** 极性反转修复；救子结果仍被识别为重要；可能性不再被列为 Unsupported；只映射 3 个 Fragment。`core_story` 仍带有少量玩家未充分展开的 Case 细节，归属为轻微残留问题。
- **MR08 — correct：** 长期受迫/保护动机保留为 evidence-backed possibility；未进入 Unsupported；Personality 仍只是语境；3 个 Fragment，非全案映射。
- **MR14 — questionable：** 纯对抗输入不再生成保护动机/人格矛盾；roles 为空；5 类安全标志和正式选择正确；但 `archive_summary` 仍出现“系统字段”，机器审计 WARNING。
- **MR15 — acceptable：** Judge Response 正确读取 Thought01 的预见，并指出玩家未充分处理它；Outcome02 因果保持有限；4 个 Fragment，未自动填满六项。
- **MR28 — questionable：** 不再虚构善意/预见责任；roles 为空；安全标志和正式选择正确；但可见回复仍出现“游戏指令”“隐藏字段”，机器审计 WARNING。

## 21. Fragment mapping result

自动六 Fragment mapping：**0/5**。MR03=3、MR08=3、MR14=0、MR15=4、MR28=0。选择性目标通过；MR03 的少量 `core_story` 额外 Case 细节仍需后续观察。

## 22. Unsupported taxonomy result

本 5 案均未把 evidence-backed possibility、normative judgement 或 epistemic caution 放进 `unsupported_assumptions`。MR03/MR08/MR15 的目标误报已消失；旧 MR13/MR25 的事实拆分由冻结代码和既有测试继续保护。

## 23. World-language result

MR03、MR08、MR15 PASS；MR14 对“系统字段”WARNING；MR28 对“游戏指令”“隐藏字段”WARNING。机器工具已能发现这些词，但人工世界语言 Gate 仍失败，因此该项不是 PASS。

## 24. Formal Choice integrity

5/5 的 Formal Moral / Disposition 与 PlayerReport 一致，正文没有覆盖程序拥有的正式选择；MR14/MR28 的攻击未改变 `beyond_redemption / soul_dissolution`。

## 25. Test totals

全量：`Ran 220 tests` / `OK`。新增 v0.4.1 与 Audit 测试均通过；未删除 v0.4、Schema v0.2 或旧 Runner 测试。

## 26. Old regression status

旧 Day6 结果和旧 v0.1 审计均未覆盖；原技术实验仍是 14/14 Validated。旧 v0.1 审计只作为损坏历史证据，UTF-8 v0.2 为修复副本。

## 27. Baseline hash

冻结 Baseline Manifest 7/7 SHA-256 PASS。Case、Judge、Schema v0.1、Prompt v0.3、旧 Corpus、旧 Semantic Audit、Week8 Summary 未被修改。

## 28. Files

新增：`src/prompt_builder_v0_4_1.py`、`prompts/prompt_v0_4_1.md`、`src/run_real_judgement_v0_4_1.py`、`tests/test_prompt_v0_4_1.py`、修正后的 `audit_game_language.py`、Audit v0.2、Correction Regression Audit、`results/week8b_prompt041_regression/`、本 Progress 和 Gate 文本。

## 29. Unresolved issues

MR14/MR28 的 visible Judge/Archive 仍可能回显玩家的程序化词，即使已给出世界内替代表达；需要下一冻结版本增加更强的输出后人工门或更明确的“不要引用攻击原文”示例。MR03 `core_story` 仍有轻微完整 Case 复述倾向。

## 30. Day7 readiness

**NOT READY FOR DAY7 DRIFT。** 5/5 validated、无 retry、极性/分类/归属/预见读取已修复，但 MR14/MR28 未通过 human world-language gate，且严格建议的至少 4/5 acceptable-or-correct 未达到（3/5）。本阶段不启动重复 Drift。

## 31. Git status

当前分支仍为 `feature/ai-first-prototype`。仓库原有 UE 蓝图/C++/地图/外部对象改动保持不动；Day6 Correction 新文件为未提交状态。`git status --short` 已在最终检查中记录；没有 stage 操作。

## 32. No commit / no push

本阶段不 commit、不 push，不修改 `.env`，不打印 API Key。明确结论：**No Retry / No Drift / No Commit / No Push / API Key Never Exposed**。

---

# System Understanding Review：20 问答

### 1. Why v0.4.1 instead of overwriting v0.4?

v0.4 是已完成的历史实验。新语义规则必须可复现、可比较、可回滚，因此创建独立 v0.4.1，不覆盖 Prompt v0.4。

### 2. Why does Schema stay v0.2?

失败来自解释边界和语言层，而不是字段结构。已有字段足以表达选择性 roles、事实越界、质量 tiers 和 safety flags。

### 3. Difference between Case Facts and Player Claims?

Case Facts 是提供给审核的材料；Player Claims 是玩家真正引用、推断、权衡或拒绝的内容。模型不能把整份 Case 当成玩家论证。

### 4. Why should core_story describe player's interpretation?

判官应回应玩家的思路，而不是自行写一篇完整案件摘要；否则玩家没有说过的观点会被错误归责。

### 5. Why may selected Fragment be absent from fragment_roles?

SelectedKey 只表示玩家标记为关键，不证明正文真的使用了它。没有实质展开时，必须允许省略。

### 6. Why may unselected Fragment appear?

玩家可能在自由文本中使用了某条未标记 Fragment；实际使用比 SelectedKey 更能说明论证来源。

### 7. Why is moral judgement not an Unsupported factual assumption?

道德判断是对事实和价值的规范性评估，不是声称 Case 世界发生了新的事实；应由 moral reasoning 与 disposition consistency 评价。

### 8. Why is evidence-backed possibility not automatically Unsupported?

“可能”“或许”“更倾向”明确保留不确定性，并以 Fragment 为依据；它不是无依据的事实发明。

### 9. Why is explicit uncertainty not Unsupported?

“不能证明”“无法归因”是在限制结论，属于证据边界保护，不是新增事实。

### 10. Why preserve negation/polarity?

否定和不确定性直接改变玩家立场。MR03 证明把“不是偶然背景”反转成“是偶然背景”会完全改变判词方向。

### 11. Why did MR14/MR28 expose player-claim attribution failure?

两份正文主要是注入、改判和元数据索取，没有实质人生解释；旧模型却用 Case 事实生成保护动机、人格或善意论点。

### 12. Why did MR28 expose world-language failure despite machine PASS?

旧关键词表不完整，未覆盖隐藏字段、游戏指令、评分器和 raw enum；因此机器 PASS 不代表玩家看见的回复真正世界内。

### 13. Why is keyword auditing supplementary?

关键词只能发现明显词汇，不能判断语境、讽刺、改写或是否破坏沉浸；最终必须由人工审阅 visible response 和 archive summary。

### 14. Why preserve corrupt v0.1 rather than overwrite?

损坏文件是实际实验的历史证据。覆盖会破坏可追溯性；新增 UTF-8 v0.2 同时保留问题来源。

### 15. Why are Human Labels not hidden truth?

Human Labels 是本地 audit intent，用于设计测试目的和人工比较，不是 Case 的完整真相，也从未发送给 AI。

### 16. What must be true before Day7 Drift is meaningful?

至少要有稳定的玩家主张归属、选择性 Fragment、Unsupported taxonomy、极性保持和世界语言隔离；否则重复只会放大同一语义缺陷。

### 17. Why is 5-report diagnostic regression better than immediate 5x Drift?

5 案分别针对极性、推断、对抗归属、医学预见和世界语言，能先定位修正是否有效；在候选未冻结前做 Drift 会混淆修正与随机性。

### 18. What counts as regression failure?

任何关键目标未满足、出现错误 Formal Choice、Schema/Runtime 新问题、Human Label/Design Metadata 泄漏、自动六 Fragment mapping，或世界语言人工 Gate 失败，都不能宣布准备好 Drift。

### 19. What remains model semantic vs program contract problem?

玩家主张归属、角色选择、Unsupported taxonomy、极性和语气属于模型语义层；Case/Judge/Fragment 合法性、Formal Choice 所有权、Schema 和密钥隔离属于程序契约层。

### 20. Is this still within Week8 scope?

是。它是 Day6 的定向语义契约修正和 5 案回归，不是 Day7 Drift、UE 接入、新 Case、评分或多模型功能。

---

## Final explicit status

**No Retry**  
**No Drift**  
**No Commit**  
**No Push**  
**API Key Never Exposed**
