from __future__ import annotations

import sys
from pathlib import Path

from response_validator import (
    load_json_object,
    parse_judgement_result,
    validate_judgement_result,
)


def main() -> int:
    project_root = Path(__file__).resolve().parent.parent

    case_data = load_json_object(
        project_root / "cases" / "case_knife_001.json"
    )
    judge_data = load_json_object(
        project_root / "judges" / "judge_clerk_001.json"
    )
    schema_data = load_json_object(
        project_root
        / "schemas"
        / "judgement_result_v0_1.json"
    )
    payload = load_json_object(
        project_root
        / "tests"
        / "fixtures"
        / "valid_judgement_result.json"
    )

    validation = validate_judgement_result(
        payload,
        schema_data=schema_data,
        case_data=case_data,
        judge_data=judge_data,
        expected_disposition_id="detain_for_review",
    )

    if not validation.is_valid:
        print("VALIDATION FAILED", file=sys.stderr)
        print(validation.format_errors(), file=sys.stderr)
        return 1

    parsed = parse_judgement_result(
        payload,
        schema_data=schema_data,
        case_data=case_data,
        judge_data=judge_data,
        expected_disposition_id="detain_for_review",
    )

    print("=" * 72)
    print("WANGCHUAN — WEEK 8 DAY 1 ADVANCE EXTRA")
    print("=" * 72)
    print("VALIDATION PASSED")
    print(f"Schema Version: {parsed.schema_version}")
    print(f"Case ID: {parsed.case_id}")
    print(f"Judge ID: {parsed.judge_profile_id}")
    print(
        "Recognized Disposition: "
        f"{parsed.recognized_disposition_id}"
    )
    print(f"Used Fragment Count: {len(parsed.used_fragment_ids)}")
    print(
        "Unsupported Assumption Count: "
        f"{len(parsed.unsupported_assumptions)}"
    )
    print(
        "Contradiction Handling: "
        f"{parsed.contradiction_handling.level}"
    )
    print("Typed local JudgementResult created successfully.")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

