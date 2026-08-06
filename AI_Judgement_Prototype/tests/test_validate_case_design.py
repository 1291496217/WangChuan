from __future__ import annotations

import copy
import contextlib
import io
import sys
import tempfile
import unittest
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent
SRC_DIR = PROJECT_ROOT / "src"
if str(SRC_DIR) not in sys.path:
    sys.path.insert(0, str(SRC_DIR))

from validate_case_design import (  # noqa: E402
    CaseFileError,
    default_case_paths,
    load_case_file,
    main,
    validate_case_documents,
)


class CaseDesignValidatorTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.door_path = PROJECT_ROOT / "cases" / "case_door_knife_001.json"
        cls.medicine_path = PROJECT_ROOT / "cases" / "case_medicine_001.json"
        cls.door = load_case_file(cls.door_path)
        cls.medicine = load_case_file(cls.medicine_path)

    def clone_door(self) -> dict:
        return copy.deepcopy(self.door)

    def clone_medicine(self) -> dict:
        return copy.deepcopy(self.medicine)

    def assert_invalid(
        self,
        cases: list[dict],
        expected_text: str,
    ) -> None:
        report = validate_case_documents(
            cases,
            labels=[f"case-{index}" for index in range(len(cases))],
        )
        self.assertFalse(report.is_valid, "expected validation failure")
        self.assertTrue(
            any(expected_text in error for error in report.errors),
            report.errors,
        )

    def test_two_valid_cases_pass(self) -> None:
        report = validate_case_documents(
            [self.clone_door(), self.clone_medicine()],
            labels=["door", "medicine"],
        )
        self.assertTrue(report.is_valid, report.errors)
        self.assertEqual(report.fragment_count, 12)

    def test_explicit_case_paths_pass(self) -> None:
        report = validate_case_documents(
            [
                load_case_file(self.door_path),
                load_case_file(self.medicine_path),
            ],
            labels=[str(self.door_path), str(self.medicine_path)],
        )
        self.assertTrue(report.is_valid, report.errors)

    def test_duplicate_case_id_and_version_is_rejected(self) -> None:
        duplicate = self.clone_medicine()
        duplicate["case_id"] = self.door["case_id"]
        duplicate["case_version"] = self.door["case_version"]
        self.assert_invalid(
            [self.clone_door(), duplicate],
            "duplicate CaseID/CaseVersion",
        )

    def test_duplicate_case_id_with_different_version_is_rejected(
        self,
    ) -> None:
        duplicate = self.clone_medicine()
        duplicate["case_id"] = self.door["case_id"]
        duplicate["case_version"] = "0.2"
        self.assert_invalid(
            [self.clone_door(), duplicate],
            "duplicate CaseID",
        )

    def test_duplicate_fragment_id_within_case_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["fragments"][1]["fragment_id"] = (
            case["fragments"][0]["fragment_id"]
        )
        self.assert_invalid([case], "duplicate value")

    def test_duplicate_fragment_id_across_cases_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["fragments"][0]["fragment_id"] = (
            self.door["fragments"][0]["fragment_id"]
        )
        self.assert_invalid([self.clone_door(), case], "duplicates")

    def test_illegal_semantic_type_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["fragments"][0]["semantic_type"] = "motive"
        self.assert_invalid([case], "unsupported value 'motive'")

    def test_illegal_source_type_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["fragments"][0]["source_type"] = "rumor"
        self.assert_invalid([case], "source_type: unsupported value 'rumor'")

    def test_information_weight_out_of_range_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["fragments"][0]["information_weight"] = 4
        self.assert_invalid([case], "expected integer 1-3")

    def test_boolean_information_weight_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["fragments"][0]["information_weight"] = True
        self.assert_invalid([case], "expected integer 1-3")

    def test_illegal_acquisition_type_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["fragments"][0]["acquisition_type"] = "generated"
        self.assert_invalid([case], "acquisition_type: unsupported value")

    def test_unknown_relation_tag_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["fragments"][0]["relation_tags"].append("heroic_theft")
        self.assert_invalid([case], "unknown relation tag 'heroic_theft'")

    def test_duplicate_relation_definition_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["relation_tag_definitions"].append(
            copy.deepcopy(case["relation_tag_definitions"][0])
        )
        self.assert_invalid([case], "relation_tag: duplicate value")

    def test_illegal_support_tag_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["fragments"][0]["disposition_support_tags"] = [
            "heroic_pardon"
        ]
        self.assert_invalid([case], "unsupported disposition")

    def test_duplicate_support_tag_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["fragments"][0]["disposition_support_tags"].append(
            case["fragments"][0]["disposition_support_tags"][0]
        )
        self.assert_invalid([case], "disposition_support_tags[2]: duplicate")

    def test_missing_action_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["fragments"][0]["semantic_type"] = "outcome"
        self.assert_invalid([case], "requires at least one action")

    def test_missing_outcome_is_rejected(self) -> None:
        case = self.clone_medicine()
        for fragment in case["fragments"]:
            if fragment["semantic_type"] == "outcome":
                fragment["semantic_type"] = "action"
        self.assert_invalid([case], "requires at least one outcome")

    def test_missing_personality_thought_relationship_diversity_is_rejected(
        self,
    ) -> None:
        case = self.clone_medicine()
        for fragment in case["fragments"]:
            if fragment["semantic_type"] in {
                "personality",
                "thought",
                "relationship",
            }:
                fragment["semantic_type"] = "action"
        self.assert_invalid(
            [case],
            "requires at least two of personality/thought/relationship",
        )

    def test_fragment_count_below_five_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["fragments"] = case["fragments"][:4]
        self.assert_invalid([case], "expected between 5 and 6 fragments")

    def test_fragment_count_above_six_is_rejected(self) -> None:
        case = self.clone_medicine()
        extra = copy.deepcopy(case["fragments"][0])
        extra["fragment_id"] = "Medicine.Extra01"
        case["fragments"].append(extra)
        self.assert_invalid([case], "expected between 5 and 6 fragments")

    def test_hidden_complete_truth_true_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["hidden_complete_truth"] = True
        self.assert_invalid([case], "hidden_complete_truth: must be false")

    def test_wrong_design_mode_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["design_mode"] = "closed_answer"
        self.assert_invalid([case], "design_mode: must equal")

    def test_missing_base_disposition_coverage_is_rejected(self) -> None:
        case = self.clone_medicine()
        for fragment in case["fragments"]:
            fragment["disposition_support_tags"] = ["soul_dissolution"]
        self.assert_invalid(
            [case],
            "at least two different base dispositions",
        )

    def test_soul_dissolution_support_is_allowed(self) -> None:
        case = self.clone_medicine()
        case["fragments"][0]["disposition_support_tags"] = [
            "soul_dissolution"
        ]
        report = validate_case_documents([case], labels=["medicine"])
        self.assertTrue(report.is_valid, report.errors)

    def test_two_outcome_fragments_are_allowed(self) -> None:
        semantic_types = [
            fragment["semantic_type"] for fragment in self.medicine["fragments"]
        ]
        self.assertEqual(semantic_types.count("outcome"), 2)
        report = validate_case_documents(
            [self.clone_medicine()],
            labels=["medicine"],
        )
        self.assertTrue(report.is_valid, report.errors)

    def test_true_morality_hidden_key_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["true_morality"] = "more_good_than_evil"
        self.assert_invalid([case], "forbidden hidden-answer field name")

    def test_nested_correct_disposition_hidden_key_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["metadata"] = {"CorrectDisposition": "ordinary_transfer"}
        self.assert_invalid([case], "forbidden hidden-answer field name")

    def test_empty_interpretation_hook_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["fragments"][0]["interpretation_hooks"] = [""]
        self.assert_invalid([case], "interpretation_hooks[0]")

    def test_empty_fragment_text_is_rejected(self) -> None:
        case = self.clone_medicine()
        case["fragments"][0]["text"] = " "
        self.assert_invalid([case], "text: expected a non-empty string")

    def test_empty_relation_tags_are_rejected(self) -> None:
        case = self.clone_medicine()
        case["fragments"][0]["relation_tags"] = []
        self.assert_invalid([case], "relation_tags: must be a non-empty array")

    def test_missing_fragment_field_is_rejected(self) -> None:
        case = self.clone_medicine()
        del case["fragments"][0]["interpretation_hooks"]
        self.assert_invalid(
            [case],
            "interpretation_hooks: required field is missing",
        )

    def test_missing_top_level_field_is_rejected(self) -> None:
        case = self.clone_medicine()
        del case["relation_tag_definitions"]
        self.assert_invalid(
            [case],
            "relation_tag_definitions: required top-level field is missing",
        )

    def test_invalid_json_is_a_file_error(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "broken.json"
            path.write_text("{not json", encoding="utf-8")
            with self.assertRaises(CaseFileError):
                load_case_file(path)

    def test_default_paths_include_only_new_cases(self) -> None:
        paths = default_case_paths(PROJECT_ROOT)
        self.assertEqual(
            [path.name for path in paths],
            ["case_door_knife_001.json", "case_medicine_001.json"],
        )
        self.assertNotIn("case_knife_001.json", [path.name for path in paths])

    def test_success_output_declares_metadata_boundary(self) -> None:
        report = validate_case_documents(
            [self.clone_door(), self.clone_medicine()],
            labels=["door", "medicine"],
        )
        output = report.format_success()
        self.assertIn("CASE DESIGN VALIDATION PASSED", output)
        self.assertIn("These are design metadata only.", output)
        self.assertIn(
            "Validation does not make them runtime evidence.",
            output,
        )
        self.assertIn("No hidden-answer fields found.", output)

    def test_main_default_command_returns_zero(self) -> None:
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            result = main([])
        self.assertEqual(result, 0)
        self.assertIn("Cases: 2", output.getvalue())
        self.assertIn("Fragments: 12", output.getvalue())

    def test_main_explicit_command_returns_zero(self) -> None:
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            result = main(
                [
                    "--case",
                    str(self.door_path),
                    "--case",
                    str(self.medicine_path),
                ]
            )
        self.assertEqual(result, 0)
        self.assertIn("Case.Medicine.001 v0.1: 6 fragments", output.getvalue())

    def test_main_invalid_json_returns_two(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "broken.json"
            path.write_text("{not json", encoding="utf-8")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                result = main(["--case", str(path)])
        self.assertEqual(result, 2)
        self.assertIn("CASE DESIGN VALIDATION ERROR", output.getvalue())


if __name__ == "__main__":
    unittest.main()
