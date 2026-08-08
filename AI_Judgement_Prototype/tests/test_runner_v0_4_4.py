from __future__ import annotations

import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "src"))

from run_real_judgement_v0_4_4 import CORRECTION_STAGE, SCHEMA_VERSION  # noqa: E402
from run_real_judgement_versioned import publication_allowed  # noqa: E402


class RunnerV044Tests(unittest.TestCase):
    def test_identity(self):
        self.assertEqual(CORRECTION_STAGE, "Week8 Day6 Selected-Key De-Anchoring Correction")
        self.assertEqual(SCHEMA_VERSION, "0.2")

    def test_publication_gate(self):
        self.assertTrue(publication_allowed(validation_passed=True, language_status="PASS"))
        self.assertFalse(publication_allowed(validation_passed=True, language_status="WARNING"))
        self.assertFalse(publication_allowed(validation_passed=False, language_status="PASS"))

    def test_runner_contains_no_key_literal(self):
        for name in ("run_real_judgement_versioned.py", "run_real_judgement_v0_4_4.py"):
            text = (ROOT / "src" / name).read_text(encoding="utf-8")
            self.assertNotIn("sk-", text)
            self.assertNotIn("DEEPSEEK_API_KEY=", text)


if __name__ == "__main__":
    unittest.main()
