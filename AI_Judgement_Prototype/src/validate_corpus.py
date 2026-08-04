from __future__ import annotations

import argparse
import sys
from collections import Counter
from pathlib import Path

from corpus_loader import load_and_validate_corpus
from response_validator import load_json_object


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Validate the Week8 human-written report corpus."
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
    args = parser.parse_args()

    project_root = Path(__file__).resolve().parent.parent
    manifest_path = (
        args.manifest
        if args.manifest.is_absolute()
        else project_root / args.manifest
    )
    case_path = (
        args.case
        if args.case.is_absolute()
        else project_root / args.case
    )

    try:
        case_data = load_json_object(case_path)
    except (FileNotFoundError, ValueError) as error:
        print(f"[CORPUS ERROR] {error}", file=sys.stderr)
        return 1

    result = load_and_validate_corpus(
        project_root=project_root,
        manifest_path=manifest_path,
        case_data=case_data,
    )

    if not result.is_valid:
        print("CORPUS VALIDATION FAILED", file=sys.stderr)
        print(result.format_errors(), file=sys.stderr)
        return 2

    group_counts = Counter(entry.group for entry in result.entries)
    disposition_counts = Counter(
        entry.labels["selected_disposition_id"]
        for entry in result.entries
    )
    preflight_counts = Counter(
        entry.labels["expected_preflight"]
        for entry in result.entries
    )

    print("=" * 72)
    print("WANGCHUAN — WEEK 8 HUMAN TEST CORPUS")
    print("=" * 72)
    print("CORPUS VALIDATION PASSED")
    print(f"Case ID: {case_data.get('case_id')}")
    print(f"Report Count: {len(result.entries)}")
    print(f"Groups: {dict(sorted(group_counts.items()))}")
    print(
        "Dispositions: "
        f"{dict(sorted(disposition_counts.items()))}"
    )
    print(
        "Expected Preflight: "
        f"{dict(sorted(preflight_counts.items()))}"
    )
    print("Human labels are local experiment intent, not hidden truth.")
    print("Human labels are not sent to the AI.")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
