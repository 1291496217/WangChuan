from __future__ import annotations

"""Single-call Day7 MR15 semantic-drift runner.

This file intentionally keeps the Day7 experiment boundary explicit: each
process performs one fresh DeepSeek request, records a raw envelope for every
outcome, and publishes a validated envelope only when all local gates pass.
There is no retry loop, conversation history, or memory between repeats.
"""

import argparse
import json
import os
import sys
import time
from pathlib import Path
from typing import Any

from ai_client import AIClientError, DeepSeekAIClient, DeepSeekClientConfig
from audit_game_language_v0_3 import audit_game_language_v0_3
from env_loader import (
    load_env_file,
    read_float_environment_value,
    read_int_environment_value,
    require_environment_value,
)
from prompt_builder_v0_4_6 import build_prompt_v0_4_6
from report_parser_v0_2 import load_player_report_v0_2
from response_validator_v0_2 import (
    load_json_object_v0_2,
    parse_judgement_result_v0_2,
    validate_judgement_result_v0_2,
)
from result_writer import (
    create_run_id,
    sha256_text,
    utc_now_iso,
    write_raw_record,
    write_validated_record,
)
from run_real_judgement_v0_2 import _paths, _report_record
from runtime_contract_v0_2 import build_validated_run_envelope_v0_2


EXPERIMENT_ID = "Week8.Day7.Drift.MR15.001"
PROMPT_VERSION = "0.4.6"
SCHEMA_VERSION = "0.2"
LANGUAGE_AUDIT_VERSION = "0.3"
CORRECTION_STAGE = "Week8 Day7 Revised Semantic Drift"
DEFAULT_RESULTS = Path("results/week8b_day7_drift_mr15_v0_1")


def _config() -> DeepSeekClientConfig:
    thinking = os.environ.get("DEEPSEEK_THINKING", "disabled").strip().lower()
    if thinking not in {"enabled", "disabled"}:
        raise ValueError("DEEPSEEK_THINKING must be enabled or disabled")
    return DeepSeekClientConfig(
        api_key=require_environment_value("DEEPSEEK_API_KEY"),
        base_url=os.environ.get("DEEPSEEK_BASE_URL", "https://api.deepseek.com").strip(),
        model=os.environ.get("DEEPSEEK_MODEL", "deepseek-v4-flash").strip(),
        timeout_seconds=read_float_environment_value(
            "DEEPSEEK_TIMEOUT_SECONDS", default=90.0, minimum=1.0, maximum=600.0
        ),
        max_tokens=read_int_environment_value(
            "DEEPSEEK_MAX_TOKENS", default=2400, minimum=256, maximum=20000
        ),
        thinking_mode=thinking,
        temperature=read_float_environment_value(
            "DEEPSEEK_TEMPERATURE", default=0.2, minimum=0.0, maximum=2.0
        ),
    )


def _metadata(
    *,
    run_id: str,
    repeat_index: int,
    started: str,
    completed: str,
    elapsed_ms: int,
    config: DeepSeekClientConfig,
    case_data: dict[str, Any],
    judge_data: dict[str, Any],
    report_path: Path,
    report: Any,
    messages: list[dict[str, str]],
    returned_model: str | None = None,
    response_id: str | None = None,
    finish_reason: str | None = None,
    usage: dict[str, Any] | None = None,
    validation_status: str = "not_run",
    publication_status: str = "not_run",
) -> dict[str, Any]:
    return {
        "run_id": run_id,
        "experiment_id": EXPERIMENT_ID,
        "repeat_index": repeat_index,
        "repeat_policy": "independent_no_history_no_memory_no_retry",
        "timestamp_utc": completed,
        "started_at_utc": started,
        "completed_at_utc": completed,
        "correction_stage": CORRECTION_STAGE,
        "provider": "deepseek",
        "model_requested": config.model,
        "model_returned": returned_model,
        "base_url": config.base_url,
        "prompt_version": PROMPT_VERSION,
        "schema_version": SCHEMA_VERSION,
        "language_audit_version": LANGUAGE_AUDIT_VERSION,
        "case_id": case_data["case_id"],
        "case_version": case_data["case_version"],
        "judge_profile_id": judge_data["judge_profile_id"],
        "judge_version": judge_data.get("version"),
        "report_id": "MR15",
        "report_path": str(report_path),
        "formal_moral_judgement_id": report.moral_judgement_id,
        "formal_disposition_id": report.disposition_id,
        "selected_key_fragment_ids": list(report.selected_key_fragment_ids),
        "elapsed_ms": elapsed_ms,
        "finish_reason": finish_reason,
        "usage": usage,
        "response_id": response_id,
        "messages_sha256": sha256_text(
            json.dumps(messages, ensure_ascii=False, sort_keys=True)
        ),
        "validation_status": validation_status,
        "publication_status": publication_status,
    }


def run_one(*, repeat_index: int, results_root: Path, confirm: bool) -> int:
    if repeat_index not in range(1, 6):
        raise ValueError("repeat_index must be between 1 and 5")
    root = Path(__file__).resolve().parent.parent
    load_env_file(root / ".env")
    config = _config()
    report_path, _case_path, case_data, judge_data = _paths(root, "MR15")
    report = load_player_report_v0_2(report_path, case_data=case_data)
    schema = load_json_object_v0_2(root / "schemas" / "judgement_result_v0_2.json")
    messages = build_prompt_v0_4_6(
        case_data=case_data,
        judge_data=judge_data,
        player_report=report,
        schema_data=schema,
    )
    if not confirm:
        answer = input("Type RUN MR15 to perform exactly one API call: ").strip()
        if answer != "RUN MR15":
            print("Cancelled before API call.")
            return 0

    run_id = create_run_id()
    started = utc_now_iso()
    begin = time.monotonic()

    try:
        response = DeepSeekAIClient(config).create_judgement(messages)
    except AIClientError as error:
        completed = utc_now_iso()
        elapsed = int((time.monotonic() - begin) * 1000)
        metadata = _metadata(
            run_id=run_id,
            repeat_index=repeat_index,
            started=started,
            completed=completed,
            elapsed_ms=elapsed,
            config=config,
            case_data=case_data,
            judge_data=judge_data,
            report_path=report_path,
            report=report,
            messages=messages,
            validation_status="provider_error",
            publication_status="blocked",
        )
        raw_path = write_raw_record(
            results_root=results_root,
            run_id=run_id,
            payload={
                "status": "provider_error",
                "metadata": metadata,
                "player_report": _report_record(report),
                "request_messages": messages,
                "error": {"type": type(error).__name__, "message": str(error)},
            },
        )
        print(f"PROVIDER_ERROR MR15 repeat={repeat_index} raw={raw_path}", file=sys.stderr)
        return 2

    completed = utc_now_iso()
    elapsed = int((time.monotonic() - begin) * 1000)
    base_metadata = _metadata(
        run_id=run_id,
        repeat_index=repeat_index,
        started=started,
        completed=completed,
        elapsed_ms=elapsed,
        config=config,
        case_data=case_data,
        judge_data=judge_data,
        report_path=report_path,
        report=report,
        messages=messages,
        returned_model=response.returned_model,
        response_id=response.response_id,
        finish_reason=response.finish_reason,
        usage=response.usage,
    )
    try:
        payload = json.loads(response.content)
    except json.JSONDecodeError as error:
        raw_path = write_raw_record(
            results_root=results_root,
            run_id=run_id,
            payload={
                "status": "json_parse_failed",
                "metadata": {
                    **base_metadata,
                    "validation_status": "json_parse_failed",
                    "publication_status": "blocked",
                },
                "player_report": _report_record(report),
                "request_messages": messages,
                "provider_content": response.content,
                "error": {"type": type(error).__name__, "message": str(error)},
            },
        )
        print(f"JSON_PARSE_FAILED MR15 repeat={repeat_index} raw={raw_path}", file=sys.stderr)
        return 3

    validation = validate_judgement_result_v0_2(
        payload, schema_data=schema, case_data=case_data, judge_data=judge_data
    )
    audit = audit_game_language_v0_3(payload) if isinstance(payload, dict) else None
    language_status = audit.status if audit else "WARNING"
    publishable = validation.is_valid and language_status == "PASS"
    final_metadata = {
        **base_metadata,
        "validation_status": "passed" if validation.is_valid else "failed",
        "publication_status": "passed" if publishable else "blocked",
    }
    raw_path = write_raw_record(
        results_root=results_root,
        run_id=run_id,
        payload={
            "status": "validated" if publishable else ("language_gate_failed" if validation.is_valid else "validation_failed"),
            "metadata": final_metadata,
            "player_report": _report_record(report),
            "request_messages": messages,
            "provider_content": response.content,
            "parsed_payload": payload,
            "validation": {
                "passed": validation.is_valid,
                "issues": [issue.__dict__ for issue in validation.issues],
            },
            "game_language_audit": audit.__dict__ if audit else None,
        },
    )
    if not validation.is_valid:
        print(f"VALIDATION_FAILED MR15 repeat={repeat_index} raw={raw_path}", file=sys.stderr)
        return 4
    if not publishable:
        print(f"LANGUAGE_GATE_FAILED MR15 repeat={repeat_index} raw={raw_path}", file=sys.stderr)
        return 5

    result = parse_judgement_result_v0_2(
        payload, schema_data=schema, case_data=case_data, judge_data=judge_data
    )
    envelope = build_validated_run_envelope_v0_2(
        run_id=run_id,
        result=result,
        case_data=case_data,
        judge_data=judge_data,
        player_report=report,
        metadata={**final_metadata, "game_language_status": language_status},
    )
    validated_path = write_validated_record(
        results_root=results_root,
        run_id=run_id,
        payload=envelope.to_dict(),
    )
    print(
        f"VALIDATED MR15 repeat={repeat_index} {run_id} elapsed_ms={elapsed} "
        f"raw={raw_path} validated={validated_path} language={language_status}"
    )
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="One independent Week8 Day7 MR15 drift call")
    parser.add_argument("--repeat-index", type=int, required=True, choices=range(1, 6))
    parser.add_argument("--results", type=Path, default=DEFAULT_RESULTS)
    parser.add_argument("--confirm", action="store_true")
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    results_root = args.results if args.results.is_absolute() else root / args.results
    return run_one(repeat_index=args.repeat_index, results_root=results_root, confirm=args.confirm)


if __name__ == "__main__":
    raise SystemExit(main())
