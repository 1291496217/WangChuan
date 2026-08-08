from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from audit_game_language_v0_2 import LEAKAGE_TERMS_V0_2

AUDIT_VERSION = "0.3"

LEAKAGE_TERMS_V0_3 = LEAKAGE_TERMS_V0_2 + (
    "player",
    "input",
    "field",
    "instruction",
    "model",
    "game",
    "玩家",
    "输入内容",
    "字段",
    "指令",
    "系统",
    "模型",
    "游戏",
    "正式提交",
    "界外信息",
    "界外指令",
)


@dataclass(frozen=True)
class GameLanguageAuditV03:
    status: str
    warnings: tuple[str, ...]
    audit_version: str = AUDIT_VERSION


def audit_game_language_v0_3(payload: dict[str, Any]) -> GameLanguageAuditV03:
    warnings: list[str] = []
    for field_name in ("judge_response", "archive_summary"):
        value = payload.get(field_name, "")
        lowered = value.casefold() if isinstance(value, str) else ""
        for term in LEAKAGE_TERMS_V0_3:
            if term.casefold() in lowered:
                warnings.append(f"{field_name}: possible out-of-world language ({term})")
    return GameLanguageAuditV03("WARNING" if warnings else "PASS", tuple(warnings))


def main() -> int:
    import argparse

    parser = argparse.ArgumentParser(description="Read-only visible world-language audit v0.3")
    parser.add_argument("payload", type=Path)
    args = parser.parse_args()
    payload = json.loads(args.payload.read_text(encoding="utf-8"))
    result = audit_game_language_v0_3(payload)
    print(result.status)
    for warning in result.warnings:
        print(f"- {warning}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
