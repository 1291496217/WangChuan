from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from report_parser_v0_2 import PlayerReportV02, PlayerReportV02Error, load_player_report_v0_2


@dataclass(frozen=True)
class CorpusV02Entry:
    report_id: str
    file_path: Path
    metadata: dict[str, Any]
    labels: dict[str, Any]
    parsed_report: PlayerReportV02 | None


@dataclass(frozen=True)
class CorpusV02Result:
    manifest: dict[str, Any]
    entries: tuple[CorpusV02Entry, ...]
    issues: tuple[str, ...]

    @property
    def is_valid(self) -> bool:
        return not self.issues

    def format_errors(self) -> str:
        return "\n".join(f"- {issue}" for issue in self.issues) or "No corpus validation errors."


def _safe_path(root: Path, relative: str) -> Path:
    target = (root / relative).resolve()
    resolved_root = root.resolve()
    if target != resolved_root and resolved_root not in target.parents:
        raise ValueError(f"path escapes project root: {relative}")
    return target


def load_and_validate_corpus_v0_2(*, project_root: Path, manifest_path: Path) -> CorpusV02Result:
    issues: list[str] = []
    entries: list[CorpusV02Entry] = []
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        return CorpusV02Result({}, (), (f"manifest: {error}",))

    if manifest.get("corpus_id") != "Corpus.MoralJudgement.Week8B.001":
        issues.append("$.corpus_id: unexpected corpus identity")
    if manifest.get("version") != "0.1":
        issues.append("$.version: must be 0.1")
    if manifest.get("report_contract_version") != "0.2":
        issues.append("$.report_contract_version: must be 0.2")
    if manifest.get("human_labels_are_hidden_truth") is not False:
        issues.append("$.human_labels_are_hidden_truth: must be false")
    if manifest.get("send_human_labels_to_ai") is not False:
        issues.append("$.send_human_labels_to_ai: must be false")

    cases = manifest.get("cases", [])
    expected_cases = {"Case.DoorKnife.001", "Case.Medicine.001"}
    valid_case_items = [item for item in cases if isinstance(item, dict)] if isinstance(cases, list) else []
    case_ids = [item.get("case_id") for item in valid_case_items]
    if (
        not isinstance(cases, list)
        or len(cases) != 2
        or len(valid_case_items) != 2
        or set(case_ids) != expected_cases
        or len(set(case_ids)) != 2
    ):
        issues.append("$.cases: must contain exactly two unique Week8B cases")
    case_map: dict[str, dict[str, Any]] = {}
    for index, item in enumerate(cases if isinstance(cases, list) else []):
        if not isinstance(item, dict):
            continue
        try:
            case_path = _safe_path(project_root, item["file"])
            data = json.loads(case_path.read_text(encoding="utf-8"))
            case_map[data.get("case_id")] = data
            if data.get("case_id") != item.get("case_id") or data.get("case_version") != item.get("case_version"):
                issues.append(f"$.cases[{index}]: case identity/version mismatch")
        except (KeyError, OSError, ValueError, json.JSONDecodeError) as error:
            issues.append(f"$.cases[{index}]: {error}")

    reports = manifest.get("reports", [])
    if not isinstance(reports, list) or not 24 <= len(reports) <= 30:
        issues.append("$.reports: count must be 24-30")
        reports = reports if isinstance(reports, list) else []
    seen_ids: set[str] = set()
    seen_files: set[Path] = set()
    required_labels = {
        "expected_interpretation_family", "expected_moral_direction",
        "expected_disposition_plausibility", "personality_use_intent",
        "thought_use_intent", "expected_fragment_roles",
        "contains_unsupported_claims", "contains_game_language_attack",
        "contains_prompt_injection", "contains_formal_choice_override_attempt",
        "argument_strength", "is_moral_disposition_inconsistent",
        "test_purpose", "expected_preflight",
    }
    boolean_labels = {
        "contains_unsupported_claims", "contains_game_language_attack",
        "contains_prompt_injection", "contains_formal_choice_override_attempt",
        "is_moral_disposition_inconsistent",
    }
    allowed_plausibility = {"strongly_supported", "defensible", "weakly_supported", "disproportionate"}
    allowed_personality = {"not_used", "integrated", "context_only", "misused_as_exoneration", "misused_as_guilt"}
    allowed_thought = {"not_used", "clear_distinction", "motive_context", "premeditation_support", "confused_with_action"}
    allowed_strength = {"strong", "adequate", "weak", "adversarial"}
    allowed_moral = {"more_good_than_evil", "mixed_merit_and_fault", "more_evil_than_good", "beyond_redemption", "ambiguous"}
    allowed_roles = {"core_support", "context", "counterevidence", "mentioned_unresolved", "explicitly_excluded"}

    for index, item in enumerate(reports):
        path = f"$.reports[{index}]"
        if not isinstance(item, dict):
            issues.append(f"{path}: must be an object")
            continue
        report_id = item.get("report_id")
        if not isinstance(report_id, str) or not report_id or report_id in seen_ids:
            issues.append(f"{path}.report_id: missing or duplicate")
        else:
            seen_ids.add(report_id)
        labels = item.get("human_labels")
        if not isinstance(labels, dict):
            issues.append(f"{path}.human_labels: must be an object")
            labels = {}
        missing = required_labels - labels.keys()
        if missing:
            issues.append(f"{path}.human_labels: missing {sorted(missing)}")
        if labels.get("expected_moral_direction") not in allowed_moral:
            issues.append(f"{path}.human_labels.expected_moral_direction: invalid")
        if labels.get("expected_disposition_plausibility") not in allowed_plausibility:
            issues.append(f"{path}.human_labels.expected_disposition_plausibility: invalid")
        if labels.get("personality_use_intent") not in allowed_personality:
            issues.append(f"{path}.human_labels.personality_use_intent: invalid")
        if labels.get("thought_use_intent") not in allowed_thought:
            issues.append(f"{path}.human_labels.thought_use_intent: invalid")
        if labels.get("argument_strength") not in allowed_strength:
            issues.append(f"{path}.human_labels.argument_strength: invalid")
        for field in boolean_labels:
            if not isinstance(labels.get(field), bool):
                issues.append(f"{path}.human_labels.{field}: must be boolean")

        try:
            file_path = _safe_path(project_root, item["file"])
            corpus_root = (project_root / "reports" / "corpus_moral_week8b").resolve()
            if file_path != corpus_root and corpus_root not in file_path.parents:
                raise ValueError(f"path escapes Week8B corpus root: {item['file']}")
            if file_path in seen_files:
                issues.append(f"{path}.file: duplicate")
            seen_files.add(file_path)
            case_data = case_map[item.get("case_id")]
            parsed = load_player_report_v0_2(file_path, case_data=case_data)
            parser_passed = True
        except (KeyError, OSError, ValueError, PlayerReportV02Error) as error:
            file_path = project_root
            parsed = None
            parser_passed = False
            if labels.get("expected_preflight") == "pass":
                issues.append(f"{path}.file: expected pass, got {error}")
        if labels.get("expected_preflight") not in {"pass", "reject"}:
            issues.append(f"{path}.human_labels.expected_preflight: invalid")
        if labels.get("expected_preflight") == "reject" and parser_passed:
            issues.append(f"{path}: expected rejection but parser passed")
        if parsed:
            for field, actual in (("case_id", parsed.case_id), ("moral_judgement_id", parsed.moral_judgement_id), ("disposition_id", parsed.disposition_id)):
                if item.get(field) != actual:
                    issues.append(f"{path}.{field}: does not match report")
            roles = labels.get("expected_fragment_roles", {})
            if not isinstance(roles, dict):
                issues.append(f"{path}.human_labels.expected_fragment_roles: must be object")
            else:
                valid_ids = {f.get("fragment_id") for f in case_data.get("fragments", [])}
                for fragment_id, role in roles.items():
                    if fragment_id not in valid_ids or role not in allowed_roles:
                        issues.append(f"{path}.human_labels.expected_fragment_roles: invalid mapping")
        entries.append(CorpusV02Entry(str(report_id), file_path, item, labels, parsed))

    return CorpusV02Result(manifest, tuple(entries), tuple(issues))
