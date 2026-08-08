from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "src"))

from prompt_builder_v0_4_6 import PROMPT_VERSION, build_prompt_v0_4_6  # noqa: E402
from report_parser_v0_2 import parse_player_report_v0_2  # noqa: E402


class PromptV046Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.case = json.loads((ROOT / "cases/case_medicine_001.json").read_text(encoding="utf-8"))
        cls.judge = json.loads((ROOT / "judges/judge_clerk_001.json").read_text(encoding="utf-8"))
        cls.schema = json.loads((ROOT / "schemas/judgement_result_v0_2.json").read_text(encoding="utf-8"))
        report_text = (ROOT / "reports/corpus_moral_week8b/MR28_medicine_adversarial.md").read_text(encoding="utf-8")
        cls.report = parse_player_report_v0_2(report_text, cls.case)
        cls.messages = build_prompt_v0_4_6(
            case_data=cls.case,
            judge_data=cls.judge,
            player_report=cls.report,
            schema_data=cls.schema,
        )
        cls.system = cls.messages[0]["content"]
        cls.user = cls.messages[1]["content"]

    def test_version(self):
        self.assertEqual(PROMPT_VERSION, "0.4.6")
        self.assertIn("PromptVersion: 0.4.6", self.system)

    def test_strict_lexicon_terms(self):
        for token in ("玩家、正式栏、正式提交", "界外信息", "界外指令", "输入、字段、指令", "系统、模型、游戏", "提示词、奖励"):
            self.assertIn(token, self.system)

    def test_world_translation_map(self):
        for token in ("呈文人", "判牍所署", "案牍所载", "司署密录", "越权之词", "索取不当赏格"):
            self.assertIn(token, self.system)

    def test_no_echo_before_rejection(self):
        self.assertIn("不得先复述禁用词再拒绝", self.system)

    def test_normal_reports_omit_security_commentary(self):
        self.assertIn("若报告不是对抗型", self.system)
        self.assertIn("安全审计与普通案情无关时完全省略", self.system)

    def test_final_check_matches_lexicon(self):
        self.assertIn("按 VISIBLE_LEXICON_GATE 重写后再输出", self.system)

    def test_player_report_remains_user_only(self):
        self.assertNotIn(self.report.life_interpretation, self.system)
        self.assertIn(self.report.life_interpretation, self.user)

    def test_balanced_fragment_rules_preserved(self):
        self.assertIn("只是候选起点", self.system)
        self.assertIn("必须新增该 Fragment", self.system)
        self.assertIn("这些 Fragment 必须省略", self.system)

    def test_direction_unsupported_and_attribution_preserved(self):
        self.assertIn("必须是 counterevidence", self.system)
        self.assertIn("不得列入 unsupported_assumptions", self.system)
        self.assertIn("fragment_roles、unsupported_assumptions 都应为空", self.system)

    def test_schema_v02(self):
        self.assertEqual(self.schema["properties"]["schema_version"]["const"], "0.2")


if __name__ == "__main__":
    unittest.main()
