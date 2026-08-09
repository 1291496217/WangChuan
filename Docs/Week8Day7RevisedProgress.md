# 《忘川河畔：见习判官》Week8 Day7 Revised Progress

**Completed:** 2026-08-09
**Branch:** `feature/ai-first-prototype`
**Experiment:** `Week8.Day7.Drift.MR15.001`
**Final status:** COMPLETED — Decision Gate B (ready with conditions)
**Commit / Push:** Not performed

## 1. Day7 Goal

Measure canonical MR15 semantic drift under the frozen Prompt v0.4.6 candidate, complete the Week8B audit, and decide whether deterministic scoring design may begin. No scoring implementation is part of Day7.

## 2. Frozen candidate and why MR15

Prompt v0.4.6, Schema v0.2, Runtime Contract v0.2, Language Audit v0.3, Judge.Clerk.001 v0.1, Case.Medicine.001 v0.1 and MR15 were frozen before Repeat 1. MR15 is canonical because its fixed formal choice is `more_good_than_evil` / `recommend_rebirth`, while its evidence includes wrongdoing, real benefit, Personality context, Relationship context and a used-but-not-selected Outcome02 resource-cost counterpoint.

## 3. Control variables and local gate

DeepSeek `deepseek-v4-flash`; temperature 0.2; thinking disabled; stream false; one fresh process per repeat; no history, memory or automatic retry. Case validator, both corpus validators, 319-test suite, `.env` ignore check and `git diff --check` passed before calls. Seven-artifact hashes are in `results/week8b_day7_drift_mr15_v0_1/day7_pre_run_control_hashes.json`.

## 4. Calls and raw/validated accounting

- Repeat 1: `20260809T151629404116Z_5e37123b`
- Repeat 2: `20260809T151640260016Z_0777a120`
- Repeat 3: `20260809T151652137996Z_8257c248`
- Repeat 4: `20260809T151702933671Z_6e8f219b`
- Repeat 5: `20260809T151712312505Z_7b549a24`
- API attempts: 5; retries: 0.
- Raw envelopes: 5; validated envelopes: 5; non-validated raw failures: 0.
- Result isolation: `results/week8b_day7_drift_mr15_v0_1/raw|validated/`.

## 5. Usage, latency and cost

- Prompt tokens: total 22,675; average 4,535; cache hit total 22,400; cache miss total 275.
- Completion tokens: total 3,057; average 611.4.
- Total tokens: total 25,732; average 5,146.4; median 5,110; min 5,106; max 5,236.
- Latency: average 6,678.8 ms; median 6,411 ms; min 5,895 ms; max 7,883 ms; sum 33,394 ms.
- Cost: no pricing configuration was available; no pricing was invented. Tokens and latency are recorded only.

## 6. Semantic stability and human audit

- Structural validation/publication: 5/5 PASS; formal choice: 5/5 preserved.
- Fragment Set: identical unordered five-fragment set; pairwise Jaccard average 1.0.
- Fragment Roles: stable directions 5/5; Outcome02 inclusion 5/5; Thought01 unjustified inclusion 0/5; six-fragment expansion 0/5.
- Used-but-not-selected stability: Outcome02 recalled 5/5 from player clauses.
- Unsupported: empty 5/5; no severity distribution.
- Personality: plausible/contextual 5/5.
- Thought: clear 2/5, partial 3/5; no Thought01 mapping. This is a conditional emphasis variation, not a material evidence-boundary failure.
- Moral reasoning: strong 3/5, adequate 2/5; disposition consistency strong 5/5.
- Rhetoric: adequate 5/5; judge_response unique 5/5; archive_summary unique 5/5.
- World-language: machine PASS 5/5 and human PASS 5/5; security-commentary residue 0/5.
- Action-claim taxonomy: non-blocking boundary observed 5/5 (three-year integrity/context enters recognized_action_claims).
- Full human audit with complete real AI replies: `reports/semantic_audit_week8b/day7_mr15_human_audit_v0_1.md` and `.json`.

## 7. Overall drift rating

**Stable semantic core with minor non-blocking variation.** Formal choice, case identity, fragment set, role directions, unsupported boundary and world-language publication are stable. Variation is limited to thought/action emphasis, moral reasoning tier and natural Judge phrasing.

## 8. Full Week8B audit and ordinary-transfer check

The full consolidation separates superseded Prompt v0.4-v0.4.5 development evidence from current v0.4.6 evidence. Day6 final MR08/MR15/MR28 remain 3/3 validated and language-passed; Day7 adds five canonical MR15 repeats. Existing MR03, MR17, MR04, MR18 and MR26 were reviewed without API calls: ordinary_transfer is substantive but can be safe-biased when evidence is weak, not merely thoughtless.

See `reports/semantic_audit_week8b/week8b_full_semantic_audit_v0_1.md` and `reports/week8_summary/Week8_Revised_Decision_Gate.md`.

## 9. Decision Gate

Recommendation: **B. READY WITH CONDITIONS**. Ready for deterministic scoring design: **CONDITIONAL**. Keep formal morality/disposition and future score ownership in program logic; AI remains a semantic explanation/evidence-role layer. No deterministic score was implemented.

## 10. Final frozen candidate, tests and hashes

Prompt v0.4.6 remained frozen after Repeat 1; Schema v0.2 remained frozen; no prompt editing during drift. Post-run validators and the full 319-test suite passed, `git diff --check` passed, and post-run control hashes matched the pre-run baseline (see `results/week8b_day7_drift_mr15_v0_1/day7_post_run_control_hashes.json`).

Files created: Day7 runner, deterministic analyzer, pre/post hash and gate records, Day7 drift audit JSON/MD, human audit JSON/MD, Week8 full audit JSON/MD, Decision Gate JSON/MD, this Progress document.

Problems / unresolved risks: taxonomy boundary for long-term integrity claims; clear/partial thought/action emphasis variation; no production price estimate; broader cases and UE response-time UX still need separate work. These did not justify reopening the frozen candidate during Day7.

Next-week handoff: design a program-owned deterministic scoring interface and test fixtures only after agreeing how semantic explanation fields are consumed; retain current publication gates and human audit.

## 11. System Understanding Review

1. **Why freeze Prompt v0.4.6?** It is the current candidate whose semantic behavior is being measured.
2. **Why would changing Prompt during repeats invalidate Drift?** It would change the treatment, so differences could no longer be attributed to same-input stochastic variation.
3. **Why use one canonical report?** A fixed MR15 isolates model variation from input variation.
4. **Why is MR15 useful?** It contains action, outcome, personality and relationship evidence, including used-but-not-selected Outcome02.
5. **Why are five different Judge sentences not automatically drift?** Prose can vary while formal choice, evidence roles and boundaries remain invariant.
6. **Surface diversity vs semantic drift?** Surface diversity changes wording; semantic drift changes meaning, roles, formal integrity, safety or evidence boundary.
7. **Why compare Fragment Sets without ordering?** Fragment order is not a semantic claim in Schema v0.2.
8. **Why can a one-step Tier change be acceptable?** Tier is explanatory quality, not program-owned morality; a one-step change is acceptable when evidence and choice stay stable.
9. **What Role changes would be material?** Reversing support/counterevidence, inventing Thought01, mapping unknown fragments, or full-case expansion.
10. **Why is Outcome02 important in MR15?** It anchors the real resource cost that prevents an overly simple benevolent reading.
11. **Why should Thought01 generally remain absent?** MR15's player report does not substantively argue prior foresight; adding it would create unsupported emphasis.
12. **Why is Unsupported count alone insufficient?** A zero count can still hide wrong roles, wrong polarity, or world-language failure.
13. **Why world-language needs machine + human audit?** Keyword detection catches known residue; human review catches context and immersion failures.
14. **Why is Language WARNING not Schema invalid?** Language audit is a separate publication/immersion layer, not JSON shape validation.
15. **Why does Publication Gate matter?** It keeps unsafe visible text raw-only and out of player-facing validated results.
16. **Why is Human Label equality not correctness?** Labels describe local experiment intent and are never hidden truth or model input.
17. **Why does drift differ from model accuracy?** Drift measures repeat consistency; it does not claim the model discovered a true morality.
18. **Why should Judge prose remain variable?** A living judge needs natural responses; fixed sentences would reduce human feel without improving semantics.
19. **Why assess ordinary_transfer with existing evidence?** It tests whether the safe option is substantively reasoned without adding API cost or input variation.
20. **Why only decide on deterministic scoring today?** The experiment can decide readiness; implementing scoring would change scope and candidate behavior.
21. **Why deterministic scoring remains program-owned?** Formal choices and gameplay consequences must remain auditable and tamper-resistant.
22. **What AI responsibilities remain appropriate?** Explain player reasoning, map evidence roles, identify unsupported claims, and speak as the Judge.
23. **What is unacceptable same-input drift?** Formal choice change, role polarity reversal, hidden-truth invention, safety leak, or publication failure.
24. **What is acceptable same-input drift?** Different concise wording, emphasis, or a one-step explanatory tier change with stable boundaries.
25. **Why post-run hashes?** They prove the treatment did not change during observation.
26. **Why no retries?** Retries add uncontrolled billed attempts and break one-repeat/one-call accounting.
27. **How does one technical failure affect audit?** It remains raw-only and lowers publication/validated counts; it is not silently retried.
28. **Which Day6 observations remain non-blocking?** Three-year integrity action-claim taxonomy residue and ordinary style variation.
29. **What risks remain before UE integration?** Scoring ownership, response-time UX, broader case coverage, adversarial language, and taxonomy refinements.
30. **Is the architecture simple enough for a portfolio prototype?** Yes: program-owned formal core, local gates, one semantic explanation call, and human audit.

## 12. Git Status

Worktree status captured after Day7 (unrelated pre-existing UE work preserved):
```text
 M Content/WangChuan/Blueprints/Enemies/BP_GhostEnemy.uasset
 M Content/WangChuan/Blueprints/Enemies/BP_GhostEnemy_New.uasset
 M Source/WangChuan/GhostEnemy.cpp
 M Source/WangChuan/GhostEnemy.h
 M Source/WangChuan/WangChuan.Build.cs
?? AI_Judgement_Prototype/reports/semantic_audit_week8b/day7_mr15_drift_audit_v0_1.json
?? AI_Judgement_Prototype/reports/semantic_audit_week8b/day7_mr15_drift_audit_v0_1.md
?? AI_Judgement_Prototype/reports/semantic_audit_week8b/day7_mr15_human_audit_v0_1.json
?? AI_Judgement_Prototype/reports/semantic_audit_week8b/day7_mr15_human_audit_v0_1.md
?? AI_Judgement_Prototype/reports/semantic_audit_week8b/week8b_full_semantic_audit_v0_1.json
?? AI_Judgement_Prototype/reports/semantic_audit_week8b/week8b_full_semantic_audit_v0_1.md
?? AI_Judgement_Prototype/reports/week8_summary/Week8_Revised_Decision_Gate.json
?? AI_Judgement_Prototype/reports/week8_summary/Week8_Revised_Decision_Gate.md
?? AI_Judgement_Prototype/results/week8b_day7_drift_mr15_v0_1/
?? AI_Judgement_Prototype/src/analyze_semantic_drift_v0_1.py
?? AI_Judgement_Prototype/src/build_day7_audit_package_v0_1.py
?? AI_Judgement_Prototype/src/run_day7_drift_mr15_v0_1.py
?? Content/WangChuan/Maps/MemoryMaze_CombatFirst_Greybox_v01.umap
?? Content/WangChuan/Maps/MemoryMaze_CombatFirst_Layout_v02.umap
?? Content/__ExternalActors__/WangChuan/
?? Content/__ExternalObjects__/WangChuan/
?? Docs/Development_Log_Week8_Expand.md
?? Docs/Week8Day7RevisedProgress.md
?? Source/WangChuan/WCGhostAIController.cpp
?? Source/WangChuan/WCGhostAIController.h
```

## 13. Explicit boundaries

- Prompt v0.4.6 remained frozen
- Schema v0.2 remained frozen
- No Automatic Retry
- No Prompt Editing During Drift
- No Deterministic Score Implemented
- No UE Integration
- No Commit
- No Push
- API Key Never Exposed
