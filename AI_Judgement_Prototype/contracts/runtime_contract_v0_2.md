# Runtime Contract v0.2

## Scope

This contract governs one Week8B v0.2 judgement call. It sits between the program-owned case/report data and the model-generated judgement result. The contract is deliberately single-call and does not make a moral or disposition choice on behalf of the model.

## Inputs

- `case_data`: one frozen case and its allowed FragmentIDs.
- `judge_data`: one frozen judge profile.
- `player_report`: one validated `PlayerReportV02` containing the player's formal moral and disposition selections, key FragmentIDs, and report text.
- `metadata`: run identity, provider/model/version metadata, timing and usage information. Secrets and `.env` contents are never included.

The prompt exposes only the runtime fragment view (`fragment_id`, `text`, `semantic_type`, `source_type`) plus the report and the fixed judgement context. Human Labels, design notes, scoring data, rewards, hidden truth, and API credentials are excluded.

## Model output

The model returns a `JudgementResultV02` conforming to `schemas/judgement_result_v0_2.json`. It may explain the evidence, distinguish action from thought, identify unsupported assumptions, assign fragment roles, and write the judge response/archive summary. It must not emit formal moral/disposition choice fields; those remain program-owned.

## Validation gates

1. JSON/schema validation checks required fields, enums, length limits, array limits, and `additionalProperties: false`.
2. Runtime validation checks `case_id`, `judge_profile_id`, allowed and unique FragmentIDs, and visible-output leakage of internal safety flags.
3. Game-language audit checks only `judge_response` and `archive_summary`; it reports `PASS` or `WARNING` and does not silently rewrite the result.

Only a result passing the schema and runtime gates is written to `results/week8b_v0_2/validated/`. The raw response is written separately for reproducibility and contains no credential value.

## Program-owned formal fields

`moral_judgement_id`, `disposition_id`, and `selected_key_fragment_ids` in the validated envelope are copied from `player_report`. A model response cannot override them. Selected keys are the player's formal evidence selection and may differ from the model's explanatory `fragment_roles`.

## Failure policy

The v0.2 runner makes exactly one provider call for a requested report. It performs no automatic retry, batch loop, provider fallback, or silent repair. A schema/runtime failure is reported and is not promoted to the validated directory.

## Non-goals

No scoring, rewards, RAG, agents, database, Unreal integration, multi-provider orchestration, or later-day feature is part of this contract.
