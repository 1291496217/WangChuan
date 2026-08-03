from __future__ import annotations

import copy
import json
import sys
import unittest
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parent.parent
SRC_DIR = PROJECT_ROOT / "src"

if str(SRC_DIR) not in sys.path:
    sys.path.insert(0, str(SRC_DIR))


from response_validator import (  # noqa: E402
    load_json_object,
    parse_judgement_result,
    validate_judgement_result,
)


class ResponseValidatorTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.case_data = load_json_object(
            PROJECT_ROOT / "cases" / "case_knife_001.json"
        )
        cls.judge_data = load_json_object(
            PROJECT_ROOT / "judges" / "judge_clerk_001.json"
        )
        cls.schema_data = load_json_object(
            PROJECT_ROOT
            / "schemas"
            / "judgement_result_v0_1.json"
        )
        cls.valid_payload = load_json_object(
            PROJECT_ROOT
            / "tests"
            / "fixtures"
            / "valid_judgement_result.json"
        )

    def validate(self, payload: object):
        return validate_judgement_result(
            payload,
            schema_data=self.schema_data,
            case_data=self.case_data,
            judge_data=self.judge_data,
            expected_disposition_id="detain_for_review",
        )

    def assert_invalid_with(
        self,
        payload: object,
        expected_text: str,
    ) -> None:
        result = self.validate(payload)

        self.assertFalse(
            result.is_valid,
            "Expected validation to fail, but it passed.",
        )
        self.assertIn(expected_text, result.format_errors())

    def test_valid_result_passes(self) -> None:
        result = self.validate(copy.deepcopy(self.valid_payload))

        self.assertTrue(result.is_valid, result.format_errors())

        parsed = parse_judgement_result(
            copy.deepcopy(self.valid_payload),
            schema_data=self.schema_data,
            case_data=self.case_data,
            judge_data=self.judge_data,
            expected_disposition_id="detain_for_review",
        )

        self.assertEqual(parsed.case_id, "Case.Knife.001")
        self.assertEqual(
            parsed.recognized_disposition_id,
            "detain_for_review",
        )
        self.assertEqual(len(parsed.used_fragment_ids), 5)

    def test_missing_required_field_is_rejected(self) -> None:
        payload = copy.deepcopy(self.valid_payload)
        del payload["core_claim"]

        self.assert_invalid_with(
            payload,
            "$.core_claim: required field is missing",
        )

    def test_extra_top_level_field_is_rejected(self) -> None:
        payload = copy.deepcopy(self.valid_payload)
        payload["reward_points"] = 100

        self.assert_invalid_with(
            payload,
            "$.reward_points: unexpected field is not allowed",
        )

    def test_illegal_rating_enum_is_rejected(self) -> None:
        payload = copy.deepcopy(self.valid_payload)
        payload["dimension_ratings"][
            "evidence_grounding"
        ] = "excellent"

        self.assert_invalid_with(
            payload,
            "is not in the allowed enum",
        )

    def test_unknown_fragment_id_is_rejected(self) -> None:
        payload = copy.deepcopy(self.valid_payload)
        payload["used_fragment_ids"][0] = "KnifeCase.Unknown99"

        self.assert_invalid_with(
            payload,
            "fragment ID does not exist in the current case",
        )

    def test_duplicate_fragment_id_is_rejected(self) -> None:
        payload = copy.deepcopy(self.valid_payload)
        payload["used_fragment_ids"][1] = (
            payload["used_fragment_ids"][0]
        )

        self.assert_invalid_with(
            payload,
            "must not contain duplicate items",
        )

    def test_unknown_disposition_id_is_rejected(self) -> None:
        payload = copy.deepcopy(self.valid_payload)
        payload["recognized_disposition_id"] = "pardon_and_release"

        self.assert_invalid_with(
            payload,
            "disposition ID is not allowed by the current case",
        )

    def test_legal_but_wrong_player_disposition_is_rejected(self) -> None:
        payload = copy.deepcopy(self.valid_payload)
        payload["recognized_disposition_id"] = "reincarnate"

        self.assert_invalid_with(
            payload,
            "does not match the disposition selected by the player",
        )

    def test_overlong_judge_response_is_rejected(self) -> None:
        payload = copy.deepcopy(self.valid_payload)
        payload["judge_response"] = "判" * 601

        self.assert_invalid_with(
            payload,
            "must contain at most 600 character(s)",
        )

    def test_wrong_case_id_is_rejected(self) -> None:
        payload = copy.deepcopy(self.valid_payload)
        payload["case_id"] = "Case.Other.001"

        self.assert_invalid_with(
            payload,
            "must match current case ID",
        )

    def test_wrong_judge_id_is_rejected(self) -> None:
        payload = copy.deepcopy(self.valid_payload)
        payload["judge_profile_id"] = "Judge.Other.001"

        self.assert_invalid_with(
            payload,
            "must match current judge ID",
        )

    def test_wrong_schema_version_is_rejected(self) -> None:
        payload = copy.deepcopy(self.valid_payload)
        payload["schema_version"] = "0.2"

        self.assert_invalid_with(
            payload,
            "must equal '0.1'",
        )

    def test_nested_extra_field_is_rejected(self) -> None:
        payload = copy.deepcopy(self.valid_payload)
        payload["unsupported_assumptions"][0][
            "confidence"
        ] = 0.5

        self.assert_invalid_with(
            payload,
            "unexpected field is not allowed",
        )

    def test_too_many_style_tags_are_rejected(self) -> None:
        payload = copy.deepcopy(self.valid_payload)
        payload["style_tags"] = [
            "evidence_driven",
            "compassionate",
            "legalistic",
            "bureaucratic",
            "rhetorical",
            "cautious",
        ]

        self.assert_invalid_with(
            payload,
            "must contain at most 5 item(s)",
        )


if __name__ == "__main__":
    unittest.main()

