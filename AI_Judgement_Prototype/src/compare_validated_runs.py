from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any


INVARIANT_METADATA_FIELDS = (
    "provider",
    "model_requested",
    "thinking_mode",
    "temperature",
    "max_tokens",
    "schema_version",
    "case_id",
    "case_version",
    "judge_profile_id",
    "judge_version",
    "report_sha256",
    "selected_disposition_id",
)

RATING_FIELDS = (
    "narrative_coherence",
    "evidence_grounding",
    "rhetorical_effectiveness",
    "disposition_alignment",
)

GENERIC_RESPONSE_PHRASES = (
    "条理清晰",
    "值得肯定",
    "证据意识",
    "总体而言",
    "表现良好",
    "处理恰当",
)

CASE_SPECIFIC_CUES = (
    "短刀",
    "钱袋",
    "便笺",
    "济",
    "还活着",
    "他们",
)


def load_json_object(file_path: Path) -> dict[str, Any]:
    if not file_path.is_file():
        raise FileNotFoundError(f"Result file not found: {file_path}")

    with file_path.open("r", encoding="utf-8") as file:
        value = json.load(file)

    if not isinstance(value, dict):
        raise ValueError(f"Expected JSON object: {file_path}")

    return value


def find_invariant_mismatches(
    baseline_metadata: dict[str, Any],
    candidate_metadata: dict[str, Any],
) -> list[str]:
    mismatches: list[str] = []

    for field in INVARIANT_METADATA_FIELDS:
        baseline_value = baseline_metadata.get(field)
        candidate_value = candidate_metadata.get(field)

        if baseline_value != candidate_value:
            mismatches.append(
                f"{field}: {baseline_value!r} != {candidate_value!r}"
            )

    return mismatches


def audit_judge_response(response: str) -> dict[str, Any]:
    generic_hits = [
        phrase for phrase in GENERIC_RESPONSE_PHRASES
        if phrase in response
    ]
    concrete_hits = [
        cue for cue in CASE_SPECIFIC_CUES
        if cue in response
    ]

    return {
        "contains_question": "？" in response or "?" in response,
        "concrete_case_cues": concrete_hits,
        "generic_phrase_hits": generic_hits,
        "character_count": len(response),
    }


def _print_rating_comparison(
    baseline_result: dict[str, Any],
    candidate_result: dict[str, Any],
) -> None:
    baseline_ratings = baseline_result.get("dimension_ratings", {})
    candidate_ratings = candidate_result.get("dimension_ratings", {})

    print("\nDimension Ratings:")
    for field in RATING_FIELDS:
        print(
            f"- {field}: "
            f"{baseline_ratings.get(field)!r} -> "
            f"{candidate_ratings.get(field)!r}"
        )


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Compare validated Prompt v0.1 and v0.2 experiment runs."
        )
    )
    parser.add_argument("--baseline", required=True, type=Path)
    parser.add_argument("--candidate", required=True, type=Path)
    args = parser.parse_args()

    try:
        baseline = load_json_object(args.baseline)
        candidate = load_json_object(args.candidate)
        baseline_metadata = baseline["metadata"]
        candidate_metadata = candidate["metadata"]
        baseline_result = baseline["judgement_result"]
        candidate_result = candidate["judgement_result"]
    except (FileNotFoundError, KeyError, ValueError, json.JSONDecodeError) as error:
        print(f"[COMPARISON ERROR] {error}", file=sys.stderr)
        return 1

    mismatches = find_invariant_mismatches(
        baseline_metadata,
        candidate_metadata,
    )

    print("=" * 72)
    print("WANGCHUAN — PROMPT V0.1 / V0.2 COMPARISON")
    print("=" * 72)
    print(
        "Prompt Versions: "
        f"{baseline_metadata.get('prompt_version')} -> "
        f"{candidate_metadata.get('prompt_version')}"
    )

    if mismatches:
        print("\nCONTROL CHECK FAILED")
        for mismatch in mismatches:
            print(f"- {mismatch}")
        print(
            "\nThe two runs changed more than Prompt Version; "
            "do not treat them as a clean A/B comparison."
        )
        return 2

    print("CONTROL CHECK PASSED")
    print("All declared experiment controls match.")

    print("\nUnsupported Assumptions:")
    print(
        "- Baseline count: "
        f"{len(baseline_result.get('unsupported_assumptions', []))}"
    )
    print(
        "- Candidate count: "
        f"{len(candidate_result.get('unsupported_assumptions', []))}"
    )

    print("\nContradiction Handling:")
    print(
        "- Baseline: "
        f"{baseline_result.get('contradiction_handling', {}).get('level')}"
    )
    print(
        "- Candidate: "
        f"{candidate_result.get('contradiction_handling', {}).get('level')}"
    )

    _print_rating_comparison(baseline_result, candidate_result)

    baseline_response = str(baseline_result.get("judge_response", ""))
    candidate_response = str(candidate_result.get("judge_response", ""))
    baseline_audit = audit_judge_response(baseline_response)
    candidate_audit = audit_judge_response(candidate_response)

    print("\nJudge Response Style Audit:")
    print(f"- Baseline: {baseline_audit}")
    print(f"- Candidate: {candidate_audit}")

    print("\nBaseline Judge Response:")
    print(baseline_response)
    print("\nCandidate Judge Response:")
    print(candidate_response)

    print("\nManual Review Questions:")
    print("1. Did v0.2 keep evidence analysis at least as accurate?")
    print("2. Did it leave unsupported_assumptions empty when appropriate?")
    print("3. Does the judge sound like a specific underworld clerk?")
    print("4. Does the concrete question make the player want to revise?")
    print("5. Did stronger style introduce invented facts or hidden truth?")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
