from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any

LEAKAGE_TERMS = (
    "prompt", "api key", "api_key", "schema", "json", "reward_points", "rating", "system message", "system prompt",
    "json field", "test case", "model instruction", "rating field", "safety policy", "hidden field", "game instruction", "system instruction",
    "scorer", "enum", "system field", "more_good_than_evil", "mixed_merit_and_fault", "more_evil_than_good",
    "beyond_redemption", "recommend_rebirth", "ordinary_transfer", "send_to_prison", "soul_dissolution",
    "提示词", "系统提示词", "API密钥", "系统消息", "模式结构", "模型指令", "测试用例", "JSON字段", "隐藏字段", "游戏指令", "系统指令", "评分器", "评分字段",
    "奖励点数", "安全策略", "枚举", "系统字段",
)


@dataclass(frozen=True)
class GameLanguageAudit:
    status: str
    warnings: tuple[str, ...]


def audit_game_language(payload: dict[str, Any]) -> GameLanguageAudit:
    warnings: list[str] = []
    for field in ("judge_response", "archive_summary"):
        value = payload.get(field, "")
        lowered = value.casefold() if isinstance(value, str) else ""
        for term in LEAKAGE_TERMS:
            if term.casefold() in lowered:
                warnings.append(f"{field}: possible system-language leakage ({term})")
    return GameLanguageAudit("WARNING" if warnings else "PASS", tuple(warnings))


def main() -> int:
    import argparse
    parser = argparse.ArgumentParser(description="Read-only Day6 game-language audit")
    parser.add_argument("payload", type=Path)
    args = parser.parse_args()
    payload = json.loads(args.payload.read_text(encoding="utf-8"))
    result = audit_game_language(payload)
    print(result.status)
    for warning in result.warnings:
        print(f"- {warning}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
