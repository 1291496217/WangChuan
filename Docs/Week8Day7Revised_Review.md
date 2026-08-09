# Week8 Day7 Revised — Independent Review

## Verdict

```text
Day7 controlled 5x experiment:
PASS

Five independent provider responses:
PASS

Same request-message SHA-256:
5 / 5

Raw / Validated pairing:
5 / 5 PASS

Formal Moral / Disposition:
5 / 5 PASS

Pre/Post frozen hashes:
7 / 7 MATCH

Fragment Set:
5 / 5 identical

Fragment Roles:
5 / 5 identical and directionally correct

Outcome02 used-but-not-selected recall:
5 / 5

Thought01 unexpected mapping:
0 / 5

Unsupported:
0 / 5

Machine World-language:
5 / 5 PASS

Human World-language:
5 / 5 PASS

Day7 MR15 Drift:
PASS — acceptable minor variation

Original Analyzer v0.1:
NEEDS CORRECTION

Original Week8 Full Audit:
TOO OPTIMISTIC IN SOME FINAL-CANDIDATE COVERAGE CLAIMS

Final Decision Recommendation:
B. READY WITH CONDITIONS
```

## What is fully verified

The uploaded package contains five distinct provider `response_id` values and repeat indices 1–5. All five use the identical request-message SHA-256:

```text
3422b8def28a3be98e71bce7bb6f12b6814165820cfa8286770564ee38fed1bc
```

All five Raw `parsed_payload` objects exactly equal their corresponding Validated `judgement_result`.

The pre/post control files show identical hashes for:

```text
Prompt v0.4.6
Schema v0.2
Runtime Contract v0.2
Language Audit v0.3
Case.Medicine.001
Judge.Clerk.001
MR15
```

Therefore the five calls form a valid same-input drift experiment.

## Independent usage verification

```text
Prompt Tokens:
22,675

Completion Tokens:
3,057

Total Tokens:
25,732

Prompt Cache Hit:
22,400

Prompt Cache Miss:
275

Latency:
average 6,678.8 ms
median 6,411 ms
min 5,895 ms
max 7,883 ms
```

These match the package.

## Real semantic result

Every repeat returns the same required five-fragment role map:

```text
Medicine.Action01       -> counterevidence
Medicine.Outcome01      -> core_support
Medicine.Outcome02      -> counterevidence
Medicine.Personality01  -> context
Medicine.Relationship01 -> core_support
```

`Medicine.Thought01` is absent 5/5.

No Unsupported assumption is produced.

The load-bearing semantic interpretation is therefore highly stable.

The actual minor variation is:

```text
thought_action_distinction:
partial / clear

moral_reasoning_tier:
adequate / strong

surface Judge wording
```

No material drift event was found.

## Analyzer v0.1 bug

The uploaded deterministic analyzer incorrectly treats `Medicine.Thought01` as part of the expected Fragment set and simultaneously treats its inclusion as undesirable.

As a result, all five correct outputs receive:

```text
missing_fragment_ids:
Medicine.Thought01

minor_drift_reason:
fragment_coverage_varies
```

This is a reporting bug, not a model failure.

The analyzer's role-direction check also compares against an allowed-role set instead of checking the actual expected role.

Use the supplied `analyze_semantic_drift_v0_2.py` or equivalent correction before treating automatic Day7 drift statistics as canonical.

## Stable taxonomy issue

There is one real systematic semantic defect:

```text
recognized_action_claims
```

contains Personality / Relationship context in all five runs, even though Prompt v0.4.6 says that field should contain actions or results only.

This is stable rather than stochastic, so it is not Drift.

However, deterministic scoring should not consume this field until the taxonomy is corrected or filtered.

## Thought/action enum issue

MR15 does not actually use or map Thought01, yet the required enum varies:

```text
clear 2/5
partial 3/5
```

Before a deterministic score consumes this enum, define how it behaves when no Thought evidence is substantively used. Otherwise scoring could turn an under-specified audit enum into artificial point variance.

## Week8 Full Audit coverage correction

The original full audit overstates several final-candidate claims.

Prompt v0.4.6's final real-call evidence is strong for the final MR08/MR15/MR28 regression and the Day7 MR15 drift.

However:

- deliberate factual-invention splitting (MR13/MR25 style) was proven on earlier Prompt candidates, not re-run on final v0.4.6;
- complete good/mixed/evil route coverage was not re-run under v0.4.6;
- strong-vs-weak report differentiation under final v0.4.6 is not established by five repeats of the same strong MR15 report.

Therefore these capabilities should be marked **CONDITIONAL** for the final candidate rather than silently promoted to PASS from historical evidence.

## Decision

The high-level recommendation remains:

```text
B. READY WITH CONDITIONS
```

This means:

> It is reasonable to begin deterministic scoring **design**, while keeping the scoring core program-owned and refusing to treat every AI semantic field as score-safe yet.

Before deterministic scoring implementation depends on the unresolved fields, perform a small final-candidate coverage regression for factual Unsupported true positives and multi-route reasoning.

No additional Day7 Drift calls are needed.
