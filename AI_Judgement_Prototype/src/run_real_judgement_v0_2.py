from __future__ import annotations

import argparse
import json
import os
import sys
import time
from pathlib import Path
from typing import Any

from ai_client import AIClientError, DeepSeekAIClient, DeepSeekClientConfig
from audit_game_language import audit_game_language
from corpus_loader_v0_2 import load_and_validate_corpus_v0_2
from env_loader import load_env_file, read_float_environment_value, read_int_environment_value, require_environment_value
from models_v0_2 import ValidatedRunEnvelopeV02
from prompt_builder_v0_4 import PROMPT_VERSION, build_prompt_v0_4
from report_parser_v0_2 import load_player_report_v0_2
from response_validator_v0_2 import load_json_object_v0_2, parse_judgement_result_v0_2, validate_judgement_result_v0_2
from result_writer import create_run_id, sha256_text, utc_now_iso, write_raw_record, write_validated_record
from runtime_contract_v0_2 import build_validated_run_envelope_v0_2

PROVIDER_NAME = "deepseek"
DEFAULT_BASE_URL = "https://api.deepseek.com"
DEFAULT_MODEL = "deepseek-v4-flash"
SCHEMA_VERSION = "0.2"


def _config() -> DeepSeekClientConfig:
    thinking = os.environ.get("DEEPSEEK_THINKING", "disabled").strip().lower()
    if thinking not in {"enabled", "disabled"}:
        raise ValueError("DEEPSEEK_THINKING must be enabled or disabled")
    return DeepSeekClientConfig(
        api_key=require_environment_value("DEEPSEEK_API_KEY"),
        base_url=os.environ.get("DEEPSEEK_BASE_URL", DEFAULT_BASE_URL).strip(),
        model=os.environ.get("DEEPSEEK_MODEL", DEFAULT_MODEL).strip(),
        timeout_seconds=read_float_environment_value("DEEPSEEK_TIMEOUT_SECONDS", default=90.0, minimum=1.0, maximum=600.0),
        max_tokens=read_int_environment_value("DEEPSEEK_MAX_TOKENS", default=2400, minimum=256, maximum=20000),
        thinking_mode=thinking,
        temperature=read_float_environment_value("DEEPSEEK_TEMPERATURE", default=0.2, minimum=0.0, maximum=2.0),
    )


def _paths(root: Path, report_id: str) -> tuple[Path, Path, dict[str, Any], dict[str, Any]]:
    manifest_path = root / "reports" / "corpus_manifest_moral_week8b_v0_1.json"
    corpus = load_and_validate_corpus_v0_2(project_root=root, manifest_path=manifest_path)
    if not corpus.is_valid:
        raise ValueError("Week8B corpus validation failed:\n" + corpus.format_errors())
    entry = next((x for x in corpus.entries if x.report_id == report_id), None)
    if entry is None or entry.parsed_report is None:
        raise ValueError(f"Report {report_id} is not a valid Week8B report")
    case_id = entry.parsed_report.case_id
    case_path = root / "cases" / ("case_door_knife_001.json" if case_id == "Case.DoorKnife.001" else "case_medicine_001.json")
    case_data = load_json_object_v0_2(case_path)
    judge_data = load_json_object_v0_2(root / "judges" / "judge_clerk_001.json")
    return entry.file_path, case_path, case_data, judge_data


def _metadata(*, run_id: str, started: str, completed: str, elapsed_ms: int, config: DeepSeekClientConfig, case_data: dict[str, Any], judge_data: dict[str, Any], report_path: Path, report: Any, messages: list[dict[str, str]], returned_model: str | None = None, response_id: str | None = None, finish_reason: str | None = None, usage: dict[str, Any] | None = None, validation_status: str = "not_run") -> dict[str, Any]:
    return {
        "run_id": run_id, "timestamp_utc": completed, "started_at_utc": started, "completed_at_utc": completed,
        "provider": PROVIDER_NAME, "model_requested": config.model, "model_returned": returned_model,
        "base_url": config.base_url, "prompt_version": PROMPT_VERSION, "schema_version": SCHEMA_VERSION,
        "case_id": case_data["case_id"], "case_version": case_data["case_version"],
        "judge_profile_id": judge_data["judge_profile_id"], "judge_version": judge_data.get("version"),
        "report_id": report_path.stem.split("_", 1)[0], "report_path": str(report_path),
        "formal_moral_judgement_id": report.moral_judgement_id, "formal_disposition_id": report.disposition_id,
        "selected_key_fragment_ids": list(report.selected_key_fragment_ids), "elapsed_ms": elapsed_ms,
        "finish_reason": finish_reason, "usage": usage, "response_id": response_id,
        "messages_sha256": sha256_text(json.dumps(messages, ensure_ascii=False, sort_keys=True)),
        "validation_status": validation_status,
    }


def _report_record(report: Any) -> dict[str, Any]:
    return {
        "case_id": report.case_id,
        "moral_judgement_id": report.moral_judgement_id,
        "disposition_id": report.disposition_id,
        "selected_key_fragment_ids": list(report.selected_key_fragment_ids),
        "life_interpretation": report.life_interpretation,
        "verdict_text": report.verdict_text,
    }


def run_one(*, report_id: str, results_root: Path, confirm: bool = False) -> int:
    root = Path(__file__).resolve().parent.parent
    load_env_file(root / ".env")
    config = _config()
    report_path, _case_path, case_data, judge_data = _paths(root, report_id)
    report = load_player_report_v0_2(report_path, case_data=case_data)
    schema = load_json_object_v0_2(root / "schemas" / "judgement_result_v0_2.json")
    messages = build_prompt_v0_4(case_data=case_data, judge_data=judge_data, player_report=report, schema_data=schema)
    if not confirm:
        answer = input(f"Type RUN {report_id} to perform exactly one API call: ").strip()
        if answer != f"RUN {report_id}":
            print("Cancelled before API call.")
            return 0
    run_id = create_run_id()
    started = utc_now_iso()
    begin = time.monotonic()
    common = _metadata(run_id=run_id, started=started, completed=started, elapsed_ms=0, config=config, case_data=case_data, judge_data=judge_data, report_path=report_path, report=report, messages=messages)
    try:
        response = DeepSeekAIClient(config).create_judgement(messages)
    except AIClientError as error:
        completed = utc_now_iso(); elapsed = int((time.monotonic() - begin) * 1000)
        common = _metadata(run_id=run_id, started=started, completed=completed, elapsed_ms=elapsed, config=config, case_data=case_data, judge_data=judge_data, report_path=report_path, report=report, messages=messages, validation_status="provider_error")
        path = write_raw_record(results_root=results_root, run_id=run_id, payload={"status": "provider_error", "metadata": common, "player_report": _report_record(report), "request_messages": messages, "error": {"type": type(error).__name__, "message": str(error)}})
        print(f"PROVIDER_ERROR {report_id} {path}", file=sys.stderr)
        return 2
    completed = utc_now_iso(); elapsed = int((time.monotonic() - begin) * 1000)
    common = _metadata(run_id=run_id, started=started, completed=completed, elapsed_ms=elapsed, config=config, case_data=case_data, judge_data=judge_data, report_path=report_path, report=report, messages=messages, returned_model=response.returned_model, response_id=response.response_id, finish_reason=response.finish_reason, usage=response.usage)
    try:
        payload = json.loads(response.content)
    except json.JSONDecodeError as error:
        path = write_raw_record(results_root=results_root, run_id=run_id, payload={"status": "json_parse_failed", "metadata": {**common, "validation_status": "json_parse_failed"}, "player_report": _report_record(report), "request_messages": messages, "provider_content": response.content, "error": {"type": type(error).__name__, "message": str(error)}})
        print(f"JSON_PARSE_FAILED {report_id} {path}", file=sys.stderr); return 3
    validation = validate_judgement_result_v0_2(payload, schema_data=schema, case_data=case_data, judge_data=judge_data)
    audit = audit_game_language(payload) if isinstance(payload, dict) else None
    raw_payload = {"status": "validated" if validation.is_valid else "validation_failed", "metadata": {**common, "validation_status": "passed" if validation.is_valid else "failed"}, "player_report": _report_record(report), "request_messages": messages, "provider_content": response.content, "parsed_payload": payload, "validation": {"passed": validation.is_valid, "issues": [x.__dict__ for x in validation.issues]}, "game_language_audit": audit.__dict__ if audit else None}
    raw_path = write_raw_record(results_root=results_root, run_id=run_id, payload=raw_payload)
    if not validation.is_valid:
        print(f"VALIDATION_FAILED {report_id} {raw_path}", file=sys.stderr); return 4
    result = parse_judgement_result_v0_2(payload, schema_data=schema, case_data=case_data, judge_data=judge_data)
    envelope = build_validated_run_envelope_v0_2(run_id=run_id, result=result, case_data=case_data, judge_data=judge_data, player_report=report, metadata={**common, "validation_status": "passed", "game_language_status": audit.status if audit else "PASS"})
    validated_path = write_validated_record(results_root=results_root, run_id=run_id, payload=envelope.to_dict())
    print(f"VALIDATED {report_id} {run_id} elapsed_ms={elapsed} raw={raw_path} validated={validated_path} language={audit.status if audit else 'PASS'}")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="One controlled Week8B v0.2 semantic call")
    parser.add_argument("--report-id", required=True)
    parser.add_argument("--results", type=Path, default=Path("results/week8b_v0_2"))
    parser.add_argument("--confirm", action="store_true", help="Use only after the exact report-specific confirmation was provided by the caller")
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    results = args.results if args.results.is_absolute() else root / args.results
    return run_one(report_id=args.report_id, results_root=results, confirm=args.confirm)


if __name__ == "__main__":
    raise SystemExit(main())
