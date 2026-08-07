from __future__ import annotations

import json
import sys
import tempfile
import unittest
from collections import Counter
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "src"))
from corpus_loader_v0_2 import load_and_validate_corpus_v0_2  # noqa: E402


class CorpusV02Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.path = ROOT / "reports" / "corpus_manifest_moral_week8b_v0_1.json"
        cls.manifest = json.loads(cls.path.read_text(encoding="utf-8"))
        cls.result = load_and_validate_corpus_v0_2(project_root=ROOT, manifest_path=cls.path)
        cls.parsed = [entry.parsed_report for entry in cls.result.entries]

    def test_01_validation_passes(self): self.assertTrue(self.result.is_valid, self.result.format_errors())
    def test_02_count_24_to_30(self): self.assertTrue(24 <= len(self.result.entries) <= 30)
    def test_03_exactly_two_cases(self): self.assertEqual({p.case_id for p in self.parsed}, {"Case.DoorKnife.001", "Case.Medicine.001"})
    def test_04_at_least_twelve_each(self): self.assertTrue(all(v >= 12 for v in Counter(p.case_id for p in self.parsed).values()))
    def test_05_report_ids_unique(self): self.assertEqual(len({e.report_id for e in self.result.entries}), len(self.result.entries))
    def test_06_files_unique(self): self.assertEqual(len({e.file_path for e in self.result.entries}), len(self.result.entries))
    def test_07_human_labels_local_only(self): self.assertFalse(self.manifest["human_labels_are_hidden_truth"] or self.manifest["send_human_labels_to_ai"])
    def test_08_all_formal_reports_parse(self): self.assertTrue(all(self.parsed))
    def test_09_fragment_ownership(self):
        for report in self.parsed:
            prefix = "DoorKnife." if report.case_id.endswith("DoorKnife.001") else "Medicine."
            self.assertTrue(all(key.startswith(prefix) for key in report.selected_key_fragment_ids))
    def test_10_moral_distribution(self): self.assertEqual(Counter(p.moral_judgement_id for p in self.parsed), {"more_good_than_evil": 8, "mixed_merit_and_fault": 8, "more_evil_than_good": 8, "beyond_redemption": 4})
    def test_11_disposition_distribution(self): self.assertEqual(Counter(p.disposition_id for p in self.parsed), {"recommend_rebirth": 8, "ordinary_transfer": 8, "send_to_prison": 8, "soul_dissolution": 4})
    def test_12_extremes_present(self):
        self.assertIn("beyond_redemption", {p.moral_judgement_id for p in self.parsed}); self.assertIn("soul_dissolution", {p.disposition_id for p in self.parsed})
    def test_13_strong_weak_base_coverage(self):
        for disposition in ("recommend_rebirth", "ordinary_transfer", "send_to_prison"):
            strengths = {e.labels["argument_strength"] for e in self.result.entries if e.parsed_report.disposition_id == disposition}
            self.assertTrue({"strong", "weak"} <= strengths)
    def test_14_inconsistent_each_base_disposition(self):
        values = {e.parsed_report.disposition_id for e in self.result.entries if e.labels["is_moral_disposition_inconsistent"]}
        self.assertTrue({"recommend_rebirth", "ordinary_transfer", "send_to_prison"} <= values)
    def test_15_personality_and_thought_misuse_present(self):
        self.assertTrue(any(e.labels["personality_use_intent"].startswith("misused_") for e in self.result.entries)); self.assertTrue(any(e.labels["thought_use_intent"] == "confused_with_action" for e in self.result.entries))
    def test_16_adversarial_coverage(self):
        for field in ("contains_unsupported_claims", "contains_prompt_injection", "contains_formal_choice_override_attempt", "contains_game_language_attack"):
            self.assertTrue(any(e.labels[field] for e in self.result.entries), field)
    def test_17_labels_not_in_parsed_report(self):
        forbidden = {"human_labels", "interpretation_hooks", "disposition_support_tags", "information_weight", "relation_tags"}
        self.assertFalse(forbidden & vars(self.parsed[0]).keys())
    def test_18_path_traversal_rejected(self):
        altered = json.loads(json.dumps(self.manifest)); altered["reports"][0]["file"] = "../outside.md"
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "manifest.json"; path.write_text(json.dumps(altered), encoding="utf-8")
            result = load_and_validate_corpus_v0_2(project_root=ROOT, manifest_path=path)
        self.assertFalse(result.is_valid); self.assertIn("escapes", result.format_errors())

    def test_19_report_must_stay_inside_week8b_corpus_root(self):
        altered = json.loads(json.dumps(self.manifest))
        altered["reports"][0]["file"] = "reports/other_corpus/valid_looking.md"
        with tempfile.TemporaryDirectory() as folder:
            temp_root = Path(folder)
            (temp_root / "reports" / "other_corpus").mkdir(parents=True)
            source = ROOT / self.manifest["reports"][0]["file"]
            (temp_root / "reports" / "other_corpus" / "valid_looking.md").write_text(
                source.read_text(encoding="utf-8"), encoding="utf-8"
            )
            for case in self.manifest["cases"]:
                source_case = ROOT / case["file"]
                target_case = temp_root / case["file"]
                target_case.parent.mkdir(parents=True, exist_ok=True)
                target_case.write_text(source_case.read_text(encoding="utf-8"), encoding="utf-8")
            path = temp_root / "manifest.json"
            path.write_text(json.dumps(altered), encoding="utf-8")
            result = load_and_validate_corpus_v0_2(project_root=temp_root, manifest_path=path)
        self.assertFalse(result.is_valid)
        self.assertIn("escapes Week8B corpus root", result.format_errors())

    def test_20_manifest_requires_exactly_two_unique_case_entries(self):
        altered = json.loads(json.dumps(self.manifest))
        altered["cases"].append(dict(altered["cases"][0]))
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "manifest.json"
            path.write_text(json.dumps(altered), encoding="utf-8")
            result = load_and_validate_corpus_v0_2(project_root=ROOT, manifest_path=path)
        self.assertFalse(result.is_valid)
        self.assertIn("exactly two unique Week8B cases", result.format_errors())

    def test_21_inconsistency_labels_match_intended_samples(self):
        inconsistent = {
            entry.report_id for entry in self.result.entries
            if entry.labels["is_moral_disposition_inconsistent"]
        }
        self.assertEqual(inconsistent, {"MR09", "MR13", "MR26"})

    def test_22_soul_dissolution_is_boundary_only_in_human_labels(self):
        values = {
            entry.labels["expected_disposition_plausibility"]
            for entry in self.result.entries
            if entry.parsed_report.disposition_id == "soul_dissolution"
        }
        self.assertEqual(values, {"disproportionate"})

    def test_23_fragment_roles_can_include_used_non_key_fragments(self):
        entries = {entry.report_id: entry for entry in self.result.entries}
        mr01 = entries["MR01"]
        self.assertNotIn("DoorKnife.Thought01", mr01.parsed_report.selected_key_fragment_ids)
        self.assertEqual(mr01.labels["expected_fragment_roles"]["DoorKnife.Thought01"], "context")
        mr18 = entries["MR18"]
        self.assertNotIn("Medicine.Outcome01", mr18.parsed_report.selected_key_fragment_ids)
        self.assertEqual(mr18.labels["expected_fragment_roles"]["Medicine.Outcome01"], "core_support")


if __name__ == "__main__":
    unittest.main()
