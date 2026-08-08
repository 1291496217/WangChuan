from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "src"))

from prompt_builder_v0_4_4 import PROMPT_VERSION, build_prompt_v0_4_4  # noqa: E402
from report_parser_v0_2 import parse_player_report_v0_2  # noqa: E402


class PromptV044Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.case = json.loads((ROOT / "cases/case_medicine_001.json").read_text(encoding="utf-8"))
        cls.judge = json.loads((ROOT / "judges/judge_clerk_001.json").read_text(encoding="utf-8"))
        cls.schema = json.loads((ROOT / "schemas/judgement_result_v0_2.json").read_text(encoding="utf-8"))
        report_text = (ROOT / "reports/corpus_moral_week8b/MR15_medicine_rigorous_rebirth.md").read_text(encoding="utf-8")
        cls.report = parse_player_report_v0_2(report_text, cls.case)
        cls.messages = build_prompt_v0_4_4(
            case_data=cls.case,
            judge_data=cls.judge,
            player_report=cls.report,
            schema_data=cls.schema,
        )
        cls.system = cls.messages[0]["content"]
        cls.user = cls.messages[1]["content"]

    def test_version(self):
        self.assertEqual(PROMPT_VERSION, "0.4.4")

    def test_selected_key_values_not_sent_as_a_field(self):
        self.assertNotIn("SelectedKeyFragmentIDs:", self.user)
        self.assertIn("SelectedKeyFragmentIDs 不提供给模型", self.system)

    def test_player_report_remains_user_only(self):
        self.assertNotIn(self.report.life_interpretation, self.system)
        self.assertIn(self.report.life_interpretation, self.user)

    def test_clause_scan_covers_all_fragments(self):
        self.assertIn("拆成语义小句", self.system)
        self.assertIn("与全部 VALID_CASE_FRAGMENTS 比对", self.system)
        self.assertIn("必须先扫完全部 Fragment", self.system)

    def test_broad_resource_cost_maps_to_unique_fragment(self):
        self.assertIn("善果不能抹去资源代价", self.system)
        self.assertIn("必须把它纳入 fragment_roles", self.system)

    def test_outcome_does_not_invent_thought(self):
        self.assertIn("不得由结果倒推出", self.system)
        self.assertIn("Thought Fragment 只有在玩家实质提到", self.system)

    def test_direction_and_personality_rules_preserved(self):
        self.assertIn("必须是 counterevidence", self.system)
        self.assertIn("统一标为 context", self.system)

    def test_unsupported_qualified_hypothesis_rule_preserved(self):
        self.assertIn("不得列入 unsupported_assumptions", self.system)

    def test_action_claim_boundary(self):
        self.assertIn("recognized_action_claims 只收录", self.system)
        self.assertIn("不得放入性格评价", self.system)

    def test_world_language_rule_preserved(self):
        self.assertIn("只概括行为意图，不复述攻击对象", self.system)
        self.assertIn("archive_summary 不是技术取证日志", self.system)

    def test_schema_stays_v02(self):
        self.assertEqual(self.schema["properties"]["schema_version"]["const"], "0.2")


if __name__ == "__main__":
    unittest.main()
