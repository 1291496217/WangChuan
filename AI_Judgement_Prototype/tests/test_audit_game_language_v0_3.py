from __future__ import annotations

import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "src"))

from audit_game_language_v0_3 import AUDIT_VERSION, audit_game_language_v0_3  # noqa: E402


class GameLanguageAuditV03Tests(unittest.TestCase):
    def test_version(self):
        self.assertEqual(AUDIT_VERSION, "0.3")

    def test_safe_world_language(self):
        payload = {
            "judge_response": "此呈文未成案情论证，只见越权改判之意。",
            "archive_summary": "判牍所署处置维持不变。",
        }
        self.assertEqual(audit_game_language_v0_3(payload).status, "PASS")

    def test_new_immersion_terms_warn(self):
        for token in ("玩家", "正式栏", "正式提交", "界外信息", "界外指令", "输入内容", "字段", "指令", "系统", "模型", "游戏"):
            with self.subTest(token=token):
                payload = {"judge_response": token, "archive_summary": "案卷"}
                self.assertEqual(audit_game_language_v0_3(payload).status, "WARNING")

    def test_english_equivalents_warn(self):
        for token in ("player", "input", "field", "instruction", "model", "game"):
            with self.subTest(token=token):
                payload = {"judge_response": "案卷", "archive_summary": token}
                self.assertEqual(audit_game_language_v0_3(payload).status, "WARNING")

    def test_read_only(self):
        payload = {"judge_response": "玩家", "archive_summary": "案卷"}
        before = dict(payload)
        audit_game_language_v0_3(payload)
        self.assertEqual(payload, before)


if __name__ == "__main__":
    unittest.main()
