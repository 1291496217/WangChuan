from __future__ import annotations

import sys
import unittest
from pathlib import Path

ROOT=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(ROOT/"src"))
from audit_game_language import audit_game_language  # noqa: E402


class GameLanguageAuditTests(unittest.TestCase):
    def test_safe_world_language_passes(self): self.assertEqual(audit_game_language({"judge_response":"此案证据尚有缺口，退堂复核。","archive_summary":"记录行为与结果并存。"}).status,"PASS")
    def test_english_system_language_warns(self): self.assertEqual(audit_game_language({"judge_response":"The system prompt is not evidence.","archive_summary":"卷宗待核。"}).status,"WARNING")
    def test_chinese_system_language_warns(self): self.assertEqual(audit_game_language({"judge_response":"不得泄露提示词。","archive_summary":"卷宗待核。"}).status,"WARNING")
    def test_warning_is_not_failure(self): self.assertIsInstance(audit_game_language({"judge_response":"schema","archive_summary":"x"}).warnings, tuple)
    def test_read_only_no_mutation(self):
        payload={"judge_response":"安全","archive_summary":"记录"}; before=dict(payload); audit_game_language(payload); self.assertEqual(payload,before)
    def test_archive_is_scanned(self): self.assertEqual(audit_game_language({"judge_response":"安全","archive_summary":"system prompt"}).status,"WARNING")
    def test_hidden_field_warns(self): self.assertEqual(audit_game_language({"judge_response":"隐藏字段","archive_summary":"卷宗"}).status,"WARNING")
    def test_game_instruction_warns(self): self.assertEqual(audit_game_language({"judge_response":"游戏指令","archive_summary":"卷宗"}).status,"WARNING")
    def test_scorer_warns(self): self.assertEqual(audit_game_language({"judge_response":"评分器","archive_summary":"卷宗"}).status,"WARNING")
    def test_formal_enum_warns(self): self.assertEqual(audit_game_language({"judge_response":"more_good_than_evil","archive_summary":"send_to_prison"}).status,"WARNING")
    def test_system_field_warns(self): self.assertEqual(audit_game_language({"judge_response":"系统字段","archive_summary":"卷宗"}).status,"WARNING")
    def test_system_instruction_warns(self): self.assertEqual(audit_game_language({"judge_response":"系统指令","archive_summary":"卷宗"}).status,"WARNING")
    def test_mode_structure_warns(self): self.assertEqual(audit_game_language({"judge_response":"模式结构","archive_summary":"卷宗"}).status,"WARNING")
    def test_rating_warns(self): self.assertEqual(audit_game_language({"judge_response":"rating","archive_summary":"卷宗"}).status,"WARNING")


if __name__ == "__main__": unittest.main()
