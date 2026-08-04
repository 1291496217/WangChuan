from __future__ import annotations

from dataclasses import dataclass
from typing import Any

from openai import (
    APIConnectionError,
    APIStatusError,
    APITimeoutError,
    AuthenticationError,
    BadRequestError,
    OpenAI,
    OpenAIError,
    RateLimitError,
)


class AIClientError(RuntimeError):
    """Base class for a controlled AI provider failure."""


class AIAuthenticationError(AIClientError):
    pass


class AIInsufficientBalanceError(AIClientError):
    pass


class AIRateLimitError(AIClientError):
    pass


class AIConnectionFailure(AIClientError):
    pass


class AIInvalidRequestError(AIClientError):
    pass


class AIProviderFailure(AIClientError):
    pass


class AIResponseError(AIClientError):
    pass


@dataclass(frozen=True)
class DeepSeekClientConfig:
    api_key: str
    base_url: str
    model: str
    timeout_seconds: float
    max_tokens: int
    thinking_mode: str
    temperature: float

    def __post_init__(self) -> None:
        if not self.api_key.strip():
            raise ValueError("DeepSeek API key cannot be empty.")

        if self.thinking_mode not in {"enabled", "disabled"}:
            raise ValueError(
                "thinking_mode must be 'enabled' or 'disabled'."
            )

        if self.max_tokens <= 0:
            raise ValueError("max_tokens must be positive.")

        if self.timeout_seconds <= 0:
            raise ValueError("timeout_seconds must be positive.")


@dataclass(frozen=True)
class DeepSeekResponse:
    response_id: str
    requested_model: str
    returned_model: str
    finish_reason: str
    content: str
    system_fingerprint: str | None
    usage: dict[str, int | None]


def _read_usage(response: Any) -> dict[str, int | None]:
    usage = getattr(response, "usage", None)

    if usage is None:
        return {
            "prompt_tokens": None,
            "completion_tokens": None,
            "total_tokens": None,
            "prompt_cache_hit_tokens": None,
            "prompt_cache_miss_tokens": None,
            "reasoning_tokens": None,
        }

    completion_details = getattr(
        usage,
        "completion_tokens_details",
        None,
    )

    return {
        "prompt_tokens": getattr(usage, "prompt_tokens", None),
        "completion_tokens": getattr(
            usage,
            "completion_tokens",
            None,
        ),
        "total_tokens": getattr(usage, "total_tokens", None),
        "prompt_cache_hit_tokens": getattr(
            usage,
            "prompt_cache_hit_tokens",
            None,
        ),
        "prompt_cache_miss_tokens": getattr(
            usage,
            "prompt_cache_miss_tokens",
            None,
        ),
        "reasoning_tokens": getattr(
            completion_details,
            "reasoning_tokens",
            None,
        ),
    }


class DeepSeekAIClient:
    """One-call DeepSeek Chat Completions client for Week8."""

    def __init__(self, config: DeepSeekClientConfig) -> None:
        self._config = config
        self._client = OpenAI(
            api_key=config.api_key,
            base_url=config.base_url,
            timeout=config.timeout_seconds,
            max_retries=0,
        )

    def create_judgement(
        self,
        messages: list[dict[str, str]],
    ) -> DeepSeekResponse:
        """
        Submit exactly one non-streaming request.

        No automatic retry is used so one experiment run corresponds to one
        billed model call and one reproducible provider response.
        """
        request_arguments: dict[str, Any] = {
            "model": self._config.model,
            "messages": messages,
            "stream": False,
            "max_tokens": self._config.max_tokens,
            "response_format": {
                "type": "json_object",
            },
            "extra_body": {
                "thinking": {
                    "type": self._config.thinking_mode,
                }
            },
        }

        if self._config.thinking_mode == "disabled":
            request_arguments["temperature"] = (
                self._config.temperature
            )

        try:
            response = self._client.chat.completions.create(
                **request_arguments
            )
        except AuthenticationError as error:
            raise AIAuthenticationError(
                "DeepSeek authentication failed. Check the local API key."
            ) from error
        except RateLimitError as error:
            raise AIRateLimitError(
                "DeepSeek rate limit was reached."
            ) from error
        except APITimeoutError as error:
            raise AIConnectionFailure(
                "DeepSeek request timed out."
            ) from error
        except APIConnectionError as error:
            raise AIConnectionFailure(
                "Could not connect to the DeepSeek API."
            ) from error
        except BadRequestError as error:
            raise AIInvalidRequestError(
                f"DeepSeek rejected the request: {error}"
            ) from error
        except APIStatusError as error:
            if error.status_code == 402:
                raise AIInsufficientBalanceError(
                    "DeepSeek account balance is insufficient."
                ) from error

            raise AIProviderFailure(
                f"DeepSeek returned HTTP {error.status_code}: {error}"
            ) from error
        except OpenAIError as error:
            raise AIProviderFailure(
                f"DeepSeek SDK request failed: {error}"
            ) from error

        if not response.choices:
            raise AIResponseError(
                "DeepSeek returned no completion choices."
            )

        choice = response.choices[0]
        finish_reason = choice.finish_reason or "unknown"

        if finish_reason != "stop":
            raise AIResponseError(
                "DeepSeek did not finish normally. "
                f"finish_reason={finish_reason!r}"
            )

        content = choice.message.content

        if content is None or not content.strip():
            raise AIResponseError(
                "DeepSeek returned empty JSON content."
            )

        return DeepSeekResponse(
            response_id=response.id,
            requested_model=self._config.model,
            returned_model=response.model,
            finish_reason=finish_reason,
            content=content,
            system_fingerprint=getattr(
                response,
                "system_fingerprint",
                None,
            ),
            usage=_read_usage(response),
        )
