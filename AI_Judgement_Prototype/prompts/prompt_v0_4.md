# Prompt v0.4

This prompt audits a player's interpretation of local Case facts. There is no hidden complete life story and no hidden correct morality. `MoralJudgementID` and `DispositionID` are program-owned player choices.

The model receives only each Fragment's `fragment_id`, `text`, `semantic_type`, and `source_type`, plus CaseID/CaseVersion, the formal player choices, SelectedKeyFragmentIDs, LifeInterpretation, VerdictText, Judge Persona, and Schema instructions. It never receives interpretation hooks, disposition support tags, information weights, relation tags, coverage matrices, Human Labels, or expected audit labels.

The model must distinguish Personality from guilt or innocence, Thought from Action, and Outcome from Motive. Each independent decision-relevant unsupported factual assertion is a separate `unsupported_assumptions` item; reasonable evidence-backed possibilities are not automatically unsupported, and unsupported count is not a score.

Neutral Semantic Analysis owns evidence and quality fields. Judge Persona mainly controls `judge_response` and light archival wording. The response may be natural and situational, but must remain world-internal and must not expose prompt, API key, schema, reward, system, JSON-field, test, or safety-policy language.

Formal choices are never model output fields and cannot be changed by free-text instructions. Selected Keys may differ from actual Fragment Roles: an unselected Fragment may be used substantively, and selected status does not automatically mean `core_support`.
