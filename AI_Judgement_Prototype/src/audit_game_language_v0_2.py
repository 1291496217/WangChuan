from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from audit_game_language import LEAKAGE_TERMS

AUDIT_VERSION = "0.2"

LEAKAGE_TERMS_V0_2 = LEAKAGE_TERMS + (
    "moraljudgementid",
    "dispositionid",
    "selectedkeyfragmentids",
    "lifeinterpretation",
    "verdicttext",
    "interpretation_hooks",
    "case_disposition_coverage",
    "reward",
    "正式栏",
    "最高奖励",
    "奖励",
    "指令覆盖",
)


@dataclass(frozen=True)
class GameLanguageAuditV02:
    status: str
    warnings: tuple[str, ...]
    audit_version: str = AUDIT_VERSION


def audit_game_language_v0_2(payload: dict[str, Any]) -> GameLanguageAuditV02:
    warnings: list[str] = []
    for field in ("judge_response", "archive_summary"):
        value = payload.get(field, "")
        lowered = value.casefold() if isinstance(value, str) else ""
        for term in LEAKAGE_TERMS_V0_2:
            if term.casefold() in lowered:
                warnings.append(f"{field}: possible system-language leakage ({term})")
    return GameLanguageAuditV02("WARNING" if warnings else "PASS", tuple(warnings))


def main() -> int:
    import argparse

    parser = argparse.ArgumentParser(description="Read-only visible world-language audit v0.2")
    parser.add_argument("payload", type=Path)
    args = parser.parse_args()
    payload = json.loads(args.payload.read_text(encoding="utf-8"))
    result = audit_game_language_v0_2(payload)
    print(result.status)
    for warning in result.warnings:
        print(f"- {warning}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
