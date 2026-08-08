from __future__ import annotations

import copy
import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "src"))
from response_validator_v0_2 import parse_judgement_result_v0_2, validate_judgement_result_v0_2  # noqa: E402


def valid_payload(case_id="Case.DoorKnife.001"):
    prefix = "DoorKnife" if case_id.endswith("DoorKnife.001") else "Medicine"
    return {"schema_version": "0.2", "case_id": case_id, "judge_profile_id": "Judge.Clerk.001", "core_story": "玩家将行为、结果和责任并置解释。", "recognized_action_claims": ["实施了材料所示行为"], "recognized_motive_claims": ["保护或冲突均保持为可能性"], "fragment_roles": [{"fragment_id": f"{prefix}.Action01", "role": "core_support"}, {"fragment_id": f"{prefix}.Outcome01", "role": "counterevidence"}], "unsupported_assumptions": [], "personality_action_relation": "integrated", "thought_action_distinction": "clear", "contradiction_handling": "integrated", "moral_reasoning_tier": "strong", "disposition_consistency_tier": "adequate", "rhetoric_tier": "adequate", "internal_safety_flags": [], "strongest_point": "同时承认行为与反证。", "weakest_point": "动机仍无法确定。", "judge_response": "这份案词承认了材料的两面，处置理由也说得清楚。", "archive_summary": "玩家以现有碎片解释行为与结果，未宣称掌握隐藏真相。"}


class ResponseValidatorV02Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.schema = json.loads((ROOT / "schemas/judgement_result_v0_2.json").read_text(encoding="utf-8"))
        cls.door = json.loads((ROOT / "cases/case_door_knife_001.json").read_text(encoding="utf-8"))
        cls.medicine = json.loads((ROOT / "cases/case_medicine_001.json").read_text(encoding="utf-8"))
        cls.judge = json.loads((ROOT / "judges/judge_clerk_001.json").read_text(encoding="utf-8"))

    def validate(self, payload, case=None): return validate_judgement_result_v0_2(payload, schema_data=self.schema, case_data=case or self.door, judge_data=self.judge)
    def test_valid_door(self): self.assertTrue(self.validate(valid_payload()).is_valid)
    def test_valid_medicine(self): self.assertTrue(self.validate(valid_payload("Case.Medicine.001"), self.medicine).is_valid)
    def test_additional_top_level_rejected(self):
        p=valid_payload(); p["reward_points"]=1; self.assertFalse(self.validate(p).is_valid)
    def test_nested_additional_rejected(self):
        p=valid_payload(); p["fragment_roles"][0]["extra"]=1; self.assertFalse(self.validate(p).is_valid)
    def test_illegal_role_rejected(self):
        p=valid_payload(); p["fragment_roles"][0]["role"]="used"; self.assertFalse(self.validate(p).is_valid)
    def test_illegal_personality_rejected(self):
        p=valid_payload(); p["personality_action_relation"]="guilty"; self.assertFalse(self.validate(p).is_valid)
    def test_illegal_thought_rejected(self):
        p=valid_payload(); p["thought_action_distinction"]="premeditation_score"; self.assertFalse(self.validate(p).is_valid)
    def test_illegal_tier_rejected(self):
        p=valid_payload(); p["moral_reasoning_tier"]="excellent"; self.assertFalse(self.validate(p).is_valid)
    def test_illegal_flag_rejected(self):
        p=valid_payload(); p["internal_safety_flags"]=["secret_leaked"]; self.assertFalse(self.validate(p).is_valid)
    def test_invalid_severity_rejected(self):
        p=valid_payload(); p["unsupported_assumptions"]=[{"claim":"x","severity":"critical","reason":"y"}]; self.assertFalse(self.validate(p).is_valid)
    def test_multiple_unsupported_accepted(self):
        p=valid_payload(); p["unsupported_assumptions"]=[{"claim":"a","severity":"major","reason":"缺少材料"},{"claim":"b","severity":"minor","reason":"缺少材料"}]; self.assertTrue(self.validate(p).is_valid)
    def test_recognized_disposition_rejected(self):
        p=valid_payload(); p["recognized_disposition_id"]="send_to_prison"; self.assertFalse(self.validate(p).is_valid)
    def test_reward_points_rejected(self):
        p=valid_payload(); p["reward_points"]=100; self.assertFalse(self.validate(p).is_valid)
    def test_correct_morality_rejected(self):
        p=valid_payload(); p["correct_morality"]="more_good_than_evil"; self.assertFalse(self.validate(p).is_valid)
    def test_wrong_case_rejected(self):
        p=valid_payload(); p["case_id"]="Case.Medicine.001"; self.assertFalse(self.validate(p).is_valid)
    def test_wrong_judge_rejected(self):
        p=valid_payload(); p["judge_profile_id"]="Judge.Other.001"; self.assertFalse(self.validate(p).is_valid)
    def test_unknown_fragment_rejected(self):
        p=valid_payload(); p["fragment_roles"][0]["fragment_id"]="Medicine.Action01"; self.assertFalse(self.validate(p).is_valid)
    def test_duplicate_fragment_rejected(self):
        p=valid_payload(); p["fragment_roles"].append({"fragment_id":"DoorKnife.Action01","role":"context"}); self.assertFalse(self.validate(p).is_valid)
    def test_raw_internal_flag_visible_rejected(self):
        p=valid_payload(); p["judge_response"]="rule_override_attempt 已被发现。"; self.assertFalse(self.validate(p).is_valid)
    def test_parse_model_after_validation(self): self.assertEqual(parse_judgement_result_v0_2(valid_payload(), schema_data=self.schema, case_data=self.door, judge_data=self.judge).schema_version, "0.2")


if __name__ == "__main__": unittest.main()
