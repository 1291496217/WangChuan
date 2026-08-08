from __future__ import annotations

import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "src"))

from run_real_judgement_v0_4_3 import CORRECTION_STAGE, SCHEMA_VERSION, publication_allowed  # noqa: E402


class RunnerV043Tests(unittest.TestCase):
    def test_identity(self):
        self.assertEqual(CORRECTION_STAGE, "Week8 Day6 Root-Cause Correction")
        self.assertEqual(SCHEMA_VERSION, "0.2")

    def test_publication_requires_schema_and_language_pass(self):
        self.assertTrue(publication_allowed(validation_passed=True, language_status="PASS"))
        self.assertFalse(publication_allowed(validation_passed=False, language_status="PASS"))
        self.assertFalse(publication_allowed(validation_passed=True, language_status="WARNING"))

    def test_runner_contains_no_key_literal(self):
        text = (ROOT / "src/run_real_judgement_v0_4_3.py").read_text(encoding="utf-8")
        self.assertNotIn("sk-", text)
        self.assertNotIn("DEEPSEEK_API_KEY=", text)


if __name__ == "__main__":
    unittest.main()
