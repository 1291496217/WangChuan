from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

from corpus_loader import CorpusEntry, load_and_validate_corpus
from report_parser import PlayerReportError, load_player_report
from response_validator import load_json_object


def _resolve(project_root: Path, path: Path) -> Path:
    if path.is_absolute():
        return path
    return project_root / path


def _show_case(case_data: dict) -> None:
    print("=" * 72)
    print("WANGCHUAN — WEEK 8 CLI EXPERIMENT FLOW")
    print("=" * 72)
    print(
        f"Case: {case_data.get('title')} "
        f"({case_data.get('case_id')} v{case_data.get('case_version')})"
    )
    print("\nFragments:")
    for index, fragment in enumerate(
        case_data.get("fragments", []),
        start=1,
    ):
        print(
            f"  {index}. [{fragment.get('fragment_id')}] "
            f"{fragment.get('text')}"
        )

    print("\nAllowed Dispositions:")
    for index, disposition in enumerate(
        case_data.get("allowed_dispositions", []),
        start=1,
    ):
        print(
            f"  {index}. {disposition.get('disposition_id')} — "
            f"{disposition.get('display_name')}"
        )


def _show_report_list(entries: tuple[CorpusEntry, ...]) -> None:
    print("\nAvailable Reports:")
    for index, entry in enumerate(entries, start=1):
        labels = entry.labels
        print(
            f"  {index:02d}. {entry.report_id} | "
            f"{entry.group} | "
            f"{labels.get('selected_disposition_id')} | "
            f"{entry.title}"
        )


def _select_entry(
    entries: tuple[CorpusEntry, ...],
    requested_id: str | None,
) -> CorpusEntry | None:
    if requested_id:
        normalized = requested_id.strip().upper()
        for entry in entries:
            if entry.report_id.upper() == normalized:
                return entry
        return None

    raw_value = input(
        "\nChoose a report number or Report ID "
        "(blank to cancel): "
    ).strip()

    if not raw_value:
        return None

    if raw_value.isdigit():
        selected_index = int(raw_value) - 1
        if 0 <= selected_index < len(entries):
            return entries[selected_index]
        return None

    normalized = raw_value.upper()
    for entry in entries:
        if entry.report_id.upper() == normalized:
            return entry

    return None


def _show_selected_entry(
    entry: CorpusEntry,
    project_root: Path,
) -> None:
    labels = entry.labels
    relative_path = entry.file_path.relative_to(project_root)

    print("\n" + "-" * 72)
    print(f"Selected: {entry.report_id} — {entry.title}")
    print(f"File: {relative_path}")
    print(f"Group: {entry.group}")
    print(
        "Disposition: "
        f"{labels.get('selected_disposition_id')}"
    )
    print(
        "Expected Preflight: "
        f"{labels.get('expected_preflight')}"
    )
    print(
        "Human Test Purpose: "
        f"{labels.get('test_purpose')}"
    )
    print(
        "Intentional Fabrication: "
        f"{labels.get('intentionally_fabricates_evidence')}"
    )
    print(
        "Contradiction Intent: "
        f"{labels.get('contradiction_handling_intent')}"
    )
    print(
        "Expected Primary Fragments: "
        + ", ".join(
            labels.get("expected_primary_fragment_ids", [])
        )
    )
    print("\nPlayer Report:")
    print(entry.file_path.read_text(encoding="utf-8").strip())
    print("-" * 72)
    print(
        "Note: the human labels above are local test intent. "
        "Only the player report is sent to the AI."
    )


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Select one human-written report and run at most one "
            "real AI judgement."
        )
    )
    parser.add_argument(
        "--manifest",
        type=Path,
        default=Path("reports/corpus_manifest_v0_1.json"),
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
        default=Path("schemas/judgement_result_v0_1.json"),
    )
    parser.add_argument("--report-id")
    parser.add_argument(
        "--list",
        action="store_true",
        help="List corpus reports without selecting one.",
    )
    args = parser.parse_args()

    project_root = Path(__file__).resolve().parent.parent
    case_path = _resolve(project_root, args.case)
    manifest_path = _resolve(project_root, args.manifest)

    try:
        case_data = load_json_object(case_path)
    except (FileNotFoundError, ValueError) as error:
        print(f"[CLI ERROR] {error}", file=sys.stderr)
        return 1

    corpus = load_and_validate_corpus(
        project_root=project_root,
        manifest_path=manifest_path,
        case_data=case_data,
    )

    if not corpus.is_valid:
        print("[CLI ERROR] Corpus validation failed.", file=sys.stderr)
        print(corpus.format_errors(), file=sys.stderr)
        return 1

    _show_case(case_data)
    _show_report_list(corpus.entries)

    if args.list:
        return 0

    selected = _select_entry(
        corpus.entries,
        args.report_id,
    )

    if selected is None:
        print("No valid report selected. Cancelled.")
        return 0

    _show_selected_entry(selected, project_root)

    expected_preflight = selected.labels.get(
        "expected_preflight"
    )

    try:
        load_player_report(
            selected.file_path,
            case_data=case_data,
        )
        parser_passed = True
        parser_error = None
    except PlayerReportError as error:
        parser_passed = False
        parser_error = error

    if expected_preflight == "reject":
        if parser_passed:
            print(
                "[CLI ERROR] This fixture was expected to fail "
                "preflight, but it passed.",
                file=sys.stderr,
            )
            return 2

        print("\nEXPECTED LOCAL REJECTION")
        print(f"Reason: {parser_error}")
        print("No API call was made.")
        return 0

    if not parser_passed:
        print(f"[CLI ERROR] {parser_error}", file=sys.stderr)
        return 2

    confirmation = input(
        f"\nType RUN {selected.report_id} to perform "
        "one billed API call: "
    ).strip()

    if confirmation != f"RUN {selected.report_id}":
        print("Cancelled before the API call.")
        return 0

    relative_report = selected.file_path.relative_to(project_root)
    relative_case = case_path.relative_to(project_root)
    judge_path = _resolve(project_root, args.judge)
    schema_path = _resolve(project_root, args.schema)
    relative_judge = judge_path.relative_to(project_root)
    relative_schema = schema_path.relative_to(project_root)

    command = [
        sys.executable,
        str(project_root / "src" / "run_real_judgement.py"),
        "--report",
        str(relative_report),
        "--case",
        str(relative_case),
        "--judge",
        str(relative_judge),
        "--schema",
        str(relative_schema),
        "--yes",
    ]

    print("\nSubmitting exactly one report to the configured model...")
    print("Human intent labels are not included in the request.")
    completed = subprocess.run(
        command,
        cwd=project_root,
        check=False,
    )

    return completed.returncode


if __name__ == "__main__":
    raise SystemExit(main())
