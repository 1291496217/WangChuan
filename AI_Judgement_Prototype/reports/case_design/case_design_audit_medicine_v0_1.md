# Case.Medicine.001 — Case Design & Information Budget Audit v0.1

**Audit scope:** Week8 Day3 Revised Advance design-only review；无 AI 调用、无 Prompt/Schema/Parser 修改。
**Source:** cases/case_medicine_001.json and cases/case_medicine_001_notes.md
**Design verdict:** Concept PASS；非杀人主题、真实善果、第三人代价和三种基础处置均有独立材料支点，适合进入结构验证与后续 Corpus 设计。

## 1. Concept Audit

| Question | Finding |
|---|---|
| 核心行为明确吗？ | 是：亡魂取走最后一包定喘散，没有登记，也没有留下药钱。 |
| 是否无杀人行为？ | 是：没有死亡事实，不依赖终局反转。 |
| 是否有有限利益损害？ | 是：药铺失去最后一包药，另一名病人未能得到该药。 |
| 是否有真实善果？ | 是：孩子服药后当夜呼吸缓解。 |
| 是否有资源冲突？ | 是：最后一包药面对不同求药者。 |
| 是否有第三人代价？ | 是：另一名病人回村途中再次发作。 |
| Personality 是否改变解释？ | 是：过去诚信既可支持一次紧急越界，也可加重受信任身份下的违背。 |
| Thought 是否表达知情而非恶意？ | 是：亡魂知道稀缺并想到可能有其他求药者，但没有希望他人受害的事实。 |
| Relationship 是否支持非亲属善意？ | 是：母亲明确双方并非亲故，承诺提供善意支点但不授予取药权。 |
| 三种基础处置是否有支点？ | 是：荐生、平籍、押狱均有材料；魂灭仅作极端测试。 |
| 平籍是否不只是“证据不足”？ | 是：它同时评价善果、越界、知情选择、信任和第三人代价。 |
| 押狱是否可成立？ | 是：知情占用、信任违背和第三人代价构成责任路线，但不能升级为故意伤害。 |
| 荐生是否可成立？ | 是：非亲属救助、过去诚信和真实善果可构成减责路线，但必须承认规则与资源责任。 |
| 是否需要死亡反转？ | 不需要：事件与后果已构成完整的局部道德冲突。 |
| 是否存在唯一答案 Fragment？ | 不存在：六条 Fragment 互相施加约束，没有一条自动决定处置。 |

## 2. Six Fragment Information Budget

| FragmentID | SemanticType | SourceType | Weight | Primary Fact | Independent Value | Main Risk | Disposition Relevance |
|---|---|---|---:|---|---|---|---|
| Medicine.Action01 | action | objective_trace | 3 | 取走最后一包药、未登记、未付款 | 建立明确规则越界 | 被写成永久占有或纯粹恶意 | ordinary_transfer, send_to_prison |
| Medicine.Outcome01 | outcome | objective_trace | 3 | 孩子服药后呼吸缓解 | 建立真实善果 | 自动洗白取药 | recommend_rebirth, ordinary_transfer |
| Medicine.Outcome02 | outcome | objective_trace | 3 | 另一病人未得药并再次发作 | 建立第三人实际代价 | 被补成完整医学因果或故意伤害 | ordinary_transfer, send_to_prison |
| Medicine.Personality01 | personality | others_testimony | 2 | 三年诚信、从未私取药材 | 改变信任与例外解释 | 被当作无罪证明或长期伪装证明 | all three base dispositions |
| Medicine.Thought01 | thought | soul_self_knowledge | 3 | 知道药品稀缺并想到其他求药者 | 建立知情选择责任 | 被升级为希望他人受害 | ordinary_transfer, send_to_prison |
| Medicine.Relationship01 | relationship | others_testimony | 2 | 非亲属送药承诺 | 建立善意与照护动机 | 被补成血缘或取药授权 | recommend_rebirth, ordinary_transfer |

Weight 表示叙事信息负载，不是善恶分、可靠度、奖励或自动处置分。

## 3. Three Defensible Interpretations

### A — 可宽恕的越界救助

**Primary Moral Judgement:** more_good_than_evil
**Primary Disposition:** recommend_rebirth

主要材料：

- Outcome01 的真实帮助结果；
- Relationship01 的非亲属照护承诺；
- Personality01 的长期诚信；
- Action01 作为必须承认的越界行为。

最低合理条件：

- 承认没有登记、没有付款和占用最后一包药；
- 承认亡魂知道资源稀缺；
- 承认另一名病人的实际代价；
- 说明善果和过去诚信构成减责，而非自动证明行为完全正当。

Counterevidence：

- Action01 的规则违反；
- Thought01 的知情；
- Outcome02 的第三人代价。

### B — 明知稀缺仍擅自分配

**Primary Moral Judgement:** more_evil_than_good
**Primary Disposition:** send_to_prison

主要材料：

- Action01 的明确越界；
- Thought01 的风险认知；
- Outcome02 的资源挤占和第三人代价；
- Personality01 带来的受信任身份责任。

最低合理条件：

- 指出亡魂明知最后一包药可能影响其他人；
- 指出他未经授权替药铺决定资源去向；
- 承认孩子确实得到帮助；
- 不得把另一病人的发作写成亡魂故意伤害；
- 不得把先前诚信改写成长期伪装。

Counterevidence：

- Outcome01 的善果；
- Relationship01 的非亲属承诺；
- Personality01 的过往记录。

### C — 善意、责任与资源代价并存

**Primary Moral Judgement:** mixed_merit_and_fault
**Primary Disposition:** ordinary_transfer

主要材料：

- Action01 的规则责任；
- Outcome01 的真实善果；
- Outcome02 的真实第三人代价；
- Thought01 的知情选择；
- Relationship01 的非亲属善意；
- Personality01 作为冲突背景。

最低合理条件：

- 同时评价目的、行为、规则、信任和后果；
- 解释为什么善意不等于无责；
- 解释为什么责任不等于恶意；
- 给出平籍的实质理由，而不是只说“我不确定”。

## 4. Failure Patterns

### D — 只看结果的功利开脱

推理链：Outcome01 → 孩子好转 → 取药必然正确。

该路线薄弱，因为它忽略未登记、最后一包药、Thought01 的风险认知和 Outcome02 的第三人代价，把善果自动等同于善意与正当性。它应作为 Day5 Corpus 的负面测试，不是正式合理解释。

### E — 只看规则的绝对归罪

推理链：未经登记取药 → 必然恶人 → 善果与救助动机全部无关。

该路线薄弱，因为它忽略孩子真实好转、非亲属承诺和先前诚信，并把规则违反升级为完整人格判断。它可以作为负面或极端测试，但不支持无依据魂灭。

## 5. Required Information Budget Questions

1. 核心行为足够明确，但没有提供永久占有动机。
2. Outcome01 是真实善果，不是自动正当性。
3. Outcome02 是真实代价，不是完整医学因果。
4. 没有杀人，也没有死亡反转。
5. Action01 单独足以建立规则责任。
6. Outcome01 单独足以改变道德解释，但不能关闭责任。
7. Outcome02 单独足以建立第三人代价，但不能证明故意伤害。
8. Personality01 改变信任解释，而非直接证明人格。
9. Thought01 增加责任，但不自动证明恶意。
10. Relationship01 提供非亲属善意，不提供取药权。
11. 最大道德张力来自 Action01、Outcome01、Outcome02 的三角关系。
12. 最容易被过度补全的是 Outcome02 的医学因果，以及 Relationship01 的私人关系。
13. 荐生最低支点是 Outcome01 + Relationship01 + Personality01，同时处理 Action01/Thought01/Outcome02。
14. 平籍的实质理由是善果、知情、规则和第三人代价并存。
15. 押狱最低支点是 Action01 + Thought01 + Outcome02，需承认 Outcome01/Relationship01 的反证。
16. 魂灭不进入正式合理 Corpus，只作 boundary_only 或 exclude。
17. ordinary_transfer 最可能成为安全选项，需要专门测试“我不确定”式空泛报告。
18. Failure D 是无脑功利开脱。
19. Failure E 是无脑规则归罪。
20. Matrix 不偷偷排序两名病人的生命价值。
21. Case 没有保存隐藏完整答案。
22. 删除任意一条后，仍应保留至少两条合理解释，但处置支点会发生变化。
23. Medicine 与 DoorKnife 的张力明显不同：前者是资源、规则与善果，后者是暴力、责任与保护结果。
24. Medicine 值得进入 Day5 Corpus，但应带有安全偏向、功利开脱和规则归罪测试。

## 6. Ablation Audit

| Removed Fragment | Weakened Reading | Remaining Interpretations | Independent Value | Disposition Impact |
|---|---|---|---|---|
| Action01 | 三条解释都失去明确行为责任 | B/C 明显变弱，A 也必须补回规则问题 | 最高责任锚点 | send_to_prison 失去最低直接支点 |
| Outcome01 | A 几乎失去真实善果支点，C 变得更偏责任 | B/C 仍可成立 | 独立的善果事实 | recommend_rebirth 显著变弱 |
| Outcome02 | B 的第三人责任和 C 的功过张力下降 | A/B/C 仍可讨论但更容易功利化 | 独立的资源代价事实 | send_to_prison 与 ordinary_transfer 反证结构减弱 |
| Personality01 | A 的过去诚信支点和 B 的信任责任减弱 | A/B/C 仍成立 | 改变行为解释，不是人格答案 | 三种处置仍可达成，但信任维度变薄 |
| Thought01 | B/C 的知情选择责任下降 | A/C 仍较强，B 更难成立 | 区分知情与恶意 | send_to_prison 需要更多依赖 Action/Outcome02 |
| Relationship01 | A 的非亲属善意支点减弱，C 更偏资源责任 | B/C 仍可成立 | 建立承诺与非亲属照护 | recommend_rebirth 的直接支点减少 |

Ablation 只用于设计审计，不修改正式 Case。

## 7. Runtime Metadata Boundary

interpretation_hooks、disposition_support_tags、information_weight 和 relation_tags 都是 Design / Audit Metadata：

- 不进入 Prompt；
- 不展示给玩家；
- 不作为运行时证据；
- 不转换为 Reward、自动评分或自动处置；
- 不证明某条解释正确。

Validator 只检查结构和元数据完整性，不检查案件是否有趣、解释是否正确或处置是否公平。

## 8. DoorKnife Comparison

| Dimension | DoorKnife | Medicine |
|---|---|---|
| Core action | 明确杀人 | 未经授权取药 |
| Main tension | 暴力责任与保护结果 | 善意、规则、资源和第三人代价 |
| Beneficial outcome | 孩子逃出 | 孩子呼吸缓解 |
| Third-party cost | 保护结果之外的后果不确定 | 另一病人未得药并再次发作 |
| Highest-weight fragments | Action01 / Outcome01 / Death01 | Action01 / Outcome01 / Outcome02 / Thought01 |
| Likely safe bias | ordinary_transfer | ordinary_transfer |
| Extreme route | soul_dissolution boundary test | soul_dissolution boundary test |

Medicine 不依赖死亡、凶手、血缘或最后证明文件，因此确实测试了不同类型的道德冲突。

## 9. Day5 Handoff

- 设计新版 Report Contract 时，不向 AI 传递本 Notes 或 Audit 的 Support Tags、Hooks 和 Weight。
- Corpus 需要加入“只看善果”和“只看规则”的负面报告。
- 需要单独测试 AI 是否擅自比较两名病人的生命价值。
- 需要检查平籍理由是否具体，而不是空泛的“无法确定”。
- 需要保持非杀人事实边界，不将第三人发作升级为故意伤害。
