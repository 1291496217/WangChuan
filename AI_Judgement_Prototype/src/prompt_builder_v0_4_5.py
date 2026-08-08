from __future__ import annotations

from pathlib import Path
from typing import Any

from prompt_builder_v0_4_4 import _pretty, build_prompt_v0_4_4

PROMPT_VERSION = "0.4.5"


def build_prompt_v0_4_5(
    *,
    case_data: dict[str, Any],
    judge_data: dict[str, Any],
    player_report: Any,
    schema_data: dict[str, Any],
) -> list[dict[str, str]]:
    """Balance Selected-Key hints with clause-anchored unselected recall."""
    messages = build_prompt_v0_4_4(
        case_data=case_data,
        judge_data=judge_data,
        player_report=player_report,
        schema_data=schema_data,
    )
    system = messages[0]["content"]
    user = messages[1]["content"]

    system = system.replace("PromptVersion: 0.4.4", f"PromptVersion: {PROMPT_VERSION}", 1)
    system = system.replace(
        "CorrectionType: selected-key de-anchoring and clause-to-fragment coverage correction",
        "CorrectionType: balanced selected-key hint and clause-anchored coverage correction",
        1,
    )

    old_deanchoring = """<SELECTED_KEY_DEANCHORING>
SelectedKeyFragmentIDs 不提供给模型，因为 v0.4.3 实测证明它会把“检索提示”变成角色分配白名单，压过自由文本中的语义使用。
角色判断必须只根据玩家自由文本与全部有效 Fragment 的语义对应关系。程序仍可在模型外保存 Selected Keys，但模型不得依赖它们决定 fragment_roles。
</SELECTED_KEY_DEANCHORING>"""
    balanced_hint = """<SELECTED_KEY_BALANCE>
SelectedKeyFragmentIDs 只是候选起点，既不是角色白名单，也不是使用证明。
先检查每个已选 Fragment 是否真的被自由文本使用；未使用就省略。随后逐句检查自由文本是否还使用了未选 Fragment；只有能指出对应玩家小句时才可新增。
新增未选 Fragment 的必要条件：玩家小句必须使用该 Fragment 独有的事实、后果、代价或冲突。仅仅因为某 Fragment 能让故事更完整、能支持一种动机、或出现在 Case 中，绝不能新增。
正例：已选列表没有“第三人缺药”Fragment，但玩家说“善果不能抹去资源代价”，且该 Case 只有该 Fragment 记载实际资源代价，则必须新增该 Fragment。
反例：玩家说“可能是保护行动”并提到孩子获救，不等于使用了 Case 中未提及的保护承诺、长期想法、受伤死亡或钥匙；这些 Fragment 必须省略。
反例：玩家只说资源代价，不等于提到行为人事前知情或预见；未明确谈思想、知情或预见时，Thought Fragment 必须省略。
</SELECTED_KEY_BALANCE>"""
    if old_deanchoring not in system:
        raise RuntimeError("Prompt v0.4.4 de-anchoring block not found")
    system = system.replace(old_deanchoring, balanced_hint, 1)

    system = system.replace(
        "1. 是否逐句扫过全部 Fragment，而不是只覆盖玩家显眼提到的几个；",
        "1. 是否逐句检查全部 Fragment，但只纳入能指向玩家具体小句的 Fragment；",
        1,
    )
    system = system.replace(
        "2. 概括性的后果、代价或冲突是否映射到其唯一对应 Fragment；",
        "2. 概括性的后果、代价或冲突是否映射到唯一对应 Fragment，同时没有借机补写其余 Case 事实；",
        1,
    )
    system = system.replace(
        "3. 是否错误地由结果补写了玩家未提到的 Thought、预见或动机；",
        "3. 每个未选但新增的 Fragment 是否都能指出玩家原文小句；是否错误地由结果补写了未提到的 Thought、关系、死亡细节、预见或动机；",
        1,
    )

    selection_line = f"SelectedKeyFragmentIDs: {_pretty(list(player_report.selected_key_fragment_ids))}\n"
    marker = f"DispositionID: {player_report.disposition_id}\n"
    if marker not in user:
        raise RuntimeError("Prompt v0.4.4 disposition marker not found")
    user = user.replace(marker, marker + selection_line, 1)
    return [{"role": "system", "content": system}, {"role": "user", "content": user}]


def build_prompt_file_v0_4_5(path: Path) -> str:
    return path.read_text(encoding="utf-8")
