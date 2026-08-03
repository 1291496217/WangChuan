# 《忘川河畔：见习判员》Week8 Day1 Advance Extra Progress

**Week Theme:** Fixed Case & Real AI Judgement Experiment Foundation  
**Advance Extra Theme:** Judge Persona, Output Contract & Schema Validation  
**Equivalent Original Plan:** Week8 Day3  
**Date:** 2026-08-03  
**Branch:** `feature/ai-first-prototype`  
**Case:** `Case.Knife.001`  
**Case Version:** `0.2`  
**Judge:** `Judge.Clerk.001`  
**Judge Version:** `0.1`  
**Schema Version:** `0.1`

---

## 1. 今日目标

在 Week8 Day1 与 Day1 Advance 已经完成的基础上，提前完成原计划 Day3 的核心任务：

```text
正式 Judge Persona
+
结构化 Output Contract
+
本地 Schema Validation
+
运行时语义 Validation
+
Typed Local Model
+
合法 / 非法结果测试
```

本阶段的重点不是连接真实 AI，而是先建立一个明确的数据边界：

```text
模型返回 JSON
≠
程序可以直接信任的数据
```

只有通过本地结构检查与当前案件语义检查后，模型输出才能被转换为正式的 `JudgementResult` 并进入后续保存、展示或程序评分流程。

---

## 2. 审核结论

**Week8 Day1 Advance Extra 全部测试通过。**

本阶段已经成功完成：

- 正式 Judge Persona `Judge.Clerk.001 v0.1`
- 正式 Judgement Result Schema `v0.1`
- 本地结构验证器
- 当前 Case / Judge / Fragment / Disposition 的运行时语义验证
- 验证结果数据结构
- Typed `JudgementResult`
- 1 份合法结构化样本
- 14 项单元测试
- 合法样本 CLI 验证
- 错误结果拒绝路径

实际测试结果：

```text
VALIDATION PASSED
validate_sample exit_code = 0
```

以及：

```text
Ran 14 tests
OK
unittest exit_code = 0
```

当前没有阻塞性错误，可以进行 Git Commit / Push，并为下一阶段真实 DeepSeek API 连接做好准备。

---

## 3. 已完成内容

### 3.1 正式 Judge Persona

新增：

```text
AI_Judgement_Prototype/
└─ judges/
   └─ judge_clerk_001.json
```

正式身份：

```text
Judge Profile ID: Judge.Clerk.001
Version: 0.1
Display Name: 值房判官
Role: 黄泉路前站案件审核者
```

判官主要关注：

- 材料对应；
- 矛盾解释；
- 处置与论证一致；
- 玩家是否虚构证据；
- 玩家是否使用修辞掩盖材料缺口；
- 玩家是否假装存在唯一真相。

回应风格：

- 克制；
- 官署化；
- 略带讽刺；
- 针对玩家具体论点。

判官边界：

- 不宣称掌握未提供的隐藏真相；
- 区分案件事实、合理推断与无依据扩写；
- 分别评价证据约束、叙事连贯和修辞效果；
- 只审核处置与论证一致性；
- 不决定奖励、功籍或经济数值。

---

### 3.2 正式 Output Schema

新增：

```text
AI_Judgement_Prototype/
└─ schemas/
   └─ judgement_result_v0_1.json
```

Schema ID：

```text
WangChuan.JudgementResult.0.1
```

Schema Version：

```text
0.1
```

当前顶层字段：

```text
schema_version
case_id
judge_profile_id
core_claim
identity_hypothesis
motive_hypothesis
recognized_disposition_id
used_fragment_ids
unsupported_assumptions
contradiction_handling
dimension_ratings
style_tags
strongest_point
weakest_point
judge_response
archive_summary
```

顶层与嵌套对象均使用：

```text
additionalProperties = false
```

这意味着模型不能把未经批准的新字段作为正式输出写入系统，例如：

```text
reward_points
confidence_score
hidden_truth
correct_answer
player_rank
```

---

### 3.3 固定枚举

#### `unsupported_assumptions.severity`

```text
minor
major
```

#### `contradiction_handling.level`

```text
ignored
minimized
acknowledged
integrated
```

#### Dimension Rating

```text
weak
adequate
strong
```

适用于：

- `narrative_coherence`
- `evidence_grounding`
- `rhetorical_effectiveness`
- `disposition_alignment`

#### `style_tags`

允许：

```text
evidence_driven
compassionate
legalistic
bureaucratic
rhetorical
speculative
sophistic
harsh
cautious
contradiction_aware
```

模型不得使用：

```text
excellent
very_good
mostly_correct
92
balanced_but_emotional
```

等未定义等级或标签。

---

### 3.4 字段与数组长度限制

当前限制包括：

```text
used_fragment_ids:
0–5 项
不得重复

unsupported_assumptions:
0–5 项

style_tags:
0–5 项
不得重复

judge_response:
最多 600 字符

archive_summary:
最多 300 字符
```

其他主要文本字段也设置了最大长度，防止：

- 模型输出失控；
- 自然语言占据过多上下文；
- 结果文件不可比较；
- 判官回复掩盖结构化分析；
- 后续 UI 和 SaveGame 难以控制。

---

## 4. 两层验证架构

### 4.1 第一层：结构验证

负责检查：

```text
字段是否存在
字段类型是否正确
是否存在额外字段
枚举是否合法
字符串是否为空或超长
数组是否超量
数组是否重复
对象内部结构是否合法
Schema Version 是否正确
```

例如：

```json
{
  "evidence_grounding": "excellent"
}
```

结构上是字符串，但不是合法枚举，因此必须被拒绝。

---

### 4.2 第二层：运行时语义验证

Schema 无法知道程序当前加载的是哪个案件、判官和玩家处置，因此 `response_validator.py` 还会根据运行时数据验证：

```text
payload.case_id
==
当前 Case ID

payload.judge_profile_id
==
当前 Judge ID

recognized_disposition_id
属于当前案件合法处置

recognized_disposition_id
==
玩家实际选择的处置

used_fragment_ids
全部存在于当前 Case
```

例如：

```json
{
  "case_id": "Case.Other.001"
}
```

该字符串符合 Case ID 格式，但不是当前案件：

```text
Case.Knife.001
```

所以仍必须拒绝。

---

## 5. 新增代码

### 5.1 `src/models.py`

新增数据结构：

```text
ValidationIssue
ValidationResult
UnsupportedAssumption
ContradictionHandling
DimensionRatings
JudgementResult
```

#### `ValidationIssue`

记录：

- 错误路径；
- 错误原因。

示例：

```text
$.used_fragment_ids[0]:
fragment ID does not exist in the current case
```

#### `ValidationResult`

拥有：

```text
issues
is_valid
format_errors()
```

它负责聚合一次验证中的所有问题，而不是只返回第一个错误。

#### `JudgementResult`

是只有在验证成功后才能创建的正式本地数据模型。

它不是 AI 原始输出，而是：

```text
通过结构验证
+
通过运行时语义验证
```

之后的可信程序对象。

---

### 5.2 `src/response_validator.py`

当前职责：

- 加载 UTF-8 JSON Object；
- 执行 Week8 所需的 JSON Schema 子集；
- 检查类型；
- 检查枚举与常量；
- 检查字符串长度和正则格式；
- 检查数组长度与重复项；
- 检查必需字段；
- 拒绝额外字段；
- 检查嵌套对象；
- 验证当前 Case ID；
- 验证当前 Judge ID；
- 验证当前 FragmentID；
- 验证合法 DispositionID；
- 验证 AI 识别的处置是否等于玩家选择；
- 验证成功后创建 Typed Model；
- 验证失败时返回清晰错误。

本阶段只实现了项目当前实际使用的 JSON Schema 关键词，没有尝试重写一整套通用 Schema 引擎。

支持的结构规则：

```text
type
const
enum
pattern
minLength
maxLength
minItems
maxItems
uniqueItems
items
required
properties
additionalProperties
```

---

### 5.3 `src/validate_sample.py`

该脚本负责：

```text
加载正式 Case
+
加载正式 Judge
+
加载 Schema
+
加载合法样本
→ 执行验证
→ 创建 Typed JudgementResult
→ 输出摘要
```

它不连接真实 AI，也不读取 `.env`。

---

## 6. 合法结构化样本

新增：

```text
tests/
└─ fixtures/
   └─ valid_judgement_result.json
```

合法样本使用：

```text
Schema Version: 0.1
Case ID: Case.Knife.001
Judge ID: Judge.Clerk.001
Disposition: detain_for_review
Used Fragment Count: 5
```

样本体现：

- 核心判断；
- 身份假设；
- 动机假设；
- Fragment 映射；
- 无依据假设；
- 矛盾处理；
- 四项有限评价；
- 风格标签；
- 最强与最弱论点；
- 判官回应；
- 归档摘要。

该样本只用于验证输出契约，不代表真实 AI 已经能够稳定生成相同质量的结果。

---

## 7. 测试结果

### 7.1 合法样本验证

执行：

```powershell
python .\src\validate_sample.py
```

完整结果：

```text
========================================================================
WANGCHUAN — WEEK 8 DAY 1 ADVANCE EXTRA
========================================================================
VALIDATION PASSED
Schema Version: 0.1
Case ID: Case.Knife.001
Judge ID: Judge.Clerk.001
Recognized Disposition: detain_for_review
Used Fragment Count: 5
Unsupported Assumption Count: 1
Contradiction Handling: integrated
Typed local JudgementResult created successfully.
[validate_sample exit_code=0]
```

审核结论：

```text
[PASS] Schema 加载成功
[PASS] Case ID 一致
[PASS] Judge ID 一致
[PASS] Disposition 合法
[PASS] Disposition 与玩家选择一致
[PASS] 5 个 FragmentID 全部合法
[PASS] Unsupported Assumption 结构合法
[PASS] Contradiction Handling 枚举合法
[PASS] Dimension Ratings 合法
[PASS] Style Tags 合法
[PASS] Typed JudgementResult 创建成功
[PASS] Exit Code = 0
```

---

### 7.2 单元测试

执行：

```powershell
python -m unittest discover -s tests -v
```

结果：

```text
----------------------------------------------------------------------
Ran 14 tests in 0.002s

OK
[unittest exit_code=0]
```

审核结论：

```text
[PASS] 14 项测试全部通过
[PASS] 没有跳过测试
[PASS] 没有失败
[PASS] 没有错误
[PASS] Exit Code = 0
```

---

## 8. 单元测试覆盖

### 8.1 合法结果通过

确认合法样本：

- `ValidationResult.is_valid == True`
- 可以创建 `JudgementResult`
- Case、Disposition 与 Fragment 数量正确。

### 8.2 缺少字段被拒绝

删除：

```text
core_claim
```

预期：

```text
required field is missing
```

### 8.3 顶层额外字段被拒绝

加入：

```text
reward_points
```

预期：

```text
unexpected field is not allowed
```

这证明 AI 不能自行决定或注入奖励字段。

### 8.4 嵌套额外字段被拒绝

在 `unsupported_assumptions` 中加入：

```text
confidence
```

预期被拒绝。

这证明 `additionalProperties = false` 不只作用于顶层。

### 8.5 非法评价枚举被拒绝

将：

```text
evidence_grounding = excellent
```

预期：

```text
is not in the allowed enum
```

### 8.6 不存在的 FragmentID 被拒绝

使用：

```text
KnifeCase.Unknown99
```

预期：

```text
fragment ID does not exist in the current case
```

### 8.7 重复 FragmentID 被拒绝

让 `used_fragment_ids` 出现重复项，预期：

```text
must not contain duplicate items
```

### 8.8 不存在的 DispositionID 被拒绝

使用：

```text
pardon_and_release
```

预期：

```text
disposition ID is not allowed by the current case
```

### 8.9 与玩家选择不一致的合法处置被拒绝

即使：

```text
reincarnate
```

是合法 Disposition，但玩家实际选择：

```text
detain_for_review
```

因此仍应拒绝。

这避免 AI 擅自修改玩家提交的正式处置。

### 8.10 超长 Judge Response 被拒绝

超过：

```text
600 characters
```

预期被拒绝。

### 8.11 错误 Case ID 被拒绝

使用：

```text
Case.Other.001
```

预期被当前 Case 语义验证拒绝。

### 8.12 错误 Judge ID 被拒绝

使用：

```text
Judge.Other.001
```

预期被拒绝。

### 8.13 错误 Schema Version 被拒绝

使用：

```text
0.2
```

而当前要求：

```text
0.1
```

预期被拒绝。

### 8.14 Style Tags 超量被拒绝

超过 5 项时，预期：

```text
must contain at most 5 item(s)
```

---

## 9. Git 状态审核

测试时状态：

```text
?? AI_Judgement_Prototype/judges/judge_clerk_001.json
?? AI_Judgement_Prototype/schemas/judgement_result_v0_1.json
?? AI_Judgement_Prototype/src/models.py
?? AI_Judgement_Prototype/src/response_validator.py
?? AI_Judgement_Prototype/src/validate_sample.py
?? AI_Judgement_Prototype/tests/fixtures/valid_judgement_result.json
?? AI_Judgement_Prototype/tests/test_response_validator.py
```

审核结论：

- 所有项目均为本阶段预期新增文件；
- 没有意外修改现有 Case；
- 没有意外修改 Prompt Builder；
- 没有真实 API 调用结果；
- `.env` 未出现；
- `.venv` 未出现；
- 当前未 Commit / Push。

---

## 10. 遇到的问题

本阶段测试没有发现阻塞性问题。

### 10.1 为什么没有使用第三方 `jsonschema`

本阶段有意使用 Python 标准库实现当前需要的验证逻辑。

原因：

- 能清楚学习每项验证由谁执行；
- 不立即引入 Python 3.13 第三方兼容性问题；
- 当前 Schema 使用的关键词有限；
- 便于理解结构检查与运行时检查的区别；
- 避免为一个固定实验提前引入复杂依赖。

这不是永久决定。

如果后续 Schema 复杂度明显增加，可以改用成熟库，但必须保留：

```text
当前 Case / Judge / Fragment / Disposition
```

相关的运行时语义验证，因为标准 JSON Schema 无法自动知道这些游戏状态。

---

### 10.2 为什么 Validator 没有接入 `main.py`

当前：

```text
main.py
```

负责：

```text
Case / Judge / Player Report
→ Prompt Preview
```

Validator 负责：

```text
AI Result
→ Validation
→ Typed Model
```

在真实 AI Client 尚未存在时强行连接两端，只会制造一个没有真实返回值的伪流程。

正确接入时机是原计划 Day4：

```text
Prompt Builder
→ DeepSeek Client
→ Raw Result
→ Response Validator
→ JudgementResult
```

---

### 10.3 Typed Model 不等于内容绝对正确

当前 Validator 可以确认：

- 数据结构合法；
- ID 合法；
- 枚举合法；
- 长度合法；
- 当前 Case / Judge 一致；
- AI 没有修改玩家处置。

但它不能完全证明：

- `core_claim` 真正理解了玩家；
- Fragment 映射在语义上准确；
- 无依据假设识别合理；
- 评价等级稳定；
- Judge Response 真正针对玩家；
- AI 没有做出偏见或错误推断。

这些属于 Day4–Day6 的真实模型语义实验，而不是 Schema 能单独解决的问题。

---

## 11. 系统架构变化

### Advance Extra 前

```text
Case
+
Judge Stub
+
Player Report
→ Prompt Preview
```

### Advance Extra 后

输入侧：

```text
Case.Knife.001
+
Judge.Clerk.001
+
Player Report
→ Prompt Builder
```

输出侧：

```text
Raw Judgement JSON
→ Schema Structural Validation
→ Runtime Semantic Validation
→ ValidationResult
→ JudgementResult
```

下一阶段将连接为：

```text
Case / Judge / Report
→ Prompt Builder
→ DeepSeek API
→ Raw JSON
→ Validator
→ Typed JudgementResult
→ Raw / Validated Result Files
```

---

# 12. System Understanding Review

## 12.1 今天新增的数据由谁拥有？

### Judge Persona

由游戏设计层拥有。

AI只能按照 Judge Persona 表现，不能自行添加新优先级、修改审判边界或将商业模型身份写入角色设定。

### Schema

由程序与数据契约层拥有。

Schema 定义：

```text
正式结果允许长什么样
```

模型不能通过返回额外字段修改契约。

### Raw Result

未来由 Provider 返回，但仍属于不可信输入。

### ValidationResult

由本地程序生成，记录 raw payload 的问题。

### JudgementResult

由本地程序在验证成功后创建，是后续正式流程可以使用的数据对象。

---

## 12.2 哪些内容是固定游戏规则？

- Case ID 由游戏决定；
- Judge ID 由游戏决定；
- FragmentID 由游戏决定；
- DispositionID 由游戏决定；
- 玩家选择的处置不能被 AI 修改；
- 枚举由 Schema 决定；
- AI不能添加奖励字段；
- AI不能添加隐藏真相字段；
- AI不能增加未定义 Fragment；
- AI不能返回任意风格标签；
- AI输出必须经过本地验证；
- 只有合法输出可以创建 Typed Model。

---

## 12.3 哪些内容交给 AI？

未来 AI 负责：

- 提取核心主张；
- 识别身份假设；
- 识别动机假设；
- 映射玩家使用的 Fragment；
- 找出无依据假设；
- 判断矛盾处理；
- 给出四项有限等级；
- 选择有限 Style Tags；
- 提炼最强与最弱论点；
- 生成 Judge Response；
- 生成 Archive Summary。

AI不决定：

- 合法字段；
- 合法枚举；
- Fragment 是否真实存在；
- 玩家处置；
- 奖励数值；
- 功籍；
- 经济；
- 隐藏答案。

---

## 12.4 哪些 AI 输出必须由程序重新验证？

所有正式字段都需要结构验证，特别是：

```text
schema_version
case_id
judge_profile_id
recognized_disposition_id
used_fragment_ids
unsupported_assumptions
contradiction_handling
dimension_ratings
style_tags
judge_response
archive_summary
```

运行时必须再次验证：

```text
Case ID
Judge ID
Disposition ID
Player-selected Disposition
FragmentID
```

---

## 12.5 今天的实现中，哪里可能发生模型漂移？

Schema 合法并不能阻止语义漂移。

可能漂移：

- `core_claim` 的提取；
- `identity_hypothesis` 的具体程度；
- `motive_hypothesis` 是否过度脑补；
- `used_fragment_ids` 映射；
- `unsupported_assumptions` 数量与严重度；
- `contradiction_handling.level`；
- 四项 Dimension Rating；
- Style Tags；
- Judge Response 的语气；
- Archive Summary 的事实边界。

Week9 将通过同一输入重复运行观察这些字段。

---

## 12.6 哪个测试能够证明当前假设是错的？

以下结果会证明当前设计需要调整：

- DeepSeek 频繁无法输出符合 Schema 的 JSON；
- 合法 JSON 仍频繁引用不存在的 Fragment；
- 模型持续改写玩家处置；
- 相同报告的 Fragment 映射大幅漂移；
- 虚构证据不能稳定进入 `unsupported_assumptions`；
- 高修辞报告持续获得高证据评价；
- Judge Response 与结构化字段互相矛盾；
- Persona 对回应没有可观察影响；
- Schema 字段过多，导致模型分析质量下降；
- 玩家无法理解四项评价的区别。

---

## 12.7 如果更换模型，哪些部分应该保持不变？

- Case ID / Version；
- FragmentID / Text；
- DispositionID；
- Judge Persona ID / Version；
- Schema Version；
- Prompt Version；
- Validator；
- Typed Models；
- 测试报告；
- 合法样本；
- 单元测试；
- Raw / Validated 文件格式；
- 玩家选择；
- 后续程序评分规则。

Provider、Model 名称与 API 调用方式可以变化。

---

## 12.8 当前代码是否为了未来扩展而过度设计？

当前没有明显过度设计。

新增模块：

```text
models.py
response_validator.py
validate_sample.py
test_response_validator.py
```

它们分别承担当前已存在的职责：

- 数据结构；
- 验证逻辑；
- 手动合法样本演示；
- 自动回归测试。

当前没有提前实现：

- 通用 Provider Adapter；
- 多模型；
- Batch Runner；
- 数据库；
- Web UI；
- Unreal HTTP；
- 多判官轮岗；
- 程序化案件；
- 完整评分；
- 复杂重试队列；
- Agent 或 Tool Calling。

标准库 Validator 只实现项目当前使用的 Schema 子集，也避免了把它扩展成不必要的通用框架。

---

## 13. 尚未解决的问题

本阶段没有阻塞性问题。

后续需要验证：

- DeepSeek V4 Flash 是否能稳定遵守当前 Schema；
- JSON Output 是否会包裹 Markdown；
- 模型是否会省略字段；
- 模型是否能正确映射五个 FragmentID；
- 模型是否会错误地把推测写成事实；
- Judge Persona 是否真正影响回复；
- Judge Response 与结构化分析是否一致；
- 当前 Schema 是否过于复杂；
- Python 3.13 与 DeepSeek 所选 SDK / HTTP 库兼容性；
- API 超时、HTTP 错误和 JSON 错误如何区分；
- Raw Result 和 Validated Result 的文件命名与防覆盖策略。

---

## 14. 下一步

下一阶段进入原计划 Day4：

```text
Real AI Connection & First Valid Response
```

主要任务：

1. 确认 DeepSeek 官方 API 文档；
2. 确认模型参数；
3. 从 `.env` 读取 API Key；
4. 实现 `ai_client.py`；
5. 设置请求超时；
6. 更新正式 Prompt Builder；
7. 提交一份严谨测试判词；
8. 保存原始 Provider 输出；
9. 执行本地 Validator；
10. 保存验证后结果；
11. 显示 CLI 摘要；
12. 记录模型、Prompt、Schema、Case 和 Judge 版本；
13. 区分网络错误、HTTP 错误、JSON 错误和验证错误。

完成标准：

```text
固定案件
→ 玩家自由判词
→ DeepSeek V4 Flash
→ 合法结构化 JSON
→ 本地验证通过
→ Typed JudgementResult
→ 针对性 Judge Response
```

---

## 15. Git 提交前检查

在项目根目录执行：

```powershell
git status --short
git add AI_Judgement_Prototype
git status --short
git diff --cached --name-only
git diff --cached
```

确认：

- `.env` 未进入暂存区；
- `.venv` 未进入暂存区；
- 没有 API Key；
- 没有真实 Provider Response；
- 7 个新增文件均属于本阶段；
- Schema Version 为 `0.1`；
- Judge Version 为 `0.1`；
- 单元测试内容与实际结果一致。

推荐 Commit：

```powershell
git commit -m "Week8 Day1 Advance Extra: add judgement schema validation"
git push origin feature/ai-first-prototype
```

---

## 16. 最终状态

```text
[PASS] 正式 Judge Persona 建立
[PASS] Judge ID 与 Version 固定
[PASS] Schema v0.1 建立
[PASS] 顶层字段固定
[PASS] 嵌套字段固定
[PASS] 枚举固定
[PASS] 字符串长度限制
[PASS] 数组长度限制
[PASS] 重复项检查
[PASS] 额外字段拒绝
[PASS] Case ID 运行时验证
[PASS] Judge ID 运行时验证
[PASS] FragmentID 运行时验证
[PASS] DispositionID 运行时验证
[PASS] 玩家处置一致性验证
[PASS] Typed JudgementResult 建立
[PASS] 合法样本验证
[PASS] 14 项单元测试
[PASS] 所有 Exit Code = 0
[PASS] 未调用真实 AI
[PASS] 未读取 .env
[PASS] .env 未进入 Git 状态
[PASS] .venv 未进入 Git 状态
[READY] Git Commit / Push
[READY] Week8 原计划 Day4
```
