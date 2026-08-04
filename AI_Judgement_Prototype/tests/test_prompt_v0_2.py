from __future__ import annotations

import sys
import unittest
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parent.parent
SRC_DIR = PROJECT_ROOT / "src"

if str(SRC_DIR) not in sys.path:
    sys.path.insert(0, str(SRC_DIR))


from prompt_builder import build_judgement_messages  # noqa: E402


CASE_DATA = {
    "case_id": "Case.Knife.001",
    "case_version": "0.2",
    "title": "染血的刀",
    "premise": "测试",
    "fragments": [],
    "allowed_dispositions": [
        {
            "disposition_id": "detain_for_review",
            "display_name": "收押复审",
        }
    ],
}

JUDGE_DATA = {
    "judge_profile_id": "Judge.Clerk.001",
    "version": "0.1",
}

SCHEMA_DATA = {
    "properties": {
        "schema_version": {
            "const": "0.1",
        }
    }
}


def build_messages():
    return build_judgement_messages(
        case_data=CASE_DATA,
        judge_data=JUDGE_DATA,
        schema_data=SCHEMA_DATA,
        player_report=(
            "DispositionID: detain_for_review\n"
            + ("材料仍然不足。" * 20)
        ),
        selected_disposition_id="detain_for_review",
    )


class PromptV02SemanticRegressionTests(unittest.TestCase):
    def test_shape_example_uses_empty_unsupported_array(self) -> None:
        self.assertIn(
            '"unsupported_assumptions": []',
            build_messages()[0]["content"],
        )

    def test_no_issue_requires_empty_array(self) -> None:
        self.assertIn(
            "如果玩家没有无依据假设，必须返回空数组",
            build_messages()[0]["content"],
        )

    def test_reasonable_possibility_is_not_unsupported(self) -> None:
        self.assertIn(
            "就不是 unsupported assumption",
            build_messages()[0]["content"],
        )

    def test_model_must_not_fill_field_for_feedback(self) -> None:
        self.assertIn(
            "不得为了提供反馈、填充字段或显得严格",
            build_messages()[0]["content"],
        )

    def test_report_remains_untrusted(self) -> None:
        self.assertIn(
            "<UNTRUSTED_PLAYER_REPORT>",
            build_messages()[1]["content"],
        )

    def test_hidden_truth_is_forbidden(self) -> None:
        self.assertIn(
            "本案不存在隐藏标准答案",
            build_messages()[0]["content"],
        )


if __name__ == "__main__":
    unittest.main()
