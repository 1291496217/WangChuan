# Day7 MR15 Semantic Drift Audit v0.2 — Review Corrected

## Result

```text
5 scheduled calls:
5

Validated:
5 / 5

Published:
5 / 5

Formal Choice Integrity:
5 / 5 PASS

Pre/Post Frozen Control Hashes:
7 / 7 MATCH

Machine World-language:
5 / 5 PASS

Human World-language:
5 / 5 PASS
```

## Corrected Drift Conclusion

**Overall: `acceptable_drift` with zero material drift events.**

The five runs preserve the same load-bearing semantics:

```text
Medicine.Action01       -> counterevidence   5/5
Medicine.Outcome01      -> core_support      5/5
Medicine.Outcome02      -> counterevidence   5/5
Medicine.Personality01  -> context           5/5
Medicine.Relationship01 -> core_support      5/5

Medicine.Thought01:
0/5 included
```

Fragment Set pairwise Jaccard is `1.0`. Unsupported assumptions remain empty `5/5`.

Variation is limited to:

```text
thought_action_distinction:
clear 2 / partial 3

moral_reasoning_tier:
strong 3 / adequate 2

Judge surface wording:
5 unique responses
```

This is minor semantic/emphasis variation rather than material evidence drift.

## Analyzer v0.1 Bug

The uploaded `analyze_semantic_drift_v0_1.py` contains a deterministic logic bug:

1. `EXPECTED_FRAGMENT_IDS = tuple(EXPECTED_ROLES)` included `Medicine.Thought01`.
2. The same Day7 design says Thought01 should normally remain absent in MR15.
3. Therefore every correct run was falsely assigned `missing_fragment_ids = ["Medicine.Thought01"]` and `fragment_coverage_varies`.
4. `role_direction_issues` checked only whether the role was one of `core_support/context/counterevidence`; it did not compare the actual role to the expected role for that Fragment.

The real outputs themselves are not affected. The bug is in local reporting.

## Stable Systematic Quality Finding

All five runs place Personality or Relationship context inside `recognized_action_claims`, despite Prompt v0.4.6 saying that field should contain only player-asserted actions/results.

This is **not drift** because it repeats consistently, but it is a real taxonomy defect.

Before deterministic scoring consumes `recognized_action_claims`, either:

- fix that taxonomy; or
- explicitly exclude this field from the first scoring design.

## Thought Field Semantics

MR15 does not substantively use `Medicine.Thought01`, and Fragment Mapping correctly excludes it 5/5.

Yet `thought_action_distinction` varies between `partial` and `clear`.

Before this enum influences deterministic scoring, define what it should mean when no Thought Fragment is actually used. `not_applicable` may be more appropriate than interpreting omission quality as `partial/clear`.

## Judge Style Observation

All five `judge_response` values are unique, but all begin with essentially the same `呈文所陈...` pattern.

This is not a Day7 blocker, but it is worth tracking if Judge personality is expected to feel less templated later.
