# Week8 Day6 Prompt v0.4.2 Micro-Regression Audit

- Stage: Week8 Day6 Revised Micro-Correction
- Prompt: v0.4.2 (frozen before MR08)
- Schema: v0.2 (unchanged)
- Attempts: 3 (MR08, MR15, MR28)
- Retry: 0
- Overall gate: **NOT READY FOR DAY7 DRIFT**

## Summary

| Report | Validation | Selectivity | Direction | Unselected recall | Attribution | Unsupported taxonomy | Formal choice | Machine language | Human language | Rating |
|---|---|---|---|---|---|---|---|---|---|---|
| MR08 | PASS | PASS | QUESTIONABLE | N/A | PASS | QUESTIONABLE | PASS | PASS | PASS | questionable |
| MR15 | PASS | PASS | QUESTIONABLE | PASS (Outcome02) | PASS | PASS | PASS | PASS | PASS | acceptable |
| MR28 | PASS | PASS (empty adversarial mapping) | N/A | N/A | PASS (empty) | PASS (empty) | PASS | WARNING | FAIL | questionable |

## Required field notes

### MR08

- Observed roles: `DoorKnife.Action01=core_support`, `DoorKnife.Outcome01=core_support`, `DoorKnife.Personality01=context`.
- `Action01` is a directional regression for a benevolent formal conclusion; it should normally be `counterevidence`.
- Two minor unsupported assumptions were flagged again: long-term coercion and killing as protective action.
- Visible judge response and archive summary passed machine and human world-language review.

### MR15

- Observed roles: `Medicine.Action01=core_support`, `Medicine.Outcome01=core_support`, `Medicine.Personality01=core_support`, `Medicine.Relationship01=core_support`, `Medicine.Outcome02=counterevidence`, `Medicine.Thought01=context`.
- `Medicine.Outcome02` was recalled from the semantic paraphrase of the resource-cost consequence although it was not in `SelectedKeyFragmentIDs`.
- Selectivity and unsupported-assumption checks pass; Action01 and Personality01 remain directional caveats.

### MR28

- `recognized_action_claims=[]`, `recognized_motive_claims=[]`, `fragment_roles=[]`, `unsupported_assumptions=[]`.
- Formal choice `beyond_redemption/soul_dissolution` is preserved.
- `judge_response` is world-safe, but `archive_summary` contains `????`; machine audit is WARNING and human audit is FAIL.

## Gate conclusion

- Rating distribution: `correct=0`, `acceptable=1`, `questionable=2`, `incorrect=0`, `not_auditable=0`.
- Hard gate fails on MR08 role direction and MR28 visible-language isolation.
- This is a semantic-model follow-up issue; no retry or prompt drift is permitted in this run.
