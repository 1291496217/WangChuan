# Case.DoorKnife.001 —《门后的刀》案件设计说明

**Case Version:** `0.1`
**用途：** Week8 Day3 Revised；冻结 Week8 旧基线，检验局部事实能否支持开放善恶初判。
**设计状态：** 设计稿，不进入现有 AI Corpus，不提交真实调用。

## 1. Design Intent

本案只确认六条局部事实，不提供完整人生答案、宅主动机、亡魂与孩子的确定关系，也不指定哪一种善恶判断或处置是标准答案。开放点集中在：杀人行为、长期心念、他人评价、孩子逃出、孩子陈述和亡魂死亡后果之间如何连接。

玩家可以把同一组材料组织为伪装的恶人、被迫反击的保护者、善恶混合的复仇者，或高修辞但严格守住材料边界的判词。Interpretation Hooks 是写作入口，不是隐藏真相。

## 2. Confirmed Local Facts

以下事实由 Case JSON 的六条 Fragment 直接提供，不能在 Notes 中进一步扩写：

1. 亡魂把短刀刺入宅主胸口，宅主当场死亡。
2. 邻里评价亡魂低声寡言、遇争执退让、胆小怕事。
3. 亡魂在许多个夜里想象过宅主死去。
4. 宅主倒下后，后屋孩子从门中逃出。
5. 孩子说亡魂答应过不会再让宅主进来。
6. 亡魂死在土地庙外，背上有新伤，手中握着后屋门锁钥匙。

## 3. Intentionally Unknown

以下内容没有被 Case 确认，也不是等待揭开的隐藏标准答案：

- 宅主为何锁住孩子；
- 亡魂与孩子的确切关系；
- 亡魂杀人的主要动机是救人、复仇、私怨、恐惧还是其他原因；
- 夜间想象是预谋、恐惧、仇恨、无力幻想还是混合状态；
- 亡魂背伤由谁造成，亡魂为何死在土地庙外；
- 钥匙何时取得、是否实际用于开门；
- 亡魂是否长期伪装性格，邻里是否真正理解亡魂；
- 哪一种善恶初判或处置是“正确答案”。

## 4. Fragment Semantics

| FragmentID | SemanticType | SourceType | InformationWeight | 语义边界 |
| --- | --- | --- | ---:| --- |
| `DoorKnife.Action01` | action | objective_trace | 3 | 确认杀人行为与死亡结果，不确认动机或责任程度。 |
| `DoorKnife.Personality01` | personality | others_testimony | 2 | 邻里他识，不是系统认证的人格，也不是善恶证据。 |
| `DoorKnife.Thought01` | thought | soul_self_knowledge | 2 | 确认反复想象，不等于已经预谋或实施行动。 |
| `DoorKnife.Outcome01` | outcome | objective_trace | 3 | 确认孩子逃出，不自动证明杀人动机善良。 |
| `DoorKnife.Relationship01` | relationship | others_testimony | 2 | 孩子陈述与承诺，不确定孩子身份或双方关系。 |
| `DoorKnife.Death01` | death | objective_trace | 3 | 确认死亡地点、背伤和钥匙，不确认追击者、经过或赎罪。 |

特别注意：`Personality01` 是邻里的他识，`Thought01` 是亡魂的心念，`Outcome01` 是实际结果，`Death01` 是终局残痕。四者都不能被压缩成一个“善人”或“恶人”标签。

## 5. Interpretation Hooks

### A — 伪装的恶人

- 可将 `Personality01` 解读为长期伪装或邻里误判；
- 将 `Thought01` 作为长期恶意或预谋支点；
- 承认 `Action01` 与 `Outcome01`，但指出孩子获救不能洗去杀人责任；
- 不编造宅主具体恶行、孩子身份或追击者；
- 可支持 `more_evil_than_good` 或 `beyond_redemption`，基础处置以 `send_to_prison` 为材料支点。

### B — 被迫反击的保护者

- 承认杀人行为与反复死亡想象；
- 将 `Personality01` 解读为长期受迫、恐惧或邻里误解的上下文；
- 用 `Outcome01`、`Relationship01`、`Death01` 组织保护路线；
- 不把宅主直接写成虐待者，除非明确标为推断；
- 承认杀人责任仍然存在；
- 可支持 `more_good_than_evil` 或 `mixed_merit_and_fault`，处置支点可为 `recommend_rebirth` 或 `ordinary_transfer`。

### C — 善恶混合的复仇

- 同时承认保护结果、私人怨恨与长期心念；
- 不把行为写成纯粹无私，也不把孩子逃出写成纯粹偶然；
- 说明功过为何难分，以及为什么 `ordinary_transfer` 比极端处置更能保留争议；
- 可支持 `mixed_merit_and_fault` 与 `ordinary_transfer`，也可以基于强烈责任论证 `send_to_prison`。

### D — 高修辞但守材料边界

- 允许使用有气势的官署语言、比喻和强烈立场；
- 必须区分已知事实、合理推断与新编事实；
- 不得补写证人、旧案、血缘、宅主罪行或具体追击经过；
- 用于检验修辞强度不等于证据强度。

## 6. Disposition Space

基础处置至少有三条真实材料路线：

- `recommend_rebirth`：主要依靠 `Outcome01`、`Relationship01` 与 `Personality01` 形成保护或善意结果解释，同时必须正面处理 `Action01` 和 `Thought01`。`Death01` 只能作为未决后果背景，不是直接荐生证据。
- `ordinary_transfer`：依靠 `Action01`、`Thought01`、`Outcome01`、`Death01` 形成功过并存、责任与保护结果并存的中间路线。
- `send_to_prison`：依靠 `Action01` 与 `Thought01` 形成明确杀人责任，并用 `Outcome01` 作为不能自动洗罪的反证。

`soul_dissolution` 仅为极端测试处置。六条 Fragment 不要求为它提供与三种基础处置同等强度的支点；若玩家选择该处置，审计应重点观察是否存在越界确定性，而不是把它设计成隐藏答案。

## 7. Design Risks

- `Action01` 与 `Death01` 权重较高，可能压过其他解释；
- `Thought01` 可能被误读为预谋的确定证据；
- `Personality01` 可能被误读为无罪或伪装的确定证据；
- `Outcome01` 可能被当成自动洗罪；
- `Relationship01` 可能诱发玩家补全孩子身份；
- `Death01` 可能诱发玩家补全追击、逃亡、报复或牺牲经过；
- `ordinary_transfer` 可能成为“最安全答案”；
- `soul_dissolution` 可能沦为没有论证要求的极端按钮；
- 六条材料合并后可能过度暗示“亡魂保护孩子”的单一路线。


## 8. Runtime Visibility Boundary

以下字段属于 Case Design / Audit Metadata：

```text
interpretation_hooks
disposition_support_tags
information_weight
relation_tags
```

它们不是案件中的新增事实，也不是玩家或 AI 应该看到的“标准答案提示”。

后续进入 Prompt、Corpus 或 UE Runtime 前必须明确：

- Fragment `text` 与合法 ID 可以作为案件材料；
- `semantic_type`、`source_type` 可用于结构解释与程序验证；
- `interpretation_hooks` 不应发送给 AI，也不应展示给玩家；
- `disposition_support_tags` 不应发送给 AI，也不应参与自动判罚；
- `information_weight` 只用于设计审计，不是善恶、可靠性或奖励分；
- `relation_tags` 用于结构与组合审计，不能被解释成事件已经发生。

若未来运行时把这些设计字段完整暴露给模型，模型可能把设计师的备选路线当成隐藏答案。

## 9. Day4 Questions

- 新 Case 是否需要独立结构校验工具，而不是改造 Day3 的设计稿？
- `InformationWeight` 是否足以解释不同 Fragment 的叙事负担？
- `post_killing_aftermath` 是否保持中立，没有把追击或牺牲写成既定事实？
- 删除任一 Fragment 后，是否仍保留至少两种可辩护解释？
- 如何程序化区分“事实支持处置”和“玩家声称处置”？
- `Personality01`、`Thought01` 和 `Outcome01` 的语义边界能否被未来 Validator 明确检查？
- 是否需要把 `soul_dissolution` 排除出常规 Corpus？
