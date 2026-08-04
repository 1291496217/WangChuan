from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path
from typing import Any


MIN_REPORT_CHARACTERS = 100
MAX_REPORT_CHARACTERS = 1200

_DISPOSITION_PATTERN = re.compile(
    r"(?im)^\s*DispositionID\s*:\s*([A-Za-z0-9_.-]+)\s*$"
)


class PlayerReportError(ValueError):
    """Raised when a player report cannot enter the experiment."""


@dataclass(frozen=True)
class PlayerReport:
    source_path: Path
    disposition_id: str
    text: str

    @property
    def character_count(self) -> int:
        return len(self.text)


def load_player_report(
    file_path: Path,
    *,
    case_data: dict[str, Any],
) -> PlayerReport:
    """Load and validate one semi-structured Markdown player report."""
    if not file_path.is_file():
        raise FileNotFoundError(f"Player report not found: {file_path}")

    text = file_path.read_text(encoding="utf-8").strip()

    if not text:
        raise PlayerReportError(f"Player report is empty: {file_path}")

    if len(text) < MIN_REPORT_CHARACTERS:
        raise PlayerReportError(
            f"Player report must contain at least "
            f"{MIN_REPORT_CHARACTERS} characters."
        )

    if len(text) > MAX_REPORT_CHARACTERS:
        raise PlayerReportError(
            f"Player report must contain at most "
            f"{MAX_REPORT_CHARACTERS} characters."
        )

    matches = _DISPOSITION_PATTERN.findall(text)

    if len(matches) != 1:
        raise PlayerReportError(
            "Player report must contain exactly one "
            "'DispositionID: <value>' line."
        )

    disposition_id = matches[0]

    allowed_ids = {
        item.get("disposition_id")
        for item in case_data.get("allowed_dispositions", [])
        if isinstance(item, dict)
    }

    if disposition_id not in allowed_ids:
        raise PlayerReportError(
            f"DispositionID {disposition_id!r} is not allowed "
            f"by case {case_data.get('case_id')!r}."
        )

    return PlayerReport(
        source_path=file_path,
        disposition_id=disposition_id,
        text=text,
    )
