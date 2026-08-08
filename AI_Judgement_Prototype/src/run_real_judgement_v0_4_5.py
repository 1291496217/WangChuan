from __future__ import annotations

import argparse
from pathlib import Path

from audit_game_language_v0_2 import AUDIT_VERSION, audit_game_language_v0_2
from prompt_builder_v0_4_5 import PROMPT_VERSION, build_prompt_v0_4_5
from run_real_judgement_versioned import run_one_versioned

SCHEMA_VERSION = "0.2"
CORRECTION_STAGE = "Week8 Day6 Balanced Fragment Recall Correction"


def run_one(*, report_id: str, results_root: Path, confirm: bool = False) -> int:
    return run_one_versioned(
        report_id=report_id,
        results_root=results_root,
        confirm=confirm,
        prompt_version=PROMPT_VERSION,
        schema_version=SCHEMA_VERSION,
        correction_stage=CORRECTION_STAGE,
        language_audit_version=AUDIT_VERSION,
        prompt_builder=build_prompt_v0_4_5,
        language_auditor=audit_game_language_v0_2,
    )


def main() -> int:
    parser = argparse.ArgumentParser(description="One controlled Prompt v0.4.5 balanced-fragment regression call")
    parser.add_argument("--report-id", required=True)
    parser.add_argument("--results", type=Path, default=Path("results/week8b_prompt045_root_cause_regression"))
    parser.add_argument("--confirm", action="store_true", help="Use only after report-specific authorization")
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    results = args.results if args.results.is_absolute() else root / args.results
    return run_one(report_id=args.report_id, results_root=results, confirm=args.confirm)


if __name__ == "__main__":
    raise SystemExit(main())
