from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

ROOT=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(ROOT/"src"))
from ai_client import DeepSeekResponse  # noqa: E402
from experiment_cli_v0_2 import main as cli_main  # noqa: E402
from prompt_builder_v0_4 import build_prompt_v0_4  # noqa: E402


class RunnerV02Tests(unittest.TestCase):
    def test_cli_requires_exact_confirmation(self):
        with patch("sys.argv", ["experiment_cli_v0_2.py", "--report", "MR01", "--confirm", "RUN MR02"]):
            with self.assertRaises(SystemExit): cli_main()
    def test_prompt_builder_is_importable_without_key(self):
        self.assertEqual(build_prompt_v0_4.__name__, "build_prompt_v0_4")
    def test_result_directory_name(self): self.assertEqual(Path("results/week8b_v0_2").parts[-1], "week8b_v0_2")
    def test_provider_response_shape_has_usage(self):
        response=DeepSeekResponse("r","m","m","stop","{}",None,{"total_tokens":1}); self.assertEqual(response.usage["total_tokens"],1)
    def test_no_api_key_literal_in_runner(self):
        text=(ROOT/"src/run_real_judgement_v0_2.py").read_text(encoding="utf-8"); self.assertNotIn("sk-",text); self.assertNotIn("DEEPSEEK_API_KEY=",text)
    def test_no_batch_loop_in_runner(self):
        text=(ROOT/"src/run_real_judgement_v0_2.py").read_text(encoding="utf-8"); self.assertNotIn("for report in",text)
    def test_no_retry_in_runner(self):
        text=(ROOT/"src/run_real_judgement_v0_2.py").read_text(encoding="utf-8"); self.assertNotIn("retry",text.lower())
    def test_results_are_separate(self): self.assertNotEqual(ROOT/"results", ROOT/"results/week8b_v0_2")


if __name__ == "__main__": unittest.main()
