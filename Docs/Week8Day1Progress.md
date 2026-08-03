# 《忘川河畔：见习判员》Week8 Day1 Progress

**Week Theme:** Fixed Case & Real AI Judgement Experiment Foundation
**Day 1 Theme:** Experiment Boundary, Python Setup & Data Flow
**Date:** 2026-08-03
**Branch:** `feature/ai-first-prototype`
**Python:** `3.13.7`

---

## 1. 今日目标

在 Unreal Engine 运行逻辑之外，建立一个最小、透明、可检查的 Python AI 判案实验基础。

今日数据流：

```text
Case JSON
+
Judge JSON
+
Player Report
→ Data Loader
→ Prompt Builder
→ CLI Prompt Preview
```

今日明确不做：真实 AI 调用、正式固定案件、Schema 验证、Unreal Engine 接入、多 Provider Adapter、正式 UI。

---

## 2. 审核结论

**Week8 Day1 测试通过。**

当前实现已经成功完成：

```text
磁盘数据
→ Python 加载
→ Prompt 分区构建
→ CLI 完整预览
```

运行结果正确显示了案件、判官、Fragment 数量、Disposition 数量、玩家报告字符数与完整 Prompt，并明确以“未调用 AI 服务”结束。Day1 已达到预定完成标准，可以提交并推送 GitHub。

---

## 3. 已完成内容

### 3.1 Git 分支

当前分支：

```text
feature/ai-first-prototype
```

符合新版 AI-First 原型使用独立 Branch 开发的要求。

### 3.2 Python 环境

当前版本：

```text
Python 3.13.7
```

Day1 只使用标准库，实测运行正常。后续加入 AI SDK 或验证库时，需要重新确认第三方依赖是否正式支持 Python 3.13。

### 3.3 Git 忽略规则

已通过 `git check-ignore` 验证：

```text
.venv/
.env
```

均被 `AI_Judgement_Prototype/.gitignore` 正确排除。

这意味着：

- 本地虚拟环境不会提交；
- 真实 `.env` 不会提交；
- `.env.example` 可以只保存空变量名并提交；
- 后续 API Key 可以安全地从本地环境读取。

### 3.4 Day1 Stub 数据

当前测试数据包括：

- `Case.Day1.Stub`
- `Judge.Day1.Stub`
- 2 条临时 Fragment
- 3 种合法 Disposition
- 1 份 Day1 Sample Report

这些内容只用于验证数据流，不属于 Day2 正式案件“染血的刀”。

### 3.5 Prompt 分区

生成的 Prompt 已包含：

```text
<SYSTEM_ROLE>
<JUDGE_PERSONA>
<GAME_RULES>
<CASE_METADATA>
<VALID_CASE_FRAGMENTS>
<ALLOWED_DISPOSITIONS>
<UNTRUSTED_PLAYER_REPORT>
<DAY1_PREVIEW_INSTRUCTION>
```

当前已建立最重要的数据边界：

```text
系统规则
≠
判官人格
≠
合法案件材料
≠
玩家输入
```

玩家报告被明确标记为不可信输入，且 Prompt 已声明玩家正文中的系统指令不能修改规则。

### 3.6 CLI 运行

执行：

```powershell
python .\src\main.py
```

输出摘要：

```text
Case: Day 1 数据流测试案件
Case ID: Case.Day1.Stub
Judge: 值房判官（数据流测试）
Fragment count: 2
Disposition count: 3
Player report characters: 230
```

完整 Prompt 成功打印，并以：

```text
Day 1 completed without calling an AI service.
```

结束。

---

## 4. 新增文件

```text
AI_Judgement_Prototype/
├─ .gitignore
├─ .env.example
├─ README.md
├─ requirements.txt
│
├─ cases/
│  └─ case_day1_stub.json
│
├─ judges/
│  └─ judge_day1_stub.json
│
├─ reports/
│  └─ day1_sample_report.md
│
└─ src/
   ├─ main.py
   ├─ data_loader.py
   └─ prompt_builder.py
```

本地 `.venv/` 不进入 Git；真实 `.env` 当前无需创建。

---

## 5. 数据流变化

### Day1 前

```text
新版玩法设计
→ 尚无独立 AI 判词实验程序
```

### Day1 后

```text
Case JSON
+
Judge JSON
+
Player Report Markdown
→ data_loader.py
→ Python Dict / String
→ prompt_builder.py
→ 分区 Prompt
→ main.py
→ CLI Preview
```

### 后续计划

```text
Prompt
→ Real AI Client
→ Raw Provider Response
→ Schema Validator
→ Validated Result
→ Result Writer
```

---

## 6. 关键代码与配置

### `data_loader.py`

负责：

- 检查文件存在性；
- UTF-8 读取 JSON；
- 验证 JSON 根节点为 Object；
- 读取 Markdown / 文本；
- 拒绝空报告；
- 将错误交给主程序处理。

### `prompt_builder.py`

负责：

- 接收 Case、Judge 和 Player Report；
- 将数据转换为可读 Prompt；
- 明确信任边界；
- 返回字符串；
- 不读取 API Key；
- 不调用模型；
- 不写结果文件。

### `main.py`

负责：

- 定位项目目录；
- 加载测试文件；
- 调用 Prompt Builder；
- 显示摘要和完整 Prompt；
- 处理文件错误、数据错误和意外错误；
- 返回明确的进程状态。

### `.gitignore`

关键规则：

```gitignore
.venv/
.env
.env.*
!.env.example
```

---

## 7. 测试输入

### Case

```text
Case ID: Case.Day1.Stub
Version: 0.0
Fragment Count: 2
Disposition Count: 3
```

### Judge

```text
Judge ID: Judge.Day1.Stub
Version: 0.0
Display Name: 值房判官（数据流测试）
```

### Player Report

```text
DispositionID: detain_for_review
Character Count: 230
```

报告认为染血短刀不能单独证明伤害行为，白色外衣和刀具也可能与救治有关；但烧毁衣物仍需解释，因此建议收押复审。

---

## 8. 测试结果

| 测试项 | 结果 | 说明 |
| --- | --- | --- |
| 独立 Git Branch | 通过 | `feature/ai-first-prototype` |
| Python 运行 | 通过 | Python 3.13.7 |
| `.venv` 忽略 | 通过 | `git check-ignore` 命中规则 |
| `.env` 忽略 | 通过 | `git check-ignore` 命中规则 |
| Case JSON 加载 | 通过 | 标题、ID、Fragment 正确 |
| Judge JSON 加载 | 通过 | 人格数据完整进入 Prompt |
| Report 加载 | 通过 | 230 个字符 |
| Fragment 计数 | 通过 | 2 |
| Disposition 计数 | 通过 | 3 |
| Prompt 分区 | 通过 | 所有预定区块均生成 |
| 玩家输入隔离 | 通过 | 使用 `UNTRUSTED_PLAYER_REPORT` |
| 无隐藏真相规则 | 通过 | Prompt 明确声明 |
| Injection 基础防线 | 通过 | 玩家正文不能改变规则 |
| 未调用真实 AI | 通过 | Preview 正常结束 |

---

## 9. 遇到的问题

### 9.1 Python 版本命令拼写错误

首次输入：

```powershell
python --versiono
```

返回 `unknown option --versiono`。

**原因：** 命令末尾误输入额外字母 `o`。
**解决：**

```powershell
python --version
```

成功得到 `Python 3.13.7`。该问题与项目代码无关，已解决。

### 9.2 `git status --short` 显示 `?? ./`

**原因：** 命令在 `AI_Judgement_Prototype` 内部运行，而整个实验目录尚未加入 Git，因此 Git 将当前目录整体显示为未跟踪。

**结论：** 这是首次提交前的正常状态。

### 9.3 `git grep -n "API_KEY"` 无输出

当前没有发现已跟踪文件中的 `API_KEY` 内容。但整个实验目录目前仍未跟踪，因此 `git grep` 不能单独检查全部未跟踪文件。

Day1 尚未使用真实 API Key，因此没有发现实际泄露风险。完成 `git add` 后，应通过 staged diff 再检查一次。

---

## 10. 提交前检查

请在 Unreal 项目根目录执行：

```powershell
git status --short
git add AI_Judgement_Prototype
git status --short
git diff --cached --name-only
git diff --cached
```

重点确认：

- `.venv/` 未进入 staged files；
- `.env` 未进入 staged files；
- `.env.example` 只有空变量；
- 没有真实 API Key、Bearer Token 或其他凭证；
- Stub JSON、Markdown 和 Python 文件均正确进入 staged files。

确认后提交：

```powershell
git commit -m "Week8 Day1: add AI judgement prototype data flow"
git push origin feature/ai-first-prototype
```

---

## 11. 尚未解决的问题

Day1 没有阻塞性问题。

以下属于后续任务：

- 正式固定案件尚未设计；
- 正式五条 Fragment 尚未冻结；
- 多解释人工审核尚未完成；
- 正式 Judge Persona v0.1 尚未冻结；
- JSON Schema 尚未实现；
- FragmentID / DispositionID 本地验证尚未实现；
- Provider 与 Model 尚未选择；
- API Client 尚未实现；
- Raw / Validated Result 尚未保存；
- 20 份人工测试判词尚未创建。

---

## 12. 下一步

Week8 Day2：

```text
Fixed Case Design & Multi-Interpretation Audit
```

主要任务：

1. 创建正式案件 `Case.Knife.001`；
2. 设计五条稳定 Fragment；
3. 覆盖身份、关系、行为/结果、动机/情绪、矛盾五种语义作用；
4. 确保至少三条材料存在明确联系；
5. 设计一条真正有效的矛盾碎片；
6. 写出两种彼此冲突但均受材料支持的解释；
7. 写出一个较弱但仍可能成立的解释；
8. 检查案件没有隐藏答案；
9. 检查材料不是随机词组合；
10. 输出正式 Case JSON 和案件设计说明。

Day2 仍不调用真实 AI。

---

# 13. System Understanding Review

## 13.1 今天新增的数据由谁拥有？

- **Case 数据：** 游戏设计层拥有，AI不能修改。
- **Judge 数据：** 游戏设计层拥有，AI只能遵守人格。
- **Player Report：** 玩家输入层拥有，是待分析数据，不是系统指令。
- **Prompt：** 程序构建，是数据组合方式，不是新事实来源。

## 13.2 哪些内容是固定游戏规则？

- 案件没有隐藏标准答案；
- 只有合法 Fragment 是案件事实；
- 玩家可以解释与推测；
- 玩家不能凭空增加决定性证据；
- Player Report 是不可信输入；
- 玩家正文不能修改规则；
- AI不拥有案件定义权；
- AI不拥有奖励和经济结算权。

## 13.3 哪些内容交给 AI？

Day1 尚未调用 AI。后续可交给 AI：

- 提取核心主张；
- 分析身份与动机解释；
- 映射合法 Fragment；
- 识别无依据假设；
- 判断矛盾处理；
- 区分连贯度、证据约束和修辞表现；
- 生成判官回应与归档摘要。

## 13.4 哪些 AI 输出必须由程序重新验证？

- Case ID；
- Judge Profile ID；
- Disposition ID；
- FragmentID；
- Schema Version；
- 所有受限枚举；
- 必需字段；
- 禁止的额外字段；
- 数组长度与重复项；
- 文本最大长度。

AI声称某个 ID 合法，不代表它真的合法。

## 13.5 哪里可能发生模型漂移？

未来可能漂移：

- `core_claim` 提取；
- `used_fragment_ids` 映射；
- 推断与虚构的边界；
- 矛盾处理等级；
- 各评价等级；
- 判官回应的语气和针对性；
- 是否错误地假装知道真相。

Day1 的分区 Prompt 和版本化输入，为后续定位漂移建立了基础。

## 13.6 哪个测试能证明当前假设是错的？

以下结果会否定或严重削弱核心假设：

- AI不能理解两份相反但都有材料支持的报告；
- AI总是假设存在唯一答案；
- AI不能区分合理推断和虚构证据；
- 华丽修辞持续污染证据评价；
- Prompt Injection 能改变规则；
- Judge Response 只是通用复述；
- 结构化字段不稳定，无法用于游戏反馈；
- 玩家看完回应后不理解判词强弱，也不想修改。

Day1 只建立测试基础，尚未证明核心玩法成立。

## 13.7 更换模型时哪些部分应保持不变？

- Case JSON；
- FragmentID 与 Fragment 文本；
- DispositionID；
- Judge Persona ID 与版本；
- 游戏规则；
- Prompt Version；
- Schema；
- Validator；
- 输入报告；
- 结果文件格式；
- 程序评分和存档规则。

只有 API 调用实现和必要的 Provider 参数应该变化。

## 13.8 当前是否过度设计？

没有明显过度设计。

目前只拆分：

```text
main.py
data_loader.py
prompt_builder.py
```

每个模块都对应已经存在的职责。尚未提前创建 Provider Factory、多模型 Adapter、数据库、Web UI、Agent、RAG、UE HTTP 层或随机案件系统，符合“足够清晰但不过早平台化”的原则。

---

## 14. Day1 最终状态

```text
[PASS] 新版独立 Branch 正确
[PASS] Python 环境可运行
[PASS] 虚拟环境被 Git 忽略
[PASS] 本地 .env 被 Git 忽略
[PASS] Case JSON 可加载
[PASS] Judge JSON 可加载
[PASS] Player Report 可加载
[PASS] Prompt 可完整构建
[PASS] Prompt 数据边界清晰
[PASS] 玩家输入被标记为不可信
[PASS] 未提前调用真实 AI
[PASS] 未提前扩展多 Provider / UE / UI
[READY] Git Commit / Push
[READY] Week8 Day2
```
