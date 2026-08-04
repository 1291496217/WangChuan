from __future__ import annotations

import hashlib
import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any
from uuid import uuid4


def utc_now_iso() -> str:
    return datetime.now(timezone.utc).isoformat()


def create_run_id() -> str:
    timestamp = datetime.now(timezone.utc).strftime(
        "%Y%m%dT%H%M%S%fZ"
    )
    return f"{timestamp}_{uuid4().hex[:8]}"


def sha256_text(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def _write_new_json(
    file_path: Path,
    payload: dict[str, Any],
) -> Path:
    file_path.parent.mkdir(parents=True, exist_ok=True)

    with file_path.open("x", encoding="utf-8") as file:
        json.dump(
            payload,
            file,
            ensure_ascii=False,
            indent=2,
        )
        file.write("\n")

    return file_path


def write_raw_record(
    *,
    results_root: Path,
    run_id: str,
    payload: dict[str, Any],
) -> Path:
    return _write_new_json(
        results_root / "raw" / f"{run_id}.json",
        payload,
    )


def write_validated_record(
    *,
    results_root: Path,
    run_id: str,
    payload: dict[str, Any],
) -> Path:
    return _write_new_json(
        results_root / "validated" / f"{run_id}.json",
        payload,
    )
