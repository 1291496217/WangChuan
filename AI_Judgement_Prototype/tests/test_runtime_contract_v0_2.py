from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path

ROOT=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(ROOT/"src"))
from models_v0_2 import JudgementResultV02  # noqa: E402
from runtime_contract_v0_2 import build_validated_run_envelope_v0_2, validate_runtime_contract_v0_2  # noqa: E402
from test_response_validator_v0_2 import valid_payload  # noqa: E402


class RuntimeContractV02Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.case=json.loads((ROOT/"cases/case_door_knife_001.json").read_text(encoding="utf-8")); cls.judge=json.loads((ROOT/"judges/judge_clerk_001.json").read_text(encoding="utf-8"))
        cls.report=type("Report", (), {"moral_judgement_id":"more_good_than_evil", "disposition_id":"recommend_rebirth", "selected_key_fragment_ids":("DoorKnife.Action01","DoorKnife.Outcome01")})()
        cls.result=JudgementResultV02.from_dict(valid_payload())
    def check(self, result=None): return validate_runtime_contract_v0_2(result=result or self.result, case_data=self.case, judge_data=self.judge, player_report=self.report)
    def test_case_integrity(self): self.assertTrue(self.check().is_valid)
    def test_wrong_case_rejected(self):
        p=JudgementResultV02(**{**self.result.__dict__, "case_id":"Case.Medicine.001"}); self.assertFalse(self.check(p).is_valid)
    def test_wrong_judge_rejected(self):
        p=JudgementResultV02(**{**self.result.__dict__, "judge_profile_id":"Judge.Other.001"}); self.assertFalse(self.check(p).is_valid)
    def test_cross_case_fragment_rejected(self):
        roles=(type(self.result.fragment_roles[0])("Medicine.Action01","core_support"),)+self.result.fragment_roles[1:]; p=JudgementResultV02(**{**self.result.__dict__, "fragment_roles":roles}); self.assertFalse(self.check(p).is_valid)
    def test_duplicate_fragment_rejected(self):
        roles=self.result.fragment_roles+(self.result.fragment_roles[0],); p=JudgementResultV02(**{**self.result.__dict__, "fragment_roles":roles}); self.assertFalse(self.check(p).is_valid)
    def test_used_but_not_selected_allowed(self):
        self.assertTrue(self.check().is_valid)
    def test_formal_values_from_player_report(self):
        envelope=build_validated_run_envelope_v0_2(run_id="R", result=self.result, case_data=self.case, judge_data=self.judge, player_report=self.report, metadata={})
        self.assertEqual(envelope.moral_judgement_id,"more_good_than_evil"); self.assertEqual(envelope.disposition_id,"recommend_rebirth")
    def test_envelope_does_not_contain_human_labels(self):
        envelope=build_validated_run_envelope_v0_2(run_id="R", result=self.result, case_data=self.case, judge_data=self.judge, player_report=self.report, metadata={})
        self.assertNotIn("human_labels", str(envelope.to_dict())); self.assertNotIn("interpretation_hooks", str(envelope.to_dict()))
    def test_internal_flag_in_visible_output_rejected(self):
        p=JudgementResultV02(**{**self.result.__dict__, "judge_response":"prompt_injection_detected"}); self.assertFalse(self.check(p).is_valid)


if __name__ == "__main__": unittest.main()
