from __future__ import annotations

import sys
from pathlib import Path

from data_loader import load_json_file, load_text_file
from prompt_builder import build_experiment_prompt


def main() -> int:
    """
    Run the Day 1 local prompt preview.
    """
    project_root = Path(__file__).resolve().parent.parent

    case_path = project_root / "cases" / "case_day1_stub.json"
    judge_path = project_root / "judges" / "judge_day1_stub.json"
    report_path = project_root / "reports" / "day1_sample_report.md"

    try:
        case_data = load_json_file(case_path)
        judge_data = load_json_file(judge_path)
        player_report = load_text_file(report_path)

        prompt = build_experiment_prompt(
            case_data=case_data,
            judge_data=judge_data,
            player_report=player_report,
        )

    except FileNotFoundError as error:
        print(f"[FILE ERROR] {error}", file=sys.stderr)
        return 1

    except ValueError as error:
        print(f"[DATA ERROR] {error}", file=sys.stderr)
        return 1

    except Exception as error:
        print(
            f"[UNEXPECTED ERROR] {type(error).__name__}: {error}",
            file=sys.stderr,
        )
        return 1

    fragments = case_data.get("fragments", [])
    dispositions = case_data.get("allowed_dispositions", [])

    print("=" * 72)
    print("WANGCHUAN — WEEK 8 DAY 1 PROMPT PREVIEW")
    print("=" * 72)
    print(f"Case: {case_data.get('title', '<missing title>')}")
    print(f"Case ID: {case_data.get('case_id', '<missing case id>')}")
    print(f"Judge: {judge_data.get('display_name', '<missing judge>')}")
    print(f"Fragment count: {len(fragments)}")
    print(f"Disposition count: {len(dispositions)}")
    print(f"Player report characters: {len(player_report)}")
    print()
    print("Generated prompt:")
    print("-" * 72)
    print(prompt)
    print("-" * 72)
    print("Day 1 completed without calling an AI service.")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())