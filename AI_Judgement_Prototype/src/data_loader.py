from __future__ import annotations

import json
from pathlib import Path
from typing import Any


def load_json_file(file_path: Path) -> dict[str, Any]:
    """
    Load and parse a UTF-8 JSON file.

    Raises:
        FileNotFoundError: If the requested file does not exist.
        ValueError: If the file does not contain a JSON object.
        json.JSONDecodeError: If the JSON syntax is invalid.
    """
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


def load_text_file(file_path: Path) -> str:
    """
    Load a UTF-8 text file and reject an empty document.

    Raises:
        FileNotFoundError: If the requested file does not exist.
        ValueError: If the file is empty.
    """
    if not file_path.is_file():
        raise FileNotFoundError(f"Text file not found: {file_path}")

    content = file_path.read_text(encoding="utf-8").strip()

    if not content:
        raise ValueError(f"Text file is empty: {file_path}")

    return content