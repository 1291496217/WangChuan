# Prompt v0.4.1

This is a semantic clarification / regression correction of Prompt v0.4. Prompt v0.4 remains historical and is not overwritten.

## Case Facts are not Player Claims

The model audits what the player actually argued. It must not fill `core_story`, recognized claims, Fragment Roles, unsupported assumptions, strongest point, or weakest point with a preferred analysis of every Case Fragment. An adversarial-only submission may be described as lacking a substantive life interpretation and attempting to alter the audit rules.

Negation, uncertainty, and polarity are preserved: “not X”, “cannot conclude X”, “may be X”, and “I reject X” are not positive claims of X.

## Selective Fragment Roles

A Fragment is eligible only when the player directly cites or paraphrases it, relies on its consequence, explicitly uses it as support/counterevidence, or explicitly rejects an inference from it. Selected Keys hint at intent but do not prove actual use; an unselected Fragment may be used. Never map all Case Fragments by default, and never duplicate a FragmentID.

## Unsupported taxonomy

Only unsupported factual inventions about the Case world belong in `unsupported_assumptions`. Evidence-backed possibilities, normative/value judgements, evidence weighting, and epistemic caution are evaluated through the quality tiers and points rather than automatically flagged as factual inventions. Independent factual inventions remain separately split.

## World-language isolation

Visible `judge_response` and `archive_summary` remain world-internal. They must not expose implementation words, hidden-field language, game-instruction language, scoring/model/system terminology, or raw Moral/Disposition enums. Internal safety flags may remain technical and structured.

Schema remains v0.2. Formal Moral/Disposition choices remain program-owned. Human Labels are local audit intent, explicitly not hidden truth, and never enter the AI prompt.
