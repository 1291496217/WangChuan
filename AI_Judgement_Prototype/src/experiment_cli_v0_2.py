from __future__ import annotations

import argparse
from pathlib import Path

from run_real_judgement_v0_2 import run_one


def main() -> int:
    parser = argparse.ArgumentParser(description="Safe single-report Week8B experiment CLI")
    parser.add_argument("--report", required=True, help="MR01-style ReportID")
    parser.add_argument("--confirm", required=True, help="Must exactly equal RUN <ReportID>")
    parser.add_argument("--results", type=Path, default=Path("results/week8b_v0_2"))
    args = parser.parse_args()
    if args.confirm != f"RUN {args.report}":
        parser.error("--confirm must exactly equal RUN <ReportID>")
    root = Path(__file__).resolve().parent.parent
    results = args.results if args.results.is_absolute() else root / args.results
    return run_one(report_id=args.report, results_root=results, confirm=True)


if __name__ == "__main__":
    raise SystemExit(main())
