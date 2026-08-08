from __future__ import annotations

import json
import re
from pathlib import Path
from typing import Any

from models import ValidationIssue, ValidationResult
from models_v0_2 import JudgementResultV02
from response_validator import _validate_schema_node

_FLAG_TOKENS = {
    "prompt_injection_detected", "illegal_field_request", "rule_override_attempt",
    "disposition_override_attempt", "system_information_request",
}


def load_json_object_v0_2(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise ValueError(f"Expected JSON object: {path}")
    return value


def _issue(issues: list[ValidationIssue], path: str, message: str) -> None:
    issues.append(ValidationIssue(path, message))


def _runtime_checks(payload: dict[str, Any], case_data: dict[str, Any], judge_data: dict[str, Any], issues: list[ValidationIssue]) -> None:
    if payload.get("case_id") != case_data.get("case_id"):
        _issue(issues, "$.case_id", "does not match current CaseID")
    if payload.get("judge_profile_id") != judge_data.get("judge_profile_id"):
        _issue(issues, "$.judge_profile_id", "does not match current JudgeID")
    allowed = {x.get("fragment_id") for x in case_data.get("fragments", []) if isinstance(x, dict)}
    seen: set[str] = set()
    for index, item in enumerate(payload.get("fragment_roles", [])):
        if not isinstance(item, dict):
            continue
        fragment_id = item.get("fragment_id")
        if fragment_id not in allowed:
            _issue(issues, f"$.fragment_roles[{index}].fragment_id", "unknown or cross-case FragmentID")
        if fragment_id in seen:
            _issue(issues, f"$.fragment_roles[{index}].fragment_id", "duplicate FragmentID across roles")
        seen.add(fragment_id)
    for field in ("judge_response", "archive_summary"):
        value = payload.get(field)
        if isinstance(value, str):
            for token in _FLAG_TOKENS:
                if re.search(rf"(?<![A-Za-z0-9_]){re.escape(token)}(?![A-Za-z0-9_])", value):
                    _issue(issues, f"$.{field}", "raw internal safety flag token must not be player-visible")


def validate_judgement_result_v0_2(payload: Any, *, schema_data: dict[str, Any], case_data: dict[str, Any], judge_data: dict[str, Any]) -> ValidationResult:
    issues: list[ValidationIssue] = []
    _validate_schema_node(payload, schema_data, "$", issues)
    if isinstance(payload, dict):
        _runtime_checks(payload, case_data, judge_data, issues)
    return ValidationResult(tuple(issues))


def parse_judgement_result_v0_2(payload: Any, *, schema_data: dict[str, Any], case_data: dict[str, Any], judge_data: dict[str, Any]) -> JudgementResultV02:
    validation = validate_judgement_result_v0_2(payload, schema_data=schema_data, case_data=case_data, judge_data=judge_data)
    if not validation.is_valid:
        raise ValueError("Schema v0.2 validation failed:\n" + validation.format_errors())
    return JudgementResultV02.from_dict(payload)
