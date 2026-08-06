from __future__ import annotations

import argparse
import json
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable, Sequence


PROJECT_ROOT = Path(__file__).resolve().parents[1]

REQUIRED_TOP_LEVEL_FIELDS = {
    "case_id",
    "case_version",
    "title",
    "design_mode",
    "hidden_complete_truth",
    "allowed_moral_judgements",
    "allowed_dispositions",
    "relation_tag_definitions",
    "fragments",
}
REQUIRED_FRAGMENT_FIELDS = {
    "fragment_id",
    "semantic_type",
    "source_type",
    "information_weight",
    "acquisition_type",
    "text",
    "relation_tags",
    "disposition_support_tags",
    "interpretation_hooks",
}
MORAL_JUDGEMENT_IDS = {
    "more_good_than_evil",
    "mixed_merit_and_fault",
    "more_evil_than_good",
    "beyond_redemption",
}
DISPOSITION_AVAILABILITY = {
    "recommend_rebirth": "base",
    "ordinary_transfer": "base",
    "send_to_prison": "base",
    "soul_dissolution": "extreme_test",
}
BASE_DISPOSITION_IDS = {
    disposition_id
    for disposition_id, availability in DISPOSITION_AVAILABILITY.items()
    if availability == "base"
}
SEMANTIC_TYPES = {
    "action",
    "personality",
    "thought",
    "outcome",
    "relationship",
    "death",
}
SOURCE_TYPES = {
    "objective_trace",
    "soul_self_knowledge",
    "others_testimony",
    "local_underworld_record",
    "obsession_echo",
}
ACQUISITION_TYPES = {"prototype_fixed"}
DIVERSITY_TYPES = {"personality", "thought", "relationship"}
FORBIDDEN_KEY_NAMES = {
    "truemorality",
    "correctlifestory",
    "correctdisposition",
    "hiddentruth",
    "canonicalinterpretation",
    "officialmoraljudgement",
    "officialdisposition",
}


class CaseFileError(Exception):
    """A file, JSON, or CLI input error, distinct from a design failure."""


@dataclass
class ValidationReport:
    is_valid: bool
    errors: list[str]
    cases: list[dict[str, Any]]
    labels: list[str]

    @property
    def fragment_count(self) -> int:
        return sum(
            len(case.get("fragments", []))
            for case in self.cases
            if isinstance(case, dict)
        )

    def format_success(self) -> str:
        ordered = sorted(
            zip(self.labels, self.cases),
            key=lambda item: (
                str(item[1].get("case_id", "")),
                str(item[1].get("case_version", "")),
            ),
        )
        lines = [
            "CASE DESIGN VALIDATION PASSED",
            f"Cases: {len(self.cases)}",
            f"Fragments: {self.fragment_count}",
        ]
        for _, case in ordered:
            lines.append(
                f"{case['case_id']} v{case['case_version']}: "
                f"{len(case['fragments'])} fragments"
            )
        lines.extend(
            [
                "Design Metadata Only: "
                "interpretation_hooks, disposition_support_tags, "
                "information_weight, relation_tags",
                "These are design metadata only.",
                "Validation does not make them runtime evidence.",
                "No hidden-answer fields found.",
            ]
        )
        return "\n".join(lines)

    def format_failure(self) -> str:
        lines = ["CASE DESIGN VALIDATION FAILED"]
        lines.extend(f"- {error}" for error in self.errors)
        return "\n".join(lines)


def _normalize_key(value: object) -> str:
    return "".join(
        character
        for character in str(value).casefold()
        if character.isalnum()
    )


def _is_non_empty_string(value: object) -> bool:
    return isinstance(value, str) and bool(value.strip())


def _format_path(label: str, suffix: str) -> str:
    if suffix.startswith("["):
        return f"{label}{suffix}"
    return f"{label}.{suffix}"


def _check_forbidden_keys(
    value: object,
    *,
    path: str,
    errors: list[str],
) -> None:
    if isinstance(value, dict):
        for key, nested in value.items():
            key_path = _format_path(path, str(key))
            if _normalize_key(key) in FORBIDDEN_KEY_NAMES:
                errors.append(
                    f"{key_path}: forbidden hidden-answer field name"
                )
            _check_forbidden_keys(nested, path=key_path, errors=errors)
    elif isinstance(value, list):
        for index, nested in enumerate(value):
            _check_forbidden_keys(
                nested,
                path=f"{path}[{index}]",
                errors=errors,
            )


def _validate_string_list(
    value: object,
    *,
    path: str,
    errors: list[str],
    allow_empty: bool = False,
) -> list[str]:
    if not isinstance(value, list):
        errors.append(f"{path}: expected an array")
        return []
    if not allow_empty and not value:
        errors.append(f"{path}: must be a non-empty array")
    values: list[str] = []
    seen: set[str] = set()
    for index, item in enumerate(value):
        item_path = f"{path}[{index}]"
        if not _is_non_empty_string(item):
            errors.append(f"{item_path}: expected a non-empty string")
            continue
        if item in seen:
            errors.append(f"{item_path}: duplicate value '{item}'")
        seen.add(item)
        values.append(item)
    return values


def _validate_case(
    case: object,
    *,
    label: str,
) -> list[str]:
    errors: list[str] = []
    if not isinstance(case, dict):
        return [f"{label}: expected a JSON object"]

    _check_forbidden_keys(case, path=label, errors=errors)

    missing = sorted(REQUIRED_TOP_LEVEL_FIELDS - set(case))
    for field_name in missing:
        errors.append(
            f"{label}.{field_name}: required top-level field is missing"
        )

    case_id = case.get("case_id")
    case_version = case.get("case_version")
    if not _is_non_empty_string(case_id):
        errors.append(f"{label}.case_id: expected a non-empty string")
    if not _is_non_empty_string(case_version):
        errors.append(
            f"{label}.case_version: expected a non-empty version string"
        )

    if case.get("design_mode") != "local_facts_open_moral_judgement":
        errors.append(
            f"{label}.design_mode: must equal "
            "'local_facts_open_moral_judgement'"
        )
    if case.get("hidden_complete_truth") is not False:
        errors.append(f"{label}.hidden_complete_truth: must be false")

    moral_items = case.get("allowed_moral_judgements")
    moral_ids: list[str] = []
    if not isinstance(moral_items, list) or not moral_items:
        errors.append(
            f"{label}.allowed_moral_judgements: expected a non-empty array"
        )
    else:
        seen: set[str] = set()
        for index, item in enumerate(moral_items):
            item_path = f"{label}.allowed_moral_judgements[{index}]"
            if not isinstance(item, dict):
                errors.append(f"{item_path}: expected an object")
                continue
            moral_id = item.get("moral_judgement_id")
            if not _is_non_empty_string(moral_id):
                errors.append(
                    f"{item_path}.moral_judgement_id: "
                    "expected a non-empty string"
                )
                continue
            if moral_id in seen:
                errors.append(
                    f"{item_path}.moral_judgement_id: duplicate value "
                    f"'{moral_id}'"
                )
            seen.add(moral_id)
            moral_ids.append(moral_id)
            if not _is_non_empty_string(item.get("display_name")):
                errors.append(
                    f"{item_path}.display_name: "
                    "expected a non-empty string"
                )
        if set(moral_ids) != MORAL_JUDGEMENT_IDS:
            errors.append(
                f"{label}.allowed_moral_judgements: IDs must be exactly "
                f"{sorted(MORAL_JUDGEMENT_IDS)}"
            )

    disposition_items = case.get("allowed_dispositions")
    disposition_ids: list[str] = []
    if not isinstance(disposition_items, list) or not disposition_items:
        errors.append(
            f"{label}.allowed_dispositions: expected a non-empty array"
        )
    else:
        seen = set()
        for index, item in enumerate(disposition_items):
            item_path = f"{label}.allowed_dispositions[{index}]"
            if not isinstance(item, dict):
                errors.append(f"{item_path}: expected an object")
                continue
            disposition_id = item.get("disposition_id")
            availability = item.get("availability")
            if not _is_non_empty_string(disposition_id):
                errors.append(
                    f"{item_path}.disposition_id: "
                    "expected a non-empty string"
                )
                continue
            if disposition_id in seen:
                errors.append(
                    f"{item_path}.disposition_id: duplicate value "
                    f"'{disposition_id}'"
                )
            seen.add(disposition_id)
            disposition_ids.append(disposition_id)
            if not _is_non_empty_string(item.get("display_name")):
                errors.append(
                    f"{item_path}.display_name: "
                    "expected a non-empty string"
                )
            if availability not in {"base", "extreme_test"}:
                errors.append(
                    f"{item_path}.availability: expected 'base' or "
                    "'extreme_test'"
                )
            elif DISPOSITION_AVAILABILITY.get(disposition_id) != availability:
                errors.append(
                    f"{item_path}.availability: '{disposition_id}' must "
                    f"be marked '{DISPOSITION_AVAILABILITY.get(disposition_id)}'"
                )
        if set(disposition_ids) != set(DISPOSITION_AVAILABILITY):
            errors.append(
                f"{label}.allowed_dispositions: IDs must be exactly "
                f"{sorted(DISPOSITION_AVAILABILITY)}"
            )

    relation_definitions = case.get("relation_tag_definitions")
    relation_tags: set[str] = set()
    if not isinstance(relation_definitions, list) or not relation_definitions:
        errors.append(
            f"{label}.relation_tag_definitions: "
            "expected a non-empty array"
        )
    else:
        for index, item in enumerate(relation_definitions):
            item_path = f"{label}.relation_tag_definitions[{index}]"
            if not isinstance(item, dict):
                errors.append(f"{item_path}: expected an object")
                continue
            tag = item.get("relation_tag")
            if not _is_non_empty_string(tag):
                errors.append(
                    f"{item_path}.relation_tag: expected a non-empty string"
                )
                continue
            if tag in relation_tags:
                errors.append(
                    f"{item_path}.relation_tag: duplicate value '{tag}'"
                )
            relation_tags.add(tag)
            if not _is_non_empty_string(item.get("description")):
                errors.append(
                    f"{item_path}.description: "
                    "expected a non-empty string"
                )

    fragments = case.get("fragments")
    fragment_ids: set[str] = set()
    semantic_types: set[str] = set()
    all_support_tags: set[str] = set()
    if not isinstance(fragments, list):
        errors.append(f"{label}.fragments: expected an array")
        fragments = []
    elif not 5 <= len(fragments) <= 6:
        errors.append(
            f"{label}.fragments: expected between 5 and 6 fragments"
        )

    for index, fragment in enumerate(fragments):
        fragment_path = f"{label}.fragments[{index}]"
        if not isinstance(fragment, dict):
            errors.append(f"{fragment_path}: expected an object")
            continue
        missing_fragment_fields = sorted(
            REQUIRED_FRAGMENT_FIELDS - set(fragment)
        )
        for field_name in missing_fragment_fields:
            errors.append(
                f"{fragment_path}.{field_name}: required field is missing"
            )

        fragment_id = fragment.get("fragment_id")
        if not _is_non_empty_string(fragment_id):
            errors.append(
                f"{fragment_path}.fragment_id: "
                "expected a non-empty string"
            )
        elif fragment_id in fragment_ids:
            errors.append(
                f"{fragment_path}.fragment_id: duplicate value "
                f"'{fragment_id}'"
            )
        else:
            fragment_ids.add(fragment_id)

        semantic_type = fragment.get("semantic_type")
        if semantic_type not in SEMANTIC_TYPES:
            errors.append(
                f"{fragment_path}.semantic_type: unsupported value "
                f"'{semantic_type}'"
            )
        elif isinstance(semantic_type, str):
            semantic_types.add(semantic_type)

        if fragment.get("source_type") not in SOURCE_TYPES:
            errors.append(
                f"{fragment_path}.source_type: unsupported value "
                f"'{fragment.get('source_type')}'"
            )

        weight = fragment.get("information_weight")
        if isinstance(weight, bool) or not isinstance(weight, int):
            errors.append(
                f"{fragment_path}.information_weight: "
                "expected integer 1-3"
            )
        elif not 1 <= weight <= 3:
            errors.append(
                f"{fragment_path}.information_weight: expected integer 1-3"
            )

        if fragment.get("acquisition_type") not in ACQUISITION_TYPES:
            errors.append(
                f"{fragment_path}.acquisition_type: unsupported value "
                f"'{fragment.get('acquisition_type')}'"
            )
        if not _is_non_empty_string(fragment.get("text")):
            errors.append(
                f"{fragment_path}.text: expected a non-empty string"
            )

        fragment_relation_tags = _validate_string_list(
            fragment.get("relation_tags"),
            path=f"{fragment_path}.relation_tags",
            errors=errors,
        )
        for tag in fragment_relation_tags:
            if tag not in relation_tags:
                errors.append(
                    f"{fragment_path}.relation_tags: unknown relation tag "
                    f"'{tag}'"
                )

        support_tags = _validate_string_list(
            fragment.get("disposition_support_tags"),
            path=f"{fragment_path}.disposition_support_tags",
            errors=errors,
        )
        for tag in support_tags:
            all_support_tags.add(tag)
            if tag not in DISPOSITION_AVAILABILITY:
                errors.append(
                    f"{fragment_path}.disposition_support_tags: "
                    f"unsupported disposition '{tag}'"
                )

        hooks = _validate_string_list(
            fragment.get("interpretation_hooks"),
            path=f"{fragment_path}.interpretation_hooks",
            errors=errors,
        )
        if hooks and len(set(hooks)) != len(hooks):
            errors.append(
                f"{fragment_path}.interpretation_hooks: duplicate values"
            )

    if "action" not in semantic_types:
        errors.append(f"{label}.fragments: requires at least one action")
    if "outcome" not in semantic_types:
        errors.append(f"{label}.fragments: requires at least one outcome")
    diversity_count = len(semantic_types & DIVERSITY_TYPES)
    if diversity_count < 2:
        errors.append(
            f"{label}.fragments: requires at least two of "
            "personality/thought/relationship"
        )

    if len(all_support_tags & BASE_DISPOSITION_IDS) < 2:
        errors.append(
            f"{label}.fragments: at least two different base dispositions "
            "must appear in support tags"
        )

    return errors


def validate_case_documents(
    cases: Sequence[object],
    *,
    labels: Sequence[str] | None = None,
) -> ValidationReport:
    if labels is None:
        labels = [f"case[{index}]" for index in range(len(cases))]
    if len(labels) != len(cases):
        raise ValueError("labels must have the same length as cases")

    errors: list[str] = []
    normalized_labels = [str(label) for label in labels]
    for case, label in zip(cases, normalized_labels):
        errors.extend(_validate_case(case, label=label))

    case_ids: dict[str, str] = {}
    case_keys: dict[tuple[object, object], str] = {}
    global_fragment_ids: dict[str, str] = {}
    for case, label in zip(cases, normalized_labels):
        if not isinstance(case, dict):
            continue

        case_id = case.get("case_id")
        if _is_non_empty_string(case_id):
            if case_id in case_ids:
                errors.append(
                    f"{label}: duplicate CaseID '{case_id}' also used by "
                    f"{case_ids[case_id]}"
                )
            else:
                case_ids[case_id] = label

        case_key = (case_id, case.get("case_version"))
        if case_key in case_keys:
            errors.append(
                f"{label}: duplicate CaseID/CaseVersion also used by "
                f"{case_keys[case_key]}"
            )
        else:
            case_keys[case_key] = label
        for fragment in case.get("fragments", []):
            if not isinstance(fragment, dict):
                continue
            fragment_id = fragment.get("fragment_id")
            if not _is_non_empty_string(fragment_id):
                continue
            if fragment_id in global_fragment_ids:
                errors.append(
                    f"{label}.fragments: FragmentID '{fragment_id}' "
                    "duplicates "
                    f"{global_fragment_ids[fragment_id]}"
                )
            else:
                global_fragment_ids[fragment_id] = label

    return ValidationReport(
        is_valid=not errors,
        errors=sorted(set(errors)),
        cases=list(cases),
        labels=normalized_labels,
    )


def load_case_file(path: Path | str) -> dict[str, Any]:
    case_path = Path(path)
    try:
        raw = case_path.read_text(encoding="utf-8")
    except OSError as exc:
        raise CaseFileError(
            f"{case_path}: unable to read file: {exc}"
        ) from exc
    try:
        value = json.loads(raw)
    except json.JSONDecodeError as exc:
        raise CaseFileError(
            f"{case_path}: invalid JSON at line {exc.lineno}, "
            f"column {exc.colno}: {exc.msg}"
        ) from exc
    if not isinstance(value, dict):
        raise CaseFileError(f"{case_path}: top-level JSON must be an object")
    return value


def default_case_paths(project_root: Path | None = None) -> list[Path]:
    root = project_root or PROJECT_ROOT
    return [
        root / "cases" / "case_door_knife_001.json",
        root / "cases" / "case_medicine_001.json",
    ]


def validate_case_paths(paths: Sequence[Path]) -> ValidationReport:
    cases = [load_case_file(path) for path in paths]
    return validate_case_documents(
        cases,
        labels=[str(path) for path in paths],
    )


def build_argument_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description=(
            "Read-only structural validator for the two Week8 design Cases."
        )
    )
    parser.add_argument(
        "--case",
        dest="case_paths",
        action="append",
        type=Path,
        help="Case JSON path; repeat for multiple Cases.",
    )
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    parser = build_argument_parser()
    args = parser.parse_args(argv)
    paths = args.case_paths or default_case_paths()
    try:
        report = validate_case_paths(paths)
    except CaseFileError as exc:
        print("CASE DESIGN VALIDATION ERROR")
        print(str(exc))
        return 2

    if report.is_valid:
        print(report.format_success())
        return 0

    print(report.format_failure())
    return 1


if __name__ == "__main__":
    sys.exit(main())
