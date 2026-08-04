# 《忘川河畔：见习判员》Week8 Day2 Progress

**Week Theme:** Fixed Case & Real AI Judgement Experiment Foundation  
**Day Theme:** Real AI Connection & First Valid Response  
**Date:** 2026-08-04  
**Branch:** `feature/ai-first-prototype`  
**Provider / Model:** DeepSeek / `deepseek-v4-flash`  
**Case:** `Case.Knife.001` v0.2  
**Judge:** `Judge.Clerk.001` v0.1  
**Prompt / Schema:** v0.1 / v0.1

---

## 1. 今日目标

完成第一次真实 AI 端到端判词审核：

```text
Case + Judge + Player Report
→ Prompt Builder
→ DeepSeek V4 Flash
→ JSON Output
→ Raw Result
→ Local Validator
→ Typed JudgementResult
→ Validated Result
```

今日重点是验证真实 Provider、结构化输出、本地验证与结果追踪，不追求最终判官文风或批量稳定性结论。

---

## 2. 审核结论

**Week8 Day2 技术闭环测试通过。**

本次成功完成：

- 本地 API Key 安全读取；
- DeepSeek V4 Flash 真实调用；
- 单次、非流式、无自动重试请求；
- 合法 JSON 返回；
- Schema 与运行时语义验证通过；
- Typed `JudgementResult` 创建；
- Raw / Validated Result 分开保存；
- Provider、模型、版本、耗时和 Token 可追溯；
- API Key 未进入 Git 或结果文件。

成功运行：

```text
Run ID: 20260804T144933407633Z_69a48dbf
Model Returned: deepseek-v4-flash
Finish Reason: stop
Elapsed: 6832 ms
Prompt Tokens: 2989
Completion Tokens: 744
Total Tokens: 3733
```

当前结论：

```text
技术可信度：通过
结构化分析：通过
证据约束：通过
处置一致性：通过
Judge Persona 风格：不足
```

因此 Day2 应记录为：

> 真实 AI 判案闭环已经成立，但值房判官的角色化表达尚未达到玩法标准。

---

## 3. 开发环境

已安装：

```text
openai 2.53.0
```

本次配置：

```text
Provider: deepseek
Model: deepseek-v4-flash
Thinking: disabled
Temperature: 0.2
Max Tokens: 2400
Timeout: 90 seconds
Stream: false
Automatic Retry: 0
```

关闭 Thinking Mode 是为了减少第一次实验变量、延迟与额外消耗。后续可在保持 Case、Judge、Prompt、Schema 和 Report 不变时进行对照实验。

---

## 4. 新增与修改文件

### 新增代码

```text
src/ai_client.py
src/env_loader.py
src/report_parser.py
src/result_writer.py
src/run_real_judgement.py
```

### 修改代码

```text
src/prompt_builder.py
```

### 新增报告与测试

```text
reports/manual/R01_rigorous_detain.md
tests/test_day2_local_components.py
reports/Week8Day2_Step10_TestResults.md
```

### 新增结果目录

```text
results/raw/.gitkeep
results/validated/.gitkeep
```

### 新增依赖记录

```text
requirements-lock.txt
```

真实 `.env`、Raw Result 和 Validated Result 均未进入 Git。

---

## 5. 数据流变化

### Day2 前

```text
Raw Judgement JSON
→ Validator
→ Typed JudgementResult
```

但 Raw JSON 仅为手写测试数据。

### Day2 后

```text
Case.Knife.001 v0.2
+
Judge.Clerk.001 v0.1
+
R01 Player Report
+
Prompt v0.1
+
Schema v0.1
→ DeepSeek V4 Flash
→ Provider JSON
→ json.loads()
→ Structural Validation
→ Runtime Semantic Validation
→ JudgementResult
→ Raw Result
→ Validated Result
```

Week8 已从本地数据契约测试进入真实模型语义实验。

---

## 6. 模块职责

### `env_loader.py`

- 从本地 `.env` 读取配置；
- 不覆盖已有系统环境变量；
- 拒绝非法配置；
- 不打印或保存 API Key。

### `report_parser.py`

- 加载玩家 Markdown 报告；
- 检查长度；
- 要求唯一 `DispositionID`；
- 验证处置属于当前 Case。

### `prompt_builder.py`

- 注入 Judge Persona、Case、Fragment、Disposition 和 Schema；
- 将 Player Report 标记为不可信输入；
- 明确要求只返回 JSON；
- 禁止隐藏真相、新证据、奖励与额外字段。

### `ai_client.py`

- 调用 DeepSeek OpenAI-compatible API；
- 设置 JSON Output、Thinking、Temperature 和 Timeout；
- 禁用自动重试；
- 区分认证、余额、限流、网络、超时与 Provider 错误；
- 检查 Choice、Finish Reason 和空 Content。

### `result_writer.py`

- 创建唯一 Run ID；
- 记录 UTC 时间、Hash 和版本；
- 防止文件覆盖；
- 分开保存 Raw / Validated Result。

### `run_real_judgement.py`

```text
Load Config
→ Load Data
→ Build Prompt
→ Confirm SEND
→ One API Call
→ Save Raw
→ Parse
→ Validate
→ Save Validated
→ CLI Summary
```

---

## 7. 离线测试

执行：

```powershell
python -m unittest discover -s tests -v
```

结果：

```text
Ran 22 tests in 0.017s
OK
Exit Code: 0
```

新增测试覆盖：

- `.env` 正常加载；
- 不覆盖系统变量；
- 非法配置被拒绝；
- 合法报告通过；
- 非法处置被拒绝；
- Prompt 包含 JSON 与不可信边界；
- 结果文件拒绝覆盖；
- Hash 结果稳定。

---

## 8. 第一次真实报告

报告：

```text
R01_rigorous_detain.md
```

类型：

```text
严谨举证型
```

处置：

```text
detain_for_review
```

报告明确区分：

- “济”字不能证明职业；
- 血痕不能证明主动伤人；
- 钱与便笺不能证明交易或胁迫；
- 残响可以支持保护，也可以支持隐瞒；
- 材料不足以散魂或直接轮回；
- 收押复审与论证一致。

---

## 9. 结构化结果审核

### Core Claim

AI正确理解：

- 材料支持多个解释方向；
- 无法确认职业、血痕来源、钱的用途或屋内人物；
- 不存在唯一结论。

```text
[PASS] 未假装掌握隐藏真相
[PASS] 未宣布唯一正确解释
```

### Identity / Motive

AI没有把“济”直接认定为医者，也没有把少量钱直接认定为交易、债务或胁迫。

```text
[PASS] 能区分提示与证明
```

### Disposition

返回：

```text
detain_for_review
```

与玩家选择一致。

```text
[PASS] 未擅自改判
[PASS] 处置与论证一致
```

### Fragment Mapping

AI返回全部五个合法 FragmentID。由于报告确实讨论了全部五条材料，本次映射合理。

```text
[PASS] 无未知 Fragment
[PASS] 无重复
[PASS] 无虚构第六条材料
```

### Contradiction Handling

返回：

```text
acknowledged
```

理由是玩家承认残响存在多种解释，没有强行统一。

该判断可接受；未来可观察 `acknowledged` 与 `integrated` 的稳定边界。

### Dimension Ratings

```text
Narrative Coherence: adequate
Evidence Grounding: strong
Rhetorical Effectiveness: adequate
Disposition Alignment: strong
```

审核认为四项维度基本独立，没有因严谨或修辞而全部给出同一等级。

### Style Tags

```text
evidence_driven
cautious
contradiction_aware
```

均合法且符合报告风格。

---

## 10. 发现的问题

### 10.1 `unsupported_assumptions` 分类轻微不协调

AI把以下内容加入无依据假设：

```text
亡魂可能是在保护某人或隐瞒事件
```

但 Reason 又说明：

```text
玩家以“可能”表述，属于合理推断，未作为事实陈述
```

这形成语义冲突：

```text
合理推断
却被放入 unsupported_assumptions
```

Schema 无法识别这种分类错误，因此属于真实模型语义质量问题。

#### Prompt v0.2 调整方向

明确要求：

```text
如果玩家没有无依据假设，返回空数组。
不得为了提供反馈而把合法的可能性推断放入该数组。
只有超出 Fragment 支持、且被玩家当作事实或关键前提使用的内容，才能进入 unsupported_assumptions。
```

---

### 10.2 Judge Response 风格化不足

当前回复更像验证器评语：

```text
条理清晰
→ 逐项肯定
→ 指出一处不足
→ 确认处置
→ 总结评价
```

主要问题：

- 使用“证据意识值得肯定”“总体而言”等通用评语；
- 缺少阴司官署称谓；
- 缺少判官职业习惯与微妙偏见；
- 对短刀、钱袋和残响的引用不够具象；
- “略带讽刺”的 Persona 没有明显体现；
- 缺少能刺激玩家重写判词的具体质问；
- 回复仍像系统审核，而非角色当面批案。

审核结论：

```text
结构化判断：合格
角色表达：偏低
Judge Persona 可观察影响：不足
```

#### Prompt v0.2 风格方向

要求 Judge Response：

1. 至少引用一项案件具体物件或原句；
2. 使用符合地府官署身份的称谓；
3. 避免固定“表扬—纠偏—总结”模板；
4. 展示一项明确的制度偏好；
5. 给出一句具体案件质疑；
6. 可有限讽刺制度或文书，但不戏谑亡魂苦难；
7. 不重复 Dimension Ratings；
8. 不写成测试报告；
9. 让玩家产生修改判词的冲动。

---

### 10.3 没有真正形成案件追问

当前回复建议进一步解释“济”字，但没有提出更具叙事张力的问题，例如：

- 他在保护谁？
- “他们”是谁？
- 若只是救人，为什么谁也走不了？
- 这点钱是代价，还是借口？

Week8 当前没有第二轮质询，因此不构成技术缺陷；但 Judge Response 是否能推动玩家重新组织故事，是后续核心体验指标。

---

### 10.4 当前 Case 仍偏向复审

本次严谨报告自然得到 `detain_for_review`，与之前的案件审核一致：

- 缺少明确善意事实支持轮回；
- 缺少明确严重伤害支持散魂；
- 同时存在疑点与证据不足；
- 复审成为安全选择。

后续必须测试：

- 严谨但支持轮回；
- 严厉但仍受材料约束；
- 雄辩低证据；
- 诡辩回避矛盾。

本次结果不能证明三种处置都拥有足够玩法空间。

---

## 11. Raw / Validated Result 审核

Raw Result：

```text
status: validated
model_returned: deepseek-v4-flash
finish_reason: stop
elapsed_ms: 6832
validation.passed: true
validation.issues: []
```

安全检查：

```text
[PASS] API Key 未写入
[PASS] Authorization Header 未写入
[PASS] .env 内容未写入
[PASS] JSON Parse Error 未发生
[PASS] Validation Error 未发生
```

Validated Result 只在 Provider 成功、JSON 可解析、Schema 与运行时验证通过后生成。

```text
[PASS] 可用于后续结果比较
[PASS] 可供未来程序评分读取
```

---

## 12. Git 状态

当前预期修改：

```text
 M .env.example
 M requirements.txt
 M src/prompt_builder.py
?? reports/manual/R01_rigorous_detain.md
?? reports/Week8Day2_Step10_TestResults.md
?? requirements-lock.txt
?? results/raw/.gitkeep
?? results/validated/.gitkeep
?? src/ai_client.py
?? src/env_loader.py
?? src/report_parser.py
?? src/result_writer.py
?? src/run_real_judgement.py
?? tests/test_day2_local_components.py
```

安全确认：

```text
[PASS] .env 未出现
[PASS] .venv 未出现
[PASS] 实际 Raw / Validated 文件未出现
[PASS] API Key 未进入 Git
```

---

# 13. System Understanding Review

## 13.1 今天新增的数据由谁拥有？

- `.env`：本地运行环境拥有，不进入 Git。
- Player Report：实验输入层拥有。
- Request Messages：Prompt Builder 生成。
- Provider Response：DeepSeek 返回，仍是不可信外部输入。
- Raw Result：实验记录层拥有，用于复现一次调用。
- Validated Result：本地程序验证后生成，可进入正式后续流程。

## 13.2 哪些内容是固定游戏规则？

- 案件没有隐藏真相；
- Fragment 是唯一合法事实；
- 玩家处置不能被 AI 修改；
- Judge Persona 与 Schema 由游戏定义；
- AI不能新增证据、奖励或字段；
- 只有验证结果可以进入正式流程；
- 一次 Run 对应一次模型调用。

## 13.3 哪些内容交给 AI？

- Core Claim；
- Identity / Motive Hypothesis；
- Fragment 映射；
- Unsupported Assumptions；
- Contradiction Handling；
- 四项有限评价；
- Style Tags；
- Strongest / Weakest Point；
- Judge Response；
- Archive Summary。

## 13.4 哪些输出必须由程序验证？

- Schema Version；
- Case ID；
- Judge ID；
- Disposition ID；
- 玩家处置一致性；
- FragmentID；
- 字段类型；
- 必需与额外字段；
- 枚举；
- 数组与字符串长度。

程序无法单独证明语义分类和角色表达质量，仍需人工审核与重复实验。

## 13.5 哪里可能发生模型漂移？

- Core Claim 提取；
- Fragment 映射；
- 合理推断与虚构的边界；
- `acknowledged` / `integrated`；
- 四项 Ratings；
- Judge Response 模板；
- “济”字的权重；
- Style Tags。

本次只有一次调用，尚不能形成稳定性结论。

## 13.6 哪个测试能证明假设是错的？

- 相反但合理判词被强行统一；
- 合理推断持续被判为虚构；
- 修辞污染证据评价；
- Prompt Injection 改变规则；
- Judge Response 对所有报告使用同一模板；
- 玩家无法从回应中获得改写动力；
- 角色化要求导致分析准确性下降；
- Case 始终只能支持复审。

## 13.7 更换模型时哪些内容应保持不变？

- Case / Fragment；
- Judge Persona；
- Report；
- Prompt / Schema Version；
- Disposition；
- Validator；
- Result Format；
- Raw / Validated 保存规则；
- 测试标准；
- 后续程序评分。

## 13.8 当前是否过度设计？

没有明显过度设计。

当前模块都服务于已经发生的真实调用；尚未加入多 Provider、Batch Runner、数据库、Web UI、Unreal 接入、Agent、RAG、多轮质询或正式评分。

---

## 14. 今日学习结论

### 模型调用成功不等于玩法成功

API、JSON、Schema 和 Validator 全部成功，但 Judge Persona 风格不足：

```text
工程闭环成功
≠
玩家体验已经成立
```

### Structured Output 不保证语义分类正确

合法 JSON 仍可能把合理推断放入错误字段：

```text
Schema 保证形状
不保证判断逻辑
```

### Persona 数据不自动产生角色感

“克制、官署化、略带讽刺”写入 Persona 后，模型仍可能优先使用安全、标准化评语。角色感需要更明确的语言行为约束与对照测试。

### AI 当前提供的不可替代价值

- 理解自由中文判词；
- 区分事实与谨慎推断；
- 映射 FragmentID；
- 分析矛盾；
- 区分证据、连贯和修辞；
- 生成针对内容的反馈。

### 程序仍应负责

- ID 与长度检查；
- Schema 与枚举；
- 版本与文件；
- Token / 耗时；
- 奖励与经济；
- 最终数值评分。

---

## 15. 尚未解决的问题

- Judge Response 角色化不足；
- `unsupported_assumptions` 边界需加强；
- Case 是否支持轮回和散魂方向；
- 相反判词是否能被合理认可；
- Prompt Injection 尚未真实测试；
- 虚构第六条 Fragment 尚未真实测试；
- 高修辞低证据尚未测试；
- Thinking Mode 尚未对比；
- 同一报告稳定性尚未测试；
- 单次美元成本尚未汇总；
- Persona 实际影响尚未证明；
- Prompt v0.1 长度是否必要仍需观察。

---

## 16. 下一步

Week8 Day3 建议进入：

```text
CLI Experiment Flow
+
Test Corpus Foundation
```

在扩展 20 份语料前，先做一次最小 Prompt v0.2 对照实验：

1. 无无依据假设时必须返回空数组；
2. 合理可能性不能放入 `unsupported_assumptions`；
3. Judge Response 至少引用一个具体物件或残响；
4. 禁止固定“表扬—不足—总结”模板；
5. 加入一句判官式具体质疑；
6. 增强官署角色感；
7. 不降低结构化字段准确性。

使用相同：

```text
Case
Judge
Schema
Model
Thinking
Temperature
Report
```

只修改 Prompt Version，比较 v0.1 与 v0.2。

---

## 17. Git 提交

提交前：

```powershell
git add AI_Judgement_Prototype
git diff --cached
```

确认 `.env`、API Key 和真实结果文件未进入暂存区。

推荐：

```powershell
git commit -m "Week8 Day2: connect DeepSeek real AI judgement flow"
git push origin feature/ai-first-prototype
```

---

## 18. 最终状态

```text
[PASS] DeepSeek V4 Flash 真实调用
[PASS] 单次调用、无自动重试
[PASS] JSON Output
[PASS] Finish Reason = stop
[PASS] Raw Result
[PASS] Schema Validation
[PASS] Runtime Validation
[PASS] Typed JudgementResult
[PASS] Validated Result
[PASS] 5 个 FragmentID 合法
[PASS] 玩家处置未被修改
[PASS] 22 项离线测试
[PASS] API Key 安全
[WATCH] Judge Persona 风格化不足
[WATCH] 合理推断被放入 unsupported_assumptions
[WATCH] Case 偏向 detain_for_review
[READY] Git Commit / Push
[READY] Prompt v0.2 对照实验
[READY] Week8 Day3
```
