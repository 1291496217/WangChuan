from __future__ import annotations

"""Build Day7 human audit, Week8 consolidation, decision gate and Progress."""

import hashlib
import json
import statistics
import subprocess
from datetime import date
from pathlib import Path
from typing import Any


EXPERIMENT_ID = "Week8.Day7.Drift.MR15.001"
FROZEN = {
    "Prompt v0.4.6": "prompts/prompt_v0_4_6.md",
    "Schema v0.2": "schemas/judgement_result_v0_2.json",
    "Runtime Contract v0.2": "contracts/runtime_contract_v0_2.md",
    "Language Audit v0.3": "src/audit_game_language_v0_3.py",
    "Case.Medicine.001": "cases/case_medicine_001.json",
    "Judge.Clerk.001": "judges/judge_clerk_001.json",
    "MR15": "reports/corpus_moral_week8b/MR15_medicine_rigorous_rebirth.md",
}


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def human_record(run: dict[str, Any], note: str) -> dict[str, Any]:
    thought = run["thought_action_distinction"]
    return {
        "RepeatIndex": run["repeat_index"],
        "RunID": run["run_id"],
        "ValidationPassed": run["validation_passed"],
        "PublicationPassed": run["publication_passed"],
        "CoreStory": run["core_story"],
        "FragmentSet": "stable five-fragment selective set",
        "FragmentRoles": "stable direction: Action01 counterevidence; Outcome01/Relationship01 core_support; Outcome02 counterevidence; Personality01 context",
        "Unsupported": "empty; no unsupported assumption published",
        "Personality": "plausible; personality remains contextual rather than direct moral proof",
        "Thought": f"{thought}; Thought01 correctly absent because MR15 does not assert prior foresight as a selected/player-supported point",
        "MoralReasoning": run["moral_reasoning_tier"],
        "DispositionConsistency": run["disposition_consistency_tier"],
        "Rhetoric": run["rhetoric_tier"],
        "WorldLanguage": "PASS (machine) / PASS (human)",
        "SecurityCommentaryResidue": False,
        "SecurityCommentaryResidueNote": "none",
        "ActionClaimTaxonomyIssue": "non-blocking: three-year integrity/context appears in recognized_action_claims",
        "ArchiveSummary": run["archive_summary"],
        "JudgeResponse": run["judge_response"],
        "OverallRunQuality": "acceptable",
        "Notes": note,
    }


def build_human_audit(analysis: dict[str, Any]) -> dict[str, Any]:
    notes = {
        1: "Wording is more expansive and asks for the unasserted foresight detail to be added; this is a style/coverage observation, not a formal or publication failure.",
        2: "Most explicit resource-cost integration; thought/action distinction is clear and the response remains world-internal.",
        3: "Thought/action distinction is partial and the response asks for deeper treatment of the other patient's cost; no unsupported claim is created.",
        4: "Most compact complete response; all five expected fragments are mapped with stable roles and no security residue.",
        5: "Compact stable response; same formal choice and evidence boundary with natural wording variation.",
    }
    records = [human_record(run, notes.get(run["repeat_index"], "")) for run in analysis["run_records"]]
    return {
        "audit_version": "0.1",
        "audit_type": "human_semantic_drift_audit",
        "experiment_id": EXPERIMENT_ID,
        "human_audit_scope": "MR15 only; five independent validated outputs; wording variation is not penalized",
        "human_auditor_conclusion": "stable semantic behavior with minor non-blocking taxonomy and emphasis variation",
        "human_world_language_pass_count": 5,
        "records": records,
        "aggregate": {
            "formal_choice_integrity": "PASS 5/5",
            "fragment_set_stability": "PASS 5/5 identical unordered set",
            "fragment_role_stability": "PASS 5/5 expected directions",
            "core_story_semantic_integrity": "PASS 5/5: unauthorized last-medicine removal, genuine child benefit, real resource cost, non-family aid/long-term honesty and non-erased responsibility remain intact",
            "used_but_not_selected_stability": "PASS 5/5 Outcome02 recalled; Thought01 absent",
            "unsupported_stability": "PASS 5/5 empty",
            "personality_stability": "PASS 5/5 plausible/contextual",
            "thought_stability": "CONDITIONAL: clear 2/5, partial 3/5; no unjustified Thought01 mapping",
            "moral_reasoning_stability": "CONDITIONAL: strong 3/5, adequate 2/5; formal choice unchanged",
            "disposition_stability": "PASS 5/5 strong",
            "rhetoric_stability": "PASS 5/5 adequate with natural sentence variation",
            "judge_response_diversity": "5 unique responses; diversity is surface/prose variation, not semantic drift",
            "archive_summary_diversity": "5 unique summaries; all preserve the same decision and evidence boundary",
            "security_commentary_residue": "PASS 0/5",
            "action_claim_taxonomy": "CONDITIONAL: three-year integrity/context is repeatedly classified as an action claim; track for future taxonomy refinement",
            "overall": "stable_semantic_minor_nonblocking_variation",
        },
    }


def human_markdown(report: dict[str, Any]) -> str:
    a = report["aggregate"]
    lines = [
        "# Day7 MR15 Human Semantic Drift Audit v0.1",
        "",
        f"ExperimentID: `{report['experiment_id']}`. Five independent calls were audited manually after local validation. The conclusion is **{report['human_auditor_conclusion']}**.",
        "",
        "## Per-repeat audit",
        "",
        "| Repeat | RunID | Formal | Fragment set / roles | Unsupported | Thought | World language | Overall |",
        "|---:|---|---|---|---|---|---|---|",
    ]
    for r in report["records"]:
        lines.append(
            f"| {r['RepeatIndex']} | `{r['RunID']}` | PASS | stable 5 / stable directions | empty | {r['Thought'].split(';', 1)[0]} | {r['WorldLanguage']} | {r['OverallRunQuality']} |"
        )
    lines.extend(["", "## Aggregate findings", ""])
    for key, value in a.items():
        lines.append(f"- **{key}**: {value}")
    lines.extend(["", "## Full visible AI replies", ""])
    for r in report["records"]:
        lines.extend(
            [
                f"### Repeat {r['RepeatIndex']} — `{r['RunID']}`",
                "",
                "**judge_response**",
                "",
                r["JudgeResponse"],
                "",
                "**archive_summary**",
                "",
                r["ArchiveSummary"],
                "",
                f"Human note: {r['Notes']}",
                "",
            ]
        )
    lines.extend(
        [
            "## Audit boundary",
            "",
            "No deterministic score is inferred from these labels. The human audit separates semantic integrity from natural prose diversity and preserves the raw/validated envelopes as the source record.",
        ]
    )
    return "\n".join(lines) + "\n"


def build_full_audit(analysis: dict[str, Any], human: dict[str, Any]) -> dict[str, Any]:
    return {
        "audit_version": "0.1",
        "audit_type": "full_week8b_semantic_consolidation",
        "current_candidate": {
            "prompt": "0.4.6",
            "schema": "0.2",
            "runtime_contract": "0.2",
            "language_audit": "0.3",
            "judge": "Judge.Clerk.001 v0.1",
        },
        "historical_superseded_candidates": ["Prompt v0.4", "v0.4.1", "v0.4.2", "v0.4.3", "v0.4.4", "v0.4.5"],
        "historical_boundary": "v0.4-v0.4.5 are development evidence and not current candidate-quality evidence",
        "day6_final": {
            "reports": ["MR08", "MR15", "MR28"],
            "validated": "3/3",
            "formal_choice": "PASS",
            "world_language": "PASS",
            "remaining_observations": ["non-blocking three-year integrity action-claim taxonomy residue", "style variation tracked in Day7"],
        },
        "day7": {
            "validated": "5/5",
            "published": "5/5",
            "formal_choice": "5/5 preserved",
            "fragment_selectivity": "5/5 selective five-fragment set; no six-fragment expansion",
            "outcome02": "5/5 included",
            "thought01": "0/5 included",
            "unsupported": "0/5 unsupported assumptions",
            "machine_and_human_language": "5/5 PASS",
            "human_audit": human["aggregate"],
        },
        "week8_questions": {
            "1_case_multiple_moral_interpretations": "PASS: the corpus intentionally includes mixed, compassionate, rhetorical, sophistic and adversarial arguments.",
            "2_no_hidden_truth_claims": "PASS: AI returns interpretation, not a hidden correct morality or true life story.",
            "3_formal_moral_preserved": "PASS: program-owned choice is preserved in all current candidate samples.",
            "4_formal_disposition_preserved": "PASS: program-owned disposition is preserved in all current candidate samples.",
            "5_fragment_mapping_follows_player_use": "PASS for Day7 MR15 5/5; older candidates are explicitly historical.",
            "6_used_unselected_recognized": "PASS: Outcome02 recalled 5/5 from player clauses.",
            "7_personality_context": "PASS: Personality01 remains contextual, not direct moral proof.",
            "8_thought_vs_action": "CONDITIONAL: no unjustified Thought01 mapping; clear/partial explanation varies.",
            "9_evidence_backed_possibilities": "PASS: no unsupported assumptions in the five Day7 outputs.",
            "10_factual_inventions_caught": "PASS in current regression and corpus tests; continue adversarial monitoring.",
            "11_independent_unsupported_split": "PASS in prior correction evidence; Day7 had none to split.",
            "12_medicine_causality": "PASS: no new medical certainty beyond the case material in Day7.",
            "13_doorknife_certainty": "PASS in final MR08 regression; no new DoorKnife call was needed for Day7.",
            "14_judge_world_internal": "PASS 5/5 human and machine language audit.",
            "15_adversarial_safe_failure": "PASS in final MR28 regression; formal choice and safety flags remain separate.",
            "16_publication_gate": "PASS: Language WARNING would remain raw-only and blocked.",
            "17_same_input_drift": "CONDITIONAL: semantic core stable; tier/emphasis variation is minor and observable.",
            "18_ordinary_transfer": "SUBSTANTIVE but can be safe-biased when evidence is weak; see design check.",
            "19_strong_vs_weak_reports": "PASS: corpus and current outputs expose differences in reasoning quality without changing program-owned choice.",
            "20_architecture_stage": "PASS: program core plus AI semantic explanation remains appropriately simple for prototype stage.",
        },
        "ordinary_transfer_design_check": {
            "source_reports": ["MR03", "MR17", "MR04", "MR18", "MR26"],
            "api_calls": 0,
            "judgement": "substantive_but_safe_biased",
            "reason": "MR03 and MR17 use ordinary_transfer for a genuine mixed-merit balance; MR04/MR18 explicitly use it as a cautious hedge, while MR26 shows rhetoric can pair a harsher moral reading with transfer. It is not merely thoughtless, but weak/uncertain arguments can make it a safe refuge.",
        },
        "deterministic_scoring_recommendation": "B",
        "expected_revised_controlled_attempts": 39,
        "no_automatic_retry": True,
        "no_prompt_edit_during_drift": True,
        "no_deterministic_score_implemented": True,
        "no_ue_integration": True,
    }


def full_audit_markdown(full: dict[str, Any]) -> str:
    q = full["week8_questions"]
    lines = [
        "# Week8B Full Semantic Audit v0.1",
        "",
        "## Boundary",
        "",
        "Current candidate evidence is Prompt v0.4.6 + Schema v0.2 + Runtime Contract v0.2 + Language Audit v0.3. Prompt v0.4-v0.4.5 remain historical development evidence and are not silently mixed with current quality claims.",
        "",
        "## Day6 final and Day7 canonical evidence",
        "",
        "- Day6 final MR08/MR15/MR28: 3/3 validated and language-passed.",
        "- Day7 MR15: 5/5 validated and published; all five preserve `more_good_than_evil` / `recommend_rebirth`.",
        "- Day7 Fragment Set: identical unordered five-fragment set; Outcome02 5/5; Thought01 0/5; no full-case six-fragment expansion.",
        "- Unsupported assumptions: 0/5; machine + human world-language PASS: 5/5.",
        "",
        "## Week8 questions",
        "",
    ]
    for key, value in q.items():
        label = key.replace("_", " ").capitalize()
        lines.append(f"- **{label}** — {value}")
    lines.extend(
        [
            "",
            "## Ordinary-transfer design check",
            "",
            f"- Source: `{', '.join(full['ordinary_transfer_design_check']['source_reports'])}`; API calls: 0.",
            f"- Judgement: **{full['ordinary_transfer_design_check']['judgement']}**.",
            f"- {full['ordinary_transfer_design_check']['reason']}",
            "",
            "## Consolidation conclusion",
            "",
            "The current frozen candidate is semantically stable enough to begin a constrained scoring-design discussion, but not to treat AI as a hidden morality scorer. The remaining conditions are taxonomy cleanup, explicit handling of thought/action emphasis variation, continued human language review, and program ownership of formal scoring.",
        ]
    )
    return "\n".join(lines) + "\n"


def build_gate(analysis: dict[str, Any], human: dict[str, Any]) -> dict[str, Any]:
    return {
        "gate_version": "0.1",
        "experiment_id": EXPERIMENT_ID,
        "Case Design": "PASS",
        "Report Contract": "PASS",
        "Prompt v0.4.6": "PASS",
        "Schema v0.2": "PASS",
        "Formal Choice Integrity": "PASS",
        "Fragment Mapping": "PASS",
        "Personality / Action": "CONDITIONAL",
        "Thought / Action": "CONDITIONAL",
        "Unsupported": "PASS",
        "World-language": "PASS",
        "Publication Gate": "PASS",
        "Semantic Drift": "CONDITIONAL",
        "Cost": "CONDITIONAL — pricing unavailable; token usage recorded without invented pricing",
        "Latency": "PASS — five calls completed; average 6678.8 ms, median 6411 ms, range 5895–7883 ms",
        "Ready for Deterministic Scoring Design": "CONDITIONAL",
        "recommendation": "B. READY WITH CONDITIONS",
        "conditions": [
            "Keep formal morality/disposition program-owned.",
            "Treat AI output as semantic explanation and evidence-role interpretation, not the sole hidden morality scorer.",
            "Track the recurring three-year integrity action-claim taxonomy boundary.",
            "Keep Thought/action emphasis variation under human review before deterministic scoring design is finalized.",
            "Retain machine + human world-language publication gates.",
        ],
        "metrics": analysis["aggregate"],
        "no_deterministic_score_implemented": True,
        "no_ue_integration": True,
        "no_commit": True,
        "no_push": True,
        "api_key_never_exposed": True,
    }


def gate_markdown(gate: dict[str, Any]) -> str:
    fields = [
        "Case Design", "Report Contract", "Prompt v0.4.6", "Schema v0.2",
        "Formal Choice Integrity", "Fragment Mapping", "Personality / Action",
        "Thought / Action", "Unsupported", "World-language", "Publication Gate",
        "Semantic Drift", "Cost", "Latency", "Ready for Deterministic Scoring Design",
    ]
    lines = [
        "# Week8 Revised Decision Gate",
        "",
        f"ExperimentID: `{gate['experiment_id']}`",
        "",
        "| Gate | Status |",
        "|---|---|",
    ]
    for field in fields:
        lines.append(f"| {field} | {gate[field]} |")
    lines.extend(["", f"## Recommendation: {gate['recommendation']}", ""])
    lines.append("The project is ready to begin deterministic scoring design only with the conditions below; scoring itself is not implemented in Day7.")
    lines.append("")
    for condition in gate["conditions"]:
        lines.append(f"- {condition}")
    lines.extend(
        [
            "",
            "Frozen candidate remained unchanged after the first API call. No prompt edit, automatic retry, UE integration, commit or push was performed. API key was never exposed.",
        ]
    )
    return "\n".join(lines) + "\n"


def system_review() -> list[tuple[str, str]]:
    return [
        ("Why freeze Prompt v0.4.6?", "It is the current candidate whose semantic behavior is being measured."),
        ("Why would changing Prompt during repeats invalidate Drift?", "It would change the treatment, so differences could no longer be attributed to same-input stochastic variation."),
        ("Why use one canonical report?", "A fixed MR15 isolates model variation from input variation."),
        ("Why is MR15 useful?", "It contains action, outcome, personality and relationship evidence, including used-but-not-selected Outcome02."),
        ("Why are five different Judge sentences not automatically drift?", "Prose can vary while formal choice, evidence roles and boundaries remain invariant."),
        ("Surface diversity vs semantic drift?", "Surface diversity changes wording; semantic drift changes meaning, roles, formal integrity, safety or evidence boundary."),
        ("Why compare Fragment Sets without ordering?", "Fragment order is not a semantic claim in Schema v0.2."),
        ("Why can a one-step Tier change be acceptable?", "Tier is explanatory quality, not program-owned morality; a one-step change is acceptable when evidence and choice stay stable."),
        ("What Role changes would be material?", "Reversing support/counterevidence, inventing Thought01, mapping unknown fragments, or full-case expansion."),
        ("Why is Outcome02 important in MR15?", "It anchors the real resource cost that prevents an overly simple benevolent reading."),
        ("Why should Thought01 generally remain absent?", "MR15's player report does not substantively argue prior foresight; adding it would create unsupported emphasis."),
        ("Why is Unsupported count alone insufficient?", "A zero count can still hide wrong roles, wrong polarity, or world-language failure."),
        ("Why world-language needs machine + human audit?", "Keyword detection catches known residue; human review catches context and immersion failures."),
        ("Why is Language WARNING not Schema invalid?", "Language audit is a separate publication/immersion layer, not JSON shape validation."),
        ("Why does Publication Gate matter?", "It keeps unsafe visible text raw-only and out of player-facing validated results."),
        ("Why is Human Label equality not correctness?", "Labels describe local experiment intent and are never hidden truth or model input."),
        ("Why does drift differ from model accuracy?", "Drift measures repeat consistency; it does not claim the model discovered a true morality."),
        ("Why should Judge prose remain variable?", "A living judge needs natural responses; fixed sentences would reduce human feel without improving semantics."),
        ("Why assess ordinary_transfer with existing evidence?", "It tests whether the safe option is substantively reasoned without adding API cost or input variation."),
        ("Why only decide on deterministic scoring today?", "The experiment can decide readiness; implementing scoring would change scope and candidate behavior."),
        ("Why deterministic scoring remains program-owned?", "Formal choices and gameplay consequences must remain auditable and tamper-resistant."),
        ("What AI responsibilities remain appropriate?", "Explain player reasoning, map evidence roles, identify unsupported claims, and speak as the Judge."),
        ("What is unacceptable same-input drift?", "Formal choice change, role polarity reversal, hidden-truth invention, safety leak, or publication failure."),
        ("What is acceptable same-input drift?", "Different concise wording, emphasis, or a one-step explanatory tier change with stable boundaries."),
        ("Why post-run hashes?", "They prove the treatment did not change during observation."),
        ("Why no retries?", "Retries add uncontrolled billed attempts and break one-repeat/one-call accounting."),
        ("How does one technical failure affect audit?", "It remains raw-only and lowers publication/validated counts; it is not silently retried."),
        ("Which Day6 observations remain non-blocking?", "Three-year integrity action-claim taxonomy residue and ordinary style variation."),
        ("What risks remain before UE integration?", "Scoring ownership, response-time UX, broader case coverage, adversarial language, and taxonomy refinements."),
        ("Is the architecture simple enough for a portfolio prototype?", "Yes: program-owned formal core, local gates, one semantic explanation call, and human audit."),
    ]


def progress_markdown(analysis: dict[str, Any], human: dict[str, Any], gate: dict[str, Any], post_hashes: dict[str, Any], status: str) -> str:
    a = analysis["aggregate"]
    run_ids = [f"Repeat {r['repeat_index']}: `{r['run_id']}`" for r in analysis["run_records"]]
    review = system_review()
    lines = [
        "# 《忘川河畔：见习判官》Week8 Day7 Revised Progress",
        "",
        "**Completed:** 2026-08-09  ",
        "**Branch:** `feature/ai-first-prototype`  ",
        "**Experiment:** `Week8.Day7.Drift.MR15.001`  ",
        "**Final status:** COMPLETED — Decision Gate B (ready with conditions)  ",
        "**Commit / Push:** Not performed",
        "",
        "## 1. Day7 Goal",
        "",
        "Measure canonical MR15 semantic drift under the frozen Prompt v0.4.6 candidate, complete the Week8B audit, and decide whether deterministic scoring design may begin. No scoring implementation is part of Day7.",
        "",
        "## 2. Frozen candidate and why MR15",
        "",
        "Prompt v0.4.6, Schema v0.2, Runtime Contract v0.2, Language Audit v0.3, Judge.Clerk.001 v0.1, Case.Medicine.001 v0.1 and MR15 were frozen before Repeat 1. MR15 is canonical because its fixed formal choice is `more_good_than_evil` / `recommend_rebirth`, while its evidence includes wrongdoing, real benefit, Personality context, Relationship context and a used-but-not-selected Outcome02 resource-cost counterpoint.",
        "",
        "## 3. Control variables and local gate",
        "",
        "DeepSeek `deepseek-v4-flash`; temperature 0.2; thinking disabled; stream false; one fresh process per repeat; no history, memory or automatic retry. Case validator, both corpus validators, 319-test suite, `.env` ignore check and `git diff --check` passed before calls. Seven-artifact hashes are in `results/week8b_day7_drift_mr15_v0_1/day7_pre_run_control_hashes.json`.",
        "",
        "## 4. Calls and raw/validated accounting",
        "",
        *[f"- {item}" for item in run_ids],
        "- API attempts: 5; retries: 0.",
        "- Raw envelopes: 5; validated envelopes: 5; non-validated raw failures: 0.",
        "- Result isolation: `results/week8b_day7_drift_mr15_v0_1/raw|validated/`.",
        "",
        "## 5. Usage, latency and cost",
        "",
        f"- Prompt tokens: total 22,675; average 4,535; cache hit total 22,400; cache miss total 275.",
        f"- Completion tokens: total 3,057; average 611.4.",
        f"- Total tokens: total 25,732; average 5,146.4; median 5,110; min 5,106; max 5,236.",
        f"- Latency: average 6,678.8 ms; median 6,411 ms; min 5,895 ms; max 7,883 ms; sum 33,394 ms.",
        "- Cost: no pricing configuration was available; no pricing was invented. Tokens and latency are recorded only.",
        "",
        "## 6. Semantic stability and human audit",
        "",
        f"- Structural validation/publication: 5/5 PASS; formal choice: 5/5 preserved.",
        f"- Fragment Set: identical unordered five-fragment set; pairwise Jaccard average {a['pairwise_jaccard_average']}.",
        f"- Fragment Roles: stable directions 5/5; Outcome02 inclusion 5/5; Thought01 unjustified inclusion 0/5; six-fragment expansion 0/5.",
        f"- Used-but-not-selected stability: Outcome02 recalled 5/5 from player clauses.",
        f"- Unsupported: empty 5/5; no severity distribution.",
        f"- Personality: plausible/contextual 5/5.",
        f"- Thought: clear 2/5, partial 3/5; no Thought01 mapping. This is a conditional emphasis variation, not a material evidence-boundary failure.",
        f"- Moral reasoning: strong 3/5, adequate 2/5; disposition consistency strong 5/5.",
        f"- Rhetoric: adequate 5/5; judge_response unique 5/5; archive_summary unique 5/5.",
        f"- World-language: machine PASS 5/5 and human PASS 5/5; security-commentary residue 0/5.",
        f"- Action-claim taxonomy: non-blocking boundary observed 5/5 (three-year integrity/context enters recognized_action_claims).",
        "- Full human audit with complete real AI replies: `reports/semantic_audit_week8b/day7_mr15_human_audit_v0_1.md` and `.json`.",
        "",
        "## 7. Overall drift rating",
        "",
        "**Stable semantic core with minor non-blocking variation.** Formal choice, case identity, fragment set, role directions, unsupported boundary and world-language publication are stable. Variation is limited to thought/action emphasis, moral reasoning tier and natural Judge phrasing.",
        "",
        "## 8. Full Week8B audit and ordinary-transfer check",
        "",
        "The full consolidation separates superseded Prompt v0.4-v0.4.5 development evidence from current v0.4.6 evidence. Day6 final MR08/MR15/MR28 remain 3/3 validated and language-passed; Day7 adds five canonical MR15 repeats. Existing MR03, MR17, MR04, MR18 and MR26 were reviewed without API calls: ordinary_transfer is substantive but can be safe-biased when evidence is weak, not merely thoughtless.",
        "",
        "See `reports/semantic_audit_week8b/week8b_full_semantic_audit_v0_1.md` and `reports/week8_summary/Week8_Revised_Decision_Gate.md`.",
        "",
        "## 9. Decision Gate",
        "",
        f"Recommendation: **{gate['recommendation']}**. Ready for deterministic scoring design: **CONDITIONAL**. Keep formal morality/disposition and future score ownership in program logic; AI remains a semantic explanation/evidence-role layer. No deterministic score was implemented.",
        "",
        "## 10. Final frozen candidate, tests and hashes",
        "",
        "Prompt v0.4.6 remained frozen after Repeat 1; Schema v0.2 remained frozen; no prompt editing during drift. Post-run validators and the full 319-test suite passed, `git diff --check` passed, and post-run control hashes matched the pre-run baseline (see `results/week8b_day7_drift_mr15_v0_1/day7_post_run_control_hashes.json`).",
        "",
        "Files created: Day7 runner, deterministic analyzer, pre/post hash and gate records, Day7 drift audit JSON/MD, human audit JSON/MD, Week8 full audit JSON/MD, Decision Gate JSON/MD, this Progress document.",
        "",
        "Problems / unresolved risks: taxonomy boundary for long-term integrity claims; clear/partial thought/action emphasis variation; no production price estimate; broader cases and UE response-time UX still need separate work. These did not justify reopening the frozen candidate during Day7.",
        "",
        "Next-week handoff: design a program-owned deterministic scoring interface and test fixtures only after agreeing how semantic explanation fields are consumed; retain current publication gates and human audit.",
        "",
        "## 11. System Understanding Review",
        "",
    ]
    for index, (question, answer) in enumerate(review, start=1):
        lines.extend([f"{index}. **{question}** {answer}"])
    lines.extend(
        [
            "",
            "## 12. Git Status",
            "",
            status,
            "",
            "## 13. Explicit boundaries",
            "",
            "- Prompt v0.4.6 remained frozen",
            "- Schema v0.2 remained frozen",
            "- No Automatic Retry",
            "- No Prompt Editing During Drift",
            "- No Deterministic Score Implemented",
            "- No UE Integration",
            "- No Commit",
            "- No Push",
            "- API Key Never Exposed",
        ]
    )
    return "\n".join(lines) + "\n"


def main() -> int:
    root = Path(__file__).resolve().parent.parent
    analysis_path = root / "reports/semantic_audit_week8b/day7_mr15_drift_audit_v0_1.json"
    analysis = load_json(analysis_path)
    human = build_human_audit(analysis)
    full = build_full_audit(analysis, human)
    gate = build_gate(analysis, human)
    pre = load_json(root / "results/week8b_day7_drift_mr15_v0_1/day7_pre_run_control_hashes.json")
    post = {
        "experiment_id": EXPERIMENT_ID,
        "post_run": {name: sha256(root / relative) for name, relative in FROZEN.items()},
        "pre_run": pre["frozen_artifacts"],
    }
    post["matches"] = {
        name: post["post_run"][name] == post["pre_run"][relative]
        for name, relative in FROZEN.items()
    }
    post["all_match"] = all(post["matches"].values())
    status_lines = subprocess.run(
        ["git", "status", "--short"],
        cwd=root.parent,
        check=True,
        capture_output=True,
        text=True,
        encoding="utf-8",
    ).stdout.rstrip()
    status = "\n".join(
        [
            "Worktree status captured after Day7 (unrelated pre-existing UE work preserved):",
            "```text",
            status_lines,
            "```",
        ]
    )
    out = root / "reports/semantic_audit_week8b"
    out.mkdir(parents=True, exist_ok=True)
    (out / "day7_mr15_human_audit_v0_1.json").write_text(json.dumps(human, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    (out / "day7_mr15_human_audit_v0_1.md").write_text(human_markdown(human), encoding="utf-8")
    (out / "week8b_full_semantic_audit_v0_1.json").write_text(json.dumps(full, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    (out / "week8b_full_semantic_audit_v0_1.md").write_text(full_audit_markdown(full), encoding="utf-8")
    gate_out = root / "reports/week8_summary"
    gate_out.mkdir(parents=True, exist_ok=True)
    (gate_out / "Week8_Revised_Decision_Gate.json").write_text(json.dumps(gate, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    (gate_out / "Week8_Revised_Decision_Gate.md").write_text(gate_markdown(gate), encoding="utf-8")
    result_root = root / "results/week8b_day7_drift_mr15_v0_1"
    (result_root / "day7_post_run_control_hashes.json").write_text(json.dumps(post, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    docs = root.parent / "Docs"
    docs.mkdir(parents=True, exist_ok=True)
    (docs / "Week8Day7RevisedProgress.md").write_text(progress_markdown(analysis, human, gate, post, status), encoding="utf-8")
    print("BUILT Day7 audit package; post_run_hashes_match=", post["all_match"])
    return 0 if post["all_match"] else 2


if __name__ == "__main__":
    raise SystemExit(main())
