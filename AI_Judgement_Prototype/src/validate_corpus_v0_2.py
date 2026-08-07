from __future__ import annotations

import sys
from collections import Counter, defaultdict
from pathlib import Path

from corpus_loader_v0_2 import load_and_validate_corpus_v0_2


def main() -> int:
    root = Path(__file__).resolve().parent.parent
    manifest_path = root / "reports" / "corpus_manifest_moral_week8b_v0_1.json"
    result = load_and_validate_corpus_v0_2(project_root=root, manifest_path=manifest_path)
    if not result.is_valid:
        print("MORAL CORPUS VALIDATION FAILED", file=sys.stderr)
        print(result.format_errors(), file=sys.stderr)
        return 2

    case_counts = Counter(e.parsed_report.case_id for e in result.entries if e.parsed_report)
    moral_counts = Counter(e.parsed_report.moral_judgement_id for e in result.entries if e.parsed_report)
    disposition_counts = Counter(e.parsed_report.disposition_id for e in result.entries if e.parsed_report)
    strength_counts = Counter(e.labels["argument_strength"] for e in result.entries)
    coverage: dict[str, set[str]] = defaultdict(set)
    for entry in result.entries:
        if entry.parsed_report:
            coverage[entry.parsed_report.disposition_id].add(entry.labels["argument_strength"])
    required_dispositions = {"recommend_rebirth", "ordinary_transfer", "send_to_prison", "soul_dissolution"}
    if set(disposition_counts) != required_dispositions:
        print("MORAL CORPUS VALIDATION FAILED: missing disposition", file=sys.stderr)
        return 2
    if moral_counts.get("beyond_redemption", 0) == 0:
        print("MORAL CORPUS VALIDATION FAILED: beyond_redemption absent", file=sys.stderr)
        return 2
    if moral_counts.get("mixed_merit_and_fault", 0) == sum(moral_counts.values()):
        print("MORAL CORPUS VALIDATION FAILED: all reports are mixed_merit_and_fault", file=sys.stderr)
        return 2
    for disposition in {"recommend_rebirth", "ordinary_transfer", "send_to_prison"}:
        if not {"strong", "weak"} <= coverage[disposition]:
            print(f"MORAL CORPUS VALIDATION FAILED: {disposition} lacks strong/weak coverage", file=sys.stderr)
            return 2
        if not any(
            entry.parsed_report
            and entry.parsed_report.disposition_id == disposition
            and entry.labels["is_moral_disposition_inconsistent"]
            for entry in result.entries
        ):
            print(
                f"MORAL CORPUS VALIDATION FAILED: {disposition} lacks moral/disposition inconsistency coverage",
                file=sys.stderr,
            )
            return 2
    if any(count < 12 for count in case_counts.values()) or len(case_counts) != 2:
        print("MORAL CORPUS VALIDATION FAILED: case distribution", file=sys.stderr)
        return 2
    required_flags = [
        "contains_unsupported_claims", "contains_prompt_injection",
        "contains_formal_choice_override_attempt", "contains_game_language_attack",
    ]
    if any(not any(e.labels[field] for e in result.entries) for field in required_flags):
        print("MORAL CORPUS VALIDATION FAILED: adversarial coverage", file=sys.stderr)
        return 2
    if not any(e.labels["thought_use_intent"] == "confused_with_action" for e in result.entries):
        print("MORAL CORPUS VALIDATION FAILED: thought confusion absent", file=sys.stderr)
        return 2
    if not any(e.labels["personality_use_intent"].startswith("misused_") for e in result.entries):
        print("MORAL CORPUS VALIDATION FAILED: personality misuse absent", file=sys.stderr)
        return 2

    print("=" * 72)
    print("WANGCHUAN — WEEK 8B MORAL REPORT CORPUS")
    print("=" * 72)
    print("MORAL CORPUS VALIDATION PASSED")
    print(f"Corpus: {result.manifest['corpus_id']} v{result.manifest['version']}")
    print(f"Reports: {len(result.entries)}")
    print(f"Cases: {dict(sorted(case_counts.items()))}")
    print(f"Moral Judgements: {dict(sorted(moral_counts.items()))}")
    print(f"Dispositions: {dict(sorted(disposition_counts.items()))}")
    print(f"Argument Strength: {dict(sorted(strength_counts.items()))}")
    print("Human labels remain local and are not part of PlayerReportV02.")
    print("Design metadata boundary: PASS")
    print("No API call")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
