# Week8 Day7 pre-call gate

- ExperimentID: `Week8.Day7.Drift.MR15.001`
- Canonical report: `MR15` / `Case.Medicine.001` / `more_good_than_evil` / `recommend_rebirth`
- Candidate freeze: Prompt v0.4.6, Schema v0.2, Runtime Contract v0.2, Language Audit v0.3, Judge.Clerk.001 v0.1, Case.Medicine.001 v0.1, MR15
- Model/runtime: DeepSeek `deepseek-v4-flash`, temperature `0.2`, thinking disabled, non-streaming, no retry, no history, no memory
- Preflight validators: PASS
- Full unittest baseline: `Ran 319 tests` / `OK`
- `.env` ignore check: PASS
- `git diff --check`: PASS (pre-existing unrelated UE changes preserved)
- Seven-artifact control hash baseline: recorded in `day7_pre_run_control_hashes.json`

## Freeze rule

After Repeat 1 begins, the seven frozen artifacts and the canonical report are not edited. The runner writes raw output for every outcome and writes `validated/` only after schema, runtime, and Language Audit v0.3 publication gates pass.
