# 《忘川河畔：见习判员》Week8 Day1 Advance Progress

**Week Theme:** Fixed Case & Real AI Judgement Experiment Foundation  
**Advance Theme:** Fixed Case Design & Multi-Interpretation Audit  
**Equivalent Original Plan:** Week8 Day2  
**Date:** 2026-08-03  
**Branch:** `feature/ai-first-prototype`  
**Case:** `Case.Knife.001`  
**Case Version:** `0.2`

---

## 1. 今日目标

在 Week8 Day1 数据流基础已经完成的前提下，提前完成原计划 Day2 的核心内容：

```text
固定案件设计
→ 五类 Fragment 建立
→ 多解释空间审核
→ Fragment 精简与修订
→ 正式案件接入 CLI
→ JSON / ID / 数据流验证
```

本次 Advance 的重点不是调用真实 AI，而是先确保：

1. 案件没有隐藏标准答案；
2. 五条 Fragment 都是合法、稳定、可观察的事实；
3. Fragment 可以支持不同甚至相反的解释方向；
4. 案件不是随机词造句；
5. 当前 Case 可以直接进入 Week8 后续 Prompt、Schema 与 AI 测试流程。

---

## 2. 审核结论

**Week8 Day1 Advance 自动化测试全部通过。**

当前正式案件已经能够：

- 以合法 JSON 加载；
- 使用 5 个稳定 FragmentID；
- 保留 3 种合法处置；
- 进入现有 Prompt Builder；
- 通过 CLI 完整显示；
- 在不调用真实 AI 的情况下正常结束；
- 以 `Case Version 0.2` 记录本轮玩家可见 Fragment 修改。

同时，设计审核发现两个值得保留到后续测试阶段的问题：

```text
Fragment 语义密度不均
+
当前案件天然偏向 detain_for_review
```

这些问题不会阻塞 Week8 Day1 Advance，也不要求立即重写案件；它们将作为 Day3–Day6 的 Prompt、Schema 和真实模型测试观察重点。

---

## 3. 已完成内容

### 3.1 正式案件建立

正式 Case：

```text
Case ID: Case.Knife.001
Case Version: 0.2
Title: 染血的刀
```

案件前提明确声明：

- 档案不完整；
- 不存在可供核对的隐藏真相；
- 玩家只能依据五条残存记忆提出解释；
- AI 判官未来也不能访问一个不存在的标准答案。

### 3.2 五类 Fragment 完成

当前 Fragment：

| FragmentID | Category | 玩家可见内容 |
| --- | --- | --- |
| `KnifeCase.Identity01` | `identity` | 灰白短褂，袖口缝着“济”字。 |
| `KnifeCase.Relationship01` | `relationship` | 一张未寄出的便笺写着：“钱我会想办法。” |
| `KnifeCase.Action01` | `action_or_result` | 短刀刃口带着血痕。 |
| `KnifeCase.Motive01` | `motive_or_emotion` | 钱袋，金额不多。 |
| `KnifeCase.Contradiction01` | `contradiction` | “若让他们看见还活着，我们谁也走不了！” |

五个 FragmentID 顺序与类型均符合预期。

### 3.3 Fragment 从长叙事线索修订为短事实

原始版本中的 Fragment 包含较多已经互相绑定的叙事信息，例如：

- 明确医疗用品；
- 明确药费；
- 明确亲属称呼；
- 多次“收拾残局”；
- 两种血迹与麻绳；
- 较完整的事件暗示。

这类内容可以组成一个可玩的固定案件，但不适合作为未来“从不同类型词库随机抽取”的基础样本，因为单条 Fragment 已经携带过多职业、关系、因果和事件顺序。

因此，本轮将其改为：

```text
短小
+
单一事实
+
低到中等指向性
+
保留解释空间
```

### 3.4 Case Version 更新

由于玩家可见的五条 Fragment 文本发生实质变化：

```text
Case Version 0.1
→
Case Version 0.2
```

这符合版本控制原则：

- 玩家可见 Case 数据变化：升级版本；
- 只修改设计备注：不必自动升级 Case Version。

### 3.5 Sample Report 同步修订

原 Day1 Sample Report 曾引用已经删除的内容：

- 白色外衣；
- 烧毁衣物；
- 明确职业推断。

为避免测试报告引用不存在的 Fragment，已同步改写为只使用 v0.2 当前材料。

当前报告：

```text
DispositionID: detain_for_review
```

核心判断：

- 只能确认亡魂与“济”字短褂、染血短刀、少量钱财以及隐瞒活人的话有关；
- 亡魂可能在帮助某人脱身；
- 也可能参与了需要掩饰的事件；
- 关系与动机仍不明确。

正式判词区分了：

```text
可观察事实
≠
可能解释
≠
无法证明的结论
```

并选择 `detain_for_review` 作为当前材料下较谨慎的处置。

---

## 4. 新增与修改文件

### 新增

```text
AI_Judgement_Prototype/
└─ cases/
   ├─ case_knife_001.json
   └─ case_knife_001_notes.md
```

### 修改

```text
AI_Judgement_Prototype/
├─ reports/
│  └─ day1_sample_report.md
└─ src/
   └─ main.py
```

### 文档记录

```text
Docs/
├─ Week8Day1AdvanceFragmentRevisionReport.md
└─ Week8Day1AdvanceTestReport.md
```

---

## 5. 数据流变化

### Day1 完成时

```text
Case.Day1.Stub
+
Judge.Day1.Stub
+
Sample Report
→ Prompt Preview
```

### Day1 Advance 完成后

```text
Case.Knife.001 v0.2
+
Judge.Day1.Stub
+
Revised Sample Report
→ Prompt Builder
→ CLI Preview
```

当前正式案件已经替代 Stub Case 进入主数据流。

Judge 仍保留为 Stub，是因为正式 Judge Persona 和 Output Contract 属于后续原计划 Day3 内容，不应在本次 Advance 中提前混合修改。

---

## 6. 案件设计审核

### 6.1 无隐藏真相

本案没有定义：

- 亡魂真实职业；
- “济”字真实含义；
- 谁写了便笺；
- 钱的来源与用途；
- 血是谁的；
- 刀是否用于伤人；
- “他们”是谁；
- 谁还活着；
- 亡魂是否有罪；
- 唯一正确处置。

人工提出的解释方向只用于检查解释空间，不会进入未来 AI Prompt 作为标准答案。

### 6.2 可支持的解释方向

#### 救助或保护

玩家可以推测：

- “济”与救助有关；
- 带血短刀可能用于处理伤口或割断某物；
- “还活着”说明有人需要被保护；
- 少量钱可能只是临时筹集的资源。

但不能直接声称亡魂是医者、伤者无辜或差役残暴。

#### 隐瞒或逃亡

玩家也可以推测：

- 刀上的血痕与暴力事件有关；
- 亡魂试图避免某群人发现活着的人；
- 少量钱与某次秘密行动有关；
- “谁也走不了”暗示共同承担后果。

但不能直接声称亡魂杀人、协助罪犯或收取犯罪报酬。

#### 经济压力下的参与

便笺与钱袋可以支持：

- 经济压力；
- 承诺；
- 临时筹钱；
- 因资源不足而参与某事。

但“债务”“胁迫”“药费”“赎金”等具体原因都不属于合法事实。

### 6.3 轻量组合关系

当前 Fragment 保留的连接包括：

```text
便笺中的钱
↔
钱袋

短刀
↔
血痕
↔
仍然活着的人

“他们”
↔
外部阻力

“济”字
↔
身份入口
```

这些联系可以让玩家构造故事，但不会自动生成完整因果链。

### 6.4 不是随机词造句

尽管每条 Fragment 都很短，五条材料仍然围绕：

```text
身份
+
钱
+
血
+
某个仍然活着的人
+
外部追索或阻力
```

形成最小语义网络。

因此，它们不是彼此完全无关的随机名词。

---

## 7. 自动化测试

### 7.1 JSON 语法测试

执行：

```powershell
python -m json.tool .\cases\case_knife_001.json > $null
```

结果：

```text
PASS: JSON syntax valid
Exit code: 0
```

### 7.2 FragmentID 测试

结果：

```text
PASS: FragmentID order and values match expected.
Fragment count: 5
```

顺序：

```text
KnifeCase.Identity01
KnifeCase.Relationship01
KnifeCase.Action01
KnifeCase.Motive01
KnifeCase.Contradiction01
```

### 7.3 CLI 数据流测试

执行：

```powershell
python .\src\main.py
```

摘要：

```text
Case: 染血的刀
Case ID: Case.Knife.001
Judge: 值房判官（数据流测试）
Fragment count: 5
Disposition count: 3
Player report characters: 314
Day 1 completed without calling an AI service.
```

结果：

```text
Exit code: 0
```

正式案件加载成功，Prompt Builder 正常工作，且没有调用真实 AI。

### 7.4 Git 状态

测试时工作区：

```text
 M AI_Judgement_Prototype/reports/day1_sample_report.md
 M AI_Judgement_Prototype/src/main.py
?? AI_Judgement_Prototype/cases/case_knife_001.json
?? AI_Judgement_Prototype/cases/case_knife_001_notes.md
?? Docs/Week8Day1AdvanceFragmentRevisionReport.md
?? Docs/Week8Day1AdvanceTestReport.md
```

以上均为本轮预期修改。

当前尚未 Commit / Push。

---

## 8. 遇到的问题

### 8.1 原 Fragment 过度绑定

#### 表现

旧版每条 Fragment 不只是一个事实，而是同时携带：

- 人物关系；
- 职业暗示；
- 金额对应；
- 事件顺序；
- 动机；
- 行为解释。

五条放在一起后，很容易自动组成一个“地下救治 / 包庇逃犯”的故事。

#### 原因

初版设计更接近一个完整手工案件，而不是未来词库中的可替换条目。

#### 解决

将每条 Fragment 缩减为单一可观察事实，并删除：

- 明确职业；
- 明确药费；
- 明确亲属；
- 明确受雇；
- 详细工具用途；
- 完整因果。

### 8.2 Sample Report 引用失效事实

#### 表现

Fragment 修订后，旧报告仍然提到：

- 白色外衣；
- 烧毁衣物；
- 明确职业。

#### 原因

测试输入没有随 Case Version 同步更新。

#### 解决

将 Sample Report 改写为只引用 v0.2 中实际存在的材料，并把职业、交易、胁迫等内容降级为可能解释，而非事实。

### 8.3 Fragment 语义密度不均

#### 表现

虽然五条 Fragment 字数都较短，但提供的可推断信息量并不一致：

- `Identity01` 提供的身份支撑较弱；
- `Relationship01` 没有明确关系对象；
- `Action01` 只确认有血痕；
- `Motive01` 的“金额不多”最为模糊；
- `Contradiction01` 同时引入“他们”“还活着”“走不了”，承担最多叙事信息。

#### 原因

“短句”不等于“低指向性”。一条短句仍然可能隐藏人物、因果、时间和事件框架。

#### 当前处理

不在本次 Advance 中继续无限重写。

将其保留为后续 AI 测试中的重要观察项：

- 模型是否过度依赖 `Contradiction01`；
- 模型是否忽略低信息 Fragment；
- 玩家报告是否总围绕同一种事件框架；
- 随机词库是否需要语义强度字段。

### 8.4 三种处置覆盖不平衡

#### 表现

当前材料最自然导向：

```text
detain_for_review
```

原因：

- 没有足够明确的善意事实支持直接轮回；
- 没有足够明确的严重伤害支持散魂；
- 同时又存在血痕和隐瞒话语，无法轻易排除疑点。

#### 影响

大量玩家可能选择“收押复审”作为安全答案，导致三种处置的玩法差异不足。

#### 当前处理

本轮不通过增加明确凶案或明确善行强行平衡。

后续应增加：

```text
处置覆盖审核
+
信息预算审核
```

观察不同报告能否在不虚构事实的前提下，为轮回、复审和严厉处置提出不同强度的论证。

---

## 9. 尚未解决的问题

本次没有阻塞性技术问题。

仍需后续验证：

- 当前五条 Fragment 是否给玩家足够的写作支点；
- `Motive01` 是否过于空泛；
- `Contradiction01` 是否过于主导叙事；
- `reincarnate` 和 `dissolve` 是否缺乏材料入口；
- AI 是否会把“济”自动解释为医疗；
- AI 是否会把“他们”自动解释为差役或追兵；
- AI 是否会编造血迹来源；
- AI 是否能区分“可能”与“已经证明”；
- 当前 Case 是否适合作为真实 AI 第一次测试；
- 未来词库是否需要额外记录语义指向强度。

---

## 10. 下一步

下一阶段进入原计划 Day3：

```text
Judge Persona
+
Output Contract
+
Schema Validation
```

主要任务：

1. 创建正式 `Judge.Clerk.001`；
2. 冻结 Judge Persona Version；
3. 创建 `judgement_result_v0_1.json`；
4. 固定所有枚举；
5. 限制数组长度；
6. 限制文本字段长度；
7. 创建本地数据模型；
8. 实现 `response_validator.py`；
9. 使用手写合法 JSON 测试通过；
10. 使用缺字段、非法枚举、错误 FragmentID、错误 DispositionID、超长文本和重复 ID 测试拒绝。

在进入真实 DeepSeek API 调用前，本地程序必须先能区分：

```text
合法 Judgement Result
和
不可进入正式实验记录的错误结果
```

---

# 11. System Understanding Review

## 11.1 今天新增的数据由谁拥有？

### Case 数据

由游戏设计层拥有，包括：

- Case ID；
- Case Version；
- Premise；
- FragmentID；
- Fragment Category；
- Fragment Text；
- Allowed Dispositions。

AI只能读取，不能修改。

### Case Notes

由开发与实验设计层拥有。

它们记录：

- 为什么这样设计；
- 哪些解释方向可被测试；
- 哪些内容不是隐藏答案；
- 当前风险是什么。

Notes 不属于玩家可见案件事实，也不应发送给模型作为正确答案。

### Sample Report

由测试输入层拥有。

它模拟玩家提交内容，用于验证数据流；未来会被正式测试语料替代。

## 11.2 哪些内容是固定游戏规则？

- 案件没有隐藏真相；
- Fragment 是唯一合法事实来源；
- 玩家可以解释，但不能伪造证据；
- AI不能新增 Fragment；
- AI不能把人工解释方向当作标准答案；
- Disposition 只能来自合法列表；
- Case Version 必须可追溯；
- 玩家可见 Fragment 修改后必须更新版本；
- AI不负责奖励和经济结算。

## 11.3 哪些内容交给 AI？

未来 AI 可以：

- 提取玩家核心主张；
- 判断玩家使用了哪些 Fragment；
- 判断玩家如何解释身份、动机与关系；
- 识别无依据假设；
- 评估是否处理矛盾；
- 区分证据约束与修辞效果；
- 生成判官回应；
- 生成归档摘要。

AI不能决定某个解释是否符合隐藏真相，因为隐藏真相不存在。

## 11.4 哪些 AI 输出必须由程序重新验证？

- `case_id`
- `judge_profile_id`
- `recognized_disposition_id`
- `used_fragment_ids`
- `unsupported_assumptions`
- `contradiction_handling.level`
- 各评价枚举
- `style_tags`
- 数组长度
- 重复 ID
- 文本长度
- 必需字段
- 禁止的额外字段
- Schema Version

尤其是 `used_fragment_ids`：模型认为自己引用了某条 Fragment，并不代表该 ID 真的存在。

## 11.5 今天的实现中，哪里可能发生模型漂移？

- “济”字是否被解释为医疗；
- 血痕是否被自动等同于伤人；
- 少量钱是否被解释为报酬；
- “他们”被解释成何种群体；
- “还活着”的对象被解释成受害者、同伙或被救者；
- `Motive01` 是否被忽略；
- `Contradiction01` 是否获得过高权重；
- 相同报告的 Fragment 映射是否一致；
- 对 `detain_for_review` 的评价是否过度宽松。

## 11.6 哪个测试能够证明当前假设是错的？

以下结果会说明当前 Case 或核心玩法需要重新设计：

- 玩家无法仅凭这五条材料写出不同解释；
- 所有合理报告都只能选择复审；
- AI持续把“济”认定为医生或医馆；
- AI持续虚构血迹来源；
- AI无法区分可能性表达和确定事实；
- AI忽略四条低信息 Fragment，只围绕矛盾句作答；
- 严谨报告和纯粹脑补得到相同证据评价；
- 玩家认为材料太空，无法形成值得提交的判词；
- Judge Response 只能重复“证据不足”。

## 11.7 如果更换模型，哪些部分应该保持不变？

- `Case.Knife.001`
- Case Version
- 五个 FragmentID
- Fragment Text
- Fragment Category
- Allowed Dispositions
- Judge Persona
- Prompt Version
- Schema Version
- Validator
- Sample / Test Reports
- Raw / Validated Result 文件结构
- 程序评分规则
- 存档字段

模型名称、API 参数和调用实现可以变化，但不能改变案件事实与输出契约。

## 11.8 当前代码是否为了未来扩展而过度设计？

没有。

本次只完成：

```text
1 个固定 Case
+
5 条 Fragment
+
1 份 Case Notes
+
1 份修订后的 Sample Report
+
现有 CLI 数据流接入
```

尚未提前开发：

- Fragment Library；
- 随机抽取器；
- Seed；
- 程序化案件生成；
- 多 Provider Adapter；
- 多判官；
- 正式评分；
- Batch Runner；
- Web UI；
- Unreal API 接入。

虽然设计备注已经开始考虑未来词库，但代码仍保持在当前实验所需的最小范围。

---

## 12. Git 提交前检查

建议在项目根目录执行：

```powershell
git status --short
git add AI_Judgement_Prototype Docs
git status --short
git diff --cached --name-only
git diff --cached
```

重点确认：

- `.env` 未进入 staged files；
- `.venv/` 未进入 staged files；
- API Key 未进入任何源码、Markdown、JSON 或日志；
- `case_knife_001.json` 的版本为 `0.2`；
- Sample Report 不再引用已删除的 Fragment；
- `main.py` 只切换到正式 Case 路径；
- 测试报告内容与实际运行结果一致。

推荐提交信息：

```text
Week8 Day1 Advance: add fixed knife case and fragment audit
```

提交命令：

```powershell
git commit -m "Week8 Day1 Advance: add fixed knife case and fragment audit"
git push origin feature/ai-first-prototype
```

---

## 13. 最终状态

```text
[PASS] 正式固定案件建立
[PASS] Case ID 稳定
[PASS] Case Version 更新为 0.2
[PASS] 五类 Fragment 完整
[PASS] FragmentID 顺序正确
[PASS] Fragment 改为短事实
[PASS] 无隐藏标准答案
[PASS] 支持多种解释方向
[PASS] Sample Report 与当前 Case 同步
[PASS] JSON 语法有效
[PASS] CLI 加载成功
[PASS] Fragment Count = 5
[PASS] Disposition Count = 3
[PASS] 未调用真实 AI
[PASS] 当前修改已记录
[WATCH] Fragment 语义密度不均
[WATCH] 处置结果偏向复审
[READY] Git Commit / Push
[READY] Week8 原计划 Day3
```
