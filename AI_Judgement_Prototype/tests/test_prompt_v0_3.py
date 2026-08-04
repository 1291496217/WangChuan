from __future__ import annotations

import sys
import unittest
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parent.parent
SRC_DIR = PROJECT_ROOT / "src"

if str(SRC_DIR) not in sys.path:
    sys.path.insert(0, str(SRC_DIR))


from prompt_builder import (  # noqa: E402
    PROMPT_VERSION,
    build_judgement_messages,
)


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


def system_prompt() -> str:
    messages = build_judgement_messages(
        case_data=CASE_DATA,
        judge_data=JUDGE_DATA,
        schema_data=SCHEMA_DATA,
        player_report=(
            "DispositionID: detain_for_review\n"
            + ("材料仍然不足。" * 20)
        ),
        selected_disposition_id="detain_for_review",
    )
    return messages[0]["content"]


class PromptV03Tests(unittest.TestCase):
    def test_prompt_version_is_v03(self) -> None:
        self.assertEqual(PROMPT_VERSION, "0.3")

    def test_prompt_rejects_fixed_response_order(self) -> None:
        self.assertIn("不设固定回应顺序", system_prompt())

    def test_prompt_has_no_mandatory_question_or_quote(self) -> None:
        prompt = system_prompt()
        self.assertIn("固定问句", prompt)
        self.assertIn("固定引用要求", prompt)
        self.assertIn("可以提出问题，也可以直接下评语", prompt)

    def test_prompt_does_not_use_reaction_categories(self) -> None:
        self.assertIn(
            "也不从预设“反应类型”中选择模板",
            system_prompt(),
        )

    def test_persona_principles_remain_stable(self) -> None:
        self.assertIn(
            "证据原则、制度立场和人格边界必须稳定",
            system_prompt(),
        )

    def test_emotion_cannot_change_evidence_boundary(self) -> None:
        self.assertIn(
            "情绪不能改变事实边界",
            system_prompt(),
        )


if __name__ == "__main__":
    unittest.main()
