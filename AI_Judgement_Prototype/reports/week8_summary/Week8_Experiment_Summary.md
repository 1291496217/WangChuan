# Week8 Experiment Summary

Generated (UTC): `2026-08-04T23:48:41.434798+00:00`

## Executive Summary

- Saved Week8 real calls: **21**; validated: **21 (100.00%)**.
- Prompt v0.3 corpus: **19** saved calls; validated: **19 (100.00%)**.
- Semantic audit: Correct 11, Acceptable 1, Questionable 7, Incorrect 0, Not Auditable 0.
- Directly acceptable semantic results: **12/19 (63.16%)**.
- Core hypothesis: **Promising**, not Proven.

## Run Inventory

| Experiment layer | Calls |
|---|---:|
| Prompt v0.1 baseline | 1 |
| Prompt v0.2 A/B | 1 |
| Prompt v0.3 corpus | 19 |
| Unclassified | 0 |

## Token, Latency and Cost

Pricing reference date: **2026-08-04**. Provider prices may change.

| Scope | Calls | Validated | Prompt tokens | Completion tokens | Total tokens | Total-token avg / median / min / max | Latency avg / median / min / max (ms) | USD | CNY |
|---|---:|---:|---:|---:|---:|---|---|---:|---:|
| All Week8 | 21 | 21 (100.00%) | 70,011 | 13,023 | 83,034 | 3954 / 3929 / 3733 / 4161 | 7939.81 / 7940 / 6031 / 10222 | $0.00642334 | CNY 0.04588100 |
| Prompt v0.3 corpus | 19 | 19 (100.00%) | 63,581 | 11,667 | 75,248 | 3960.42 / 3929 / 3801 / 4161 | 8027.63 / 8007 / 6031 / 10222 | $0.00514346 | CNY 0.03673900 |

- All Week8 cost method: `cache_aware_exact_from_recorded_split`; average cost/run: $0.00030587 / CNY 0.00218481.
- v0.3 cost method: `cache_aware_exact_from_recorded_split`; average cost/run: $0.00027071 / CNY 0.00193363.

## Failure Summary

| Scope | Provider failures | JSON parse failures | Validation failures | Incomplete pairs |
|---|---:|---:|---:|---:|
| All Week8 | 0 | 0 | 0 | 0 |
| Prompt v0.3 corpus | 0 | 0 | 0 | 0 |

## Prompt v0.3 Structured Output Distribution

- Recognized dispositions: `{"detain_for_review": 8, "dissolve": 4, "reincarnate": 7}`
- Contradiction handling: `{"acknowledged": 11, "ignored": 3, "integrated": 1, "minimized": 4}`
- Used Fragment count: `{"0": 1, "2": 1, "3": 2, "4": 5, "5": 10}`
- Unsupported assumption count: `{"0": 12, "1": 2, "3": 3, "4": 2}`
- Dimension ratings:
  - `narrative_coherence`: `{"adequate": 13, "strong": 1, "weak": 5}`
  - `evidence_grounding`: `{"adequate": 9, "strong": 2, "weak": 8}`
  - `rhetorical_effectiveness`: `{"adequate": 11, "strong": 4, "weak": 4}`
  - `disposition_alignment`: `{"adequate": 2, "strong": 9, "weak": 8}`
- Style tags: `{"bureaucratic": 1, "cautious": 10, "compassionate": 4, "contradiction_aware": 10, "evidence_driven": 4, "harsh": 3, "legalistic": 3, "rhetorical": 4, "sophistic": 4, "speculative": 4}`

## Semantic Audit

- Ratings: `{"acceptable": 1, "correct": 11, "incorrect": 0, "not_auditable": 0, "questionable": 7}`
- Core hypothesis: `promising`.
- Major successes: structured validation, disposition integrity, reasonable-possibility handling, rhetoric/evidence separation, and prompt-injection resistance.
- Major risks: Fragment Mapping instability, complex unsupported-claim splitting, game-language leakage, sparse evidence density, and detain-for-review safety bias.

## Week8 Completion

- Technical prototype: **Completed**.
- Semantic feasibility: **Promising**.
- Production readiness: **Not Ready**.
- Recording: intentionally deferred by the user because this is an experimental prototype; it is not a Push blocker.

## Warnings

- None.
