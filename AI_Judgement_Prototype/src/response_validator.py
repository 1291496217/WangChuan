from __future__ import annotations

import json
import re
from pathlib import Path
from typing import Any

from models import JudgementResult, ValidationIssue, ValidationResult


def load_json_object(file_path: Path) -> dict[str, Any]:
    """Load a UTF-8 JSON file whose root must be an object."""
    if not file_path.is_file():
        raise FileNotFoundError(f"JSON file not found: {file_path}")

    with file_path.open("r", encoding="utf-8") as file:
        data = json.load(file)

    if not isinstance(data, dict):
        raise ValueError(
            f"Expected a JSON object at the root of {file_path}, "
            f"but received {type(data).__name__}."
        )

    return data


def _add_issue(
    issues: list[ValidationIssue],
    path: str,
    message: str,
) -> None:
    issues.append(ValidationIssue(path=path, message=message))


def _json_type_matches(value: Any, expected_type: str) -> bool:
    if expected_type == "object":
        return isinstance(value, dict)

    if expected_type == "array":
        return isinstance(value, list)

    if expected_type == "string":
        return isinstance(value, str)

    if expected_type == "integer":
        return isinstance(value, int) and not isinstance(value, bool)

    if expected_type == "number":
        return (
            isinstance(value, (int, float))
            and not isinstance(value, bool)
        )

    if expected_type == "boolean":
        return isinstance(value, bool)

    if expected_type == "null":
        return value is None

    return False


def _validate_schema_node(
    value: Any,
    schema_node: dict[str, Any],
    path: str,
    issues: list[ValidationIssue],
) -> None:
    """
    Validate the subset of JSON Schema used by Week8.

    Supported keywords:
    type, const, enum, pattern, minLength, maxLength,
    minItems, maxItems, uniqueItems, items,
    required, properties, additionalProperties.
    """
    expected_type = schema_node.get("type")

    if expected_type is not None:
        if not _json_type_matches(value, expected_type):
            _add_issue(
                issues,
                path,
                f"expected type '{expected_type}', "
                f"received '{type(value).__name__}'",
            )
            return

    if "const" in schema_node and value != schema_node["const"]:
        _add_issue(
            issues,
            path,
            f"must equal {schema_node['const']!r}",
        )

    if "enum" in schema_node and value not in schema_node["enum"]:
        _add_issue(
            issues,
            path,
            f"value {value!r} is not in the allowed enum",
        )

    if isinstance(value, str):
        min_length = schema_node.get("minLength")
        max_length = schema_node.get("maxLength")
        pattern = schema_node.get("pattern")

        if min_length is not None and len(value) < min_length:
            _add_issue(
                issues,
                path,
                f"must contain at least {min_length} character(s)",
            )

        if max_length is not None and len(value) > max_length:
            _add_issue(
                issues,
                path,
                f"must contain at most {max_length} character(s)",
            )

        if pattern is not None and re.fullmatch(pattern, value) is None:
            _add_issue(
                issues,
                path,
                f"value does not match required pattern {pattern!r}",
            )

    if isinstance(value, list):
        min_items = schema_node.get("minItems")
        max_items = schema_node.get("maxItems")

        if min_items is not None and len(value) < min_items:
            _add_issue(
                issues,
                path,
                f"must contain at least {min_items} item(s)",
            )

        if max_items is not None and len(value) > max_items:
            _add_issue(
                issues,
                path,
                f"must contain at most {max_items} item(s)",
            )

        if schema_node.get("uniqueItems") is True:
            serialized_items = [
                json.dumps(item, ensure_ascii=False, sort_keys=True)
                for item in value
            ]

            if len(serialized_items) != len(set(serialized_items)):
                _add_issue(
                    issues,
                    path,
                    "must not contain duplicate items",
                )

        item_schema = schema_node.get("items")
        if isinstance(item_schema, dict):
            for index, item in enumerate(value):
                _validate_schema_node(
                    item,
                    item_schema,
                    f"{path}[{index}]",
                    issues,
                )

    if isinstance(value, dict):
        properties = schema_node.get("properties", {})
        required_fields = set(schema_node.get("required", []))
        actual_fields = set(value.keys())

        for missing_field in sorted(required_fields - actual_fields):
            _add_issue(
                issues,
                f"{path}.{missing_field}",
                "required field is missing",
            )

        if schema_node.get("additionalProperties") is False:
            allowed_fields = set(properties.keys())

            for extra_field in sorted(actual_fields - allowed_fields):
                _add_issue(
                    issues,
                    f"{path}.{extra_field}",
                    "unexpected field is not allowed",
                )

        for field_name, field_schema in properties.items():
            if field_name in value:
                _validate_schema_node(
                    value[field_name],
                    field_schema,
                    f"{path}.{field_name}",
                    issues,
                )


def _validate_runtime_semantics(
    payload: dict[str, Any],
    case_data: dict[str, Any],
    judge_data: dict[str, Any],
    expected_disposition_id: str | None,
    issues: list[ValidationIssue],
) -> None:
    """
    Validate facts that JSON Schema alone cannot know.

    These checks depend on the currently loaded Case, Judge, and player input.
    """
    current_case_id = case_data.get("case_id")
    current_judge_id = judge_data.get("judge_profile_id")

    if payload.get("case_id") != current_case_id:
        _add_issue(
            issues,
            "$.case_id",
            f"must match current case ID {current_case_id!r}",
        )

    if payload.get("judge_profile_id") != current_judge_id:
        _add_issue(
            issues,
            "$.judge_profile_id",
            f"must match current judge ID {current_judge_id!r}",
        )

    allowed_disposition_ids = {
        item.get("disposition_id")
        for item in case_data.get("allowed_dispositions", [])
        if isinstance(item, dict)
    }

    recognized_disposition_id = payload.get(
        "recognized_disposition_id"
    )

    if (
        isinstance(recognized_disposition_id, str)
        and recognized_disposition_id not in allowed_disposition_ids
    ):
        _add_issue(
            issues,
            "$.recognized_disposition_id",
            "disposition ID is not allowed by the current case",
        )

    if (
        expected_disposition_id is not None
        and recognized_disposition_id != expected_disposition_id
    ):
        _add_issue(
            issues,
            "$.recognized_disposition_id",
            "does not match the disposition selected by the player",
        )

    allowed_fragment_ids = {
        item.get("fragment_id")
        for item in case_data.get("fragments", [])
        if isinstance(item, dict)
    }

    used_fragment_ids = payload.get("used_fragment_ids")

    if isinstance(used_fragment_ids, list):
        for index, fragment_id in enumerate(used_fragment_ids):
            if (
                isinstance(fragment_id, str)
                and fragment_id not in allowed_fragment_ids
            ):
                _add_issue(
                    issues,
                    f"$.used_fragment_ids[{index}]",
                    "fragment ID does not exist in the current case",
                )


def validate_judgement_result(
    payload: Any,
    *,
    schema_data: dict[str, Any],
    case_data: dict[str, Any],
    judge_data: dict[str, Any],
    expected_disposition_id: str | None = None,
) -> ValidationResult:
    """
    Validate one raw judgement result.

    Layer 1: structural validation against the local schema.
    Layer 2: semantic validation against the active Case, Judge,
             and optional player-selected disposition.
    """
    issues: list[ValidationIssue] = []

    _validate_schema_node(
        payload,
        schema_data,
        "$",
        issues,
    )

    if isinstance(payload, dict):
        _validate_runtime_semantics(
            payload=payload,
            case_data=case_data,
            judge_data=judge_data,
            expected_disposition_id=expected_disposition_id,
            issues=issues,
        )

    return ValidationResult(issues=tuple(issues))


def parse_judgement_result(
    payload: Any,
    *,
    schema_data: dict[str, Any],
    case_data: dict[str, Any],
    judge_data: dict[str, Any],
    expected_disposition_id: str | None = None,
) -> JudgementResult:
    """
    Validate raw data and convert it into the typed local model.

    Raises:
        ValueError: If any structural or semantic validation fails.
    """
    validation = validate_judgement_result(
        payload,
        schema_data=schema_data,
        case_data=case_data,
        judge_data=judge_data,
        expected_disposition_id=expected_disposition_id,
    )

    if not validation.is_valid:
        raise ValueError(
            "Judgement result validation failed:\n"
            f"{validation.format_errors()}"
        )

    return JudgementResult.from_validated_dict(payload)

