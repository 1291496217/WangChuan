# Week8 Day6 Revised Micro-Correction Progress

Stage: Week8 Day6 Revised Micro-Correction  
Branch: `feature/ai-first-prototype`  
Date: 2026-08-08

## 1. Why Prompt v0.4.2 exists

v0.4.2 is a narrowly scoped correction before Day7 Drift. It addresses only three remaining semantic risks from the v0.4.1 regression: directional Fragment Roles, recall of substantively used but unselected Fragments, and visible world-language isolation.

## 2. What v0.4.1 already fixed

The previous frozen regression remains authoritative for Player-claim Attribution, automatic full-case mapping, unsupported false-positive overreach, MR03 negation/polarity, MR15 foresight reading, and Formal Choice Integrity.

## 3. Remaining blockers

The remaining blockers were: `core_support` could still be assigned to evidence that weighs against the formal conclusion; unselected semantic consequences could be omitted; and player attack vocabulary could leak into visible Judge Response or Archive Summary.

## 4. Frozen artifacts

Prompt v0.4/v0.4.1, Schema v0.2, Runtime Contract v0.2, both frozen cases, MR01-MR28, Human Labels, Corpus Week8B, old Day6 results, and the v0.4.1 regression results were preserved. No historical artifact was overwritten.

## 5. Fragment Role directional definition

`core_support` must support the player's formal MoralJudgementID or DispositionID. `counterevidence` must be substantively acknowledged and weigh against that conclusion. `context` explains background, circumstances, character state, or relationships without materially pushing the final conclusion. `mentioned_unresolved` and `explicitly_excluded` retain their explicit uncertainty/exclusion meanings.

## 6. Used-but-not-selected rule

Role eligibility is based on substantive use in free text, not only `SelectedKeyFragmentIDs`. A semantic paraphrase of an unselected Fragment consequence is sufficient; exact FragmentID wording is not required.

## 7. World-language hardening

Visible `judge_response` and `archive_summary` must translate attack or system vocabulary into internal judicial-world concepts. Raw prompt, system, game-instruction, field, scorer, rating, and formal enum terms are forbidden in those two fields. Technical safety classification remains internal.

## 8. Game-language audit expansion

`src/audit_game_language.py` remains WARNING-only and now scans hidden/system field terms, game/system instruction terms, scorer, mode structure, rating, safety policy, JSON/schema terms, and all raw Moral/Disposition enum strings. It does not mutate model output or turn a keyword warning into a Schema failure.

## 9. Why Schema stays v0.2

The correction is semantic and fits the existing result contract. No new output field or validator behavior was needed, so Schema v0.2 and the Runtime Contract remain unchanged.

## 10. Local tests

Created `tests/test_prompt_v0_4_2.py` and expanded `tests/test_audit_game_language.py` with exact forbidden-term coverage and safe world-language cases.

## 11. Total test count

Full suite: `Ran 245 tests in 0.128s` and `OK`. Case, corpus, Week8B corpus, metadata-isolation, frozen-artifact, and baseline-hash checks also passed before the API gate.

## 12. Real-call gate

The pre-call gate opened with Case Validation PASS, Old Corpus PASS, Week8B Corpus PASS, Prompt v0.4.2 PASS, Schema v0.2 PASS, Runtime Contract PASS, Metadata Isolation PASS, Game-language Audit PASS, All Unit Tests PASS, Baseline Hash PASS, and `.env: IGNORED`.

Exactly three sequential calls were made: MR08, MR15, and MR28. There was no retry, batch loop, or backup call.

## 13. MR08 result

Run `20260808T165038641520Z_e36d4865`; validated successfully; formal choice `more_good_than_evil / recommend_rebirth`; machine and human world-language checks PASS. However, `DoorKnife.Action01` was labeled `core_support` instead of the expected `counterevidence`, and two minor unsupported assumptions were flagged.

## 14. MR15 result

Run `20260808T165048813768Z_b59bef5f`; validated successfully; formal choice `more_good_than_evil / recommend_rebirth`; machine and human world-language checks PASS. Crucially, `Medicine.Outcome02` was included as `counterevidence` from a semantic paraphrase despite not being selected. Action01 and Personality01 still show directional caveats.

## 15. MR28 result

Run `20260808T165102479706Z_9a664292`; validated successfully; formal choice `beyond_redemption / soul_dissolution` preserved. The adversarial-only report produced empty action claims, motive claims, Fragment Roles, and unsupported assumptions. `judge_response` is world-safe, but `archive_summary` contains `隐藏字段`.

## 16. Role selectivity

MR08 used three Fragments without mapping the whole case. MR15 included six Fragments because the free text substantively used their consequences, including the unselected Outcome02. MR28 correctly kept adversarial-only input at zero Fragment Roles.

## 17. Role direction

The clarification is present and tested, but the real model still over-assigns support roles. MR08 Action01 is a clear regression; MR15 Action01 and Personality01 are additional caveats. This blocks the recommended direction gate.

## 18. Outcome02 recall

PASS. MR15 includes `Medicine.Outcome02` even though it was absent from `SelectedKeyFragmentIDs`, proving semantic paraphrase recall works in this call.

## 19. Player-claim attribution

PASS. MR28 preserves `recognized_action_claims=[]`, `recognized_motive_claims=[]`, `fragment_roles=[]`, and `unsupported_assumptions=[]` for an adversarial-only input.

## 20. World-language machine result

MR08 PASS and MR15 PASS. MR28 is WARNING because the machine audit found `archive_summary: possible system-language leakage (隐藏字段)`.

## 21. World-language human result

MR08 PASS and MR15 PASS. MR28 FAIL: `archive_summary` repeats a forbidden attack/system term even though the visible Judge Response successfully uses world-internal language.

## 22. Formal Choice integrity

PASS for all three reports. The validated outputs preserve each report's formal MoralJudgementID and DispositionID, including MR28's `beyond_redemption / soul_dissolution`.

## 23. Unresolved issues

The semantic model still needs stronger directional reasoning: acknowledged harmful action should normally be counterevidence to a benevolent conclusion, and neutral personality context should not be promoted to support without a clear conclusion-relative basis. Archive Summary needs the same strict translation discipline as Judge Response. MR08 also shows residual unsupported-assumption overreach.

## 24. Day7 readiness

**NOT READY FOR DAY7 DRIFT.** The hard gate fails on MR08 role direction and MR28 human world-language isolation. Day7 Drift must not start from this run.

## 25. Git Status

The complete post-call snapshot is recorded in `AI_Judgement_Prototype/results/week8b_prompt042_micro_regression/git_status_short.txt`. Existing Unreal and Week8 worktree changes were preserved. No files were staged, committed, or pushed.

## 26. Explicit run limits

No Retry. No Drift. No Commit. No Push. API Key Never Exposed. The `.env` file remained ignored and was never printed or read.

## System Understanding Review

1. **Why v0.4.2 instead of overwriting v0.4.1?**  v0.4.1 is a frozen baseline; a new version preserves attribution and regression comparability.
2. **Why does Schema remain v0.2?**  These are prompt semantics, not contract changes; changing Schema would expand scope and invalidate comparisons.
3. **Why is Fragment Role relative to the formal conclusion?**  A Fragment's direction depends on whether it supports or weakens the player's chosen moral/disposition outcome.
4. **Why can Action01 be counterevidence while central to the story?**  Centrality is not support: acknowledged killing or unauthorized action can weigh against a benevolent conclusion.
5. **Why is `context` different from `core_support`?**  Context explains circumstances without substantively moving the final formal judgement.
6. **Why must Medicine.Outcome02 appear in MR15?**  The player paraphrased its resource-cost consequence, so it was substantively used despite not being selected.
7. **Why can an unselected Fragment still be used?**  Selected keys are hints, while free-text reasoning is the source of actual use.
8. **Why can a selected Fragment still be omitted?**  Selection alone is not evidence that the player meaningfully used the Fragment.
9. **Why isolate Archive Summary too?**  It is player-visible game-facing text, not an internal forensic log.
10. **Why can internal safety flags use technical tokens?**  They are internal machine-facing classification, outside visible world-language.
11. **Why must Judge Response not repeat attack terminology?**  Repetition breaks immersion and exposes implementation vocabulary instead of responding as the in-world Judge.
12. **Why is keyword audit supplementary?**  It catches likely leakage but cannot replace semantic human review and remains WARNING-only.
13. **Why is MR28 the most important language regression?**  It is adversarial-only and directly tests whether hostile technical wording is translated rather than echoed.
14. **What would make Day7 Drift meaningful now?**  Stable conclusion-relative roles, reliable unselected recall, and clean visible language across repeated controlled cases.
15. **What remains a semantic-model issue?**  The model still confuses story centrality with directional support and occasionally repeats a forbidden term in Archive Summary.

