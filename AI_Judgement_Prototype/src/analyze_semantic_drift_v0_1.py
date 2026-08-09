from __future__ import annotations

"""Deterministic, local-only semantic drift analyzer for Day7 MR15 repeats."""

import argparse
import json
import statistics
from collections import Counter
from pathlib import Path
from typing import Any

from audit_game_language_v0_3 import audit_game_language_v0_3


EXPECTED_ROLES = {
    "Medicine.Action01": "counterevidence",
    "Medicine.Outcome01": "core_support",
    "Medicine.Outcome02": "counterevidence",
    "Medicine.Personality01": "context",
    "Medicine.Relationship01": "core_support",
    "Medicine.Thought01": "context",
}
EXPECTED_FRAGMENT_IDS = tuple(EXPECTED_ROLES)
ROLE_DIRECTIONS = {"core_support", "counterevidence", "context"}
ACTION_TAXONOMY_HINTS = (
    "诚信",
    "账目",
    "三年",
    "动机",
    "非亲",
    "资源代价",
)
SECURITY_RESIDUE_HINTS = (
    "越权",
    "索密",
    "攻击",
    "安全",
    "审计",
    "禁录",
    "系统",
    "模型",
    "玩家",
    "字段",
)


def _load_envelopes(results_root: Path) -> list[dict[str, Any]]:
    envelopes: list[dict[str, Any]] = []
    for path in sorted((results_root / "validated").glob("*.json")):
        value = json.loads(path.read_text(encoding="utf-8"))
        value["_path"] = str(path)
        envelopes.append(value)
    return envelopes


def _counter(values: list[Any]) -> dict[str, int]:
    return dict(sorted(Counter(str(value) for value in values).items()))


def _jaccard(left: set[str], right: set[str]) -> float:
    union = left | right
    return 1.0 if not union else round(len(left & right) / len(union), 4)


def _run_record(envelope: dict[str, Any]) -> dict[str, Any]:
    metadata = envelope.get("metadata", {})
    result = envelope.get("judgement_result", {})
    roles = result.get("fragment_roles", [])
    fragment_ids = [item.get("fragment_id") for item in roles if isinstance(item, dict)]
    role_by_id = {
        item.get("fragment_id"): item.get("role")
        for item in roles
        if isinstance(item, dict)
    }
    unsupported = result.get("unsupported_assumptions", [])
    unsupported_count = len(unsupported) if isinstance(unsupported, list) else 0
    response = result.get("judge_response", "")
    archive = result.get("archive_summary", "")
    visible = f"{response}\n{archive}"
    audit = audit_game_language_v0_3(result)
    unexpected_ids = sorted(set(fragment_ids) - set(EXPECTED_FRAGMENT_IDS))
    missing_ids = sorted(set(EXPECTED_FRAGMENT_IDS) - set(fragment_ids))
    full_case = set(fragment_ids) == set(EXPECTED_FRAGMENT_IDS)
    thought_included = "Medicine.Thought01" in fragment_ids
    wrong_role_direction = {
        fragment_id: {"expected": EXPECTED_ROLES[fragment_id], "actual": role_by_id.get(fragment_id)}
        for fragment_id in fragment_ids
        if fragment_id in EXPECTED_ROLES
        and role_by_id.get(fragment_id) not in ROLE_DIRECTIONS
    }
    action_claims = result.get("recognized_action_claims", [])
    action_claim_text = " | ".join(str(x) for x in action_claims)
    action_taxonomy_issue = any(hint in action_claim_text for hint in ACTION_TAXONOMY_HINTS)
    security_residue_terms = sorted({hint for hint in SECURITY_RESIDUE_HINTS if hint in visible})

    material_reasons: list[str] = []
    minor_reasons: list[str] = []
    if envelope.get("case_id") != "Case.Medicine.001":
        material_reasons.append("case_mismatch")
    if envelope.get("moral_judgement_id") != "more_good_than_evil":
        material_reasons.append("formal_moral_mismatch")
    if envelope.get("disposition_id") != "recommend_rebirth":
        material_reasons.append("formal_disposition_mismatch")
    if full_case:
        material_reasons.append("full_case_six_fragment_expansion")
    if unexpected_ids:
        material_reasons.append("unknown_fragment_id")
    if thought_included:
        material_reasons.append("thought_fragment_included")
    if unsupported_count:
        minor_reasons.append("unsupported_assumption_present")
    if missing_ids and not thought_included:
        minor_reasons.append("fragment_coverage_varies")
    if action_taxonomy_issue:
        minor_reasons.append("action_claim_taxonomy_boundary")
    if security_residue_terms:
        minor_reasons.append("security_commentary_residue")
    if audit.status != "PASS":
        material_reasons.append("world_language_gate_not_pass")
    if material_reasons:
        drift_label = "material_drift"
    elif minor_reasons:
        drift_label = "minor_drift"
    else:
        drift_label = "stable"
    return {
        "repeat_index": metadata.get("repeat_index"),
        "run_id": envelope.get("run_id"),
        "source_path": envelope.get("_path"),
        "validation_passed": metadata.get("validation_status") == "passed",
        "publication_passed": metadata.get("publication_status") == "passed",
        "case_id": envelope.get("case_id"),
        "formal_moral_judgement_id": envelope.get("moral_judgement_id"),
        "formal_disposition_id": envelope.get("disposition_id"),
        "core_story": result.get("core_story", ""),
        "fragment_ids": sorted(fragment_ids),
        "fragment_roles": [{"fragment_id": k, "role": role_by_id[k]} for k in sorted(role_by_id)],
        "missing_fragment_ids": missing_ids,
        "unexpected_fragment_ids": unexpected_ids,
        "full_case_six_fragment_expansion": full_case,
        "thought01_included": thought_included,
        "role_direction_issues": wrong_role_direction,
        "unsupported_count": unsupported_count,
        "unsupported_assumptions": unsupported,
        "unsupported_severity": _counter([x.get("severity") for x in unsupported if isinstance(x, dict)]),
        "personality_action_relation": result.get("personality_action_relation"),
        "thought_action_distinction": result.get("thought_action_distinction"),
        "contradiction_handling": result.get("contradiction_handling"),
        "moral_reasoning_tier": result.get("moral_reasoning_tier"),
        "disposition_consistency_tier": result.get("disposition_consistency_tier"),
        "rhetoric_tier": result.get("rhetoric_tier"),
        "internal_safety_flags": result.get("internal_safety_flags", []),
        "recognized_action_claims": action_claims,
        "recognized_motive_claims": result.get("recognized_motive_claims", []),
        "action_claim_taxonomy_issue": action_taxonomy_issue,
        "security_commentary_residue_terms": security_residue_terms,
        "world_language_status": audit.status,
        "world_language_warnings": list(audit.warnings),
        "judge_response": response,
        "archive_summary": archive,
        "judge_response_characters": len(response),
        "archive_summary_characters": len(archive),
        "drift_label": drift_label,
        "material_drift_reasons": material_reasons,
        "minor_drift_reasons": minor_reasons,
        "usage": metadata.get("usage"),
        "elapsed_ms": metadata.get("elapsed_ms"),
    }


def analyze(results_root: Path) -> dict[str, Any]:
    envelopes = _load_envelopes(results_root)
    runs = [_run_record(envelope) for envelope in envelopes]
    runs.sort(key=lambda item: (item.get("repeat_index") is None, item.get("repeat_index")))
    # The analyzer is intentionally scoped to the five validated envelopes.
    # Raw files are counted by filename only so no provider content is re-read.
    validated_ids = {run["run_id"] for run in runs}
    raw_paths = sorted((results_root / "raw").glob("*.json"))
    failure_paths = [str(path) for path in raw_paths if path.stem not in validated_ids]
    sets = [set(run["fragment_ids"]) for run in runs]
    pairwise = []
    for index, left in enumerate(runs):
        for right in runs[index + 1 :]:
            pairwise.append(
                {
                    "left_repeat": left["repeat_index"],
                    "right_repeat": right["repeat_index"],
                    "jaccard": _jaccard(set(left["fragment_ids"]), set(right["fragment_ids"])),
                }
            )
    all_fragment_ids = sorted(set().union(*sets)) if sets else []
    inclusion_frequency = {
        fragment_id: sum(fragment_id in fragment_set for fragment_set in sets)
        for fragment_id in all_fragment_ids
    }
    role_frequency = Counter(
        role["role"]
        for run in runs
        for role in run["fragment_roles"]
    )
    latencies = [run["elapsed_ms"] for run in runs if isinstance(run.get("elapsed_ms"), (int, float))]
    usage_rows = [run.get("usage") for run in runs if isinstance(run.get("usage"), dict)]
    def usage_stat(name: str) -> dict[str, Any]:
        values = [u.get(name) for u in usage_rows if isinstance(u.get(name), (int, float))]
        return {
            "sum": sum(values),
            "average": round(statistics.mean(values), 2) if values else None,
            "median": statistics.median(values) if values else None,
            "min": min(values) if values else None,
            "max": max(values) if values else None,
        }
    stable = sum(run["drift_label"] == "stable" for run in runs)
    minor = sum(run["drift_label"] == "minor_drift" for run in runs)
    material = sum(run["drift_label"] == "material_drift" for run in runs)
    if len(failure_paths) >= 3 or not runs:
        overall = "unusable"
    elif material:
        overall = "concerning_drift"
    elif minor:
        overall = "acceptable_drift_with_minor_findings"
    else:
        overall = "stable"
    return {
        "analyzer": "analyze_semantic_drift_v0_1",
        "analyzer_mode": "local_deterministic_no_api",
        "experiment_id": "Week8.Day7.Drift.MR15.001",
        "results_root": str(results_root),
        "expected_repeat_count": 5,
        "validated_repeat_count": len(runs),
        "raw_nonvalidated_count": len(failure_paths),
        "raw_nonvalidated_paths": failure_paths,
        "run_records": runs,
        "aggregate": {
            "overall_drift_label": overall,
            "stable_runs": stable,
            "minor_drift_runs": minor,
            "material_drift_runs": material,
            "all_validation_passed": all(run["validation_passed"] for run in runs) and len(runs) == 5,
            "all_publication_passed": all(run["publication_passed"] for run in runs) and len(runs) == 5,
            "world_language_status": _counter([run["world_language_status"] for run in runs]),
            "fragment_set_frequency": _counter(["|".join(run["fragment_ids"]) for run in runs]),
            "fragment_inclusion_frequency": inclusion_frequency,
            "fragment_role_frequency": dict(sorted(role_frequency.items())),
            "pairwise_jaccard": pairwise,
            "pairwise_jaccard_average": round(statistics.mean([x["jaccard"] for x in pairwise]), 4) if pairwise else None,
            "unsupported_count_distribution": _counter([run["unsupported_count"] for run in runs]),
            "unsupported_severity_distribution": _counter([
                severity
                for run in runs
                for severity, count in run["unsupported_severity"].items()
                for _ in range(count)
            ]),
            "personality_action_relation_distribution": _counter([run["personality_action_relation"] for run in runs]),
            "thought_action_distinction_distribution": _counter([run["thought_action_distinction"] for run in runs]),
            "contradiction_handling_distribution": _counter([run["contradiction_handling"] for run in runs]),
            "moral_reasoning_tier_distribution": _counter([run["moral_reasoning_tier"] for run in runs]),
            "disposition_consistency_tier_distribution": _counter([run["disposition_consistency_tier"] for run in runs]),
            "rhetoric_tier_distribution": _counter([run["rhetoric_tier"] for run in runs]),
            "outcome02_inclusion_count": inclusion_frequency.get("Medicine.Outcome02", 0),
            "thought01_unexpected_inclusion_count": sum(run["thought01_included"] for run in runs),
            "security_commentary_residue_count": sum(bool(run["security_commentary_residue_terms"]) for run in runs),
            "action_claim_taxonomy_issue_count": sum(run["action_claim_taxonomy_issue"] for run in runs),
            "judge_response_unique_count": len({run["judge_response"] for run in runs}),
            "archive_summary_unique_count": len({run["archive_summary"] for run in runs}),
            "judge_response_length": {
                "min": min([run["judge_response_characters"] for run in runs], default=None),
                "max": max([run["judge_response_characters"] for run in runs], default=None),
                "average": round(statistics.mean([run["judge_response_characters"] for run in runs]), 2) if runs else None,
            },
            "archive_summary_length": {
                "min": min([run["archive_summary_characters"] for run in runs], default=None),
                "max": max([run["archive_summary_characters"] for run in runs], default=None),
                "average": round(statistics.mean([run["archive_summary_characters"] for run in runs]), 2) if runs else None,
            },
            "judge_response_question_count": sum("？" in run["judge_response"] or "?" in run["judge_response"] for run in runs),
            "judge_response_opening_prefix_unique_count": len({run["judge_response"][:8] for run in runs}),
            "latency_ms": {
                "sum": sum(latencies),
                "average": round(statistics.mean(latencies), 2) if latencies else None,
                "median": statistics.median(latencies) if latencies else None,
                "min": min(latencies) if latencies else None,
                "max": max(latencies) if latencies else None,
            },
            "usage": {
                "prompt_tokens": usage_stat("prompt_tokens"),
                "completion_tokens": usage_stat("completion_tokens"),
                "total_tokens": usage_stat("total_tokens"),
                "prompt_cache_hit_tokens": usage_stat("prompt_cache_hit_tokens"),
                "prompt_cache_miss_tokens": usage_stat("prompt_cache_miss_tokens"),
            },
        },
    }


def _md(report: dict[str, Any]) -> str:
    agg = report["aggregate"]
    lines = [
        "# Day7 MR15 Semantic Drift Audit v0.1",
        "",
        "- Analyzer: local deterministic; no API call",
        f"- Validated repeats: {report['validated_repeat_count']}/5; raw non-validated: {report['raw_nonvalidated_count']}",
        f"- Overall drift: **{agg['overall_drift_label']}**",
        f"- Formal choice: all repeats fixed at `more_good_than_evil` / `recommend_rebirth`",
        f"- Publication: validation={agg['all_validation_passed']}, language={agg['all_publication_passed']}",
        "",
        "## Repeat observations",
        "",
        "| Repeat | RunID | Fragments | Unsupported | Personality | Thought | Tiers (moral/disp/rhetoric) | Language | Drift |",
        "|---:|---|---|---:|---|---|---|---|---|",
    ]
    for run in report["run_records"]:
        fragments = ", ".join(run["fragment_ids"])
        tiers = "/".join(
            [run["moral_reasoning_tier"], run["disposition_consistency_tier"], run["rhetoric_tier"]]
        )
        lines.append(
            f"| {run['repeat_index']} | `{run['run_id']}` | {fragments} | {run['unsupported_count']} | "
            f"{run['personality_action_relation']} | {run['thought_action_distinction']} | {tiers} | "
            f"{run['world_language_status']} | {run['drift_label']} |"
        )
    lines.extend(
        [
            "",
            "## Aggregate",
            "",
            f"- Fragment-set pairwise Jaccard average: `{agg['pairwise_jaccard_average']}`.",
            f"- Fragment inclusion: `{json.dumps(agg['fragment_inclusion_frequency'], ensure_ascii=False)}`.",
            f"- Role frequency: `{json.dumps(agg['fragment_role_frequency'], ensure_ascii=False)}`.",
            f"- Unsupported distribution: `{json.dumps(agg['unsupported_count_distribution'], ensure_ascii=False)}`.",
            f"- Personality distribution: `{json.dumps(agg['personality_action_relation_distribution'], ensure_ascii=False)}`.",
            f"- Thought/action distribution: `{json.dumps(agg['thought_action_distinction_distribution'], ensure_ascii=False)}`.",
            f"- Security-commentary residue: `{agg['security_commentary_residue_count']}`; action-claim taxonomy boundary: `{agg['action_claim_taxonomy_issue_count']}`.",
            f"- Judge response unique count: `{agg['judge_response_unique_count']}`; archive unique count: `{agg['archive_summary_unique_count']}`.",
            f"- Judge response length: `{json.dumps(agg['judge_response_length'], ensure_ascii=False)}`; archive length: `{json.dumps(agg['archive_summary_length'], ensure_ascii=False)}`; question count: `{agg['judge_response_question_count']}`; opening-prefix diversity: `{agg['judge_response_opening_prefix_unique_count']}`.",
            f"- Latency ms: `{json.dumps(agg['latency_ms'], ensure_ascii=False)}`.",
            f"- Usage (no pricing applied): `{json.dumps(agg['usage'], ensure_ascii=False)}`.",
            "",
            "## Interpretation boundary",
            "",
            "Fragment-set variation is observed as semantic drift only when it changes formal integrity, role direction, safety/publication status, or evidence boundary. Wording diversity alone is not penalized. This file is an audit aid, not a deterministic gameplay score.",
        ]
    )
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description="Analyze Day7 MR15 results locally without an API call")
    parser.add_argument("--results", type=Path, default=Path("results/week8b_day7_drift_mr15_v0_1"))
    parser.add_argument("--json-out", type=Path, default=Path("reports/semantic_audit_week8b/day7_mr15_drift_audit_v0_1.json"))
    parser.add_argument("--md-out", type=Path, default=Path("reports/semantic_audit_week8b/day7_mr15_drift_audit_v0_1.md"))
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    results_root = args.results if args.results.is_absolute() else root / args.results
    report = analyze(results_root)
    json_out = args.json_out if args.json_out.is_absolute() else root / args.json_out
    md_out = args.md_out if args.md_out.is_absolute() else root / args.md_out
    json_out.parent.mkdir(parents=True, exist_ok=True)
    md_out.parent.mkdir(parents=True, exist_ok=True)
    json_out.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    md_out.write_text(_md(report), encoding="utf-8")
    print(f"ANALYZED {len(report['run_records'])} validated repeat(s) -> {json_out} {md_out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
