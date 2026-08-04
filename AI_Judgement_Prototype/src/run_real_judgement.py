from __future__ import annotations

import argparse
import json
import os
import sys
import time
from pathlib import Path
from typing import Any

from ai_client import (
    AIClientError,
    DeepSeekAIClient,
    DeepSeekClientConfig,
)
from env_loader import (
    EnvironmentConfigurationError,
    load_env_file,
    read_float_environment_value,
    read_int_environment_value,
    require_environment_value,
)
from prompt_builder import (
    PROMPT_VERSION,
    build_judgement_messages,
)
from report_parser import (
    PlayerReportError,
    load_player_report,
)
from response_validator import (
    load_json_object,
    parse_judgement_result,
    validate_judgement_result,
)
from result_writer import (
    create_run_id,
    sha256_text,
    utc_now_iso,
    write_raw_record,
    write_validated_record,
)


PROVIDER_NAME = "deepseek"
DEFAULT_BASE_URL = "https://api.deepseek.com"
DEFAULT_MODEL = "deepseek-v4-flash"


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Submit one WangChuan player report to DeepSeek and "
            "validate the structured judgement."
        )
    )
    parser.add_argument(
        "--report",
        type=Path,
        default=Path(
            "reports/manual/R01_rigorous_detain.md"
        ),
        help="Path relative to the prototype root.",
    )
    parser.add_argument(
        "--case",
        type=Path,
        default=Path("cases/case_knife_001.json"),
    )
    parser.add_argument(
        "--judge",
        type=Path,
        default=Path("judges/judge_clerk_001.json"),
    )
    parser.add_argument(
        "--schema",
        type=Path,
        default=Path(
            "schemas/judgement_result_v0_1.json"
        ),
    )
    parser.add_argument(
        "--results",
        type=Path,
        default=Path("results"),
    )
    parser.add_argument(
        "--yes",
        action="store_true",
        help="Skip the interactive SEND confirmation.",
    )
    parser.add_argument(
        "--show-prompt",
        action="store_true",
        help="Print messages before the API call.",
    )
    return parser.parse_args()


def _resolve_from_root(
    project_root: Path,
    path: Path,
) -> Path:
    if path.is_absolute():
        return path

    return project_root / path


def _build_config() -> DeepSeekClientConfig:
    thinking_mode = os.environ.get(
        "DEEPSEEK_THINKING",
        "disabled",
    ).strip().lower()

    if thinking_mode not in {"enabled", "disabled"}:
        raise EnvironmentConfigurationError(
            "DEEPSEEK_THINKING must be 'enabled' or 'disabled'."
        )

    return DeepSeekClientConfig(
        api_key=require_environment_value(
            "DEEPSEEK_API_KEY"
        ),
        base_url=os.environ.get(
            "DEEPSEEK_BASE_URL",
            DEFAULT_BASE_URL,
        ).strip(),
        model=os.environ.get(
            "DEEPSEEK_MODEL",
            DEFAULT_MODEL,
        ).strip(),
        timeout_seconds=read_float_environment_value(
            "DEEPSEEK_TIMEOUT_SECONDS",
            default=90.0,
            minimum=1.0,
            maximum=600.0,
        ),
        max_tokens=read_int_environment_value(
            "DEEPSEEK_MAX_TOKENS",
            default=2400,
            minimum=256,
            maximum=20000,
        ),
        thinking_mode=thinking_mode,
        temperature=read_float_environment_value(
            "DEEPSEEK_TEMPERATURE",
            default=0.2,
            minimum=0.0,
            maximum=2.0,
        ),
    )


def _confirm_send(*, skip_confirmation: bool) -> bool:
    if skip_confirmation:
        return True

    response = input(
        "\nType SEND to perform one billed API call: "
    ).strip()

    return response == "SEND"


def _messages_hash(
    messages: list[dict[str, str]],
) -> str:
    serialized = json.dumps(
        messages,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
    )
    return sha256_text(serialized)


def _create_common_metadata(
    *,
    run_id: str,
    started_at: str,
    completed_at: str,
    elapsed_ms: int,
    config: DeepSeekClientConfig,
    case_data: dict[str, Any],
    judge_data: dict[str, Any],
    schema_data: dict[str, Any],
    report_path: Path,
    report_text: str,
    selected_disposition_id: str,
    messages: list[dict[str, str]],
) -> dict[str, Any]:
    return {
        "run_id": run_id,
        "started_at_utc": started_at,
        "completed_at_utc": completed_at,
        "elapsed_ms": elapsed_ms,
        "provider": PROVIDER_NAME,
        "model_requested": config.model,
        "base_url": config.base_url,
        "thinking_mode": config.thinking_mode,
        "temperature": (
            config.temperature
            if config.thinking_mode == "disabled"
            else None
        ),
        "max_tokens": config.max_tokens,
        "timeout_seconds": config.timeout_seconds,
        "prompt_version": PROMPT_VERSION,
        "schema_version": schema_data.get(
            "properties",
            {},
        ).get(
            "schema_version",
            {},
        ).get(
            "const",
        ),
        "case_id": case_data.get("case_id"),
        "case_version": case_data.get("case_version"),
        "judge_profile_id": judge_data.get(
            "judge_profile_id"
        ),
        "judge_version": judge_data.get("version"),
        "report_path": str(report_path),
        "report_sha256": sha256_text(report_text),
        "messages_sha256": _messages_hash(messages),
        "selected_disposition_id": selected_disposition_id,
    }


def main() -> int:
    args = _parse_args()
    project_root = Path(__file__).resolve().parent.parent

    try:
        load_env_file(project_root / ".env")
        config = _build_config()

        case_path = _resolve_from_root(
            project_root,
            args.case,
        )
        judge_path = _resolve_from_root(
            project_root,
            args.judge,
        )
        schema_path = _resolve_from_root(
            project_root,
            args.schema,
        )
        report_path = _resolve_from_root(
            project_root,
            args.report,
        )
        results_root = _resolve_from_root(
            project_root,
            args.results,
        )

        case_data = load_json_object(case_path)
        judge_data = load_json_object(judge_path)
        schema_data = load_json_object(schema_path)
        report = load_player_report(
            report_path,
            case_data=case_data,
        )

        messages = build_judgement_messages(
            case_data=case_data,
            judge_data=judge_data,
            schema_data=schema_data,
            player_report=report.text,
            selected_disposition_id=(
                report.disposition_id
            ),
        )
    except (
        EnvironmentConfigurationError,
        FileNotFoundError,
        json.JSONDecodeError,
        PlayerReportError,
        ValueError,
    ) as error:
        print(
            f"[LOCAL CONFIGURATION ERROR] {error}",
            file=sys.stderr,
        )
        return 1

    print("=" * 72)
    print("WANGCHUAN — WEEK 8 DAY 2 REAL AI JUDGEMENT")
    print("=" * 72)
    print(f"Provider: {PROVIDER_NAME}")
    print(f"Model: {config.model}")
    print(f"Thinking: {config.thinking_mode}")
    print(
        "Temperature: "
        + (
            str(config.temperature)
            if config.thinking_mode == "disabled"
            else "not used"
        )
    )
    print(f"Case: {case_data.get('case_id')}")
    print(f"Case Version: {case_data.get('case_version')}")
    print(
        f"Judge: {judge_data.get('judge_profile_id')}"
    )
    print(f"Judge Version: {judge_data.get('version')}")
    print(f"Prompt Version: {PROMPT_VERSION}")
    print(
        "Schema Version: "
        f"{schema_data['properties']['schema_version']['const']}"
    )
    print(f"Report: {report_path}")
    print(f"Report Characters: {report.character_count}")
    print(
        f"Selected Disposition: {report.disposition_id}"
    )
    print("Planned API Calls: 1")
    print("API Key: loaded locally (value hidden)")

    if args.show_prompt:
        print("\n" + "-" * 72)
        print(
            json.dumps(
                messages,
                ensure_ascii=False,
                indent=2,
            )
        )
        print("-" * 72)

    if not _confirm_send(
        skip_confirmation=args.yes
    ):
        print("Cancelled before the API call.")
        return 0

    run_id = create_run_id()
    started_at = utc_now_iso()
    start_time = time.monotonic()

    client = DeepSeekAIClient(config)

    try:
        provider_response = client.create_judgement(
            messages
        )
    except AIClientError as error:
        elapsed_ms = int(
            (time.monotonic() - start_time) * 1000
        )
        completed_at = utc_now_iso()

        common_metadata = _create_common_metadata(
            run_id=run_id,
            started_at=started_at,
            completed_at=completed_at,
            elapsed_ms=elapsed_ms,
            config=config,
            case_data=case_data,
            judge_data=judge_data,
            schema_data=schema_data,
            report_path=report_path,
            report_text=report.text,
            selected_disposition_id=(
                report.disposition_id
            ),
            messages=messages,
        )

        raw_path = write_raw_record(
            results_root=results_root,
            run_id=run_id,
            payload={
                "status": "provider_error",
                "metadata": common_metadata,
                "player_report": report.text,
                "request_messages": messages,
                "error": {
                    "type": type(error).__name__,
                    "message": str(error),
                },
            },
        )

        print(
            f"[PROVIDER ERROR] {error}",
            file=sys.stderr,
        )
        print(f"Error record: {raw_path}")
        return 2

    elapsed_ms = int(
        (time.monotonic() - start_time) * 1000
    )
    completed_at = utc_now_iso()

    common_metadata = _create_common_metadata(
        run_id=run_id,
        started_at=started_at,
        completed_at=completed_at,
        elapsed_ms=elapsed_ms,
        config=config,
        case_data=case_data,
        judge_data=judge_data,
        schema_data=schema_data,
        report_path=report_path,
        report_text=report.text,
        selected_disposition_id=report.disposition_id,
        messages=messages,
    )

    try:
        parsed_payload = json.loads(
            provider_response.content
        )
    except json.JSONDecodeError as error:
        raw_path = write_raw_record(
            results_root=results_root,
            run_id=run_id,
            payload={
                "status": "json_parse_failed",
                "metadata": common_metadata,
                "player_report": report.text,
                "request_messages": messages,
                "provider_response": {
                    "response_id": (
                        provider_response.response_id
                    ),
                    "model_returned": (
                        provider_response.returned_model
                    ),
                    "finish_reason": (
                        provider_response.finish_reason
                    ),
                    "system_fingerprint": (
                        provider_response.system_fingerprint
                    ),
                    "usage": provider_response.usage,
                    "content": provider_response.content,
                },
                "error": {
                    "type": type(error).__name__,
                    "message": str(error),
                },
            },
        )

        print(
            f"[JSON ERROR] {error}",
            file=sys.stderr,
        )
        print(f"Raw record: {raw_path}")
        return 3

    validation = validate_judgement_result(
        parsed_payload,
        schema_data=schema_data,
        case_data=case_data,
        judge_data=judge_data,
        expected_disposition_id=(
            report.disposition_id
        ),
    )

    raw_path = write_raw_record(
        results_root=results_root,
        run_id=run_id,
        payload={
            "status": (
                "validated"
                if validation.is_valid
                else "validation_failed"
            ),
            "metadata": {
                **common_metadata,
                "model_returned": (
                    provider_response.returned_model
                ),
                "response_id": (
                    provider_response.response_id
                ),
                "finish_reason": (
                    provider_response.finish_reason
                ),
                "system_fingerprint": (
                    provider_response.system_fingerprint
                ),
                "usage": provider_response.usage,
            },
            "player_report": report.text,
            "request_messages": messages,
            "provider_content": (
                provider_response.content
            ),
            "parsed_payload": parsed_payload,
            "validation": {
                "passed": validation.is_valid,
                "issues": [
                    {
                        "path": issue.path,
                        "message": issue.message,
                    }
                    for issue in validation.issues
                ],
            },
        },
    )

    if not validation.is_valid:
        print(
            "[VALIDATION FAILED]",
            file=sys.stderr,
        )
        print(
            validation.format_errors(),
            file=sys.stderr,
        )
        print(f"Raw record: {raw_path}")
        return 4

    judgement = parse_judgement_result(
        parsed_payload,
        schema_data=schema_data,
        case_data=case_data,
        judge_data=judge_data,
        expected_disposition_id=(
            report.disposition_id
        ),
    )

    validated_path = write_validated_record(
        results_root=results_root,
        run_id=run_id,
        payload={
            "metadata": {
                **common_metadata,
                "model_returned": (
                    provider_response.returned_model
                ),
                "response_id": (
                    provider_response.response_id
                ),
                "finish_reason": (
                    provider_response.finish_reason
                ),
                "system_fingerprint": (
                    provider_response.system_fingerprint
                ),
                "usage": provider_response.usage,
            },
            "judgement_result": parsed_payload,
        },
    )

    print("\n" + "=" * 72)
    print("REAL AI JUDGEMENT VALIDATED")
    print("=" * 72)
    print(f"Run ID: {run_id}")
    print(
        f"Model Returned: "
        f"{provider_response.returned_model}"
    )
    print(f"Elapsed: {elapsed_ms} ms")
    print(
        "Total Tokens: "
        f"{provider_response.usage.get('total_tokens')}"
    )
    print(f"Core Claim: {judgement.core_claim}")
    print(
        "Used Fragments: "
        f"{len(judgement.used_fragment_ids)}"
    )
    print(
        "Unsupported Assumptions: "
        f"{len(judgement.unsupported_assumptions)}"
    )
    print(
        "Contradiction Handling: "
        f"{judgement.contradiction_handling.level}"
    )
    print(
        "Evidence Grounding: "
        f"{judgement.dimension_ratings.evidence_grounding}"
    )
    print(
        "Disposition Alignment: "
        f"{judgement.dimension_ratings.disposition_alignment}"
    )
    print("\nJudge Response:")
    print(judgement.judge_response)
    print(f"\nRaw Record: {raw_path}")
    print(f"Validated Record: {validated_path}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
