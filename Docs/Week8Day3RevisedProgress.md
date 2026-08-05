# 《忘川河畔：见习判员》Week8 Day3 Revised Progress

**Project:** WangChuan / 《忘川河畔》
**Prototype:** `AI_Judgement_Prototype`
**Branch:** `feature/ai-first-prototype`
**Stage:** Week8 Day3 Revised
**Date:** 2026-08-05
**Theme:** Freeze Baseline & Design `Case.DoorKnife.001`
**Status:** Completed
**Real AI Calls:** 0
**Commit / Push:** Not Yet

---

# 1. Day3 目标

Day3 不继续修改旧 Knife Case，也不开始新的 AI Prompt 实验。

今天的目标是：

```text
冻结旧 Week8 AI 实验基线
+
设计第二个开放善恶初判案件
+
验证案件本身是否值得交给 AI 测试
```

需要回答的核心问题：

> 六条局部事实能否支持多种受到证据约束的善恶解释，而不会暗藏完整人生答案或唯一正确处置？

Day3 只进行 Case Design、纸面解释、信息预算和本地结构检查。

今天明确没有进行：

- Prompt v0.4；
- Schema v0.2；
- Report Parser v0.2；
- 新版 Corpus；
- 新 Judge；
- 第二个 Provider；
- 真实 DeepSeek 调用；
- UE Integration；
- 自动评分；
- Reward System。

---

# 2. 起始状态

Week8 前半已经完成并冻结：

```text
Case.Knife.001 v0.2
Judge.Clerk.001 v0.1
Prompt v0.3
Schema v0.1
Corpus.Knife.Week8.001 v0.1
```

旧实验结果：

```text
All Week8 Real Calls:
21

Prompt v0.3 Corpus Calls:
19

Semantic Audit Reports:
19
```

Semantic Audit 结论：

```text
Correct:       11
Acceptable:     1
Questionable:   7
Incorrect:      0
Not Auditable:  0

Core Hypothesis:
Promising
```

Day3 必须保留这些结果作为回归基线，不能为了新版设计而修改或重新解释旧数据。

---

# 3. Baseline Freeze

创建：

```text
AI_Judgement_Prototype/
└─ baselines/
   └─ baseline_manifest_week8_v0_1.json
```

Baseline ID：

```text
Baseline.Week8.AIJudgement.001
```

Baseline Version：

```text
0.1
```

## 3.1 冻结版本

```text
Case:
Case.Knife.001 v0.2

Judge:
Judge.Clerk.001 v0.1

Prompt:
v0.3

Schema:
v0.1

Corpus:
Corpus.Knife.Week8.001 v0.1

Provider:
deepseek

Model:
deepseek-v4-flash

Thinking:
disabled

Temperature:
0.2

Max Tokens:
2400

Streaming:
false

Automatic Retry:
false
```

## 3.2 Manifest 记录内容

Manifest 记录：

- 当前 Branch；
- 固定版本；
- 模型配置；
- 关键 Artifact 路径；
- 关键 Artifact SHA-256；
- 21 次 Week8 调用；
- 19 次 Prompt v0.3 Corpus 调用；
- 19 份 Semantic Audit；
- 已知回归问题；
- Freeze Policy。

## 3.3 固定回归案例

```text
R03:
Fragment false-positive Identity

R05:
Fragment missing Identity

R09:
Fragment missing Motive

R10:
Complex unsupported claims under-split

R17:
Fragment missing Contradiction
+
core fabricated claim not separated

R18:
Game-language leakage

R19:
Disposition override attempt
```

## 3.4 Freeze Policy

```text
overwrite_old_artifacts:
false

overwrite_old_results:
false

reinterpret_old_results_as_new_schema:
false
```

## 3.5 Baseline Hash

本地已完成关键文件 SHA-256 核对。

结果：

```text
BASELINE HASH VERIFICATION PASSED
```

说明旧 Case、Judge、Prompt、Schema、Corpus 与旧 Summary 未因 Day3 发生变化。

---

# 4. Canonical Week8 Development Log

`Docs/Development_Log_Week8.md` 在最初工作区中曾显示为删除。

用户已经恢复该文件。

当前状态：

```text
Development_Log_Week8.md:
Restored

Modified by Day3:
No
```

Day3 不修改 canonical Week8 Development Log。

Day3–Day7 的新版结论将在后续统一整理，不应提前覆盖原 Week8 实验记录。

---

# 5. 新 Case

创建：

```text
AI_Judgement_Prototype/
└─ cases/
   ├─ case_door_knife_001.json
   └─ case_door_knife_001_notes.md
```

Case Identity：

```text
CaseID:
Case.DoorKnife.001

CaseVersion:
0.1

Title:
门后的刀

DesignMode:
local_facts_open_moral_judgement

hidden_complete_truth:
false
```

本案不是“什么都不知道”。

本案采用：

```text
有局部事实
无完整人生答案
无唯一善恶结论
```

---

# 6. 四级善恶初判

```text
more_good_than_evil
善多于恶

mixed_merit_and_fault
功过难分

more_evil_than_good
恶多于善

beyond_redemption
罪无可赦
```

当前 Day3 设计能够为前三种主要善恶方向提供解释路径。

```text
beyond_redemption
```

保留为极端测试输入。

它目前不是本案独立成立的第四条标准解释，也不要求与其他善恶方向拥有同等支撑。

---

# 7. 四种命运处置

基础处置：

```text
recommend_rebirth
荐生

ordinary_transfer
平籍入殿

send_to_prison
押狱
```

极端测试处置：

```text
soul_dissolution
魂灭
```

本案目前能够为三个基础处置分别提供真实材料支点。

`soul_dissolution` 只用于未来极端边界测试，不要求本案为其提供与基础处置同等强度的支持。

---

# 8. 六条正式 Fragment

## 8.1 `DoorKnife.Action01`

```text
亡魂把短刀刺入宅主胸口，宅主当场死亡。
```

结构：

```text
SemanticType:
action

SourceType:
objective_trace

InformationWeight:
3

RelationTags:
killing_event
public_persona_conflict

DispositionSupportTags:
ordinary_transfer
send_to_prison
```

确认的最低事实：

- 亡魂实施了刺杀；
- 宅主当场死亡。

不确认：

- 是否预谋；
- 是否自卫；
- 是否为了救孩子；
- 是否出于复仇；
- 宅主此前做过什么。

---

## 8.2 `DoorKnife.Personality01`

```text
邻里都说他低声寡言，遇到争执便退让，是最没胆子惹事的人。
```

结构：

```text
SemanticType:
personality

SourceType:
others_testimony

InformationWeight:
2

RelationTags:
public_persona_conflict

DispositionSupportTags:
recommend_rebirth
ordinary_transfer
send_to_prison
```

确认的最低事实：

- 邻里这样评价亡魂。

不确认：

- 亡魂客观上就是胆小善良；
- 邻里真正了解亡魂；
- 亡魂一直在伪装；
- 该评价能够直接证明无罪。

关键边界：

```text
他人之识
≠
系统确认的客观人格
```

---

## 8.3 `DoorKnife.Thought01`

```text
在许多个夜里，他都想象过宅主死去的样子。
```

结构：

```text
SemanticType:
thought

SourceType:
soul_self_knowledge

InformationWeight:
2

RelationTags:
killing_event
long_term_fear_or_hatred

DispositionSupportTags:
ordinary_transfer
send_to_prison
```

确认的最低事实：

- 亡魂长期想象过宅主死亡。

不确认：

- 已经制定杀人计划；
- 想象一定代表恶意；
- 每次想象都具有相同动机；
- 想象必然导致最终行为。

关键边界：

```text
思想事实
≠
行为事实
```

可能解释：

- 预谋；
- 仇恨；
- 恐惧；
- 无力幻想；
- 长期受迫；
- 混合动机。

---

## 8.4 `DoorKnife.Outcome01`

```text
宅主倒下后，锁在后屋的孩子从门中逃了出来。
```

结构：

```text
SemanticType:
outcome

SourceType:
objective_trace

InformationWeight:
3

RelationTags:
killing_event
child_confinement

DispositionSupportTags:
recommend_rebirth
ordinary_transfer
send_to_prison
```

确认的最低事实：

- 宅主倒下后；
- 后屋孩子逃出。

不确认：

- 亡魂杀人就是为了救孩子；
- 孩子为何被锁；
- 宅主锁孩子的具体原因；
- 亡魂是否亲自开门；
- 善意结果能够自动洗去杀人责任。

关键边界：

```text
实际结果
≠
自动证明行为动机
```

---

## 8.5 `DoorKnife.Relationship01`

```text
“他答应过我，不会再让那个人进来。”
```

结构：

```text
SemanticType:
relationship

SourceType:
others_testimony

InformationWeight:
2

RelationTags:
child_confinement
long_term_fear_or_hatred

DispositionSupportTags:
recommend_rebirth
ordinary_transfer
send_to_prison
```

确认的最低事实：

- 孩子说亡魂作出过该承诺。

不确认：

- 孩子和亡魂的血缘；
- “那个人”具体做过什么；
- 承诺是否一定指向宅主；
- 亡魂最终行为是否完全出于该承诺。

---

## 8.6 `DoorKnife.Death01`

```text
亡魂死在土地庙外，背上有新伤，手中仍握着后屋门锁的钥匙。
```

结构：

```text
SemanticType:
death

SourceType:
objective_trace

InformationWeight:
3

RelationTags:
post_killing_aftermath
child_confinement

DispositionSupportTags:
ordinary_transfer
send_to_prison
```

确认的最低事实：

- 亡魂死在土地庙外；
- 背上有新伤；
- 手中有后屋钥匙。

不确认：

- 是否遭到追击；
- 是否在逃亡；
- 是否遭报复；
- 是否为了救孩子牺牲；
- 背伤由谁造成；
- 钥匙何时取得；
- 钥匙是否真正用于开门。

`Death01` 不作为直接荐生支点。

它属于：

```text
未决终局与行为后果背景
```

而不是：

```text
牺牲证明
赎罪证明
保护动机证明
```

---

# 9. RelationTag 修正

原设计曾使用：

```text
post_killing_pursuit
```

这个名称会把“发生过追击”暗中写入 Case Design。

但 Case 实际只确认：

- 背部新伤；
- 土地庙外死亡；
- 手握钥匙。

因此已经改为：

```text
post_killing_aftermath
```

正式说明：

```text
围绕亡魂带伤死亡、土地庙外终局与后屋钥匙，
不指定追击、逃亡、报复或牺牲经过。
```

这一修正避免 RelationTag 成为隐藏事实通道。

---

# 10. SourceType 与 SemanticType

本案使用六种 SemanticType：

```text
action
personality
thought
outcome
relationship
death
```

本案实际使用三种 SourceType：

```text
objective_trace
soul_self_knowledge
others_testimony
```

两类字段含义不同：

```text
SemanticType:
这条 Fragment 在叙事和判断中扮演什么作用

SourceType:
这条 Fragment 的信息来自何种认知来源
```

例如：

```text
Personality01:
SemanticType = personality
SourceType = others_testimony
```

这意味着它是人格相关材料，但内容仍只是邻里评价。

---

# 11. InformationWeight

```text
1:
轻量上下文

2:
中等信息

3:
核心事实
```

本案权重：

```text
Action01:
3

Personality01:
2

Thought01:
2

Outcome01:
3

Relationship01:
2

Death01:
3
```

`InformationWeight` 只表示该 Fragment 承担多少叙事信息。

它不是：

- 善恶分；
- 有罪分；
- 可靠性分；
- 处置分；
- Reward；
- AI Rating；
- 自动权重。

---

# 12. 设计元数据边界

以下字段属于 Case Design / Audit Metadata：

```text
interpretation_hooks
disposition_support_tags
information_weight
relation_tags
```

这些字段不是案件中的新增事实。

后续进入 Prompt、Corpus 或 UE Runtime 前必须遵守：

```text
Fragment text:
可以作为玩家与 AI 的案件材料

FragmentID:
可以作为程序稳定 ID

semantic_type:
可以用于结构解释与 Validator

source_type:
可以用于来源边界与 Validator

interpretation_hooks:
不得发送给 AI
不得展示给玩家

disposition_support_tags:
不得发送给 AI
不得用于自动判罚

information_weight:
不得转换为善恶、可靠性、奖励或罪责分数

relation_tags:
用于结构与组合审计
不得被解释为事件已经发生
```

如果未来把 Hooks 与 Support Tags 一起发送给 AI，就会形成隐藏答案通道。

---

# 13. 四条人工解释路径

创建独立设计审计：

```text
reports/case_design/
└─ case_design_audit_door_knife_v0_1.md
```

四条解释不是正式真相。

它们用于验证：

> 同一组局部事实能否形成多种相互竞争、但仍受到材料约束的解释。

---

## 13.1 Interpretation A — 伪装的恶人

Primary Moral Judgement：

```text
more_evil_than_good
```

Primary Disposition：

```text
send_to_prison
```

核心材料：

```text
Action01
Thought01
Personality01
```

解释逻辑：

- 亡魂确实杀死宅主；
- 长期想象宅主死亡可以成为敌意或预谋支点；
- 邻里评价可能只是表象、误解或长期伪装；
- 孩子逃出是实际结果，但不能自动证明杀人动机善良。

必须承认的反证：

```text
Outcome01
Relationship01
Death01
```

这条路线不能编造：

- 宅主具体恶行；
- 孩子身份；
- 旧案；
- 追击者；
- 亡魂此前犯罪记录。

`beyond_redemption` 可以在未来作为极端版本测试，但本解释不会将其写成已被证明的正式结论。

---

## 13.2 Interpretation B — 被迫反击的保护者

Primary Moral Judgement：

```text
more_good_than_evil
```

Primary Disposition：

```text
recommend_rebirth
```

直接材料支点：

```text
Outcome01
Relationship01
Personality01
```

解释逻辑：

- 宅主死亡后，孩子确实逃出；
- 亡魂曾作出阻止“那个人”进入的承诺；
- 邻里认为亡魂长期退让；
- 暴力行为可能是长期受迫、恐惧或保护压力下的突破。

必须正面处理：

```text
Action01
Thought01
```

即：

- 亡魂确实杀死宅主；
- 亡魂长期想象过宅主死亡；
- 保护结果不自动证明纯粹善意；
- 荐生仍必须承担杀人责任与长期心念的解释负担。

`Death01` 只能提供未决后果背景，不是直接荐生证据。

---

## 13.3 Interpretation C — 善恶混合的复仇

Primary Moral Judgement：

```text
mixed_merit_and_fault
```

Primary Disposition：

```text
ordinary_transfer
```

核心材料：

```text
Action01
Thought01
Outcome01
Relationship01
```

解释逻辑：

- 亡魂可能同时具有保护动机与私人怨恨；
- 孩子逃出是实际有利结果；
- 长期死亡想象说明行为不一定是瞬间纯粹自卫；
- 杀人责任与保护结果可以同时存在；
- 善意、复仇、恐惧与积怨不必互相排斥。

这条路线不能把亡魂写成纯粹英雄，也不能把孩子逃出写成毫无意义的偶然。

---

## 13.4 Interpretation D — 高修辞但守材料边界

Primary Moral Judgement：

```text
more_evil_than_good
```

Primary Disposition：

```text
send_to_prison
```

目的：

- 验证立场强烈不等于证据薄弱；
- 验证高修辞可以仍然守材料边界；
- 验证 Judge 不应因为文字有气势就自动降低证据评分。

允许：

- 强烈立场；
- 比喻；
- 有力度的判词；
- 明确责任判断。

禁止：

- 新证人；
- 新血缘；
- 新旧案；
- 新宅主罪行；
- 新追击者；
- 新逃亡经过；
- 把推断写成客观事实。

D 不是新的第四种善恶方向。

它主要验证：

```text
表达风格
≠
证据质量
```

---

# 14. 解释覆盖结论

主要善恶方向：

```text
more_good_than_evil:
Supported

mixed_merit_and_fault:
Supported

more_evil_than_good:
Supported

beyond_redemption:
Extreme test only
```

基础处置：

```text
recommend_rebirth:
Supported

ordinary_transfer:
Supported

send_to_prison:
Supported
```

关键覆盖：

```text
Personality as disguise:
Covered

Personality as pressure / misunderstanding:
Covered

Thought as planning / hostility:
Covered

Thought as fear / struggle context:
Covered

Beneficial result does not prove motive:
Covered

Killing responsibility and protective result can coexist:
Covered
```

---

# 15. Disposition 支点

## 15.1 `recommend_rebirth`

直接支点：

```text
Outcome01
Relationship01
Personality01
```

必须处理的负面材料：

```text
Action01
Thought01
```

不得依赖：

```text
Death01 = 牺牲
```

因为这一点没有被确认。

---

## 15.2 `ordinary_transfer`

支点：

```text
Action01
Thought01
Outcome01
Relationship01
Death01
```

可以形成：

```text
行为严重
+
动机和结果复杂
+
功过相互牵制
```

当前风险：

```text
ordinary_transfer
```

可能成为新版 Case 的安全答案。

这不是当前 Case 的隐藏答案，而是后续 Corpus 必须测试的处置偏向。

---

## 15.3 `send_to_prison`

支点：

```text
Action01
Thought01
```

辅助：

```text
Personality01
```

可解释为：

- 邻里误判；
- 长期隐藏；
- 表面退让与实际行为冲突。

必须处理的反证：

```text
Outcome01
Relationship01
Death01
```

---

## 15.4 `soul_dissolution`

当前定位：

```text
Extreme test only
```

Day3 不要求本案证明魂灭合理。

后续需要判断：

- 是否进入正常 Corpus；
- 是否只在对抗性测试中出现；
- 是否需要额外极端事实门槛；
- 是否容易成为玩家无脑极端按钮。

---

# 16. Information Budget Audit

本案的核心责任轴：

```text
Action01
+
Thought01
```

本案的核心保护／结果轴：

```text
Outcome01
+
Relationship01
```

人格冲突轴：

```text
Personality01
```

终局不确定性：

```text
Death01
```

## 16.1 最大反转

最大反转来自：

```text
Action01:
亡魂杀死宅主

Outcome01:
孩子因此逃出
```

这一组合迫使玩家同时面对：

- 行为责任；
- 实际结果；
- 动机不确定；
- 结果是否减轻责任。

## 16.2 最大隐藏答案风险

最容易过度指向“保护孩子”的组合：

```text
Outcome01
+
Relationship01
+
Death01
```

其中：

- Outcome 提供孩子逃出的事实；
- Relationship 提供保护承诺；
- Death 提供钥匙和带伤死亡。

修正后的设计明确：

```text
Death01
```

不直接支撑荐生，也不确认牺牲、追击或救援经过。

## 16.3 最大安全选项风险

```text
ordinary_transfer
```

可能同时容纳：

- 杀人责任；
- 保护结果；
- 长期心念；
- 人格冲突；
- 未决终局。

后续 Corpus 必须验证它是否成为所有谨慎报告的默认选择。

---

# 17. Ablation Audit

## 17.1 Remove `Action01`

影响：

- 失去明确杀人责任锚点；
- Case 更容易偏向保护与悲情；
- `send_to_prison` 路线明显变弱；
- 仍可解释长期冲突、孩子逃出和亡魂死亡。

结论：

```text
Action01 is indispensable
```

---

## 17.2 Remove `Personality01`

影响：

- 失去“外界认知与暴力行为冲突”；
- 伪装、长期受迫和邻里误解路线变弱；
- 行为与动机仍可形成多种解释。

结论：

```text
Case remains valid
Personality has independent interpretive value
```

---

## 17.3 Remove `Thought01`

影响：

- 预谋、长期仇恨和反复恐惧的证据支点减弱；
- 行为更容易被解释为当场反击；
- `send_to_prison` 仍有 Action 支点，但严厉路线明显变弱。

结论：

```text
Thought meaningfully changes motive interpretation
```

---

## 17.4 Remove `Outcome01`

影响：

- 失去最强的有利结果；
- 荐生路线明显变弱；
- Case 更容易变成杀人＋长期心念的责任案；
- Relationship 仍保留保护可能，但缺少实际结果。

结论：

```text
Outcome01 is indispensable
```

---

## 17.5 Remove `Relationship01`

影响：

- 孩子逃出仍然成立；
- 但保护承诺和亡魂主动关注孩子的材料消失；
- 荐生路线仍可能存在，但动机支点明显下降。

结论：

```text
Relationship has independent motive/context value
```

---

## 17.6 Remove `Death01`

影响：

- 失去终局残痕；
- 不再有背伤、土地庙与钥匙；
- Case 仍然拥有至少三种主要解释；
- 保护英雄路线的暗示反而减弱。

结论：

```text
Death01 enriches aftermath
but is not required for core moral ambiguity
```

---

# 18. Ablation 总结

六次纸面删除测试均未使 Case 只剩唯一解释。

其中：

```text
Action01:
central responsibility anchor

Outcome01:
central beneficial-result counterweight
```

两者承担最大信息重量。

`Death01` 信息量高，但最容易被玩家和 AI 过度补全。

---

# 19. Case Notes

创建：

```text
cases/case_door_knife_001_notes.md
```

内容包括：

1. Design Intent；
2. Confirmed Local Facts；
3. Intentionally Unknown；
4. Fragment Semantics；
5. Interpretation Hooks；
6. Disposition Space；
7. Design Risks；
8. Runtime Visibility Boundary；
9. Day4 Questions。

## 19.1 Intentionally Unknown

Case 不保存以下完整答案：

- 宅主为何锁孩子；
- 孩子与亡魂的确切关系；
- 亡魂的主要杀人动机；
- 夜间想象的唯一含义；
- 背伤来源；
- 死亡经过；
- 钥匙使用经过；
- 邻里是否真正了解亡魂；
- 亡魂是否长期伪装；
- 正确善恶初判；
- 正确命运处置。

这些不是“之后要揭露的隐藏真相”。

它们是：

```text
Case 没有保存的完整人生答案
```

---

# 20. 新增文件

```text
AI_Judgement_Prototype/
├─ baselines/
│  └─ baseline_manifest_week8_v0_1.json
│
├─ cases/
│  ├─ case_door_knife_001.json
│  └─ case_door_knife_001_notes.md
│
└─ reports/
   └─ case_design/
      └─ case_design_audit_door_knife_v0_1.md

Docs/
└─ Week8Day3RevisedProgress.md
```

---

# 21. 修改文件

Day3 没有修改旧核心基线：

```text
cases/case_knife_001.json
judges/judge_clerk_001.json
schemas/judgement_result_v0_1.json
src/prompt_builder.py
reports/corpus_manifest_v0_1.json
reports/semantic_audit/
reports/week8_summary/
results/raw/
results/validated/
Docs/Development_Log_Week8.md
```

Day3 文件全部为新增文件或新 Case 专属文件。

---

# 22. 数据流变化

Day3 前：

```text
Old Knife Case
→ Prompt v0.3
→ DeepSeek
→ Raw
→ Validated
→ Semantic Audit
```

Day3 后新增设计层：

```text
Frozen Week8 Baseline
        |
        +--> Baseline Manifest
        |
        +--> Case.DoorKnife.001
                |
                +--> Six Local Facts
                +--> Semantic Types
                +--> Source Types
                +--> Neutral Relation Tags
                +--> Design-only Support Metadata
                |
                +--> Case Notes
                +--> Four Interpretation Audit
                +--> Information Budget Audit
                +--> Six Ablation Tests
```

新 Case 尚未接入：

- Existing CLI；
- Prompt Builder；
- Provider；
- Corpus；
- AI Validator；
- UE Runtime。

---

# 23. 本地验证结果

修正后的 Day3 文件完成以下检查：

```text
DoorKnife JSON Syntax:
PASS

Baseline Manifest JSON Syntax:
PASS

Day3 Inline Structural Check:
PASS

Baseline SHA-256 Verification:
PASS

Old Corpus Validation:
PASS

Corpus Report Count:
20

Expected Preflight:
19 pass / 1 reject

Old Unit Tests:
44 PASS

git diff --check:
PASS
```

结构检查确认：

```text
CaseID:
Case.DoorKnife.001

CaseVersion:
0.1

Fragment Count:
6

Fragment IDs:
Unique

SemanticTypes:
Six required types covered

InformationWeight:
All within 1–3

SourceTypes:
Legal

RelationTags:
Declared and legal

DispositionSupportTags:
Legal

Base Disposition Coverage:
At least two, actually all three

hidden_complete_truth:
false
```

安全与范围检查：

```text
Real AI Calls:
0

Prompt Modified:
No

Schema Modified:
No

Parser Modified:
No

Old Corpus Modified:
No

Old Results Modified:
No

Development_Log_Week8.md Modified:
No

Commit:
No

Push:
No
```

---

# 24. 遇到的问题与修正

## 24.1 `post_killing_pursuit` 预设追击

### Problem

RelationTag 名称暗示已经发生追击。

### Cause

设计者试图描述杀人后的终局后果，却把一种可能解释写进结构 ID。

### Fix

改为：

```text
post_killing_aftermath
```

### Status

```text
Resolved
```

---

## 24.2 `Death01` 荐生支点口径冲突

### Problem

JSON 没有给 `Death01` `recommend_rebirth` Support Tag，但旧 Notes / Audit 一度把它作为直接荐生支点。

### Cause

终局残痕容易被自动解释为保护、牺牲或赎罪。

### Fix

统一规则：

```text
Death01:
unresolved aftermath context

Direct recommend_rebirth support:
No
```

荐生直接支点改为：

```text
Outcome01
Relationship01
Personality01
```

### Status

```text
Resolved
```

---

## 24.3 Moral Judgement 覆盖夸大

### Problem

旧 Audit 一度让同一 Interpretation 同时选择两个 Moral Judgement，并宣称覆盖四种方向。

### Cause

把极端测试方向与正式独立解释混在一起。

### Fix

每条 Interpretation 记录唯一 Primary Moral Judgement 与唯一 Primary Disposition。

最终：

```text
Three primary moral directions:
Supported

beyond_redemption:
Extreme test only
```

### Status

```text
Resolved
```

---

## 24.4 设计元数据可能形成隐藏答案

### Problem

如果把以下字段发送给 AI：

```text
interpretation_hooks
disposition_support_tags
```

模型将直接看到设计师预先列出的解释和处置路线。

### Cause

设计审计字段与 Runtime Evidence 没有显式隔离。

### Fix

在 Notes 与 Audit 中加入 Runtime Visibility Boundary。

### Status

```text
Resolved as design boundary
Must be enforced by Day4 validator and future Prompt Builder
```

---

## 24.5 Development Log 曾显示删除

### Problem

初始工作区中：

```text
Docs/Development_Log_Week8.md
```

曾显示为删除。

### Fix

用户已恢复。

### Status

```text
Resolved
Unchanged by Day3
```

---

# 25. 当前未解决问题

以下问题不是 Day3 失败，而是 Day4–Day7 需要验证的内容：

1. 六条 Fragment 写成玩家报告后，是否仍然拥有平衡解释空间；
2. `ordinary_transfer` 是否成为无脑安全答案；
3. AI 是否会把 `Thought01` 直接等同于预谋；
4. AI 是否会把 `Personality01` 当作客观人格事实；
5. AI 是否会把 `Outcome01` 自动等同于善意动机；
6. AI 是否会把 `Death01` 自动补成牺牲或追击故事；
7. Fragment Mapping 如何区分：
   - 核心使用；
   - 弱使用；
   - 仅列举；
   - 反证提及；
   - 明确排除；
8. `beyond_redemption / soul_dissolution` 是否应进入普通 Corpus；
9. 后续对抗输入如何实现世界内拒答；
10. Case Design Metadata 如何保证不会进入 AI Prompt。

---

# 26. Day4 Handoff

Day4 应进入：

```text
Second Contrast Case
+
Formal Case Structure Validator
```

重点不应是调用 AI。

Day4 应验证：

- 新 Case 必需顶层字段；
- Case Version；
- Fragment 数量；
- FragmentID 唯一；
- SemanticType 合法；
- SourceType 合法；
- InformationWeight 范围；
- RelationTag 引用；
- DispositionSupportTag 引用；
- Moral Judgement 与 Disposition ID；
- `hidden_complete_truth == false`；
- Design Metadata 不进入 Runtime Evidence；
- Baseline Hash 保持不变；
- 旧 Corpus 与 44 项测试继续通过。

Day4 还应创建第二个对照 Case，用于避免新版系统只会处理“杀人＋保护”类型案件。

---

# 27. System Understanding Review

## 27.1 今天新增的事实数据由谁拥有？

由：

```text
Case.DoorKnife.001
```

拥有。

AI 不拥有这些事实，也不能修改这些事实。

---

## 27.2 哪些内容属于 Case Design？

- CaseID；
- Case Version；
- FragmentID；
- Fragment Text；
- SemanticType；
- SourceType；
- InformationWeight；
- RelationTags；
- AcquisitionType；
- Allowed Moral Judgements；
- Allowed Dispositions；
- Design-only Support Metadata。

---

## 27.3 哪些内容属于玩家正式选择？

Day3 尚未创建 Player Report。

未来玩家拥有：

- 善恶初判；
- 命运处置；
- 自由判词；
- 选择使用哪些 Fragment；
- 如何解释 Fragment 间关系。

---

## 27.4 哪些内容允许 AI 解释？

AI可以解释：

- 行为动机；
- 性格与行为冲突；
- 心念与行动的关系；
- 结果与责任的关系；
- 不同 Fragment 如何支持或削弱玩家论证；
- 玩家是否把推断写成事实；
- 判词是否自洽。

AI不能解释成已确认事实：

- 宅主罪行；
- 孩子身份；
- 背伤来源；
- 追击者；
- 亡魂的唯一动机；
- 隐藏人生真相。

---

## 27.5 哪些内容未来必须由程序验证？

- CaseID；
- Case Version；
- FragmentID；
- SemanticType；
- SourceType；
- InformationWeight；
- RelationTag；
- Moral Judgement ID；
- Disposition ID；
- 玩家正式选择；
- Runtime Schema；
- Design-only Metadata 不进入 Prompt；
- AI不得修改玩家处置。

---

## 27.6 Personality 在本案中的作用

`Personality01` 提供：

```text
邻里的社会认知
```

它可以被解释为：

- 真实退让；
- 长期受迫；
- 外界误解；
- 伪装；
- 极端情况下性格突破。

它不是善恶结论。

---

## 27.7 Thought 在本案中的作用

`Thought01` 确认亡魂长期想象宅主死亡。

它可以改变：

- 预谋解释；
- 仇恨解释；
- 恐惧解释；
- 无力幻想解释；
- 动机混合解释。

它不能自动证明最终行为已经长期计划。

---

## 27.8 什么测试可能证明 Case 设计失败？

如果未来 Corpus 发现：

- 所有合理报告都选择同一处置；
- 不使用 Design Metadata 就无法产生不同解释；
- 删除任意弱 Fragment 都不影响解释；
- Outcome 自动洗白杀人；
- Thought 自动证明预谋；
- Death 自动证明牺牲；
- AI与人类都只能得出“保护孩子”；

则说明 Case 设计失败或信息预算失衡。

---

## 27.9 什么测试可能证明 Prompt v0.4 失败？

如果 AI：

- 发明宅主罪行；
- 发明孩子身份；
- 将 Thought 当行为；
- 将 Outcome 当动机；
- 把他人评价当客观人格；
- 修改玩家处置；
- 使用 Design Support Tags 作为答案；
- 在 Judge Response 中泄露系统术语；

则说明 Prompt 或 Runtime Input Boundary 失败。

---

## 27.10 哪些问题属于模型能力？

- 模糊人格证词映射；
- 心念与行动区分；
- 结果与动机区分；
- 复杂多重动机理解；
- Fragment 的弱使用与反证使用识别；
- 高修辞与证据质量分离。

非法 ID、非法枚举和字段泄漏属于工程与 Validator 问题，不应交给模型自行保证。

---

## 27.11 当前是否过度设计？

没有。

Day3 只新增：

- Baseline Manifest；
- 一个新 Case；
- Case Notes；
- 一份纸面 Design Audit；
- Progress。

没有提前实现：

- Validator；
- Runtime Adapter；
- Prompt；
- Corpus；
- AI 调用；
- UE 系统。

范围仍然受控。

---

# 28. Day3 完成度

```text
[PASS] Branch Confirmed
[PASS] Old Knife Baseline Unchanged
[PASS] Development_Log_Week8.md Restored
[PASS] Baseline Manifest Created
[PASS] Key Artifact Hashes Recorded
[PASS] Baseline Hash Verification
[PASS] Regression Reports Recorded
[PASS] Case.DoorKnife.001 v0.1
[PASS] Six Fragments
[PASS] Six SemanticTypes
[PASS] Legal SourceTypes
[PASS] InformationWeight
[PASS] RelationTags
[PASS] Neutral post_killing_aftermath
[PASS] DispositionSupportTags
[PASS] prototype_fixed AcquisitionType
[PASS] Case Notes
[PASS] Confirmed Local Facts
[PASS] Intentionally Unknown
[PASS] Runtime Visibility Boundary
[PASS] Four Interpretation Paths
[PASS] Three Primary Moral Directions
[PASS] Three Base Disposition Routes
[PASS] Personality Opposing Interpretations
[PASS] Thought Is Not Action
[PASS] Information Budget Audit
[PASS] Six Ablation Audits
[PASS] DoorKnife JSON Syntax
[PASS] Baseline Manifest JSON Syntax
[PASS] Inline Structural Check
[PASS] Old Corpus Validation
[PASS] 44 Unit Tests
[PASS] git diff --check
[PASS] No Real AI Call
[PASS] No Prompt Modification
[PASS] No Schema Modification
[PASS] No Parser Modification
[PASS] No Commit
[PASS] No Push
```

---

# 29. 最终结论

Day3 已正式完成。

```text
Baseline Freeze:
PASS

DoorKnife Case Design:
PASS

Multiple Interpretations:
PASS

Three Primary Moral Directions:
PASS

Three Base Dispositions:
PASS

Hidden Complete Truth:
Not Encoded

Design Metadata Boundary:
Defined

Information Budget:
Audited

Ablation:
PASS

Old Regression Baseline:
Preserved

Technical Scope:
Controlled
```

`Case.DoorKnife.001` 值得进入 Day4。

本案比旧 Knife Case 提供了更明确的局部事实与更真实的善恶张力，同时没有将“保护孩子”写成官方答案。

当前最大的后续风险不是案件空洞，而是：

```text
ordinary_transfer 安全偏向
+
AI 过度补全保护故事
+
Design Metadata 泄漏
+
Thought / Outcome / Personality 的语义误用
```

这些问题应由 Day4 的结构约束与第二个对照 Case，以及 Day5–Day6 的 Corpus / Prompt 实验继续验证。
