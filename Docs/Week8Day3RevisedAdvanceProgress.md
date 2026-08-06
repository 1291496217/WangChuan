# 《忘川河畔：见习判员》Week8 Day3 Revised Advance Progress

**Project:** WangChuan / 《忘川河畔》
**Prototype:** `AI_Judgement_Prototype`
**Branch:** `feature/ai-first-prototype`
**Stage:** Week8 Day3 Revised Advance
**Plan Mapping:** Revised Week8 Plan — Original Day4
**Date:** 2026-08-05
**Theme:** Second Non-killing Case & Disposition Coverage Audit
**Status:** Completed after Review Correction
**Real AI Calls:** 0
**Commit / Push:** Not Yet

---

# 1. 今日目标

Day3 Revised Advance 对应新版 Week8 计划中的原 Day4。

本阶段没有继续修改 Prompt，也没有发起新的 AI 实验。

目标是验证：

```text
同一套开放善恶初判 Case 结构
能否支持第二种明显不同的道德冲突
```

具体任务：

```text
第二宗非杀人案件
+
Medicine Case Notes
+
三条可辩护解释
+
两条失败模式
+
双 Case Disposition Coverage Matrix
+
只读 Case Validator
+
Validator Unit Tests
```

今天需要回答：

> 新版系统是否不仅能处理“杀人是否可宽恕”，也能处理善意、规则、资源稀缺、信任与第三人代价之间的冲突？

---

# 2. 起始状态

Day3 Revised 已完成：

```text
Baseline.Week8.AIJudgement.001 v0.1
Case.DoorKnife.001 v0.1
DoorKnife Case Notes
DoorKnife Design Audit
DoorKnife Information Budget Audit
DoorKnife Ablation Audit
```

旧 Week8 AI 实验基线继续冻结：

```text
Case.Knife.001 v0.2
Judge.Clerk.001 v0.1
Prompt v0.3
Schema v0.1
Corpus.Knife.Week8.001 v0.1
```

Day3 Revised Advance 不修改：

- 旧 Knife Case；
- DoorKnife Case；
- Baseline Manifest；
- Judge；
- Prompt；
- Schema；
- Parser；
- 旧 Corpus；
- Raw / Validated / Audited Results；
- `Development_Log_Week8.md`。

---

# 3. 为什么第二 Case 必须是非杀人型

`Case.DoorKnife.001` 的责任锚点是：

```text
亡魂确实杀死宅主
```

如果第二 Case 仍然围绕：

- 杀人；
- 自卫；
- 复仇；
- 保护孩子；
- 牺牲；
- 隐藏凶手；

就无法证明新结构能够处理其他类型的道德判断。

因此第二 Case 改为：

```text
资源稀缺
+
未经授权取用
+
真实善意结果
+
真实第三人代价
+
先前诚信与本次行为冲突
+
明知风险仍作选择
```

它不依赖：

- 死亡；
- 终局反转；
- 秘密血缘；
- 新证人；
- 最后出现的证明文件；
- “其实全是误会”。

---

# 4. 新 Case

创建：

```text
AI_Judgement_Prototype/
└─ cases/
   ├─ case_medicine_001.json
   └─ case_medicine_001_notes.md
```

Case Identity：

```text
CaseID:
Case.Medicine.001

CaseVersion:
0.1

Title:
未归还的药

DesignMode:
local_facts_open_moral_judgement

hidden_complete_truth:
false
```

本案核心问题：

> 善意目的和真实善果，能否抵消未经授权取走最后一包药、对稀缺资源的擅自分配，以及另一名病人的实际代价？

本案不保存：

- 哪一个病人更值得救；
- 哪一种药物分配方式正确；
- 亡魂的唯一动机；
- 唯一善恶判断；
- 唯一命运处置。

---

# 5. 正式善恶判断与处置

## 5.1 Moral Judgement IDs

```text
more_good_than_evil
mixed_merit_and_fault
more_evil_than_good
beyond_redemption
```

## 5.2 Disposition IDs

基础处置：

```text
recommend_rebirth
ordinary_transfer
send_to_prison
```

极端测试处置：

```text
soul_dissolution
```

Medicine Case 为三个基础处置提供材料路线。

`soul_dissolution` 没有正常合理支点，只保留为边界或不成比例报告测试。

---

# 6. 七个中立 RelationTag

```text
unregistered_medicine_removal
child_treatment_outcome
scarce_medicine_competition
other_patient_unserved
public_integrity_conflict
foreseen_resource_tradeoff
non_family_care_commitment
```

这些 Tag 只描述材料之间的结构关系。

没有使用：

```text
heroic_theft
justified_rescue
selfish_stealing
criminal_abuse
deserved_punishment
```

因此 RelationTag 没有预先写入善恶结论。

---

# 7. 六条 Medicine Fragment

## 7.1 `Medicine.Action01`

```text
夜里，亡魂从药铺取走店中最后一包定喘散，没有登记，也没有留下药钱。
```

结构：

```text
SemanticType:
action

SourceType:
objective_trace

InformationWeight:
3
```

确认：

- 亡魂取走最后一包药；
- 没有登记；
- 没有留下药钱。

不确认：

- 是否准备之后补钱；
- 是否打算永久占有；
- 是否完全出于善意；
- 是否认为自己拥有分配权；
- 是否存在紧急授权。

作用：

```text
明确行为责任锚点
```

---

## 7.2 `Medicine.Outcome01`

```text
药被送到河边棚屋后，一个喘息急促的孩子服下；当夜，孩子的呼吸缓了下来。
```

结构：

```text
SemanticType:
outcome

SourceType:
objective_trace

InformationWeight:
3
```

确认：

- 药送到孩子处；
- 孩子服药；
- 当夜呼吸缓解。

不确认：

- 药是唯一原因；
- 孩子若没有药一定恶化；
- 亡魂取药时的唯一动机；
- 善果自动证明取药正当。

关键边界：

```text
真实有利结果
≠
行为自动正当
```

---

## 7.3 `Medicine.Outcome02`

```text
第二日清晨，药铺因没有定喘散，未能给另一名来求药的病人配药；那人回村途中再次发作，被同行者抬回家。
```

结构：

```text
SemanticType:
outcome

SourceType:
objective_trace

InformationWeight:
3
```

确认：

- 药铺没有定喘散；
- 另一名病人没有得到该药；
- 该病人再次发作；
- 被同行者抬回家。

不确认：

- 得到该药就一定不会发作；
- 亡魂应为全部病情负责；
- 亡魂希望该病人受害；
- 两名病人谁更严重；
- 两人的生命价值可以被系统排序。

关键边界：

```text
真实资源挤占和第三人代价
≠
完整医学因果
≠
故意伤害
```

---

## 7.4 `Medicine.Personality01`

```text
掌柜和两个伙计都说，他在药铺做了三年，账目从未少过一文，也从未私取药材。
```

结构：

```text
SemanticType:
personality

SourceType:
others_testimony

InformationWeight:
2
```

确认：

- 掌柜与伙计这样评价亡魂过去的工作记录。

不确认：

- 亡魂客观上一直诚实；
- 本次必然只是例外；
- 过去诚信能够自动免除本次责任；
- 过去诚信只是为了利用信任；
- 证词覆盖亡魂全部人格。

它可以支持相反解释：

```text
一次紧急越界
```

也可以支持：

```text
受信任身份使越界责任更重
```

---

## 7.5 `Medicine.Thought01`

```text
取药前，他知道那是店里最后一包，也想过天亮后可能还有人来求药。
```

结构：

```text
SemanticType:
thought

SourceType:
soul_self_knowledge

InformationWeight:
3
```

确认：

- 亡魂知道资源稀缺；
- 亡魂意识到可能有其他求药者。

不确认：

- 他知道另一名病人一定会来；
- 他希望另一人受害；
- 他认为孩子的生命更重要；
- 他计算过两人的生存概率。

关键边界：

```text
明知可能代价
≠
希望代价发生
```

Thought 增加选择责任，但不自动证明恶意。

---

## 7.6 `Medicine.Relationship01`

```text
孩子的母亲说：“我们与他并非亲故。他去年见过孩子发病，答应若再遇到有药的时候，会替我送来。”
```

结构：

```text
SemanticType:
relationship

SourceType:
others_testimony

InformationWeight:
2
```

确认：

- 母亲称双方并非亲属；
- 亡魂见过孩子发病；
- 亡魂作出过送药承诺。

不确认：

- 亡魂没有其他私人关系；
- 承诺完全无私；
- 母亲知道全部动机；
- 承诺授予取药权；
- 亡魂可以不计后果兑现承诺。

---

# 8. Medicine Concept Audit

Concept Audit 结论：

```text
PASS
```

已经确认：

- 核心行为明确；
- 没有杀人；
- 不依赖死亡反转；
- 存在真实善果；
- 存在真实第三人代价；
- 存在稀缺资源冲突；
- Personality 改变信任解释；
- Thought 增加责任但不证明恶意；
- Relationship 支持非亲属照护；
- 三种基础处置均有材料支点；
- 没有 Fragment 单独给出唯一答案。

Medicine 与 DoorKnife 的差异：

| Dimension | DoorKnife | Medicine |
|---|---|---|
| Core Action | 杀死宅主 | 未登记取走最后一包药 |
| Main Responsibility | 暴力与死亡责任 | 资源、规则和信任责任 |
| Beneficial Result | 孩子逃出 | 孩子呼吸缓解 |
| Third-party Cost | 终局后果不完全 | 另一病人未得药并发作 |
| Thought Role | 长期死亡想象 | 明知药物稀缺 |
| Main Risk | 过度补全保护故事 | 功利开脱或规则绝对化 |

结论：

> 两案使用相同结构语言，但形成了明显不同的道德张力。

---

# 9. Intentionally Unknown

Medicine Case 不保存以下完整答案：

- 定喘散是否是唯一有效治疗；
- 孩子好转是否完全由药导致；
- 两名病人谁更严重；
- 另一病人若服药是否一定不会发作；
- 亡魂是否准备补钱；
- 亡魂是否准备归还同类药物；
- 药铺是否存在紧急取药规则；
- 掌柜当时是否会同意；
- 亡魂是否有其他个人利益；
- 亡魂是否希望另一病人受害；
- 母亲是否知道全部动机；
- 过去诚信是否覆盖全部人格；
- 正确善恶判断；
- 正确命运处置。

这些不是等待揭露的隐藏真相。

它们是：

```text
Case 没有保存的完整答案
```

---

# 10. 三条可辩护解释

## 10.1 A — 可宽恕的越界救助

Primary Moral Judgement：

```text
more_good_than_evil
```

Primary Disposition：

```text
recommend_rebirth
```

主要支点：

```text
Medicine.Outcome01
Medicine.Personality01
Medicine.Relationship01
```

必须处理：

```text
Medicine.Action01
Medicine.Thought01
Medicine.Outcome02
```

最低合理条件：

- 承认未经登记取药；
- 承认没有留下药钱；
- 承认这是最后一包药；
- 承认亡魂知道可能有其他求药者；
- 承认另一病人没有得到药并再次发作；
- 解释为什么非亲属救助、过去诚信和真实善果构成减责；
- 不能把善果写成自动正当性。

---

## 10.2 B — 明知稀缺仍擅自分配

Primary Moral Judgement：

```text
more_evil_than_good
```

Primary Disposition：

```text
send_to_prison
```

主要支点：

```text
Medicine.Action01
Medicine.Thought01
Medicine.Outcome02
Medicine.Personality01
```

必须处理：

```text
Medicine.Outcome01
Medicine.Relationship01
```

最低合理条件：

- 指出亡魂明知药物稀缺；
- 指出未经授权改变资源归属；
- 指出受信任身份带来的责任；
- 指出另一病人的实际代价；
- 承认孩子确实得到帮助；
- 不得把另一病人的发作写成故意伤害；
- 不得把三年诚信写成长期伪装事实。

---

## 10.3 C — 善意、责任与资源代价并存

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
Medicine.Action01
Medicine.Outcome01
Medicine.Outcome02
Medicine.Thought01
Medicine.Relationship01
```

辅助：

```text
Medicine.Personality01
```

最低合理条件：

- 明确认可孩子得到帮助；
- 明确认可未经授权；
- 明确认可亡魂知情；
- 明确认可另一病人的实际代价；
- 解释为什么善意不等于无责；
- 解释为什么责任不等于恶意；
- 说明功过为什么同时具有实质意义。

平籍不能只依赖：

```text
我不知道
证据不足
双方都有道理
```

---

# 11. 两条失败模式

## 11.1 D — 只看结果的功利开脱

错误链条：

```text
孩子好转
→
取药必然正确
→
无需承担其他责任
```

问题：

- 忽略未登记；
- 忽略未付款；
- 忽略最后一包药；
- 忽略亡魂知情；
- 忽略另一病人代价；
- 把有利结果自动等同于行为正当。

它不是正式合理解释。

后续应进入负面 Corpus。

---

## 11.2 E — 只看规则的绝对归罪

错误链条：

```text
未经登记取药
→
亡魂必然是恶人
→
善果与救助动机全部无意义
```

问题：

- 忽略孩子真实好转；
- 忽略非亲属救助；
- 忽略过去诚信；
- 把一次规则越界升级为完整人格判断；
- 可能无依据要求魂灭。

它不是正式合理解释。

后续应进入负面或极端 Corpus。

---

# 12. Medicine Information Budget

| Fragment | Weight | Main Function |
|---|---:|---|
| Action01 | 3 | 明确行为与规则责任 |
| Outcome01 | 3 | 真实善果 |
| Outcome02 | 3 | 第三人实际代价 |
| Personality01 | 2 | 信任与例外解释 |
| Thought01 | 3 | 知情选择责任 |
| Relationship01 | 2 | 非亲属照护承诺 |

最大道德张力来自：

```text
Action01
+
Outcome01
+
Outcome02
```

即：

```text
明确越界
+
真实善果
+
真实第三人代价
```

最容易被过度补全：

```text
Outcome02:
被扩写成完整医学因果或故意伤害

Relationship01:
被补写成隐藏血缘或特殊授权
```

最容易成为安全处置：

```text
ordinary_transfer
```

后续必须测试其是否退化为无内容的折中。

---

# 13. Medicine Ablation Audit

## 13.1 Remove `Medicine.Action01`

影响：

- 失去明确规则越界；
- 押狱路线明显变弱；
- 平籍失去行为责任锚点；
- 荐生更容易变成单向善果故事。

结论：

```text
Action01 is indispensable
```

---

## 13.2 Remove `Medicine.Outcome01`

影响：

- 失去真实善果；
- 荐生路线明显变弱；
- Case 更偏向规则与第三人责任；
- 平籍仍可成立，但功的一侧变薄。

结论：

```text
Outcome01 is indispensable for rebirth route
```

---

## 13.3 Remove `Medicine.Outcome02`

影响：

- 失去最强第三人代价；
- 押狱与平籍路线明显变弱；
- Case 更容易退化为“善意偷药是否可原谅”。

结论：

```text
Outcome02 has independent value
```

---

## 13.4 Remove `Medicine.Personality01`

影响：

- 失去过去诚信；
- 失去受信任身份的责任维度；
- 三种处置仍可成立；
- 对一次例外与信任违背的解释变薄。

结论：

```text
Case remains valid
Personality changes interpretation
```

---

## 13.5 Remove `Medicine.Thought01`

影响：

- 亡魂知情程度下降；
- 押狱路线明显变弱；
- 无法确认亡魂预见到资源竞争；
- 行为更容易被解释为紧急冲动。

结论：

```text
Thought independently increases responsibility
```

---

## 13.6 Remove `Medicine.Relationship01`

影响：

- 失去非亲属照护承诺；
- 荐生路线明显变弱；
- 善意仍可从送药行为推断，但缺少长期承诺支点；
- 平籍与押狱仍可成立。

结论：

```text
Relationship has independent benevolent-context value
```

---

# 14. 双 Case Disposition Coverage Matrix

创建：

```text
reports/case_design/
├─ case_disposition_coverage_v0_1.json
└─ case_disposition_coverage_v0_1.md
```

Coverage ID：

```text
CaseDispositionCoverage.Week8B.001
```

Coverage Version：

```text
0.1
```

顶层边界：

```text
design_metadata_only:
true

runtime_visibility:
do_not_expose_to_ai_or_player
```

Coverage Matrix 不是：

- 隐藏答案；
- AI Prompt；
- 玩家提示；
- 自动处置规则；
- Reward Table；
- TrueMorality；
- CorrectDisposition。

## 14.1 DoorKnife Coverage

### `recommend_rebirth`

支点：

```text
DoorKnife.Outcome01
DoorKnife.Relationship01
DoorKnife.Personality01
```

反证：

```text
DoorKnife.Action01
DoorKnife.Thought01
```

Risk：

```text
medium
```

### `ordinary_transfer`

支点：

```text
DoorKnife.Action01
DoorKnife.Thought01
DoorKnife.Outcome01
DoorKnife.Relationship01
```

Risk：

```text
high
```

原因：

```text
容易成为安全答案
```

### `send_to_prison`

支点：

```text
DoorKnife.Action01
DoorKnife.Thought01
```

Risk：

```text
medium
```

### `soul_dissolution`

正常材料支点：

```text
None
```

Corpus Recommendation：

```text
boundary_only
```

---

## 14.2 Medicine Coverage

### `recommend_rebirth`

支点：

```text
Medicine.Outcome01
Medicine.Personality01
Medicine.Relationship01
```

反证：

```text
Medicine.Action01
Medicine.Thought01
Medicine.Outcome02
```

Risk：

```text
medium
```

### `ordinary_transfer`

支点：

```text
Medicine.Action01
Medicine.Outcome01
Medicine.Outcome02
Medicine.Thought01
Medicine.Relationship01
```

Risk：

```text
high
```

原因：

```text
最可能成为空泛折中
```

### `send_to_prison`

支点：

```text
Medicine.Action01
Medicine.Thought01
Medicine.Outcome02
```

反证：

```text
Medicine.Outcome01
Medicine.Relationship01
Medicine.Personality01
```

Risk：

```text
medium
```

### `soul_dissolution`

正常材料支点：

```text
None
```

Corpus Recommendation：

```text
boundary_only
```

---

# 15. Coverage Matrix 主要发现

1. 两个 Case 的三种基础处置都有材料路线；
2. 魂灭不需要平均支持；
3. DoorKnife 的押狱路线更强，因为存在明确杀人；
4. Medicine 的荐生路线更直接，因为存在非亲属救助与实际好转；
5. Medicine 的押狱路线依赖：
   - 知情稀缺；
   - 未经授权；
   - 信任责任；
   - 第三人代价；
6. 两个 Case 的 `ordinary_transfer` 都是最高安全偏向风险；
7. 平籍必须说明功与过为何同时成立，不能只表达不确定；
8. Coverage Matrix 只用于设计与 Corpus 规划，不能进入 AI Runtime。

Coverage Matrix 独立检查：

```text
Cases:
2

Disposition Rows:
8

Result:
PASS
```

---

# 16. Case Validator

创建：

```text
src/validate_case_design.py
```

## 16.1 技术边界

Validator：

- Python Standard Library Only；
- 不读取 `.env`；
- 不导入 AI Client；
- 不发起网络请求；
- 不修改 Case；
- 不生成 Prompt；
- 不调用 Provider；
- 支持默认两个新 Case；
- 支持重复 `--case`；
- 使用稳定错误排序；
- 可作为 Module 导入。

Exit Codes：

```text
0:
All Cases Valid

1:
Design Validation Failed

2:
File / JSON / CLI Error
```

## 16.2 默认范围

```text
cases/case_door_knife_001.json
cases/case_medicine_001.json
```

默认不验证：

```text
cases/case_knife_001.json
```

旧 Knife 属于旧结构基线。

## 16.3 Validator 检查内容

- 顶层必需字段；
- `design_mode`；
- `hidden_complete_truth == false`；
- Moral Judgement IDs；
- Disposition IDs；
- Availability；
- RelationTag Definition；
- Fragment Count `5–6`；
- 至少一个 Action；
- 至少一个 Outcome；
- Personality / Thought / Relationship 至少两类；
- FragmentID；
- SemanticType；
- SourceType；
- InformationWeight；
- AcquisitionType；
- Text；
- RelationTag 引用；
- SupportTag 引用；
- Interpretation Hook；
- 至少两个基础处置获得 Design Support；
- CaseID 唯一；
- CaseID + CaseVersion 组合唯一；
- FragmentID 跨 Case 唯一；
- 递归禁止隐藏答案字段。

禁止字段包括：

```text
TrueMorality
CorrectLifeStory
CorrectDisposition
HiddenTruth
CanonicalInterpretation
OfficialMoralJudgement
OfficialDisposition
```

检查忽略：

- 大小写；
- `_`；
- `-`。

## 16.4 Validator 明确不能证明

Validator 不能证明：

- Case 有趣；
- Case 公平；
- 某种解释正确；
- 某种处置合理；
- AI 能理解；
- 玩家会喜欢；
- Fragment 文学质量；
- 处置路线同等强；
- Corpus 能覆盖真实玩家行为。

成功输出明确：

```text
These are design metadata only.
Validation does not make them runtime evidence.
```

---

# 17. 审核时发现并修正的问题

Codex 原始 Validator 已通过 37 项新测试，但人工审核发现：

> 多文件验证时只拒绝完全相同的 `CaseID + CaseVersion`，没有拒绝“相同 CaseID、不同 Version”。

任务要求同时满足：

```text
CaseID unique
+
CaseID / CaseVersion pair unique
```

原始实现：

```text
Case.DoorKnife.001 v0.1
Case.DoorKnife.001 v0.2
```

会被允许。

这会给后续同时加载多个 Case 时造成 ID 冲突，因为 Runtime 主要以 CaseID 识别案件。

## 17.1 修正

Validator 新增：

```text
case_ids
```

独立索引。

现在分别拒绝：

```text
Duplicate CaseID
```

和：

```text
Duplicate CaseID / CaseVersion
```

## 17.2 新回归测试

新增：

```text
test_duplicate_case_id_with_different_version_is_rejected
```

测试：

```text
Case.DoorKnife.001 v0.1
+
Case.DoorKnife.001 v0.2
```

必须返回设计验证失败。

## 17.3 修正后结果

```text
New Validator Tests:
38 / 38 PASS

Full Unit Test Suite:
82 / 82 PASS
```

这是本次审核发现的唯一阻塞性实现缺口。

Medicine Case、Coverage Matrix、Notes 与 Audit 不需要修改。

---

# 18. Unit Tests

原有测试：

```text
44
```

Codex 最初新增：

```text
37
```

审核修正新增：

```text
1
```

最终：

```text
82 Tests
PASS
```

新 Validator Tests 覆盖：

- 两个合法 Case；
- 相同 CaseID / Version；
- 相同 CaseID、不同 Version；
- FragmentID 单 Case 重复；
- FragmentID 跨 Case 重复；
- RelationTag；
- SupportTag；
- SemanticType；
- SourceType；
- Weight；
- AcquisitionType；
- Action / Outcome；
- 类型多样性；
- Fragment Count；
- Hidden Truth；
- DesignMode；
- 基础处置覆盖；
- 双 Outcome；
- 空 Hook；
- 空 Text；
- JSON Error；
- CLI Exit Code；
- 默认路径；
- Metadata Boundary；
- 隐藏答案字段。

---

# 19. 本地验证结果

## 19.1 Codex Result Package

```text
Medicine JSON:
PASS

Coverage JSON:
PASS

Case Validator:
PASS

Cases:
2

Fragments:
12

Coverage Matrix:
PASS

Coverage Cases:
2

Disposition Rows:
8

Old Corpus:
PASS

Reports:
20

Expected:
19 pass / 1 reject

Original Unit Tests:
81 PASS

Baseline Hash:
PASS

Immutable Artifacts:
7

Secret / Whitespace Scan:
PASS

.env Ignore:
PASS

git diff --check:
PASS
```

## 19.2 审核修正后独立复测

```text
Python Compile:
PASS

Corrected Validator:
PASS

Corrected New Tests:
38 / 38 PASS

Reconstructed Full Test Suite:
82 / 82 PASS

Independent Medicine / Coverage JSON Check:
PASS
```

没有发起 API 调用。

---

# 20. 新增文件

```text
AI_Judgement_Prototype/
├─ cases/
│  ├─ case_medicine_001.json
│  └─ case_medicine_001_notes.md
│
├─ reports/
│  └─ case_design/
│     ├─ case_design_audit_medicine_v0_1.md
│     ├─ case_disposition_coverage_v0_1.json
│     └─ case_disposition_coverage_v0_1.md
│
├─ src/
│  └─ validate_case_design.py
│
└─ tests/
   └─ test_validate_case_design.py

Docs/
└─ Week8Day3RevisedAdvanceProgress.md
```

---

# 21. 修改文件

本阶段没有修改旧项目文件。

审核修正只替换本阶段新增的：

```text
src/validate_case_design.py
tests/test_validate_case_design.py
Docs/Week8Day3RevisedAdvanceProgress.md
```

保持不变：

```text
Case.Knife.001
Case.DoorKnife.001
Baseline Manifest
Judge
Prompt
Schema
Parser
Old Corpus
Old Results
Development_Log_Week8.md
```

---

# 22. 数据流变化

Day3 Revised 后：

```text
DoorKnife Case
→ Paper Design Audit
```

Day3 Revised Advance 后：

```text
DoorKnife Case
+
Medicine Case
        |
        v
Read-only Case Validator
        |
        +--> Structural Errors
        |    → Local Reject
        |
        v
Case Design Valid
        |
        +--> Information Budget Audit
        +--> Ablation Audit
        +--> Disposition Coverage Matrix
```

仍然没有进入：

- Prompt Builder；
- AI Client；
- Provider；
- Player Report Contract；
- Moral Corpus v0.2；
- Runtime Evidence；
- UE；
- Reward。

---

# 23. 当前版本

```text
Baseline Manifest:
v0.1

DoorKnife Case:
v0.1

Medicine Case:
v0.1

Case Disposition Coverage:
v0.1

Case Validator:
Initial Structural Version
+
CaseID Uniqueness Review Fix

Prompt:
v0.3 unchanged

Schema:
v0.1 unchanged

Judge:
v0.1 unchanged
```

---

# 24. 遇到的问题、原因与解决方式

## 24.1 Medicine 使用两个 Outcome

### Problem

Validator 不能要求 SemanticType 全部唯一。

### Cause

Medicine 同时需要：

- 对孩子的有利结果；
- 对另一病人的代价。

### Solution

允许同一 Case 出现两个 `outcome`。

### Status

```text
Resolved
```

---

## 24.2 Coverage Matrix 可能被当成正确答案

### Problem

Support Fragment 和 Minimum Conditions 很像判决标准。

### Cause

设计审计需要记录可辩护路线。

### Solution

顶层明确：

```text
design_metadata_only = true
runtime_visibility = do_not_expose_to_ai_or_player
```

### Status

```text
Resolved as architecture boundary
```

---

## 24.3 平籍可能成为安全选项

### Problem

两个 Case 都容易选择：

```text
ordinary_transfer
```

### Cause

它可以容纳不确定性与功过冲突。

### Solution

Coverage Matrix 要求：

- 不能只说“不确定”；
- 必须明确功和过；
- 必须解释两者为何同时重要；
- Day5 Corpus 加入空泛折中测试。

### Status

```text
Unresolved gameplay risk
```

---

## 24.4 Outcome02 医学因果过度补全

### Problem

AI或玩家可能写成：

```text
亡魂差点害死另一人
```

### Cause

“未得到药 + 再次发作”容易被自动补成完整因果。

### Solution

Notes 与 Audit 明确：

```text
第三人代价成立
完整医学因果未知
故意伤害不成立
```

### Status

```text
Design boundary defined
Needs Corpus / Prompt testing
```

---

## 24.5 相同 CaseID 不同 Version 未被拒绝

### Problem

原 Validator 只检查：

```text
CaseID + CaseVersion
```

没有单独检查 CaseID。

### Cause

实现只建立了组合 Key。

### Solution

新增：

```text
case_ids
case_keys
```

两个独立索引，并增加回归测试。

### Status

```text
Resolved during review
```

---

# 25. 当前未解决问题

1. Medicine 是否会退化成功利主义选择题；
2. AI 是否只看 Outcome01 自动荐生；
3. AI 是否只看 Action01 自动押狱；
4. AI 是否把 Outcome02 写成故意伤害；
5. AI 是否比较两名病人的生命价值；
6. Personality 是否会被误当作无罪证明；
7. Relationship 是否会被补写成隐藏血缘；
8. `ordinary_transfer` 是否成为无脑安全答案；
9. Design Metadata 是否会在 Day5 / Day6 被错误送入 Prompt；
10. `soul_dissolution` 是否只应进入边界 Corpus；
11. 新 Report Contract 如何表达：
    - Moral Judgement；
    - Disposition；
    - 自由判词；
    - Fragment 使用；
    - 未使用与反证使用；
12. 新 Corpus 如何同时覆盖两个 Case。

---

# 26. Day5 Handoff

下一阶段应进入：

```text
New Report Contract
+
Moral Corpus v0.2
+
Local Validation
```

Day5 必须继承：

- CaseID 与 Version；
- 玩家正式 Moral Judgement；
- 玩家正式 Disposition；
- FragmentID；
- Fragment Text；
- SemanticType；
- SourceType；
- Design Metadata 不进入 Prompt；
- 平籍需要实质理由；
- Thought 不等于行为；
- Outcome 不等于动机；
- 他人证词不等于客观人格；
- 医学结果不等于完整因果；
- Coverage Matrix 不是真相；
- 玩家正文不能改正式选择。

Day5 Corpus 应至少包含：

- DoorKnife 三种合理路线；
- Medicine 三种合理路线；
- 空泛平籍；
- 功利开脱；
- 规则绝对归罪；
- 隐藏血缘补写；
- 医学因果补写；
- Prompt Injection；
- 修改正式善恶判断；
- 修改正式处置；
- 使用设计元数据的攻击；
- 极短或无关报告。

Day5 仍不应直接调用真实 AI。

---

# 27. System Understanding Review

## 27.1 Medicine 中哪些是确认事实？

- 取走最后一包药；
- 未登记；
- 未付款；
- 孩子服药后当夜呼吸缓解；
- 另一病人未得药并再次发作；
- 掌柜和伙计的诚信评价；
- 亡魂知道药品稀缺；
- 亡魂想到可能有其他求药者；
- 母亲称双方非亲属；
- 亡魂作出过送药承诺。

---

## 27.2 哪些医学因果仍未知？

- 药是否是孩子好转的唯一原因；
- 药是否是唯一有效治疗；
- 另一病人得到药后是否一定不会发作；
- 两名病人的病情严重程度；
- 药铺是否有替代治疗。

---

## 27.3 Personality 为什么不能证明无罪？

因为它是：

```text
他人对过去工作表现的评价
```

不是：

```text
系统确认的完整人格
```

过去诚信可以减轻惯犯或长期占有解释，也可以加重受信任身份下的责任。

---

## 27.4 Thought 为什么增加责任但不证明恶意？

它确认亡魂预见到稀缺资源可能影响其他人。

但没有确认：

- 希望其他人受害；
- 知道具体病人会来；
- 已经比较生命价值；
- 以伤害为目的。

---

## 27.5 Relationship 为什么支持善意却不授予取药权？

它确认：

- 非亲属承诺；
- 先前照护意愿。

但承诺不能改变：

- 药物所有权；
- 药铺规则；
- 资源分配权；
- 对其他求药者的责任。

---

## 27.6 平籍为什么不能只是“不确定”？

因为平籍的合理性必须来自：

```text
真实善果
+
真实规则责任
+
知情选择
+
真实第三人代价
+
善意动机
```

而不是因为玩家拒绝判断。

---

## 27.7 Coverage Matrix 为什么不是隐藏答案？

因为它只记录：

- 可辩护路线；
- 反证；
- 最低论证条件；
- Corpus 风险。

它不声明：

- 正确善恶；
- 正确处置；
- 唯一人生；
- 自动分数。

---

## 27.8 Validator 能证明什么？

- JSON 结构合法；
- 正式 ID 完整；
- Fragment 结构合法；
- 引用合法；
- 无重复 ID；
- 无明显隐藏答案字段；
- Case 满足最低结构多样性；
- Design Metadata 边界被声明。

---

## 27.9 Validator 不能证明什么？

- Case 是否有趣；
- Case 是否平衡；
- AI是否理解；
- 某条 Interpretation 是否正确；
- 某个 Disposition 是否公平；
- 玩家是否会形成多样判断；
- 文学质量；
- 最终游戏体验。

---

## 27.10 哪些 Design Metadata 不能进入 Prompt？

```text
interpretation_hooks
disposition_support_tags
information_weight
relation_tags
```

---

## 27.11 DoorKnife 与 Medicine 有何不同？

DoorKnife：

```text
杀人责任
+
长期心念
+
保护结果
```

Medicine：

```text
未经授权
+
稀缺资源
+
真实善果
+
第三人代价
+
知情选择
+
信任责任
```

---

## 27.12 什么测试会证明 Medicine Case 失败？

- 所有合理报告都选择同一处置；
- 平籍只需要说“不确定”；
- 荐生只看孩子好转；
- 押狱只看未付款；
- AI必须使用 Design Metadata 才能形成多解；
- Outcome02 总被理解为故意伤害；
- 两名病人的生命价值被自动排序。

---

## 27.13 什么测试会证明 Validator 失败？

- 相同 CaseID 不同 Version 未被拒绝；
- 跨 Case FragmentID 重复未被拒绝；
- 未声明 RelationTag 通过；
- 隐藏答案字段通过；
- Design Metadata 被误当作 Runtime Evidence；
- Validator 宣称某处置正确；
- Validator 修改 Case 文件。

---

## 27.14 当前实现是否过度设计？

没有。

今天只新增：

- 一个 Case；
- Notes；
- Design Audit；
- Coverage Matrix；
- 只读 Validator；
- Unit Tests；
- Progress。

没有进入：

- Prompt；
- Schema；
- Parser；
- Corpus v0.2；
- AI；
- UE；
- Reward。

---

# 28. Git Status

Codex Result Package 记录的工作区：

```text
?? AI_Judgement_Prototype/cases/case_medicine_001.json
?? AI_Judgement_Prototype/cases/case_medicine_001_notes.md
?? AI_Judgement_Prototype/reports/case_design/case_design_audit_medicine_v0_1.md
?? AI_Judgement_Prototype/reports/case_design/case_disposition_coverage_v0_1.json
?? AI_Judgement_Prototype/reports/case_design/case_disposition_coverage_v0_1.md
?? AI_Judgement_Prototype/src/validate_case_design.py
?? AI_Judgement_Prototype/tests/test_validate_case_design.py
?? Docs/Week8Day3RevisedAdvanceProgress.md
```

应用审核修正后，文件路径不变。

---

# 29. 完成度

```text
[PASS] Branch Confirmed
[PASS] Old Baseline Unchanged
[PASS] Baseline Hash
[PASS] DoorKnife Unchanged
[PASS] Medicine Concept Audit
[PASS] Case.Medicine.001 v0.1
[PASS] Six Medicine Fragments
[PASS] Non-killing Moral Conflict
[PASS] Real Beneficial Outcome
[PASS] Real Third-party Cost
[PASS] Personality Interpretation Conflict
[PASS] Thought Adds Responsibility
[PASS] Relationship Adds Non-family Care
[PASS] Intentionally Unknown
[PASS] Three Defensible Interpretations
[PASS] Two Failure Patterns
[PASS] Three Base Disposition Routes
[PASS] Soul Dissolution Boundary
[PASS] Information Budget Audit
[PASS] Six Ablation Audits
[PASS] Two-case Coverage JSON
[PASS] Two-case Coverage Markdown
[PASS] Coverage Design-only Boundary
[PASS] Read-only Case Validator
[PASS] CaseID Unique
[PASS] CaseID / Version Unique
[PASS] FragmentID Unique
[PASS] Hidden-answer Field Detection
[PASS] 38 New Validator Tests
[PASS] 44 Old Tests
[PASS] 82 Total Tests
[PASS] Old Corpus Validation
[PASS] Coverage Matrix Check
[PASS] Baseline Hash
[PASS] Secret / Whitespace Scan
[PASS] .env Ignored
[PASS] git diff --check
[PASS] No Real AI Call
[PASS] No Prompt Modification
[PASS] No Schema Modification
[PASS] No Parser Modification
[PASS] No Commit
[PASS] No Push
```

---

# 30. 最终结论

```text
Medicine Concept:
PASS

Non-killing Contrast:
PASS

Three Base Dispositions:
PASS

Hidden Complete Truth:
Not Encoded

Two-case Coverage:
PASS

Case Validator:
PASS after CaseID uniqueness correction

Old Regression:
Preserved

Unit Tests:
82 / 82 PASS

Real AI Calls:
0
```

Medicine Case 值得进入 Day5 Corpus 设计。

它没有复刻 DoorKnife 的杀人责任，而是建立了：

```text
善意
+
规则
+
稀缺资源
+
信任
+
真实善果
+
真实第三人代价
```

之间的冲突。

当前最重要的后续风险是：

```text
ordinary_transfer 安全偏向
+
Outcome02 医学因果过度补全
+
功利开脱
+
规则绝对归罪
+
Design Metadata 泄漏
```

这些问题应在下一阶段的 New Report Contract 与 Moral Corpus v0.2 中进行本地验证，而不是提前修改 Prompt 或调用真实 AI。
