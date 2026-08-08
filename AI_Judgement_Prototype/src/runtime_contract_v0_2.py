from __future__ import annotations

from typing import Any

from models import ValidationIssue, ValidationResult
from models_v0_2 import JudgementResultV02, ValidatedRunEnvelopeV02

_INTERNAL_FLAGS = {
    "prompt_injection_detected", "illegal_field_request", "rule_override_attempt",
    "disposition_override_attempt", "system_information_request",
}


def validate_runtime_contract_v0_2(*, result: JudgementResultV02, case_data: dict[str, Any], judge_data: dict[str, Any], player_report: Any) -> ValidationResult:
    issues: list[ValidationIssue] = []
    if result.case_id != case_data.get("case_id"):
        issues.append(ValidationIssue("$.case_id", "does not match Case"))
    if result.judge_profile_id != judge_data.get("judge_profile_id"):
        issues.append(ValidationIssue("$.judge_profile_id", "does not match Judge"))
    allowed = {x.get("fragment_id") for x in case_data.get("fragments", []) if isinstance(x, dict)}
    ids = [item.fragment_id for item in result.fragment_roles]
    for index, fragment_id in enumerate(ids):
        if fragment_id not in allowed:
            issues.append(ValidationIssue(f"$.fragment_roles[{index}].fragment_id", "unknown FragmentID"))
    if len(ids) != len(set(ids)):
        issues.append(ValidationIssue("$.fragment_roles", "duplicate FragmentID"))
    if any(flag in result.judge_response or flag in result.archive_summary for flag in _INTERNAL_FLAGS):
        issues.append(ValidationIssue("$.judge_response", "internal flag leaked into visible output"))
    return ValidationResult(tuple(issues))


def build_validated_run_envelope_v0_2(*, run_id: str, result: JudgementResultV02, case_data: dict[str, Any], judge_data: dict[str, Any], player_report: Any, metadata: dict[str, Any]) -> ValidatedRunEnvelopeV02:
    validation = validate_runtime_contract_v0_2(result=result, case_data=case_data, judge_data=judge_data, player_report=player_report)
    if not validation.is_valid:
        raise ValueError("Runtime Contract v0.2 failed:\n" + validation.format_errors())
    return ValidatedRunEnvelopeV02(
        run_id=run_id, case_id=case_data["case_id"], case_version=case_data["case_version"],
        judge_profile_id=judge_data["judge_profile_id"], judge_version=judge_data.get("version", ""),
        moral_judgement_id=player_report.moral_judgement_id, disposition_id=player_report.disposition_id,
        selected_key_fragment_ids=tuple(player_report.selected_key_fragment_ids), result=result, metadata=metadata,
    )
