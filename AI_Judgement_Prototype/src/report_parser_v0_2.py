from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path
from typing import Any


MIN_LIFE_CHARACTERS = 40
MAX_LIFE_CHARACTERS = 900
MIN_VERDICT_CHARACTERS = 20
MAX_VERDICT_CHARACTERS = 600
MIN_COMBINED_CHARACTERS = 60
MAX_COMBINED_CHARACTERS = 1400

ALLOWED_MORAL_JUDGEMENTS = {
    "more_good_than_evil",
    "mixed_merit_and_fault",
    "more_evil_than_good",
    "beyond_redemption",
}
ALLOWED_DISPOSITIONS = {
    "recommend_rebirth",
    "ordinary_transfer",
    "send_to_prison",
    "soul_dissolution",
}
_SCALAR_FIELDS = {"CaseID", "MoralJudgementID", "DispositionID"}
_ALL_HEADER_FIELDS = _SCALAR_FIELDS | {"SelectedKeyFragmentIDs"}
_FIELD_RE = re.compile(r"^([A-Za-z][A-Za-z0-9_]*):\s*(.*?)\s*$")
_SECTION_RE = re.compile(r"(?m)^##\s+(LifeInterpretation|VerdictText)\s*$")


class PlayerReportV02Error(ValueError):
    """Raised when a v0.2 player report fails local structural preflight."""


@dataclass(frozen=True)
class PlayerReportV02:
    case_id: str
    moral_judgement_id: str
    disposition_id: str
    selected_key_fragment_ids: tuple[str, ...]
    life_interpretation: str
    verdict_text: str
    source_path: Path | None = None


def _fail(message: str) -> None:
    raise PlayerReportV02Error(message)


def parse_player_report_v0_2(
    text: str,
    case_data: dict[str, Any],
    *,
    source_path: Path | None = None,
) -> PlayerReportV02:
    """Parse the frozen Report Contract v0.2 without judging its argument."""
    if not isinstance(text, str) or not text.strip():
        _fail("Player report is empty.")
    normalized = text.replace("\r\n", "\n").replace("\r", "\n").strip()

    sections = list(_SECTION_RE.finditer(normalized))
    if [match.group(1) for match in sections] != [
        "LifeInterpretation",
        "VerdictText",
    ]:
        _fail("Report must contain exactly one LifeInterpretation section followed by one VerdictText section.")

    metadata = normalized[: sections[0].start()]
    life = normalized[sections[0].end() : sections[1].start()].strip()
    verdict = normalized[sections[1].end() :].strip()

    values: dict[str, str] = {}
    key_fragments: list[str] = []
    in_fragment_block = False
    for raw_line in metadata.splitlines():
        line = raw_line.strip()
        if not line or line.startswith("# "):
            continue
        match = _FIELD_RE.fullmatch(line)
        if match:
            name, value = match.groups()
            if name not in _ALL_HEADER_FIELDS:
                _fail(f"Unknown header field: {name}")
            if name in values or (name == "SelectedKeyFragmentIDs" and in_fragment_block):
                _fail(f"Duplicate header field: {name}")
            if name == "SelectedKeyFragmentIDs":
                if value:
                    _fail("SelectedKeyFragmentIDs must use a Markdown list.")
                in_fragment_block = True
                values[name] = ""
            else:
                if not value:
                    _fail(f"Header field {name} cannot be empty.")
                values[name] = value
                in_fragment_block = False
            continue
        if line.startswith("-") and in_fragment_block:
            fragment_id = line[1:].strip()
            if not fragment_id:
                _fail("Key Fragment cannot be empty.")
            key_fragments.append(fragment_id)
            continue
        _fail(f"Invalid metadata line: {line}")

    missing = _ALL_HEADER_FIELDS - values.keys()
    if missing:
        _fail("Missing header field(s): " + ", ".join(sorted(missing)))

    current_case_id = case_data.get("case_id")
    if values["CaseID"] != current_case_id:
        _fail(f"CaseID must match current case {current_case_id!r}.")
    if values["MoralJudgementID"] not in ALLOWED_MORAL_JUDGEMENTS:
        _fail("Unknown MoralJudgementID.")
    if values["DispositionID"] not in ALLOWED_DISPOSITIONS:
        _fail("Unknown DispositionID.")

    case_morals = {
        item.get("moral_judgement_id")
        for item in case_data.get("allowed_moral_judgements", [])
        if isinstance(item, dict)
    }
    case_dispositions = {
        item.get("disposition_id")
        for item in case_data.get("allowed_dispositions", [])
        if isinstance(item, dict)
    }
    if values["MoralJudgementID"] not in case_morals:
        _fail("MoralJudgementID is not allowed by the current case.")
    if values["DispositionID"] not in case_dispositions:
        _fail("DispositionID is not allowed by the current case.")

    if not 2 <= len(key_fragments) <= 4:
        _fail("SelectedKeyFragmentIDs must contain 2 to 4 items.")
    if len(key_fragments) != len(set(key_fragments)):
        _fail("SelectedKeyFragmentIDs must be unique.")
    allowed_fragments = {
        item.get("fragment_id")
        for item in case_data.get("fragments", [])
        if isinstance(item, dict)
    }
    unknown = [item for item in key_fragments if item not in allowed_fragments]
    if unknown:
        _fail("Key Fragment does not belong to the current case: " + ", ".join(unknown))

    if not MIN_LIFE_CHARACTERS <= len(life) <= MAX_LIFE_CHARACTERS:
        _fail(f"LifeInterpretation must contain {MIN_LIFE_CHARACTERS}-{MAX_LIFE_CHARACTERS} characters.")
    if not MIN_VERDICT_CHARACTERS <= len(verdict) <= MAX_VERDICT_CHARACTERS:
        _fail(f"VerdictText must contain {MIN_VERDICT_CHARACTERS}-{MAX_VERDICT_CHARACTERS} characters.")
    combined = len(life) + len(verdict)
    if not MIN_COMBINED_CHARACTERS <= combined <= MAX_COMBINED_CHARACTERS:
        _fail(f"Combined free text must contain {MIN_COMBINED_CHARACTERS}-{MAX_COMBINED_CHARACTERS} characters.")

    return PlayerReportV02(
        case_id=values["CaseID"],
        moral_judgement_id=values["MoralJudgementID"],
        disposition_id=values["DispositionID"],
        selected_key_fragment_ids=tuple(key_fragments),
        life_interpretation=life,
        verdict_text=verdict,
        source_path=source_path,
    )


def validate_player_report_v0_2(text: str, case_data: dict[str, Any]) -> PlayerReportV02:
    return parse_player_report_v0_2(text, case_data)


def load_player_report_v0_2(file_path: Path, *, case_data: dict[str, Any]) -> PlayerReportV02:
    if not file_path.is_file():
        raise FileNotFoundError(f"Player report not found: {file_path}")
    return parse_player_report_v0_2(
        file_path.read_text(encoding="utf-8"),
        case_data,
        source_path=file_path,
    )
