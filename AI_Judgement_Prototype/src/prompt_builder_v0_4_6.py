from __future__ import annotations

from pathlib import Path
from typing import Any

from prompt_builder_v0_4_5 import build_prompt_v0_4_5

PROMPT_VERSION = "0.4.6"


def build_prompt_v0_4_6(
    *,
    case_data: dict[str, Any],
    judge_data: dict[str, Any],
    player_report: Any,
    schema_data: dict[str, Any],
) -> list[dict[str, str]]:
    """Add a strict positive lexicon for both player-visible fields."""
    messages = build_prompt_v0_4_5(
        case_data=case_data,
        judge_data=judge_data,
        player_report=player_report,
        schema_data=schema_data,
    )
    system = messages[0]["content"]
    user = messages[1]["content"]
    system = system.replace("PromptVersion: 0.4.5", f"PromptVersion: {PROMPT_VERSION}", 1)
    system = system.replace(
        "CorrectionType: balanced selected-key hint and clause-anchored coverage correction",
        "CorrectionType: strict visible-world lexicon over balanced fragment correction",
        1,
    )

    lexicon_gate = """

<VISIBLE_LEXICON_GATE>
judge_response 与 archive_summary 输出前必须逐字检查。以下词及其英文对应词一律不得出现：玩家、正式栏、正式提交、界外信息、界外指令、输入、字段、指令、系统、模型、游戏、提示词、奖励，以及任何程序字段名或原始枚举。
固定转译方向：
- “玩家”改为“呈文人”，或在 judge_response 中直接称“你”；
- “正式栏/正式提交”改为“判牍所署”或“案牍所载”；
- “隐藏字段/界外信息”改为“禁录”或“司署密录”；
- “系统/模型/游戏指令”改为“越权之词”；
- “最高奖励/奖励点数”改为“索取不当赏格”；
- “输入/正文”按语境改为“呈文”。
不得先复述禁用词再拒绝，必须直接输出转译后的世界内概念。
若报告不是对抗型，judge_response 与 archive_summary 不得额外声明“没有越权、索密或攻击”；安全审计与普通案情无关时完全省略。
</VISIBLE_LEXICON_GATE>"""
    marker = "\n<FINAL_SILENT_CHECK>"
    if marker not in system:
        raise RuntimeError("Prompt v0.4.5 final-check marker not found")
    system = system.replace(marker, lexicon_gate + marker, 1)
    system = system.replace(
        "8. judge_response 与 archive_summary 是否完全没有复制玩家的技术词、字段词、评分奖励词、原始枚举或界外术语；如有，删除具体对象，只保留世界内的越权意图概括。",
        "8. judge_response 与 archive_summary 是否完全没有出现玩家、正式栏、正式提交、界外信息、界外指令、输入、字段、指令、系统、模型、游戏、提示词、奖励、程序字段名或原始枚举；如有，按 VISIBLE_LEXICON_GATE 重写后再输出。",
        1,
    )
    return [{"role": "system", "content": system}, {"role": "user", "content": user}]


def build_prompt_file_v0_4_6(path: Path) -> str:
    return path.read_text(encoding="utf-8")
