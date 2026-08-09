# Week8 Revised Decision Gate v0.2 — Review Corrected

## Recommendation

**B. READY WITH CONDITIONS**

Day7's canonical MR15 Drift itself passes: the frozen v0.4.6 candidate produced five publishable outputs with identical evidence sets and role directions, stable formal player authority, and no world-language failure.

The original high-level recommendation is therefore retained.

## Corrected Gate

| Gate | Status |
|---|---|
| Case Design | PASS |
| Report Contract | PASS |
| Prompt v0.4.6 | **CONDITIONAL** |
| Schema v0.2 | PASS |
| Formal Choice Integrity | PASS |
| Fragment Mapping | PASS |
| Personality / Action | **CONDITIONAL** |
| Thought / Action | **CONDITIONAL** |
| Unsupported | **CONDITIONAL** |
| World-language | PASS |
| Publication Gate | PASS |
| Semantic Drift | **PASS** |
| Cost | CONDITIONAL |
| Latency | PASS |
| Ready for Deterministic Scoring Design | **CONDITIONAL** |

## Why some gates were tightened

### Unsupported

The final v0.4.6 candidate was not directly re-run on the deliberate factual-invention cases that previously demonstrated independent Unsupported splitting.

Earlier Prompt evidence is useful development evidence, but it should not be silently treated as final-candidate proof after several substantive Prompt revisions.

Therefore:

```text
Unsupported:
CONDITIONAL
```

until a small v0.4.6 factual-invention regression is performed.

### Prompt v0.4.6 current-candidate coverage

Final v0.4.6 evidence is strong for:

```text
MR08
MR15
MR28
+
MR15 5x Drift
```

but it does not directly re-run all good/mixed/evil and strong/weak semantic routes under the final Prompt.

This does not block deterministic scoring **design**, but it prevents claiming that every semantic capability has already been fully validated on v0.4.6.

### Personality / Action

`recognized_action_claims` misclassifies Personality/Relationship context in all five MR15 repeats.

Stable repetition means this is not Drift, but the field should not be used as a deterministic scoring input yet.

### Thought / Action

Thought01 is correctly excluded from Fragment Mapping `5/5`, while `thought_action_distinction` still varies `clear/partial`.

Its semantics need clarification for reports that do not actually invoke Thought evidence.

## Scoring-design boundary

It is reasonable to begin discussing/building a **program-owned deterministic scoring interface**, provided the first design does not assume every current AI semantic field is already score-safe.

In particular, initially prefer stable program-owned inputs and audited fields:

```text
Formal MoralJudgementID
Formal DispositionID
SelectedKeyFragmentIDs
validated Fragment Roles
Unsupported results only after final-candidate true-positive regression
```

Do not make AI a hidden morality scorer.
