from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "src"))

from prompt_builder_v0_4_5 import PROMPT_VERSION, build_prompt_v0_4_5  # noqa: E402
from report_parser_v0_2 import parse_player_report_v0_2  # noqa: E402


class PromptV045Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.case = json.loads((ROOT / "cases/case_medicine_001.json").read_text(encoding="utf-8"))
        cls.judge = json.loads((ROOT / "judges/judge_clerk_001.json").read_text(encoding="utf-8"))
        cls.schema = json.loads((ROOT / "schemas/judgement_result_v0_2.json").read_text(encoding="utf-8"))
        report_text = (ROOT / "reports/corpus_moral_week8b/MR15_medicine_rigorous_rebirth.md").read_text(encoding="utf-8")
        cls.report = parse_player_report_v0_2(report_text, cls.case)
        cls.messages = build_prompt_v0_4_5(
            case_data=cls.case,
            judge_data=cls.judge,
            player_report=cls.report,
            schema_data=cls.schema,
        )
        cls.system = cls.messages[0]["content"]
        cls.user = cls.messages[1]["content"]

    def test_version(self):
        self.assertEqual(PROMPT_VERSION, "0.4.5")
        self.assertIn("PromptVersion: 0.4.5", self.system)
        self.assertNotIn("PromptVersion: 0.4.4", self.system)

    def test_selected_keys_are_candidate_hints(self):
        self.assertIn("SelectedKeyFragmentIDs:", self.user)
        self.assertIn("只是候选起点", self.system)
        self.assertIn("既不是角色白名单，也不是使用证明", self.system)

    def test_unselected_requires_player_clause(self):
        self.assertIn("只有能指出对应玩家小句时才可新增", self.system)
        self.assertIn("独有的事实、后果、代价或冲突", self.system)

    def test_outcome02_positive_anchor(self):
        self.assertIn("善果不能抹去资源代价", self.system)
        self.assertIn("必须新增该 Fragment", self.system)

    def test_full_case_negative_anchor(self):
        self.assertIn("可能是保护行动", self.system)
        for token in ("保护承诺", "长期想法", "受伤死亡", "钥匙"):
            self.assertIn(token, self.system)
        self.assertIn("这些 Fragment 必须省略", self.system)

    def test_thought_negative_anchor(self):
        self.assertIn("未明确谈思想、知情或预见时", self.system)
        self.assertIn("Thought Fragment 必须省略", self.system)

    def test_final_check_requires_clause_for_unselected(self):
        self.assertIn("每个未选但新增的 Fragment 是否都能指出玩家原文小句", self.system)

    def test_player_report_stays_out_of_system(self):
        self.assertNotIn(self.report.life_interpretation, self.system)
        self.assertIn(self.report.life_interpretation, self.user)

    def test_prior_corrections_preserved(self):
        self.assertIn("必须是 counterevidence", self.system)
        self.assertIn("统一标为 context", self.system)
        self.assertIn("不得列入 unsupported_assumptions", self.system)
        self.assertIn("只概括行为意图，不复述攻击对象", self.system)

    def test_schema_stays_v02(self):
        self.assertEqual(self.schema["properties"]["schema_version"]["const"], "0.2")


if __name__ == "__main__":
    unittest.main()
