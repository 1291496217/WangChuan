from __future__ import annotations

from dataclasses import dataclass
from typing import Any


@dataclass(frozen=True)
class FragmentRoleV02:
    fragment_id: str
    role: str


@dataclass(frozen=True)
class UnsupportedAssumptionV02:
    claim: str
    severity: str
    reason: str


@dataclass(frozen=True)
class JudgementResultV02:
    schema_version: str
    case_id: str
    judge_profile_id: str
    core_story: str
    recognized_action_claims: tuple[str, ...]
    recognized_motive_claims: tuple[str, ...]
    fragment_roles: tuple[FragmentRoleV02, ...]
    unsupported_assumptions: tuple[UnsupportedAssumptionV02, ...]
    personality_action_relation: str
    thought_action_distinction: str
    contradiction_handling: str
    moral_reasoning_tier: str
    disposition_consistency_tier: str
    rhetoric_tier: str
    internal_safety_flags: tuple[str, ...]
    strongest_point: str
    weakest_point: str
    judge_response: str
    archive_summary: str

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "JudgementResultV02":
        return cls(
            schema_version=data["schema_version"], case_id=data["case_id"], judge_profile_id=data["judge_profile_id"],
            core_story=data["core_story"], recognized_action_claims=tuple(data["recognized_action_claims"]),
            recognized_motive_claims=tuple(data["recognized_motive_claims"]),
            fragment_roles=tuple(FragmentRoleV02(**x) for x in data["fragment_roles"]),
            unsupported_assumptions=tuple(UnsupportedAssumptionV02(**x) for x in data["unsupported_assumptions"]),
            personality_action_relation=data["personality_action_relation"], thought_action_distinction=data["thought_action_distinction"],
            contradiction_handling=data["contradiction_handling"], moral_reasoning_tier=data["moral_reasoning_tier"],
            disposition_consistency_tier=data["disposition_consistency_tier"], rhetoric_tier=data["rhetoric_tier"],
            internal_safety_flags=tuple(data["internal_safety_flags"]), strongest_point=data["strongest_point"],
            weakest_point=data["weakest_point"], judge_response=data["judge_response"], archive_summary=data["archive_summary"],
        )


@dataclass(frozen=True)
class ValidatedRunEnvelopeV02:
    run_id: str
    case_id: str
    case_version: str
    judge_profile_id: str
    judge_version: str
    moral_judgement_id: str
    disposition_id: str
    selected_key_fragment_ids: tuple[str, ...]
    result: JudgementResultV02
    metadata: dict[str, Any]

    def to_dict(self) -> dict[str, Any]:
        result = self.result
        return {
            "run_id": self.run_id, "case_id": self.case_id, "case_version": self.case_version,
            "judge_profile_id": self.judge_profile_id, "judge_version": self.judge_version,
            "moral_judgement_id": self.moral_judgement_id, "disposition_id": self.disposition_id,
            "selected_key_fragment_ids": list(self.selected_key_fragment_ids),
            "metadata": self.metadata,
            "judgement_result": {
                **{k: v for k, v in result.__dict__.items() if k not in {"fragment_roles", "unsupported_assumptions"}},
                "fragment_roles": [x.__dict__ for x in result.fragment_roles],
                "unsupported_assumptions": [x.__dict__ for x in result.unsupported_assumptions],
                "internal_safety_flags": list(result.internal_safety_flags),
                "recognized_action_claims": list(result.recognized_action_claims),
                "recognized_motive_claims": list(result.recognized_motive_claims),
            },
        }
