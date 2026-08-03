from __future__ import annotations

from dataclasses import dataclass
from typing import Any


@dataclass(frozen=True)
class ValidationIssue:
    """One structural or semantic validation failure."""

    path: str
    message: str

    def __str__(self) -> str:
        return f"{self.path}: {self.message}"


@dataclass(frozen=True)
class ValidationResult:
    """Collection of validation issues for one AI result."""

    issues: tuple[ValidationIssue, ...]

    @property
    def is_valid(self) -> bool:
        return len(self.issues) == 0

    def format_errors(self) -> str:
        if self.is_valid:
            return "No validation errors."

        return "\n".join(f"- {issue}" for issue in self.issues)


@dataclass(frozen=True)
class UnsupportedAssumption:
    claim: str
    severity: str
    reason: str

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "UnsupportedAssumption":
        return cls(
            claim=data["claim"],
            severity=data["severity"],
            reason=data["reason"],
        )


@dataclass(frozen=True)
class ContradictionHandling:
    level: str
    explanation: str

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "ContradictionHandling":
        return cls(
            level=data["level"],
            explanation=data["explanation"],
        )


@dataclass(frozen=True)
class DimensionRatings:
    narrative_coherence: str
    evidence_grounding: str
    rhetorical_effectiveness: str
    disposition_alignment: str

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "DimensionRatings":
        return cls(
            narrative_coherence=data["narrative_coherence"],
            evidence_grounding=data["evidence_grounding"],
            rhetorical_effectiveness=data["rhetorical_effectiveness"],
            disposition_alignment=data["disposition_alignment"],
        )


@dataclass(frozen=True)
class JudgementResult:
    """Typed local model created only after validation succeeds."""

    schema_version: str
    case_id: str
    judge_profile_id: str
    core_claim: str
    identity_hypothesis: str
    motive_hypothesis: str
    recognized_disposition_id: str
    used_fragment_ids: tuple[str, ...]
    unsupported_assumptions: tuple[UnsupportedAssumption, ...]
    contradiction_handling: ContradictionHandling
    dimension_ratings: DimensionRatings
    style_tags: tuple[str, ...]
    strongest_point: str
    weakest_point: str
    judge_response: str
    archive_summary: str

    @classmethod
    def from_validated_dict(cls, data: dict[str, Any]) -> "JudgementResult":
        """
        Build the typed model from data that has already passed validation.

        Do not call this directly on raw provider output.
        """
        return cls(
            schema_version=data["schema_version"],
            case_id=data["case_id"],
            judge_profile_id=data["judge_profile_id"],
            core_claim=data["core_claim"],
            identity_hypothesis=data["identity_hypothesis"],
            motive_hypothesis=data["motive_hypothesis"],
            recognized_disposition_id=data["recognized_disposition_id"],
            used_fragment_ids=tuple(data["used_fragment_ids"]),
            unsupported_assumptions=tuple(
                UnsupportedAssumption.from_dict(item)
                for item in data["unsupported_assumptions"]
            ),
            contradiction_handling=ContradictionHandling.from_dict(
                data["contradiction_handling"]
            ),
            dimension_ratings=DimensionRatings.from_dict(
                data["dimension_ratings"]
            ),
            style_tags=tuple(data["style_tags"]),
            strongest_point=data["strongest_point"],
            weakest_point=data["weakest_point"],
            judge_response=data["judge_response"],
            archive_summary=data["archive_summary"],
        )

