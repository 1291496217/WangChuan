from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from report_parser import PlayerReportError, load_player_report


ALLOWED_PREFLIGHT_VALUES = {"pass", "reject"}


@dataclass(frozen=True)
class CorpusIssue:
    path: str
    message: str

    def __str__(self) -> str:
        return f"{self.path}: {self.message}"


@dataclass(frozen=True)
class CorpusEntry:
    report_id: str
    file_path: Path
    title: str
    group: str
    labels: dict[str, Any]


@dataclass(frozen=True)
class CorpusValidationResult:
    entries: tuple[CorpusEntry, ...]
    issues: tuple[CorpusIssue, ...]

    @property
    def is_valid(self) -> bool:
        return len(self.issues) == 0

    def format_errors(self) -> str:
        if self.is_valid:
            return "No corpus validation errors."

        return "\n".join(f"- {issue}" for issue in self.issues)


def _add_issue(
    issues: list[CorpusIssue],
    path: str,
    message: str,
) -> None:
    issues.append(CorpusIssue(path=path, message=message))


def _resolve_safe_path(
    project_root: Path,
    relative_value: str,
) -> Path:
    candidate = (project_root / relative_value).resolve()
    root = project_root.resolve()

    if candidate != root and root not in candidate.parents:
        raise ValueError(
            f"Corpus file escapes the project root: {relative_value}"
        )

    return candidate


def load_and_validate_corpus(
    *,
    project_root: Path,
    manifest_path: Path,
    case_data: dict[str, Any],
) -> CorpusValidationResult:
    """Load the corpus manifest and validate all local report fixtures."""
    issues: list[CorpusIssue] = []
    entries: list[CorpusEntry] = []

    if not manifest_path.is_file():
        return CorpusValidationResult(
            entries=(),
            issues=(
                CorpusIssue(
                    path="$",
                    message=f"manifest not found: {manifest_path}",
                ),
            ),
        )

    try:
        manifest = json.loads(
            manifest_path.read_text(encoding="utf-8")
        )
    except json.JSONDecodeError as error:
        return CorpusValidationResult(
            entries=(),
            issues=(
                CorpusIssue(
                    path="$",
                    message=f"invalid manifest JSON: {error}",
                ),
            ),
        )

    if not isinstance(manifest, dict):
        return CorpusValidationResult(
            entries=(),
            issues=(
                CorpusIssue(
                    path="$",
                    message="manifest root must be an object",
                ),
            ),
        )

    if manifest.get("case_id") != case_data.get("case_id"):
        _add_issue(
            issues,
            "$.case_id",
            "must match the currently loaded case",
        )

    raw_reports = manifest.get("reports")
    if not isinstance(raw_reports, list):
        _add_issue(
            issues,
            "$.reports",
            "must be an array",
        )
        raw_reports = []

    if manifest.get("report_count") != len(raw_reports):
        _add_issue(
            issues,
            "$.report_count",
            "does not match the reports array length",
        )

    allowed_fragment_ids = {
        fragment.get("fragment_id")
        for fragment in case_data.get("fragments", [])
        if isinstance(fragment, dict)
    }
    allowed_dispositions = {
        item.get("disposition_id")
        for item in case_data.get("allowed_dispositions", [])
        if isinstance(item, dict)
    }

    seen_ids: set[str] = set()
    seen_files: set[Path] = set()

    for index, raw_entry in enumerate(raw_reports):
        path = f"$.reports[{index}]"

        if not isinstance(raw_entry, dict):
            _add_issue(issues, path, "must be an object")
            continue

        report_id = raw_entry.get("report_id")
        relative_file = raw_entry.get("file")
        title = raw_entry.get("title")
        group = raw_entry.get("group")
        labels = raw_entry.get("human_intent_labels")

        if not isinstance(report_id, str) or not report_id:
            _add_issue(issues, f"{path}.report_id", "must be a non-empty string")
            continue

        if report_id in seen_ids:
            _add_issue(issues, f"{path}.report_id", "duplicate report ID")
        seen_ids.add(report_id)

        if not isinstance(relative_file, str) or not relative_file:
            _add_issue(issues, f"{path}.file", "must be a non-empty string")
            continue

        try:
            report_path = _resolve_safe_path(
                project_root,
                relative_file,
            )
        except ValueError as error:
            _add_issue(issues, f"{path}.file", str(error))
            continue

        if report_path in seen_files:
            _add_issue(issues, f"{path}.file", "duplicate report file")
        seen_files.add(report_path)

        if not report_path.is_file():
            _add_issue(
                issues,
                f"{path}.file",
                f"report file not found: {relative_file}",
            )
            continue

        if not isinstance(title, str) or not title:
            _add_issue(issues, f"{path}.title", "must be a non-empty string")
            title = report_id

        if not isinstance(group, str) or not group:
            _add_issue(issues, f"{path}.group", "must be a non-empty string")
            group = "unknown"

        if not isinstance(labels, dict):
            _add_issue(
                issues,
                f"{path}.human_intent_labels",
                "must be an object",
            )
            labels = {}

        preflight = labels.get("expected_preflight")
        if preflight not in ALLOWED_PREFLIGHT_VALUES:
            _add_issue(
                issues,
                f"{path}.human_intent_labels.expected_preflight",
                "must be 'pass' or 'reject'",
            )

        selected_disposition = labels.get(
            "selected_disposition_id"
        )
        if selected_disposition not in allowed_dispositions:
            _add_issue(
                issues,
                f"{path}.human_intent_labels.selected_disposition_id",
                "is not allowed by the current case",
            )

        expected_fragments = labels.get(
            "expected_primary_fragment_ids"
        )
        if not isinstance(expected_fragments, list):
            _add_issue(
                issues,
                f"{path}.human_intent_labels.expected_primary_fragment_ids",
                "must be an array",
            )
            expected_fragments = []

        for fragment_index, fragment_id in enumerate(
            expected_fragments
        ):
            if fragment_id not in allowed_fragment_ids:
                _add_issue(
                    issues,
                    (
                        f"{path}.human_intent_labels."
                        f"expected_primary_fragment_ids[{fragment_index}]"
                    ),
                    "does not exist in the current case",
                )

        try:
            parsed_report = load_player_report(
                report_path,
                case_data=case_data,
            )
            parser_passed = True
        except (PlayerReportError, FileNotFoundError):
            parsed_report = None
            parser_passed = False

        if preflight == "pass" and not parser_passed:
            _add_issue(
                issues,
                f"{path}.human_intent_labels.expected_preflight",
                "expected parser pass, but report was rejected",
            )

        if preflight == "reject" and parser_passed:
            _add_issue(
                issues,
                f"{path}.human_intent_labels.expected_preflight",
                "expected parser rejection, but report passed",
            )

        if (
            parsed_report is not None
            and parsed_report.disposition_id
            != selected_disposition
        ):
            _add_issue(
                issues,
                f"{path}.human_intent_labels.selected_disposition_id",
                "does not match the report's DispositionID",
            )

        entries.append(
            CorpusEntry(
                report_id=report_id,
                file_path=report_path,
                title=title,
                group=group,
                labels=labels,
            )
        )

    return CorpusValidationResult(
        entries=tuple(entries),
        issues=tuple(issues),
    )
