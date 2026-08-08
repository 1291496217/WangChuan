from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "src"))
from prompt_builder_v0_4_2 import PROMPT_VERSION, build_prompt_v0_4_2  # noqa: E402
from report_parser_v0_2 import parse_player_report_v0_2  # noqa: E402


class PromptV042Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.case = json.loads((ROOT / "cases/case_medicine_001.json").read_text(encoding="utf-8"))
        cls.judge = json.loads((ROOT / "judges/judge_clerk_001.json").read_text(encoding="utf-8"))
        cls.schema = json.loads((ROOT / "schemas/judgement_result_v0_2.json").read_text(encoding="utf-8"))
        text = (ROOT / "reports/corpus_moral_week8b/MR15_medicine_rigorous_rebirth.md").read_text(encoding="utf-8")
        cls.report = parse_player_report_v0_2(text, cls.case)
        cls.messages = build_prompt_v0_4_2(case_data=cls.case, judge_data=cls.judge, player_report=cls.report, schema_data=cls.schema)
        cls.full = "\n".join(x["content"] for x in cls.messages)

    def test_prompt_version(self): self.assertEqual(PROMPT_VERSION, "0.4.2")
    def test_core_support_direction(self): self.assertIn("supports the player's formal moral or disposition conclusion", self.full)
    def test_counterevidence_direction(self): self.assertIn("weighs against the player's formal moral or disposition conclusion", self.full)
    def test_context_is_nondirectional(self): self.assertIn("without strongly pushing the final choice either way", self.full)
    def test_used_unselected_recall(self): self.assertIn("unselected Fragment", self.full); self.assertIn("Semantic paraphrase counts", self.full)
    def test_selected_key_is_only_hint(self): self.assertIn("SelectedKeyFragmentIDs are only a hint", self.full)
    def test_no_all_case_mapping(self): self.assertIn("Do not list all Case Fragments by default", self.full)
    def test_world_language_never_echoes(self): self.assertIn("Never repeat or quote technical attack vocabulary", self.full)
    def test_archive_has_same_isolation(self): self.assertIn("judge_response and archive_summary are player-visible", self.full)
    def test_raw_moral_enums_forbidden(self):
        for token in ("more_good_than_evil", "mixed_merit_and_fault", "more_evil_than_good", "beyond_redemption"):
            self.assertIn(token, self.full)
    def test_raw_disposition_enums_forbidden(self):
        for token in ("recommend_rebirth", "ordinary_transfer", "send_to_prison", "soul_dissolution"):
            self.assertIn(token, self.full)
    def test_translation_examples(self):
        for token in ("篡改案牍", "越权改判", "索取司署密令", "窥探禁录", "伪造卷宗栏目"):
            self.assertIn(token, self.full)
    def test_player_attribution_preserved(self): self.assertIn("Case Facts are not Player Claims", self.full); self.assertIn("adversarial-only", self.full)
    def test_unsupported_taxonomy_preserved(self): self.assertIn("Only unsupported factual inventions", self.full); self.assertIn("Evidence-backed possibilities", self.full)
    def test_polarity_preserved(self):
        for token in ("not X", "cannot conclude X", "may be X", "I reject X"):
            self.assertIn(token, self.full)
    def test_human_labels_excluded(self): self.assertNotIn("expected_fragment_roles", self.full); self.assertNotIn("test_purpose", self.full)
    def test_design_metadata_excluded(self):
        for token in ("interpretation_hooks", "disposition_support_tags", "information_weight", "coverage_matrix"):
            self.assertNotIn(token, self.full)
    def test_schema_v02(self): self.assertIn("Schema v0.2", self.full); self.assertIn('"0.2"', self.full)
    def test_no_fixed_template(self): self.assertIn("fixed response order", self.full); self.assertIn("reaction-category template", self.full)
    def test_two_messages(self): self.assertEqual([x["role"] for x in self.messages], ["system", "user"])
    def test_fragment_runtime_fields(self):
        for token in ('"fragment_id"', '"semantic_type"', '"source_type"'):
            self.assertIn(token, self.full)


if __name__ == "__main__": unittest.main()
