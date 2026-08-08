from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "src"))

from audit_game_language_v0_2 import AUDIT_VERSION, audit_game_language_v0_2  # noqa: E402


class GameLanguageAuditV02Tests(unittest.TestCase):
    def test_version(self):
        self.assertEqual(AUDIT_VERSION, "0.2")

    def test_safe_world_language_passes(self):
        payload = {
            "judge_response": "呈文未成实质案情论证，只见越权改判之意。",
            "archive_summary": "判牍维持不变，案卷退回。",
        }
        self.assertEqual(audit_game_language_v0_2(payload).status, "PASS")

    def test_formal_field_names_warn(self):
        for token in ("MoralJudgementID", "DispositionID", "SelectedKeyFragmentIDs"):
            with self.subTest(token=token):
                self.assertEqual(audit_game_language_v0_2({"judge_response": token, "archive_summary": "案卷"}).status, "WARNING")

    def test_design_metadata_names_warn(self):
        for token in ("interpretation_hooks", "case_disposition_coverage_v0_1"):
            with self.subTest(token=token):
                self.assertEqual(audit_game_language_v0_2({"judge_response": "案卷", "archive_summary": token}).status, "WARNING")

    def test_reward_and_ui_language_warn(self):
        for token in ("最高奖励", "正式栏", "指令覆盖"):
            with self.subTest(token=token):
                self.assertEqual(audit_game_language_v0_2({"judge_response": token, "archive_summary": "案卷"}).status, "WARNING")

    def test_frozen_mr28_is_detected(self):
        results = ROOT / "results/week8b_prompt042_micro_regression/validated"
        mr28 = None
        for path in results.glob("*.json"):
            candidate = json.loads(path.read_text(encoding="utf-8"))
            if candidate["metadata"]["report_id"] == "MR28":
                mr28 = candidate["judgement_result"]
                break
        self.assertIsNotNone(mr28)
        audit = audit_game_language_v0_2(mr28)
        self.assertEqual(audit.status, "WARNING")
        self.assertTrue(any("archive_summary" in warning for warning in audit.warnings))

    def test_read_only(self):
        payload = {"judge_response": "最高奖励", "archive_summary": "案卷"}
        before = dict(payload)
        audit_game_language_v0_2(payload)
        self.assertEqual(payload, before)


if __name__ == "__main__":
    unittest.main()
