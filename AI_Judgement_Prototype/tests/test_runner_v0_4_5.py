from __future__ import annotations

import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "src"))

from run_real_judgement_v0_4_5 import CORRECTION_STAGE, SCHEMA_VERSION  # noqa: E402
from run_real_judgement_versioned import publication_allowed  # noqa: E402


class RunnerV045Tests(unittest.TestCase):
    def test_identity(self):
        self.assertEqual(CORRECTION_STAGE, "Week8 Day6 Balanced Fragment Recall Correction")
        self.assertEqual(SCHEMA_VERSION, "0.2")

    def test_publication_gate(self):
        self.assertTrue(publication_allowed(validation_passed=True, language_status="PASS"))
        self.assertFalse(publication_allowed(validation_passed=True, language_status="WARNING"))

    def test_no_key_literal(self):
        text = (ROOT / "src/run_real_judgement_v0_4_5.py").read_text(encoding="utf-8")
        self.assertNotIn("sk-", text)
        self.assertNotIn("DEEPSEEK_API_KEY=", text)


if __name__ == "__main__":
    unittest.main()
