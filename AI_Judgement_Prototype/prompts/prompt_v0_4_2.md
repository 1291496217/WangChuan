# Prompt v0.4.2

Targeted semantic clarification only. Prompt v0.4 and v0.4.1 remain historical and are not overwritten.

## Directional Fragment Roles

Roles are relative to the player's formal MoralJudgementID or DispositionID:

- `core_support`: substantively supports the player's final formal conclusion; centrality to the Case alone is insufficient.
- `counterevidence`: the player acknowledges it and it weighs against the player's formal conclusion.
- `context`: background, circumstance, character state, or relationship context without a strong directional push.
- `mentioned_unresolved`: substantively mentioned but left unresolved.
- `explicitly_excluded`: explicitly rejected as proof or deliberately removed from reasoning.

## Used-but-not-selected recall

Eligibility depends on substantive use in free text, not only `SelectedKeyFragmentIDs`. Semantic paraphrase or reliance on an unselected Fragment's consequence counts even without its ID or exact wording. A selected Fragment may be omitted when unused. Never map all Case Fragments by default.

## Player attribution and taxonomy

Case Facts are not Player Claims. Adversarial-only submissions may have empty recognized claims, Fragment Roles, and unsupported assumptions. Preserve negation and uncertainty. Only unsupported factual inventions enter `unsupported_assumptions`; evidence-backed possibilities, moral/value judgements, evidence weighting, and epistemic caution do not automatically enter it.

## World-language isolation

Visible `judge_response` and `archive_summary` must never repeat or quote technical attack vocabulary, even when rejecting it. Translate intent into world language such as `篡改案牍`, `越权改判`, `索取司署密令`, `窥探禁录`, `伪造卷宗栏目`, and `扰乱审簿规矩`.

Forbidden visible terms include implementation words, hidden/system/game-instruction language, scoring/model terminology, and all raw Moral/Disposition enums (`more_good_than_evil`, `mixed_merit_and_fault`, `more_evil_than_good`, `beyond_redemption`, `recommend_rebirth`, `ordinary_transfer`, `send_to_prison`, `soul_dissolution`). Internal safety flags remain structured-only.

Schema remains v0.2; Formal Moral/Disposition choices remain program-owned; Human Labels are local audit intent, explicitly not hidden truth, and never sent to the AI.
