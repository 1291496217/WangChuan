"""Generate a simple, clean, game-ready flame ignition sound."""

from __future__ import annotations

import math
import random
import struct
import wave
from pathlib import Path


SAMPLE_RATE = 48_000
DURATION_SECONDS = 1.5
SEED = 0x5743
OUTPUT = (
    Path(__file__).resolve().parents[2]
    / "Content"
    / "WangChuan"
    / "Audio"
    / "Source"
    / "SFX_NetherLantern_Ignite.wav"
)


def smoothstep(edge0: float, edge1: float, value: float) -> float:
    if edge0 == edge1:
        return float(value >= edge1)
    x = max(0.0, min(1.0, (value - edge0) / (edge1 - edge0)))
    return x * x * (3.0 - 2.0 * x)


def exp_decay(value: float, start: float, decay: float) -> float:
    return 0.0 if value < start else math.exp(-(value - start) / decay)


def write_pcm24(path: Path, samples: list[float]) -> None:
    peak = max(abs(sample) for sample in samples)
    target_peak = 10.0 ** (-3.0 / 20.0)
    gain = target_peak / max(peak, 1.0e-9)
    max_int = (1 << 23) - 1

    path.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(path), "wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(3)
        wav.setframerate(SAMPLE_RATE)

        frames = bytearray()
        for sample in samples:
            integer = max(-max_int, min(max_int, round(sample * gain * max_int)))
            if integer < 0:
                integer += 1 << 24
            frames.extend(
                (integer & 0xFF, (integer >> 8) & 0xFF, (integer >> 16) & 0xFF)
            )
        wav.writeframes(frames)


def generate() -> list[float]:
    rng = random.Random(SEED)
    frame_count = round(SAMPLE_RATE * DURATION_SECONDS)
    output = [0.0] * frame_count

    # Two gentle one-pole filters form a restrained low-mid "foof" without hiss.
    soft_noise = 0.0
    body_noise = 0.0
    tail_noise = 0.0
    ignition_phase = 0.0

    for index in range(frame_count):
        t = index / SAMPLE_RATE
        white = rng.uniform(-1.0, 1.0)
        soft_noise += 0.085 * (white - soft_noise)
        body_noise += 0.018 * (white - body_noise)
        tail_noise += 0.008 * (white - tail_noise)
        warm_air = soft_noise - body_noise

        # One concise ignition gesture: quick swell, soft catch, short decay.
        whoosh_env = (
            smoothstep(0.03, 0.16, t)
            * exp_decay(t, 0.16, 0.27)
            * (1.0 - smoothstep(0.72, 1.05, t))
        )

        # A quiet low pulse gives the ignition a readable start without a "boom".
        ignition_age = t - 0.135
        ignition = 0.0
        if ignition_age >= 0.0:
            ignition_frequency = 105.0 - 28.0 * smoothstep(0.0, 0.24, ignition_age)
            ignition_phase += math.tau * ignition_frequency / SAMPLE_RATE
            ignition = (
                math.sin(ignition_phase)
                * math.exp(-ignition_age / 0.12)
                * 0.075
            )

        # Barely audible flame body confirms that the fire has caught.
        flame_tail_env = (
            smoothstep(0.14, 0.25, t)
            * exp_decay(t, 0.25, 0.42)
            * (1.0 - smoothstep(1.10, 1.48, t))
        )
        sample = warm_air * 0.62 * whoosh_env
        sample += tail_noise * 0.14 * flame_tail_env
        sample += ignition

        # Click-free file boundaries.
        boundary_fade = smoothstep(0.0, 0.008, t) * (
            1.0 - smoothstep(1.40, DURATION_SECONDS, t)
        )
        output[index] = sample * boundary_fade

    return output


if __name__ == "__main__":
    write_pcm24(OUTPUT, generate())
    print(OUTPUT)
