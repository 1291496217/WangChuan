from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "src"))
from prompt_builder_v0_4 import PROMPT_VERSION, build_prompt_v0_4  # noqa: E402
from report_parser_v0_2 import parse_player_report_v0_2  # noqa: E402


class PromptV04Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.case = json.loads((ROOT / "cases/case_door_knife_001.json").read_text(encoding="utf-8"))
        cls.judge = json.loads((ROOT / "judges/judge_clerk_001.json").read_text(encoding="utf-8"))
        cls.schema = json.loads((ROOT / "schemas/judgement_result_v0_2.json").read_text(encoding="utf-8"))
        text = (ROOT / "reports/corpus_moral_week8b/MR01_doorknife_rigorous_rebirth.md").read_text(encoding="utf-8")
        cls.report = parse_player_report_v0_2(text, cls.case)
        cls.messages = build_prompt_v0_4(case_data=cls.case, judge_data=cls.judge, player_report=cls.report, schema_data=cls.schema)
        cls.full = "\n".join(x["content"] for x in cls.messages)

    def test_prompt_version(self): self.assertEqual(PROMPT_VERSION, "0.4")
    def test_two_messages(self): self.assertEqual([x["role"] for x in self.messages], ["system", "user"])
    def test_semantic_type_and_source_type_present(self): self.assertIn("semantic_type", self.full); self.assertIn("source_type", self.full)
    def test_case_fields_present(self): self.assertIn("Case.DoorKnife.001", self.full); self.assertIn("0.1", self.full)
    def test_formal_moral_and_disposition_present(self): self.assertIn("more_good_than_evil", self.full); self.assertIn("recommend_rebirth", self.full)
    def test_selected_keys_and_free_text_present(self): self.assertIn("SelectedKeyFragmentIDs", self.full); self.assertIn("LifeInterpretation", self.full); self.assertIn("VerdictText", self.full)
    def test_judge_persona_present(self): self.assertIn("Judge.Clerk.001", self.full)
    def test_schema_instructions_present(self): self.assertIn('"schema_version"', self.full); self.assertIn('"0.2"', self.full)
    def test_design_metadata_absent(self):
        for token in ("interpretation_hooks", "disposition_support_tags", "information_weight", "relation_tags", "case_disposition_coverage_v0_1"):
            self.assertNotIn(token, self.full)
    def test_human_labels_absent(self):
        for token in ("expected_fragment_roles", "expected_moral_direction", "argument_strength", "test_purpose", "Human Labels"):
            self.assertNotIn(token, self.full)
    def test_fragment_payload_is_reduced(self): self.assertIn('"fragment_id"', self.full); self.assertNotIn('"relation_tags"', self.full)
    def test_personality_principle(self): self.assertIn("Personality is context", self.full); self.assertIn("guilt or innocence", self.full)
    def test_thought_action_principle(self): self.assertIn("Thought is not Action", self.full)
    def test_outcome_motive_principle(self): self.assertIn("Outcome is not Motive", self.full)
    def test_unsupported_splitting(self): self.assertIn("independent decision-relevant unsupported", self.full); self.assertIn("unsupported count is not a score", self.full)
    def test_no_hidden_truth(self): self.assertIn("no hidden complete life story", self.full); self.assertIn("no hidden correct morality", self.full)
    def test_formal_choices_are_not_model_output(self): self.assertIn("Never replace", self.full); self.assertNotIn("recognized_disposition_id", self.full)
    def test_game_language_isolation(self): self.assertIn("world-internal", self.full); self.assertIn("API key", self.full)
    def test_no_fixed_choreography(self):
        self.assertIn("fixed response order", self.full)
        self.assertIn("reaction-category template", self.full)
    def test_no_reward_decision(self): self.assertIn("reward", self.full.lower())


if __name__ == "__main__": unittest.main()
