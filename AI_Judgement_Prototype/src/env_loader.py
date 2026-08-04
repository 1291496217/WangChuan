from __future__ import annotations

import os
from pathlib import Path


class EnvironmentConfigurationError(ValueError):
    """Raised when a required local configuration value is missing or invalid."""


def load_env_file(file_path: Path) -> None:
    """
    Load a simple KEY=VALUE .env file into os.environ.

    Existing environment variables are not overwritten. This loader supports
    the simple values required by the Week8 prototype; it intentionally does
    not implement the full dotenv specification.
    """
    if not file_path.is_file():
        return

    for line_number, raw_line in enumerate(
        file_path.read_text(encoding="utf-8-sig").splitlines(),
        start=1,
    ):
        line = raw_line.strip()

        if not line or line.startswith("#"):
            continue

        if "=" not in line:
            raise EnvironmentConfigurationError(
                f"{file_path}:{line_number} must use KEY=VALUE format."
            )

        key, value = line.split("=", 1)
        key = key.strip()
        value = value.strip()

        if not key:
            raise EnvironmentConfigurationError(
                f"{file_path}:{line_number} contains an empty key."
            )

        if (
            len(value) >= 2
            and value[0] == value[-1]
            and value[0] in {"'", '"'}
        ):
            value = value[1:-1]

        os.environ.setdefault(key, value)


def require_environment_value(name: str) -> str:
    """Return a required non-empty environment value."""
    value = os.environ.get(name, "").strip()

    if not value:
        raise EnvironmentConfigurationError(
            f"Required environment variable {name!r} is missing or empty."
        )

    return value


def read_int_environment_value(
    name: str,
    *,
    default: int,
    minimum: int,
    maximum: int,
) -> int:
    """Read and range-check an integer environment value."""
    raw_value = os.environ.get(name)

    if raw_value is None or not raw_value.strip():
        return default

    try:
        value = int(raw_value)
    except ValueError as error:
        raise EnvironmentConfigurationError(
            f"{name} must be an integer."
        ) from error

    if value < minimum or value > maximum:
        raise EnvironmentConfigurationError(
            f"{name} must be between {minimum} and {maximum}."
        )

    return value


def read_float_environment_value(
    name: str,
    *,
    default: float,
    minimum: float,
    maximum: float,
) -> float:
    """Read and range-check a floating-point environment value."""
    raw_value = os.environ.get(name)

    if raw_value is None or not raw_value.strip():
        return default

    try:
        value = float(raw_value)
    except ValueError as error:
        raise EnvironmentConfigurationError(
            f"{name} must be a number."
        ) from error

    if value < minimum or value > maximum:
        raise EnvironmentConfigurationError(
            f"{name} must be between {minimum} and {maximum}."
        )

    return value
