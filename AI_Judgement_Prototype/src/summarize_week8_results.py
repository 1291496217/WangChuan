"""Summarize saved Week 8 judgement runs without contacting an AI provider.

This script is stdlib-only. It never loads .env, imports the provider client,
or performs a network request. It joins saved raw/validated result pairs,
separates prompt experiment layers, joins the semantic-audit snapshot, computes
token/latency/cost statistics, and writes stable Markdown and JSON summaries.
"""

from __future__ import annotations

import argparse
import json
import re
import statistics
from collections import Counter
from datetime import datetime, timezone
from decimal import Decimal, InvalidOperation
from pathlib import Path
from typing import Any, Iterable


PRICING = {
    "pricing_date": "2026-08-04",
    "usd_per_1m_tokens": {
        "cache_hit_input": "0.0028",
        "cache_miss_input": "0.14",
        "output": "0.28",
    },
    "cny_per_1m_tokens": {
        "cache_hit_input": "0.02",
        "cache_miss_input": "1.0",
        "output": "2.0",
    },
    "note": (
        "Week8 reference prices. Provider prices may change. "
        "Recorded cache splits are used when available; otherwise all prompt "
        "tokens are conservatively priced as cache miss."
    ),
}

RUN_ID_RE = re.compile(
    r"(?P<run_id>\d{8}T\d{6}\d{6}Z_[0-9a-fA-F]+)"
)
REPORT_ID_RE = re.compile(
    r"(?P<report_id>R\d{2})(?:[_./\\-]|$)",
    re.IGNORECASE,
)

DIMENSION_KEYS = (
    "narrative_coherence",
    "evidence_grounding",
    "rhetorical_effectiveness",
    "disposition_alignment",
)

PROVIDER_FAILURE_STATUSES = {
    "provider_error",
    "request_failed",
    "api_error",
    "network_error",
    "timeout",
}
PARSE_FAILURE_STATUSES = {
    "parse_failed",
    "json_parse_failed",
    "invalid_json",
}
VALIDATION_FAILURE_STATUSES = {
    "validation_failed",
    "invalid_result",
}


def _load_json(
    path: Path,
    warnings: list[str],
) -> dict[str, Any] | None:
    try:
        with path.open("r", encoding="utf-8") as handle:
            value = json.load(handle)
    except (OSError, json.JSONDecodeError) as error:
        warnings.append(f"Unreadable JSON: {path} ({error})")
        return None

    if not isinstance(value, dict):
        warnings.append(f"JSON root is not an object: {path}")
        return None

    return value


def _decimal(value: Any, default: str = "0") -> Decimal:
    try:
        return Decimal(str(value))
    except (InvalidOperation, TypeError, ValueError):
        return Decimal(default)


def _money(value: Decimal) -> str:
    return format(value.quantize(Decimal("0.00000001")), "f")


def _safe_int(value: Any) -> int | None:
    if isinstance(value, bool):
        return None
    if isinstance(value, (int, float)):
        return int(value)
    return None


def _counter(values: Iterable[Any]) -> dict[str, int]:
    counter = Counter(str(value) for value in values)
    return dict(sorted(counter.items()))


def _stats(values: list[int]) -> dict[str, int | float | None]:
    if not values:
        return {
            "min": None,
            "max": None,
            "average": None,
            "median": None,
        }

    return {
        "min": min(values),
        "max": max(values),
        "average": round(statistics.mean(values), 2),
        "median": statistics.median(values),
    }


def _report_id(metadata: dict[str, Any]) -> str | None:
    report_path = str(metadata.get("report_path", ""))
    match = REPORT_ID_RE.search(report_path)
    if match is None:
        return None
    return match.group("report_id").upper()


def _classify(
    metadata: dict[str, Any],
    report_id: str | None,
) -> str:
    prompt_version = str(metadata.get("prompt_version", ""))

    if prompt_version == "0.1":
        return "day2_baseline"

    if prompt_version == "0.2":
        return "prompt_v02_ab"

    if (
        prompt_version == "0.3"
        and report_id is not None
        and 1 <= int(report_id[1:]) <= 19
    ):
        return "prompt_v03_corpus"

    return "unclassified"


def _usage(
    metadata: dict[str, Any],
    warnings: list[str],
    source: str,
) -> dict[str, Any]:
    raw_usage = metadata.get("usage")
    usage = raw_usage if isinstance(raw_usage, dict) else {}

    prompt_tokens = _safe_int(usage.get("prompt_tokens")) or 0
    completion_tokens = _safe_int(usage.get("completion_tokens")) or 0
    total_tokens = _safe_int(usage.get("total_tokens"))

    if total_tokens is None:
        total_tokens = prompt_tokens + completion_tokens
        warnings.append(
            f"Derived total_tokens because it was missing: {source}"
        )

    hit_value = _safe_int(
        usage.get(
            "prompt_cache_hit_tokens",
            usage.get("cache_hit_tokens"),
        )
    )
    miss_value = _safe_int(
        usage.get(
            "prompt_cache_miss_tokens",
            usage.get("cache_miss_tokens"),
        )
    )

    cache_split_recorded = hit_value is not None or miss_value is not None
    cache_hit_tokens = hit_value or 0
    cache_miss_tokens = miss_value or 0

    if not cache_split_recorded:
        cache_miss_tokens = prompt_tokens
        pricing_mode = "conservative_all_prompt_as_cache_miss"
    else:
        pricing_mode = "cache_aware"
        split_total = cache_hit_tokens + cache_miss_tokens

        if split_total < prompt_tokens:
            remainder = prompt_tokens - split_total
            cache_miss_tokens += remainder
            warnings.append(
                f"Cache split below prompt total; "
                f"added {remainder} tokens as cache miss: {source}"
            )
        elif split_total > prompt_tokens:
            warnings.append(
                f"Cache split exceeds prompt_tokens: {source}"
            )

    return {
        "prompt_tokens": prompt_tokens,
        "completion_tokens": completion_tokens,
        "total_tokens": total_tokens,
        "prompt_cache_hit_tokens": cache_hit_tokens,
        "prompt_cache_miss_tokens": cache_miss_tokens,
        "cache_split_recorded": cache_split_recorded,
        "pricing_mode": pricing_mode,
    }


def _cost(usage: dict[str, Any]) -> dict[str, str]:
    million = Decimal(1_000_000)

    hit_tokens = Decimal(usage["prompt_cache_hit_tokens"])
    miss_tokens = Decimal(usage["prompt_cache_miss_tokens"])
    output_tokens = Decimal(usage["completion_tokens"])

    usd = (
        hit_tokens
        * Decimal(PRICING["usd_per_1m_tokens"]["cache_hit_input"])
        + miss_tokens
        * Decimal(PRICING["usd_per_1m_tokens"]["cache_miss_input"])
        + output_tokens
        * Decimal(PRICING["usd_per_1m_tokens"]["output"])
    ) / million

    cny = (
        hit_tokens
        * Decimal(PRICING["cny_per_1m_tokens"]["cache_hit_input"])
        + miss_tokens
        * Decimal(PRICING["cny_per_1m_tokens"]["cache_miss_input"])
        + output_tokens
        * Decimal(PRICING["cny_per_1m_tokens"]["output"])
    ) / million

    return {
        "usd": _money(usd),
        "cny": _money(cny),
    }


def _judgement_from_validated(
    validated: dict[str, Any] | None,
) -> dict[str, Any]:
    if not isinstance(validated, dict):
        return {}

    judgement = validated.get("judgement_result")
    if isinstance(judgement, dict):
        return judgement

    # Some earlier fixtures may store the validated payload at the root.
    if "schema_version" in validated and "core_claim" in validated:
        return validated

    return {}


def _metadata_from_pair(
    raw: dict[str, Any] | None,
    validated: dict[str, Any] | None,
) -> dict[str, Any]:
    for source in (raw, validated):
        if not isinstance(source, dict):
            continue
        metadata = source.get("metadata")
        if isinstance(metadata, dict):
            return metadata
    return {}


def _run_id_from_file(path: Path) -> str | None:
    match = RUN_ID_RE.search(path.stem)
    if match is None:
        return None
    return match.group("run_id")


def _index_results(
    directory: Path,
    warnings: list[str],
    kind: str,
) -> dict[str, list[tuple[Path, dict[str, Any]]]]:
    result: dict[str, list[tuple[Path, dict[str, Any]]]] = {}

    if not directory.is_dir():
        warnings.append(f"{kind} result directory missing: {directory}")
        return result

    for path in sorted(directory.glob("*.json")):
        if not path.is_file():
            continue

        value = _load_json(path, warnings)
        if value is None:
            continue

        metadata = value.get("metadata")
        metadata = metadata if isinstance(metadata, dict) else {}

        run_id = str(
            metadata.get("run_id")
            or _run_id_from_file(path)
            or path.stem
        )
        result.setdefault(run_id, []).append((path, value))

    for run_id, entries in result.items():
        if len(entries) > 1:
            warnings.append(
                f"Duplicate {kind} run_id: "
                f"{run_id} ({len(entries)} files)"
            )

    return result


def _load_audit(
    path: Path,
    warnings: list[str],
) -> dict[str, Any]:
    if not path.is_file():
        warnings.append(f"Semantic audit file missing: {path}")
        return {"available": False}

    audit = _load_json(path, warnings)
    if audit is None:
        return {"available": False}

    return {
        "available": True,
        "audit_version": audit.get("audit_version"),
        "case_id": audit.get("case_id"),
        "prompt_version": audit.get("prompt_version"),
        "report_count": audit.get("report_count"),
        "ratings_summary": audit.get("ratings_summary", {}),
        "data_integrity": audit.get("data_integrity", {}),
        "core_hypothesis_assessment": audit.get(
            "core_hypothesis_assessment",
            {},
        ),
        "recommended_follow_up": audit.get(
            "recommended_follow_up",
            {},
        ),
    }


def _structured_distributions(
    runs: list[dict[str, Any]],
) -> dict[str, Any]:
    contradiction_levels: list[str] = []
    used_fragment_counts: list[int] = []
    unsupported_counts: list[int] = []
    style_tags: list[str] = []
    dimension_values: dict[str, list[str]] = {
        key: [] for key in DIMENSION_KEYS
    }

    for run in runs:
        contradiction = run.get("contradiction_handling")
        if contradiction:
            contradiction_levels.append(str(contradiction))

        used_fragment_counts.append(
            int(run.get("used_fragment_count", 0))
        )
        unsupported_counts.append(
            int(run.get("unsupported_assumption_count", 0))
        )

        for tag in run.get("style_tags", []):
            style_tags.append(str(tag))

        ratings = run.get("dimension_ratings")
        if not isinstance(ratings, dict):
            continue

        for key in DIMENSION_KEYS:
            value = ratings.get(key)
            if value is not None:
                dimension_values[key].append(str(value))

    return {
        "recognized_disposition_id": _counter(
            run.get("recognized_disposition_id", "missing")
            for run in runs
        ),
        "contradiction_handling_level": _counter(
            contradiction_levels
        ),
        "dimension_ratings": {
            key: _counter(values)
            for key, values in dimension_values.items()
        },
        "used_fragment_count": _counter(used_fragment_counts),
        "unsupported_assumption_count": _counter(
            unsupported_counts
        ),
        "style_tags": _counter(style_tags),
    }


def _aggregate(
    runs: list[dict[str, Any]],
) -> dict[str, Any]:
    usage_keys = (
        "prompt_tokens",
        "completion_tokens",
        "total_tokens",
        "prompt_cache_hit_tokens",
        "prompt_cache_miss_tokens",
    )
    usage_totals = {
        key: sum(int(run["usage"].get(key, 0)) for run in runs)
        for key in usage_keys
    }

    statuses = [
        str(run.get("status", "missing")).lower()
        for run in runs
    ]
    validation_values = [
        run.get("validation_passed")
        for run in runs
    ]

    provider_failures = sum(
        status in PROVIDER_FAILURE_STATUSES
        for status in statuses
    )
    parse_failures = sum(
        status in PARSE_FAILURE_STATUSES
        for status in statuses
    )
    validation_failures = sum(
        status in VALIDATION_FAILURE_STATUSES
        or value is False
        for status, value in zip(statuses, validation_values)
    )
    incomplete_pairs = sum(
        not run.get("raw_present")
        or not run.get("validated_present")
        for run in runs
    )

    validated_count = sum(
        value is True for value in validation_values
    )
    call_count = len(runs)

    cache_modes = {
        str(run["usage"].get("pricing_mode"))
        for run in runs
    }
    if not cache_modes:
        cost_method = "not_available"
    elif cache_modes == {"cache_aware"}:
        cost_method = "cache_aware_exact_from_recorded_split"
    elif cache_modes == {
        "conservative_all_prompt_as_cache_miss"
    }:
        cost_method = "conservative_upper_bound"
    else:
        cost_method = "mixed_cache_aware_and_conservative"

    elapsed = [
        int(run["elapsed_ms"])
        for run in runs
        if _safe_int(run.get("elapsed_ms")) is not None
    ]

    prompt_tokens = [
        int(run["usage"]["prompt_tokens"]) for run in runs
    ]
    completion_tokens = [
        int(run["usage"]["completion_tokens"]) for run in runs
    ]
    total_tokens = [
        int(run["usage"]["total_tokens"]) for run in runs
    ]

    return {
        "call_count": call_count,
        "validated_count": validated_count,
        "validation_success_rate": (
            round(validated_count / call_count, 6)
            if call_count
            else None
        ),
        "provider_failure_count": provider_failures,
        "json_parse_failure_count": parse_failures,
        "validation_failure_count": validation_failures,
        "incomplete_pair_count": incomplete_pairs,
        "status_distribution": _counter(statuses),
        "selected_disposition_distribution": _counter(
            run.get("selected_disposition_id", "missing")
            for run in runs
        ),
        "recognized_disposition_distribution": _counter(
            run.get("recognized_disposition_id", "missing")
            for run in runs
        ),
        "usage_totals": usage_totals,
        "token_stats_per_run": {
            "prompt_tokens": _stats(prompt_tokens),
            "completion_tokens": _stats(completion_tokens),
            "total_tokens": _stats(total_tokens),
        },
        "latency_ms": _stats(elapsed),
        "cost": _cost(usage_totals),
        "cost_method": cost_method,
        "average_cost_per_run": {
            "usd": (
                _money(
                    _decimal(_cost(usage_totals)["usd"])
                    / Decimal(call_count)
                )
                if call_count
                else None
            ),
            "cny": (
                _money(
                    _decimal(_cost(usage_totals)["cny"])
                    / Decimal(call_count)
                )
                if call_count
                else None
            ),
        },
        "structured_distributions": _structured_distributions(
            runs
        ),
    }


def collect(
    raw_dir: Path,
    validated_dir: Path,
    audit_path: Path,
) -> dict[str, Any]:
    warnings: list[str] = []

    raw_by_id = _index_results(
        raw_dir,
        warnings,
        "raw",
    )
    validated_by_id = _index_results(
        validated_dir,
        warnings,
        "validated",
    )

    for run_id in sorted(set(raw_by_id) - set(validated_by_id)):
        warnings.append(
            f"Raw result has no validated pair: {run_id}"
        )

    for run_id in sorted(set(validated_by_id) - set(raw_by_id)):
        warnings.append(
            f"Validated result has no raw pair: {run_id}"
        )

    runs: list[dict[str, Any]] = []

    for run_id in sorted(set(raw_by_id) | set(validated_by_id)):
        raw_entries = raw_by_id.get(run_id, [])
        validated_entries = validated_by_id.get(run_id, [])

        raw_path, raw = (
            raw_entries[0]
            if raw_entries
            else (None, None)
        )
        validated_path, validated = (
            validated_entries[0]
            if validated_entries
            else (None, None)
        )

        metadata = _metadata_from_pair(raw, validated)
        report_id = _report_id(metadata)
        classification = _classify(metadata, report_id)

        judgement = _judgement_from_validated(validated)

        validation = (
            raw.get("validation", {})
            if isinstance(raw, dict)
            else {}
        )
        validation = (
            validation if isinstance(validation, dict) else {}
        )

        usage = _usage(
            metadata,
            warnings,
            run_id,
        )

        used_fragments = judgement.get(
            "used_fragment_ids",
            [],
        )
        if not isinstance(used_fragments, list):
            warnings.append(
                f"used_fragment_ids is not an array: {run_id}"
            )
            used_fragments = []

        unsupported = judgement.get(
            "unsupported_assumptions",
            [],
        )
        if not isinstance(unsupported, list):
            warnings.append(
                f"unsupported_assumptions is not an array: "
                f"{run_id}"
            )
            unsupported = []

        contradiction = judgement.get(
            "contradiction_handling",
            {},
        )
        contradiction = (
            contradiction
            if isinstance(contradiction, dict)
            else {}
        )

        ratings = judgement.get("dimension_ratings", {})
        ratings = ratings if isinstance(ratings, dict) else {}

        style_tags = judgement.get("style_tags", [])
        style_tags = (
            style_tags if isinstance(style_tags, list) else []
        )

        selected_disposition = (
            metadata.get("selected_disposition_id")
            or judgement.get("recognized_disposition_id")
        )

        status = (
            raw.get("status", "missing")
            if isinstance(raw, dict)
            else "missing"
        )

        runs.append(
            {
                "run_id": run_id,
                "timestamp_utc": (
                    metadata.get("timestamp_utc")
                    or metadata.get("created_at_utc")
                    or metadata.get("started_at_utc")
                ),
                "report_id": report_id,
                "classification": classification,
                "provider": metadata.get("provider"),
                "model_requested": metadata.get(
                    "model_requested"
                ),
                "model_returned": metadata.get(
                    "model_returned"
                ),
                "finish_reason": metadata.get(
                    "finish_reason"
                ),
                "prompt_version": metadata.get(
                    "prompt_version"
                ),
                "schema_version": metadata.get(
                    "schema_version"
                ),
                "case_id": metadata.get("case_id"),
                "case_version": metadata.get(
                    "case_version"
                ),
                "judge_profile_id": metadata.get(
                    "judge_profile_id"
                ),
                "judge_version": metadata.get(
                    "judge_version"
                ),
                "report_path": metadata.get(
                    "report_path"
                ),
                "raw_file": (
                    raw_path.name if raw_path else None
                ),
                "validated_file": (
                    validated_path.name
                    if validated_path
                    else None
                ),
                "raw_present": raw is not None,
                "validated_present": validated is not None,
                "status": status,
                "validation_passed": validation.get(
                    "passed"
                ),
                "elapsed_ms": metadata.get(
                    "elapsed_ms"
                ),
                "usage": usage,
                "cost": _cost(usage),
                "selected_disposition_id": (
                    selected_disposition
                ),
                "recognized_disposition_id": (
                    judgement.get(
                        "recognized_disposition_id"
                    )
                ),
                "used_fragment_count": len(
                    used_fragments
                ),
                "unsupported_assumption_count": len(
                    unsupported
                ),
                "contradiction_handling": (
                    contradiction.get("level")
                ),
                "dimension_ratings": ratings,
                "style_tags": style_tags,
            }
        )

    corpus_runs = [
        run
        for run in runs
        if run["classification"] == "prompt_v03_corpus"
    ]
    all_week8_runs = [
        run
        for run in runs
        if run["classification"] != "unclassified"
    ]

    return {
        "summary_version": "0.2",
        "generated_at_utc": datetime.now(
            timezone.utc
        ).isoformat(),
        "pricing_reference_date": PRICING[
            "pricing_date"
        ],
        "pricing": PRICING,
        "run_inventory": {
            "day2_baseline": [
                run["run_id"]
                for run in runs
                if run["classification"] == "day2_baseline"
            ],
            "prompt_v02_ab": [
                run["run_id"]
                for run in runs
                if run["classification"] == "prompt_v02_ab"
            ],
            "prompt_v03_corpus": [
                run["run_id"] for run in corpus_runs
            ],
            "unclassified": [
                run["run_id"]
                for run in runs
                if run["classification"] == "unclassified"
            ],
        },
        "all_week8_calls": _aggregate(all_week8_runs),
        "prompt_v03_corpus": _aggregate(corpus_runs),
        "runs": runs,
        "semantic_audit": _load_audit(
            audit_path,
            warnings,
        ),
        "warnings": sorted(set(warnings)),
    }


def _percentage(
    numerator: int,
    denominator: int,
) -> str:
    if denominator == 0:
        return "n/a"
    return f"{100 * numerator / denominator:.2f}%"


def _format_distribution(
    value: dict[str, int],
) -> str:
    if not value:
        return "`{}`"
    return "`" + json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
    ) + "`"


def _markdown_scope_row(
    name: str,
    scope: dict[str, Any],
) -> str:
    latency = scope["latency_ms"]
    token_stats = scope["token_stats_per_run"]["total_tokens"]

    return (
        f"| {name} | {scope['call_count']} | "
        f"{scope['validated_count']} "
        f"({_percentage(scope['validated_count'], scope['call_count'])}) | "
        f"{scope['usage_totals']['prompt_tokens']:,} | "
        f"{scope['usage_totals']['completion_tokens']:,} | "
        f"{scope['usage_totals']['total_tokens']:,} | "
        f"{token_stats['average']} / {token_stats['median']} / "
        f"{token_stats['min']} / {token_stats['max']} | "
        f"{latency['average']} / {latency['median']} / "
        f"{latency['min']} / {latency['max']} | "
        f"${scope['cost']['usd']} | "
        f"CNY {scope['cost']['cny']} |"
    )


def markdown(summary: dict[str, Any]) -> str:
    all_calls = summary["all_week8_calls"]
    corpus = summary["prompt_v03_corpus"]
    audit = summary["semantic_audit"]

    ratings = (
        audit.get("ratings_summary", {})
        if audit.get("available")
        else {}
    )
    correct = int(ratings.get("correct", 0))
    acceptable = int(ratings.get("acceptable", 0))
    report_count = int(
        audit.get("report_count")
        or corpus["call_count"]
    )
    directly_usable = correct + acceptable

    corpus_dist = corpus["structured_distributions"]

    lines = [
        "# Week8 Experiment Summary",
        "",
        f"Generated (UTC): `{summary['generated_at_utc']}`",
        "",
        "## Executive Summary",
        "",
        (
            f"- Saved Week8 real calls: **{all_calls['call_count']}**; "
            f"validated: **{all_calls['validated_count']} "
            f"({_percentage(all_calls['validated_count'], all_calls['call_count'])})**."
        ),
        (
            f"- Prompt v0.3 corpus: **{corpus['call_count']}** saved calls; "
            f"validated: **{corpus['validated_count']} "
            f"({_percentage(corpus['validated_count'], corpus['call_count'])})**."
        ),
        (
            f"- Semantic audit: Correct {ratings.get('correct', 0)}, "
            f"Acceptable {ratings.get('acceptable', 0)}, "
            f"Questionable {ratings.get('questionable', 0)}, "
            f"Incorrect {ratings.get('incorrect', 0)}, "
            f"Not Auditable {ratings.get('not_auditable', 0)}."
        ),
        (
            f"- Directly acceptable semantic results: "
            f"**{directly_usable}/{report_count} "
            f"({_percentage(directly_usable, report_count)})**."
        ),
        "- Core hypothesis: **Promising**, not Proven.",
        "",
        "## Run Inventory",
        "",
        "| Experiment layer | Calls |",
        "|---|---:|",
        (
            "| Prompt v0.1 baseline | "
            f"{len(summary['run_inventory']['day2_baseline'])} |"
        ),
        (
            "| Prompt v0.2 A/B | "
            f"{len(summary['run_inventory']['prompt_v02_ab'])} |"
        ),
        (
            "| Prompt v0.3 corpus | "
            f"{len(summary['run_inventory']['prompt_v03_corpus'])} |"
        ),
        (
            "| Unclassified | "
            f"{len(summary['run_inventory']['unclassified'])} |"
        ),
        "",
        "## Token, Latency and Cost",
        "",
        (
            "Pricing reference date: "
            f"**{summary['pricing_reference_date']}**. "
            "Provider prices may change."
        ),
        "",
        (
            "| Scope | Calls | Validated | Prompt tokens | Completion tokens | "
            "Total tokens | Total-token avg / median / min / max | "
            "Latency avg / median / min / max (ms) | USD | CNY |"
        ),
        (
            "|---|---:|---:|---:|---:|---:|---|---|---:|---:|"
        ),
        _markdown_scope_row("All Week8", all_calls),
        _markdown_scope_row("Prompt v0.3 corpus", corpus),
        "",
        (
            f"- All Week8 cost method: `{all_calls['cost_method']}`; "
            f"average cost/run: ${all_calls['average_cost_per_run']['usd']} "
            f"/ CNY {all_calls['average_cost_per_run']['cny']}."
        ),
        (
            f"- v0.3 cost method: `{corpus['cost_method']}`; "
            f"average cost/run: ${corpus['average_cost_per_run']['usd']} "
            f"/ CNY {corpus['average_cost_per_run']['cny']}."
        ),
        "",
        "## Failure Summary",
        "",
        "| Scope | Provider failures | JSON parse failures | Validation failures | Incomplete pairs |",
        "|---|---:|---:|---:|---:|",
        (
            f"| All Week8 | {all_calls['provider_failure_count']} | "
            f"{all_calls['json_parse_failure_count']} | "
            f"{all_calls['validation_failure_count']} | "
            f"{all_calls['incomplete_pair_count']} |"
        ),
        (
            f"| Prompt v0.3 corpus | {corpus['provider_failure_count']} | "
            f"{corpus['json_parse_failure_count']} | "
            f"{corpus['validation_failure_count']} | "
            f"{corpus['incomplete_pair_count']} |"
        ),
        "",
        "## Prompt v0.3 Structured Output Distribution",
        "",
        (
            "- Recognized dispositions: "
            + _format_distribution(
                corpus_dist["recognized_disposition_id"]
            )
        ),
        (
            "- Contradiction handling: "
            + _format_distribution(
                corpus_dist["contradiction_handling_level"]
            )
        ),
        (
            "- Used Fragment count: "
            + _format_distribution(
                corpus_dist["used_fragment_count"]
            )
        ),
        (
            "- Unsupported assumption count: "
            + _format_distribution(
                corpus_dist["unsupported_assumption_count"]
            )
        ),
        "- Dimension ratings:",
    ]

    for key in DIMENSION_KEYS:
        lines.append(
            f"  - `{key}`: "
            + _format_distribution(
                corpus_dist["dimension_ratings"].get(
                    key,
                    {},
                )
            )
        )

    lines.extend(
        [
            (
                "- Style tags: "
                + _format_distribution(
                    corpus_dist["style_tags"]
                )
            ),
            "",
            "## Semantic Audit",
            "",
            (
                "- Ratings: "
                + _format_distribution(ratings)
            ),
            (
                "- Core hypothesis: "
                f"`{audit.get('core_hypothesis_assessment', {}).get('rating', 'unknown')}`."
            ),
            "- Major successes: structured validation, disposition integrity, reasonable-possibility handling, rhetoric/evidence separation, and prompt-injection resistance.",
            "- Major risks: Fragment Mapping instability, complex unsupported-claim splitting, game-language leakage, sparse evidence density, and detain-for-review safety bias.",
            "",
            "## Week8 Completion",
            "",
            "- Technical prototype: **Completed**.",
            "- Semantic feasibility: **Promising**.",
            "- Production readiness: **Not Ready**.",
            "- Recording: intentionally deferred by the user because this is an experimental prototype; it is not a Push blocker.",
            "",
            "## Warnings",
            "",
        ]
    )

    warnings = summary.get("warnings", [])
    if warnings:
        lines.extend(f"- {warning}" for warning in warnings)
    else:
        lines.append("- None.")

    lines.append("")
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
    )
    parser.add_argument(
        "--raw-dir",
        type=Path,
        default=Path("results/raw"),
    )
    parser.add_argument(
        "--validated-dir",
        type=Path,
        default=Path("results/validated"),
    )
    parser.add_argument(
        "--audit-json",
        type=Path,
        default=Path(
            "reports/semantic_audit/"
            "semantic_audit_results_v0_1.json"
        ),
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("reports/week8_summary"),
    )
    parser.add_argument(
        "--output-md",
        type=Path,
    )
    parser.add_argument(
        "--output-json",
        type=Path,
    )
    args = parser.parse_args()

    summary = collect(
        args.raw_dir,
        args.validated_dir,
        args.audit_json,
    )

    output_md = (
        args.output_md
        or args.output_dir / "Week8_Experiment_Summary.md"
    )
    output_json = (
        args.output_json
        or args.output_dir
        / "week8_experiment_summary_v0_2.json"
    )

    output_md.parent.mkdir(
        parents=True,
        exist_ok=True,
    )
    output_json.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    output_json.write_text(
        json.dumps(
            summary,
            ensure_ascii=False,
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    output_md.write_text(
        markdown(summary),
        encoding="utf-8",
    )

    all_calls = summary["all_week8_calls"]
    corpus = summary["prompt_v03_corpus"]

    print("WEEK8 SUMMARY GENERATED")
    print(
        "All Week8 real calls: "
        f"{all_calls['call_count']}"
    )
    print(
        "Prompt v0.3 corpus calls: "
        f"{corpus['call_count']}"
    )
    print(
        "Validated: "
        f"{all_calls['validated_count']}/"
        f"{all_calls['call_count']}"
    )
    print(
        "All Week8 cost: "
        f"USD ${all_calls['cost']['usd']} / "
        f"CNY {all_calls['cost']['cny']}"
    )
    print(
        "Audit ratings: "
        f"{summary['semantic_audit'].get('ratings_summary', {})}"
    )
    print(f"Markdown: {output_md}")
    print(f"JSON: {output_json}")

    if summary["warnings"]:
        print("WARNINGS:")
        for warning in summary["warnings"]:
            print(f"- {warning}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
