from __future__ import annotations

import json
import sys
import unittest
from collections import Counter
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parent.parent
SRC_DIR = PROJECT_ROOT / "src"

if str(SRC_DIR) not in sys.path:
    sys.path.insert(0, str(SRC_DIR))


from corpus_loader import load_and_validate_corpus  # noqa: E402
from report_parser import PlayerReportError, load_player_report  # noqa: E402
from response_validator import load_json_object  # noqa: E402


class CorpusTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.case_data = load_json_object(
            PROJECT_ROOT / "cases" / "case_knife_001.json"
        )
        cls.manifest_path = (
            PROJECT_ROOT / "reports" / "corpus_manifest_v0_1.json"
        )
        cls.manifest = json.loads(
            cls.manifest_path.read_text(encoding="utf-8")
        )
        cls.result = load_and_validate_corpus(
            project_root=PROJECT_ROOT,
            manifest_path=cls.manifest_path,
            case_data=cls.case_data,
        )

    def test_corpus_validation_passes(self) -> None:
        self.assertTrue(
            self.result.is_valid,
            self.result.format_errors(),
        )

    def test_corpus_contains_twenty_reports(self) -> None:
        self.assertEqual(len(self.result.entries), 20)

    def test_report_ids_are_unique(self) -> None:
        report_ids = [entry.report_id for entry in self.result.entries]
        self.assertEqual(len(report_ids), len(set(report_ids)))

    def test_five_groups_contain_four_reports_each(self) -> None:
        counts = Counter(entry.group for entry in self.result.entries)
        self.assertEqual(len(counts), 5)
        self.assertTrue(all(count == 4 for count in counts.values()))

    def test_all_three_dispositions_are_present(self) -> None:
        values = {
            entry.labels["selected_disposition_id"]
            for entry in self.result.entries
        }
        self.assertEqual(
            values,
            {"reincarnate", "detain_for_review", "dissolve"},
        )

    def test_nineteen_reports_pass_preflight(self) -> None:
        count = sum(
            entry.labels["expected_preflight"] == "pass"
            for entry in self.result.entries
        )
        self.assertEqual(count, 19)

    def test_one_report_is_expected_local_rejection(self) -> None:
        rejected = [
            entry
            for entry in self.result.entries
            if entry.labels["expected_preflight"] == "reject"
        ]
        self.assertEqual(len(rejected), 1)
        self.assertEqual(rejected[0].report_id, "R20")

    def test_all_pass_reports_parse(self) -> None:
        for entry in self.result.entries:
            if entry.labels["expected_preflight"] != "pass":
                continue
            parsed = load_player_report(
                entry.file_path,
                case_data=self.case_data,
            )
            self.assertEqual(
                parsed.disposition_id,
                entry.labels["selected_disposition_id"],
            )

    def test_r20_is_rejected_before_api(self) -> None:
        entry = next(
            item
            for item in self.result.entries
            if item.report_id == "R20"
        )
        with self.assertRaises(PlayerReportError):
            load_player_report(
                entry.file_path,
                case_data=self.case_data,
            )

    def test_human_labels_are_not_hidden_truth(self) -> None:
        self.assertFalse(
            self.manifest["human_labels_are_hidden_truth"]
        )


if __name__ == "__main__":
    unittest.main()
