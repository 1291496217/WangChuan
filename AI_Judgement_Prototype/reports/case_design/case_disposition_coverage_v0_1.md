# Two-case Disposition Coverage Matrix v0.1

**Coverage ID:** CaseDispositionCoverage.Week8B.001
**Scope:** Case.DoorKnife.001 v0.1 + Case.Medicine.001 v0.1
**Design metadata only:** Yes
**Runtime visibility:** Do not expose to AI or player runtime.

This matrix is a Case Design Audit Asset. It is not a hidden answer, AI Prompt, player hint, automatic disposition rule, Reward Table, or runtime evidence.

## Coverage Table

| Case | Disposition | Support Fragments | Required Inferences | Counterevidence | Minimum Conditions | Mindless Risk | Corpus Recommendation |
|---|---|---|---|---|---|---|---|
| DoorKnife | recommend_rebirth | Outcome01, Relationship01, Personality01 | Protective result/promise mitigates but does not erase killing | Action01, Thought01 | Admit killing and death imagery; do not make Death01 a sacrifice | medium | include |
| DoorKnife | ordinary_transfer | Action01, Thought01, Outcome01, Relationship01 | Responsibility and protection coexist | Death01, Personality01 | Explain substantive merit/fault conflict | high | include |
| DoorKnife | send_to_prison | Action01, Thought01 | Lethal act and conflicted thought support responsibility | Outcome01, Relationship01, Death01 | Address killing and counterevidence | medium | include |
| DoorKnife | soul_dissolution | none | None required; boundary test only | Outcome01, Relationship01, Death01 | No normal support condition | high | boundary_only |
| Medicine | recommend_rebirth | Outcome01, Personality01, Relationship01 | Non-family care, prior integrity and real outcome mitigate | Action01, Thought01, Outcome02 | Admit unauthorized taking, scarcity and third-party cost | medium | include |
| Medicine | ordinary_transfer | Action01, Outcome01, Outcome02, Thought01, Relationship01 | Good intent/result and rule/resource responsibility coexist | Personality01 | Explain why merit and fault are both material | high | include |
| Medicine | send_to_prison | Action01, Thought01, Outcome02 | Knowing scarcity and taking without authorization creates responsibility | Outcome01, Relationship01, Personality01 | Address benefit; do not claim intentional harm | medium | include |
| Medicine | soul_dissolution | none | None required; boundary test only | Outcome01, Personality01, Relationship01 | No normal support condition | high | boundary_only |

## Main Findings

- **More balanced toward ordinary transfer:** Medicine. It has a direct beneficial outcome and a direct third-party cost, while the unauthorized action and risk awareness remain material.
- **Stronger rebirth route:** Medicine has a non-family care promise, prior integrity testimony and a real child-treatment outcome. DoorKnife's rebirth route must still overcome a confirmed killing.
- **Stronger prison route:** DoorKnife has a confirmed lethal action; Medicine has a clear prison route through informed scarcity, trust violation and third-party cost, but without malicious intent.
- **Soul dissolution:** neither Case needs average or normal support for soul dissolution. It remains an extreme boundary test.
- **Why this is not the answer:** the Matrix records admissible support routes and audit conditions. It does not select a correct moral judgement or disposition.
- **Why ordinary transfer is not “I am uncertain”:** each ordinary-transfer route must explain how merit and fault coexist and why neither side erases the other.

## Runtime Boundary

The following remain design/audit metadata only:

- interpretation_hooks
- disposition_support_tags
- information_weight
- relation_tags

Do not expose them to AI or player runtime. They must not become prompt evidence, reward values, automatic scores or automatic dispositions.

## Case Contrast

DoorKnife tests explicit lethal responsibility against a protective result. Medicine tests a non-killing resource decision where good intent and real good outcome coexist with unauthorized taking, known scarcity and a real third-party cost. The shared structure proves reuse; the different tensions prove the second Case is not a renamed DoorKnife.
