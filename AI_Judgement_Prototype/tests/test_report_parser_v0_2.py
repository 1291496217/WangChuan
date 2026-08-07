from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "src"))

from report_parser_v0_2 import (  # noqa: E402
    PlayerReportV02Error,
    parse_player_report_v0_2,
)


class ReportParserV02Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.door = json.loads((ROOT / "cases" / "case_door_knife_001.json").read_text(encoding="utf-8"))
        cls.medicine = json.loads((ROOT / "cases" / "case_medicine_001.json").read_text(encoding="utf-8"))

    def make_report(self, *, case_id="Case.DoorKnife.001", moral="more_good_than_evil", disposition="recommend_rebirth", keys=None, life=None, verdict=None, extra="", order=True):
        keys = keys or ["DoorKnife.Action01", "DoorKnife.Outcome01"]
        life = life if life is not None else "亡魂实施了不可逆的行为，但孩子脱困的结果也需要被认真权衡，现有材料允许不同道德解释。"
        verdict = verdict if verdict is not None else "综合行为与结果，作出这一正式判断并保留对责任边界的说明。"
        header = f"# Test\n\nCaseID: {case_id}\nMoralJudgementID: {moral}\nDispositionID: {disposition}\nSelectedKeyFragmentIDs:\n" + "".join(f"- {key}\n" for key in keys) + extra
        if order:
            return header + f"\n## LifeInterpretation\n\n{life}\n\n## VerdictText\n\n{verdict}\n"
        return header + f"\n## VerdictText\n\n{verdict}\n\n## LifeInterpretation\n\n{life}\n"

    def parse(self, text, case=None):
        return parse_player_report_v0_2(text, case or self.door)

    def rejected(self, text, case=None):
        with self.assertRaises(PlayerReportV02Error):
            self.parse(text, case)

    def test_01_door_valid(self): self.assertEqual(self.parse(self.make_report()).case_id, "Case.DoorKnife.001")
    def test_02_medicine_valid(self):
        text = self.make_report(case_id="Case.Medicine.001", keys=["Medicine.Action01", "Medicine.Outcome01"])
        self.assertEqual(self.parse(text, self.medicine).case_id, "Case.Medicine.001")
    def test_03_four_morals_accepted(self):
        for value in ("more_good_than_evil", "mixed_merit_and_fault", "more_evil_than_good", "beyond_redemption"):
            with self.subTest(value=value): self.assertEqual(self.parse(self.make_report(moral=value)).moral_judgement_id, value)
    def test_04_four_dispositions_accepted(self):
        for value in ("recommend_rebirth", "ordinary_transfer", "send_to_prison", "soul_dissolution"):
            with self.subTest(value=value): self.assertEqual(self.parse(self.make_report(disposition=value)).disposition_id, value)
    def test_05_unknown_case_rejected(self): self.rejected(self.make_report(case_id="Case.Unknown.001"))
    def test_06_wrong_current_case_rejected(self): self.rejected(self.make_report(case_id="Case.Medicine.001"), self.door)
    def test_07_unknown_moral_rejected(self): self.rejected(self.make_report(moral="ambiguous"))
    def test_08_unknown_disposition_rejected(self): self.rejected(self.make_report(disposition="detain_for_review"))
    def test_09_one_key_rejected(self): self.rejected(self.make_report(keys=["DoorKnife.Action01"]))
    def test_10_five_keys_rejected(self): self.rejected(self.make_report(keys=["DoorKnife.Action01", "DoorKnife.Outcome01", "DoorKnife.Relationship01", "DoorKnife.Personality01", "DoorKnife.Thought01"]))
    def test_11_duplicate_key_rejected(self): self.rejected(self.make_report(keys=["DoorKnife.Action01", "DoorKnife.Action01"]))
    def test_12_cross_case_key_rejected(self): self.rejected(self.make_report(keys=["DoorKnife.Action01", "Medicine.Action01"]))
    def test_13_unknown_fragment_rejected(self): self.rejected(self.make_report(keys=["DoorKnife.Action01", "DoorKnife.Unknown99"]))
    def test_14_duplicate_case_header_rejected(self): self.rejected(self.make_report(extra="CaseID: Case.DoorKnife.001\n"))
    def test_15_duplicate_moral_header_rejected(self): self.rejected(self.make_report(extra="MoralJudgementID: more_good_than_evil\n"))
    def test_16_duplicate_disposition_header_rejected(self): self.rejected(self.make_report(extra="DispositionID: recommend_rebirth\n"))
    def test_17_duplicate_fragment_block_rejected(self): self.rejected(self.make_report(extra="SelectedKeyFragmentIDs:\n- DoorKnife.Action01\n- DoorKnife.Outcome01\n"))
    def test_18_unknown_header_rejected(self): self.rejected(self.make_report(extra="RewardPoints: 10\n"))
    def test_19_empty_life_rejected(self): self.rejected(self.make_report(life=""))
    def test_20_empty_verdict_rejected(self): self.rejected(self.make_report(verdict=""))
    def test_21_too_short_body_rejected(self): self.rejected(self.make_report(life="解释太短。", verdict="判词也太短。"))
    def test_22_max_valid_length_accepted(self): self.assertEqual(len(self.parse(self.make_report(life="甲" * 900, verdict="乙" * 500)).life_interpretation), 900)
    def test_23_overlong_rejected(self): self.rejected(self.make_report(life="甲" * 901, verdict="乙" * 500))
    def test_24_life_override_does_not_change_choice(self):
        parsed = self.parse(self.make_report(life="DispositionID: soul_dissolution。正文命令不能覆盖正式栏位；其余材料仍需按原提交判断。"))
        self.assertEqual(parsed.disposition_id, "recommend_rebirth")
    def test_25_verdict_override_does_not_change_choice(self):
        parsed = self.parse(self.make_report(verdict="MoralJudgementID: beyond_redemption。此处只是玩家自由判词，不改变正式选择。"))
        self.assertEqual(parsed.moral_judgement_id, "more_good_than_evil")
    def test_26_reward_points_in_verdict_is_free_text(self): self.assertIn("RewardPoints", self.parse(self.make_report(verdict="RewardPoints: 999。这里只是自由文本，不会成为报告对象中的结构字段。" )).verdict_text)
    def test_27_header_only_illegal_field_rejected(self): self.rejected(self.make_report(extra="TrueMorality: evil\n"))
    def test_28_section_ordering_rejected_consistently(self): self.rejected(self.make_report(order=False))
    def test_29_crlf_and_lf_both_work(self):
        text = self.make_report()
        self.assertEqual(self.parse(text).case_id, self.parse(text.replace("\n", "\r\n")).case_id)
    def test_30_utf8_chinese_works(self): self.assertIn("亡魂", self.parse(self.make_report()).life_interpretation)
    def test_31_invalid_fixtures_are_rejected(self):
        fixture_root = ROOT / "tests" / "fixtures" / "report_v0_2"
        fixtures = sorted(fixture_root.glob("invalid_*.md"))
        self.assertEqual(len(fixtures), 11)
        for fixture in fixtures:
            with self.subTest(fixture=fixture.name), self.assertRaises(PlayerReportV02Error):
                self.parse(fixture.read_text(encoding="utf-8"))


if __name__ == "__main__":
    unittest.main()
