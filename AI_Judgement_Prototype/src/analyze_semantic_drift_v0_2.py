from __future__ import annotations

"""Review-corrected local semantic-drift analyzer for Week8 Day7 MR15.

This version fixes the v0.1 expected-fragment contradiction and separates
stable quality defects from stochastic drift.
"""

import argparse
import json
import statistics
from collections import Counter
from pathlib import Path
from typing import Any

EXPECTED_REQUIRED_ROLES = {
    "Medicine.Action01": "counterevidence",
    "Medicine.Outcome01": "core_support",
    "Medicine.Outcome02": "counterevidence",
    "Medicine.Personality01": "context",
    "Medicine.Relationship01": "core_support",
}
OPTIONAL_CASE_FRAGMENT_IDS = {"Medicine.Thought01"}
ALLOWED_CASE_FRAGMENT_IDS = set(EXPECTED_REQUIRED_ROLES) | OPTIONAL_CASE_FRAGMENT_IDS

ACTION_TAXONOMY_HINTS = (
    "诚信",
    "账目",
    "三年",
    "非亲",
    "亲故",
    "应诺",
    "答应",
)

def load_runs(results_root: Path) -> list[dict[str, Any]]:
    rows = []
    for path in sorted((results_root / "validated").glob("*.json")):
        data = json.loads(path.read_text(encoding="utf-8"))
        result = data["judgement_result"]
        metadata = data["metadata"]
        roles = {x["fragment_id"]: x["role"] for x in result["fragment_roles"]}

        missing_required = sorted(set(EXPECTED_REQUIRED_ROLES) - set(roles))
        unexpected = sorted(set(roles) - ALLOWED_CASE_FRAGMENT_IDS)
        role_mismatches = {
            fid: {"expected": expected, "actual": roles.get(fid)}
            for fid, expected in EXPECTED_REQUIRED_ROLES.items()
            if fid in roles and roles[fid] != expected
        }

        action_text = " | ".join(result.get("recognized_action_claims", []))
        rows.append({
            "repeat_index": metadata["repeat_index"],
            "run_id": data["run_id"],
            "validation_passed": metadata.get("validation_status") == "passed",
            "publication_passed": metadata.get("publication_status") == "passed",
            "formal_moral_ok": data["moral_judgement_id"] == "more_good_than_evil",
            "formal_disposition_ok": data["disposition_id"] == "recommend_rebirth",
            "fragment_roles": roles,
            "fragment_ids": sorted(roles),
            "missing_required": missing_required,
            "unexpected": unexpected,
            "role_mismatches": role_mismatches,
            "thought01_included": "Medicine.Thought01" in roles,
            "unsupported_count": len(result.get("unsupported_assumptions", [])),
            "personality": result.get("personality_action_relation"),
            "thought": result.get("thought_action_distinction"),
            "moral_tier": result.get("moral_reasoning_tier"),
            "disposition_tier": result.get("disposition_consistency_tier"),
            "rhetoric_tier": result.get("rhetoric_tier"),
            "language_status": metadata.get("game_language_status"),
            "action_claim_taxonomy_issue": any(x in action_text for x in ACTION_TAXONOMY_HINTS),
            "elapsed_ms": metadata.get("elapsed_ms"),
            "usage": metadata.get("usage", {}),
        })
    return sorted(rows, key=lambda x: x["repeat_index"])

def analyze(results_root: Path) -> dict[str, Any]:
    runs = load_runs(results_root)
    role_signatures = [
        tuple(sorted(run["fragment_roles"].items()))
        for run in runs
    ]
    fragment_signatures = [tuple(run["fragment_ids"]) for run in runs]

    material_findings = []
    if any(not x["validation_passed"] or not x["publication_passed"] for x in runs):
        material_findings.append("validation_or_publication_failure")
    if any(not x["formal_moral_ok"] or not x["formal_disposition_ok"] for x in runs):
        material_findings.append("formal_choice_instability")
    if any(x["missing_required"] or x["unexpected"] or x["role_mismatches"] for x in runs):
        material_findings.append("fragment_mapping_failure")
    if any(x["thought01_included"] for x in runs):
        material_findings.append("unexpected_thought01_mapping")
    if any(x["unsupported_count"] for x in runs):
        material_findings.append("unsupported_boundary_failure")
    if any(x["language_status"] != "PASS" for x in runs):
        material_findings.append("world_language_failure")
    if len(set(role_signatures)) > 1 or len(set(fragment_signatures)) > 1:
        material_findings.append("fragment_role_or_set_drift")

    minor_variation = []
    if len({x["thought"] for x in runs}) > 1:
        minor_variation.append("thought_action_enum_variation")
    if len({x["moral_tier"] for x in runs}) > 1:
        minor_variation.append("moral_reasoning_tier_variation")
    if len({x["personality"] for x in runs}) > 1:
        minor_variation.append("personality_enum_variation")
    if len({x["disposition_tier"] for x in runs}) > 1:
        minor_variation.append("disposition_tier_variation")

    if material_findings:
        overall = "concerning_drift"
    elif minor_variation:
        overall = "acceptable_drift"
    else:
        overall = "stable"

    latencies = [x["elapsed_ms"] for x in runs if isinstance(x["elapsed_ms"], (int, float))]
    return {
        "analyzer": "analyze_semantic_drift_v0_2_review_corrected",
        "experiment_id": "Week8.Day7.Drift.MR15.001",
        "validated_repeat_count": len(runs),
        "runs": runs,
        "aggregate": {
            "overall_drift": overall,
            "material_findings": material_findings,
            "minor_variation": minor_variation,
            "fragment_set_unique_count": len(set(fragment_signatures)),
            "fragment_role_map_unique_count": len(set(role_signatures)),
            "outcome02_inclusion_count": sum("Medicine.Outcome02" in x["fragment_roles"] for x in runs),
            "thought01_inclusion_count": sum(x["thought01_included"] for x in runs),
            "unsupported_distribution": dict(Counter(x["unsupported_count"] for x in runs)),
            "thought_distribution": dict(Counter(x["thought"] for x in runs)),
            "moral_tier_distribution": dict(Counter(x["moral_tier"] for x in runs)),
            "disposition_tier_distribution": dict(Counter(x["disposition_tier"] for x in runs)),
            "action_claim_taxonomy_issue_count": sum(x["action_claim_taxonomy_issue"] for x in runs),
            "systematic_quality_findings": [
                "recognized_action_claims taxonomy issue is stable across repeats and is reported separately from stochastic drift."
            ],
            "latency_ms": {
                "average": statistics.mean(latencies) if latencies else None,
                "median": statistics.median(latencies) if latencies else None,
                "min": min(latencies) if latencies else None,
                "max": max(latencies) if latencies else None,
            },
        },
    }

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--results", type=Path, required=True)
    parser.add_argument("--json-out", type=Path)
    args = parser.parse_args()
    report = analyze(args.results)
    text = json.dumps(report, ensure_ascii=False, indent=2) + "\n"
    if args.json_out:
        args.json_out.parent.mkdir(parents=True, exist_ok=True)
        args.json_out.write_text(text, encoding="utf-8")
    else:
        print(text)
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
