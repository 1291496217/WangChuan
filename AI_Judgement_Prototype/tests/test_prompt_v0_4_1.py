from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "src"))
from prompt_builder_v0_4_1 import PROMPT_VERSION, build_prompt_v0_4_1  # noqa: E402
from report_parser_v0_2 import parse_player_report_v0_2  # noqa: E402


class PromptV041Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.case = json.loads((ROOT / "cases/case_door_knife_001.json").read_text(encoding="utf-8"))
        cls.judge = json.loads((ROOT / "judges/judge_clerk_001.json").read_text(encoding="utf-8"))
        cls.schema = json.loads((ROOT / "schemas/judgement_result_v0_2.json").read_text(encoding="utf-8"))
        text = (ROOT / "reports/corpus_moral_week8b/MR01_doorknife_rigorous_rebirth.md").read_text(encoding="utf-8")
        cls.report = parse_player_report_v0_2(text, cls.case)
        cls.messages = build_prompt_v0_4_1(case_data=cls.case, judge_data=cls.judge, player_report=cls.report, schema_data=cls.schema)
        cls.full = "\n".join(x["content"] for x in cls.messages)

    def test_prompt_id(self): self.assertEqual(PROMPT_VERSION, "0.4.1")
    def test_case_facts_are_not_player_claims(self): self.assertIn("Case Facts are not Player Claims", self.full)
    def test_player_claim_attribution(self): self.assertIn("what the player actually argued", self.full)
    def test_adversarial_only_rule(self): self.assertIn("does not develop a substantive life interpretation", self.full)
    def test_selective_fragment_rule(self): self.assertIn("Never map all Case Fragments by default", self.full)
    def test_selected_key_is_not_proof(self): self.assertIn("SelectedKeyFragmentIDs are a hint about intent", self.full)
    def test_used_unselected_is_allowed(self): self.assertIn("an unselected Fragment may be included", self.full)
    def test_unsupported_taxonomy(self): self.assertIn("Only unsupported factual inventions", self.full)
    def test_possibilities_not_automatic_unsupported(self): self.assertIn("Evidence-backed possibilities", self.full)
    def test_normative_not_automatic_unsupported(self): self.assertIn("normative or moral judgements", self.full)
    def test_epistemic_caution_not_automatic_unsupported(self): self.assertIn("epistemic caution", self.full)
    def test_polarity_preservation(self):
        for token in ("not X", "cannot conclude X", "may be X", "I reject X"):
            self.assertIn(token, self.full)
    def test_world_language_enums(self):
        for token in ("hidden field", "game instruction", "scorer", "enum", "more_good_than_evil", "send_to_prison"):
            self.assertIn(token, self.full)
    def test_schema_stays_v02(self): self.assertIn("Schema v0.2", self.full); self.assertIn('"0.2"', self.full)
    def test_human_labels_absent(self):
        for token in ("expected_fragment_roles", "expected_moral_direction", "test_purpose", "Human Labels"):
            self.assertNotIn(token, self.full)
    def test_design_metadata_absent(self):
        for token in ("interpretation_hooks", "disposition_support_tags", "information_weight", "coverage_matrix"):
            self.assertNotIn(token, self.full)
    def test_no_fixed_template(self): self.assertIn("fixed response order", self.full); self.assertIn("reaction-category template", self.full)


if __name__ == "__main__": unittest.main()
