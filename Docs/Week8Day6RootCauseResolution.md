# Week8 Day6 Root-Cause Resolution

## Outcome

Prompt v0.4.6 passes the focused MR08/MR15/MR28 regression and is **READY FOR DAY7 DRIFT**. Schema v0.2 and Runtime Contract v0.2 remain unchanged.

## What was actually wrong

- The v0.4.2 builder embedded untrusted player text in the system message.
- Role prose did not force a conclusion-relative classification sequence.
- Selected Keys alternated between acting as a whitelist and, when removed, allowing full-case expansion.
- Qualified, evidence-linked possibilities were not excluded strongly enough from unsupported facts.
- Language warnings did not prevent unsafe output from entering validated results.

## Implemented solution

- Moved the complete player report into a separate `user` message.
- Added explicit harmful-action/cost → `counterevidence` rules for lenient conclusions.
- Fixed Personality evidence to `context` when used.
- Required new unsupported facts to be definite assertions without Fragment anchors.
- Balanced Selected Keys as candidate hints with clause-anchored unselected recall.
- Added positive and negative examples that include MR15 Outcome02 while excluding unused Thought/Relationship/Death details.
- Added strict world-language audit v0.3.
- Added a publication gate: WARNING results remain raw and cannot enter validated.

## Final results

- MR08: exact three expected roles; unsupported empty; language PASS.
- MR15: Outcome02 recalled, Thought01 omitted, directions correct; unsupported empty; language PASS.
- MR28: empty claims/roles/unsupported, formal choice preserved, five safety flags preserved; machine and human language PASS.
- Full tests: `Ran 319 tests`, `OK`.

## Remaining non-blocking observations

- MR08 Archive Summary contains an unnecessary assurance that no override or secret-seeking occurred.
- MR15 classifies the three-year integrity record inside action claims; it would be cleaner as personality/context only.

These do not affect the corrected gates, but Day7 Drift should track whether they recur.

No commit or push was performed. Existing Unreal worktree changes were preserved. API Key Never Exposed.
