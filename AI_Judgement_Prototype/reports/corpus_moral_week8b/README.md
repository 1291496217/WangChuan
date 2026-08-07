# Corpus.MoralJudgement.Week8B.001 v0.1

本目录含 28 份结构合法、语义质量刻意多样的人工报告。Human Labels 仅存于 Manifest，既不是隐藏真相，也绝不发送给 AI。

本地策略采用 **C with structural minimum**：只拒绝结构无效、字段越界、Key Fragment 所属错误、空白/极端长度；薄弱、草率、高修辞或对抗性的可提交报告留给未来 AI 评价。


## Human Label Role Semantics

`expected_fragment_roles` 描述 Fragment 在**这份玩家报告实际论证中的角色**，不是 Case 的客观真相，也不要求只覆盖 `SelectedKeyFragmentIDs`。

因此：

- 正文实质使用但未列为 Key 的合法 Fragment 也可以出现在 Human Label 中；
- `core_support` 表示它实际支撑该报告的主要论证；
- `counterevidence` 表示它构成该报告必须面对的反向材料；
- `context` 表示它改变解释但不是主要结论支点；
- `mentioned_unresolved` 表示报告提到但没有解决；
- `explicitly_excluded` 表示报告明确把该材料排除或忽略。

这一区分用于后续 Semantic Audit，绝不发送给 AI。
