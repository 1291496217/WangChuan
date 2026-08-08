from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "src"))

from prompt_builder_v0_4_3 import PROMPT_VERSION, build_prompt_v0_4_3  # noqa: E402
from report_parser_v0_2 import parse_player_report_v0_2  # noqa: E402


class PromptV043Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.case = json.loads((ROOT / "cases/case_medicine_001.json").read_text(encoding="utf-8"))
        cls.judge = json.loads((ROOT / "judges/judge_clerk_001.json").read_text(encoding="utf-8"))
        cls.schema = json.loads((ROOT / "schemas/judgement_result_v0_2.json").read_text(encoding="utf-8"))
        report_text = (ROOT / "reports/corpus_moral_week8b/MR28_medicine_adversarial.md").read_text(encoding="utf-8")
        cls.report = parse_player_report_v0_2(report_text, cls.case)
        cls.messages = build_prompt_v0_4_3(
            case_data=cls.case,
            judge_data=cls.judge,
            player_report=cls.report,
            schema_data=cls.schema,
        )
        cls.system = cls.messages[0]["content"]
        cls.user = cls.messages[1]["content"]
        cls.full = "\n".join(message["content"] for message in cls.messages)

    def test_prompt_version(self):
        self.assertEqual(PROMPT_VERSION, "0.4.3")

    def test_true_role_boundary(self):
        self.assertEqual([message["role"] for message in self.messages], ["system", "user"])
        self.assertNotIn(self.report.life_interpretation, self.system)
        self.assertNotIn(self.report.verdict_text, self.system)
        self.assertIn(self.report.life_interpretation, self.user)
        self.assertIn("<UNTRUSTED_PLAYER_REPORT>", self.user)

    def test_attack_metadata_stays_out_of_system_message(self):
        self.assertNotIn("interpretation_hooks", self.system)
        self.assertNotIn("case_disposition_coverage_v0_1", self.system)
        self.assertIn("interpretation_hooks", self.user)

    def test_role_is_relative_to_formal_conclusion(self):
        self.assertIn("推动玩家的正式 MoralJudgementID 或 DispositionID", self.system)
        self.assertIn("故事中心性", self.system)

    def test_acknowledged_harm_is_counterevidence_for_lenient_choice(self):
        self.assertIn("必须是 counterevidence", self.system)
        self.assertIn("仍应记责", self.system)
        self.assertIn("善果不能抹去资源代价", self.system)

    def test_personality_is_context_only(self):
        self.assertIn("Personality 类型只描述性格或过往表现", self.system)
        self.assertIn("统一标为 context", self.system)

    def test_selected_key_is_only_hint(self):
        self.assertIn("SelectedKeyFragmentIDs 仅是检索提示", self.system)
        self.assertIn("已选但未实际使用", self.system)

    def test_unselected_semantic_recall(self):
        self.assertIn("语义改写方式使用一个未选 Fragment", self.system)
        self.assertIn("必须纳入 fragment_roles", self.system)

    def test_no_automatic_full_case_mapping(self):
        self.assertIn("不得默认列出全部 Case Fragments", self.system)

    def test_qualified_hypothesis_is_not_unsupported(self):
        for marker in ("可能", "倾向于", "更像", "不能确定"):
            self.assertIn(marker, self.system)
        self.assertIn("不得列入 unsupported_assumptions", self.system)

    def test_unsupported_requires_asserted_new_fact(self):
        self.assertIn("作为确定事实断言", self.system)
        self.assertIn("没有任何 Fragment 锚点", self.system)

    def test_adversarial_only_attribution_remains_empty(self):
        self.assertIn("recognized_action_claims", self.system)
        self.assertIn("fragment_roles、unsupported_assumptions 都应为空", self.system)

    def test_visible_text_omits_attack_object(self):
        self.assertIn("只概括行为意图，不复述攻击对象", self.system)
        self.assertIn("archive_summary 不是技术取证日志", self.system)

    def test_final_silent_check_covers_both_visible_fields(self):
        self.assertIn("<FINAL_SILENT_CHECK>", self.system)
        self.assertIn("judge_response 与 archive_summary", self.system)
        self.assertIn("删除具体对象", self.system)

    def test_formal_choice_preservation(self):
        self.assertIn("Preserve them exactly", self.system)

    def test_schema_stays_v02(self):
        self.assertIn("Schema v0.2", self.system)
        self.assertEqual(self.schema["properties"]["schema_version"]["const"], "0.2")

    def test_human_labels_and_design_metadata_are_excluded(self):
        for token in (
            "expected_fragment_roles",
            "test_purpose",
            "disposition_support_tags",
            "information_weight",
            "coverage_matrix",
        ):
            self.assertNotIn(token, self.system)

    def test_runtime_fragment_fields_remain_minimal(self):
        for token in ('"fragment_id"', '"text"', '"semantic_type"', '"source_type"'):
            self.assertIn(token, self.system)

    def test_no_general_fixed_response_template(self):
        self.assertIn("Do not use a fixed response order", self.system)


if __name__ == "__main__":
    unittest.main()
