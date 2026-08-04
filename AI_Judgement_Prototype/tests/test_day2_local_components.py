from __future__ import annotations

import json
import os
import sys
import tempfile
import unittest
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parent.parent
SRC_DIR = PROJECT_ROOT / "src"

if str(SRC_DIR) not in sys.path:
    sys.path.insert(0, str(SRC_DIR))


from env_loader import (  # noqa: E402
    EnvironmentConfigurationError,
    load_env_file,
)
from prompt_builder import (  # noqa: E402
    PROMPT_VERSION,
    build_judgement_messages,
)
from report_parser import (  # noqa: E402
    PlayerReportError,
    load_player_report,
)
from result_writer import (  # noqa: E402
    sha256_text,
    write_raw_record,
)


CASE_DATA = {
    "case_id": "Case.Knife.001",
    "case_version": "0.2",
    "title": "染血的刀",
    "premise": "测试案件",
    "fragments": [
        {
            "fragment_id": "KnifeCase.Identity01",
            "category": "identity",
            "text": "灰白短褂，袖口缝着“济”字。",
        }
    ],
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
    "display_name": "值房判官",
}

SCHEMA_DATA = {
    "properties": {
        "schema_version": {
            "const": "0.1",
        }
    }
}


class Day2LocalComponentTests(unittest.TestCase):
    def test_env_loader_reads_values(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            env_path = Path(directory) / ".env"
            env_path.write_text(
                "WC_DAY2_TEST_VALUE=loaded\n",
                encoding="utf-8",
            )

            previous = os.environ.pop(
                "WC_DAY2_TEST_VALUE",
                None,
            )

            try:
                load_env_file(env_path)
                self.assertEqual(
                    os.environ["WC_DAY2_TEST_VALUE"],
                    "loaded",
                )
            finally:
                os.environ.pop(
                    "WC_DAY2_TEST_VALUE",
                    None,
                )
                if previous is not None:
                    os.environ[
                        "WC_DAY2_TEST_VALUE"
                    ] = previous

    def test_env_loader_does_not_override_existing(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            env_path = Path(directory) / ".env"
            env_path.write_text(
                "WC_DAY2_TEST_EXISTING=file\n",
                encoding="utf-8",
            )

            previous = os.environ.get(
                "WC_DAY2_TEST_EXISTING"
            )
            os.environ[
                "WC_DAY2_TEST_EXISTING"
            ] = "process"

            try:
                load_env_file(env_path)
                self.assertEqual(
                    os.environ[
                        "WC_DAY2_TEST_EXISTING"
                    ],
                    "process",
                )
            finally:
                if previous is None:
                    os.environ.pop(
                        "WC_DAY2_TEST_EXISTING",
                        None,
                    )
                else:
                    os.environ[
                        "WC_DAY2_TEST_EXISTING"
                    ] = previous

    def test_env_loader_rejects_malformed_line(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            env_path = Path(directory) / ".env"
            env_path.write_text(
                "NOT_A_PAIR\n",
                encoding="utf-8",
            )

            with self.assertRaises(
                EnvironmentConfigurationError
            ):
                load_env_file(env_path)

    def test_report_parser_accepts_valid_report(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.md"
            report_path.write_text(
                "DispositionID: detain_for_review\n\n"
                + ("材料仍然不足，需要继续复核。" * 10),
                encoding="utf-8",
            )

            report = load_player_report(
                report_path,
                case_data=CASE_DATA,
            )

            self.assertEqual(
                report.disposition_id,
                "detain_for_review",
            )

    def test_report_parser_rejects_unknown_disposition(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.md"
            report_path.write_text(
                "DispositionID: unknown\n\n"
                + ("材料仍然不足，需要继续复核。" * 10),
                encoding="utf-8",
            )

            with self.assertRaises(PlayerReportError):
                load_player_report(
                    report_path,
                    case_data=CASE_DATA,
                )

    def test_prompt_contains_json_and_untrusted_boundary(
        self,
    ) -> None:
        messages = build_judgement_messages(
            case_data=CASE_DATA,
            judge_data=JUDGE_DATA,
            schema_data=SCHEMA_DATA,
            player_report=(
                "DispositionID: detain_for_review\n"
                + ("材料仍然不足。" * 20)
            ),
            selected_disposition_id=(
                "detain_for_review"
            ),
        )

        self.assertEqual(len(messages), 2)
        self.assertIn(
            "JSON",
            messages[0]["content"],
        )
        self.assertIn(
            "<UNTRUSTED_PLAYER_REPORT>",
            messages[1]["content"],
        )
        self.assertIn(
            "Case.Knife.001",
            messages[0]["content"],
        )
        self.assertIn(
            "Judge.Clerk.001",
            messages[0]["content"],
        )
        self.assertEqual(PROMPT_VERSION, "0.1")

    def test_result_writer_refuses_overwrite(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            write_raw_record(
                results_root=root,
                run_id="same",
                payload={"value": 1},
            )

            with self.assertRaises(FileExistsError):
                write_raw_record(
                    results_root=root,
                    run_id="same",
                    payload={"value": 2},
                )

    def test_sha256_is_stable(self) -> None:
        self.assertEqual(
            sha256_text("abc"),
            sha256_text("abc"),
        )
        self.assertNotEqual(
            sha256_text("abc"),
            sha256_text("abcd"),
        )


if __name__ == "__main__":
    unittest.main()
