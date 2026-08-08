# Prompt v0.4.4

Targeted follow-up to the frozen Prompt v0.4.3 run. v0.4.3 fixed role direction, unsupported overreach, the user/system trust boundary, and visible world language, but MR15 still omitted an unselected Fragment whose consequence it had recognized.

## Root cause

`SelectedKeyFragmentIDs` continued to act as an implicit whitelist. In MR15 the model described the unselected third-person resource consequence in other fields while assigning roles only to the four selected Fragments.

## De-anchoring

Selected Keys remain stored by the program but are no longer sent to the semantic model. Fragment-role eligibility is derived only from the player's free text and all runtime Fragments.

## Clause-to-Fragment coverage

The model first splits `LifeInterpretation` and `VerdictText` into semantic clauses, then compares every clause with every runtime Fragment. A broad phrase counts when it uses a Fragment's unique consequence. For example, “善果不能抹去资源代价” maps to the Fragment describing the actual third-person cost even without quoting its details.

The reverse inference is forbidden: mentioning a consequence does not prove the player used a Thought Fragment about prior knowledge or foresight.

## Field boundaries

Action claims contain only player-asserted actions or outcomes. Personality evidence, motive summaries, evidence weighting, and formal moral conclusions do not belong in action claims. Case facts not used by the player must not be added to recognized claims or visible archival text.

Prompt v0.4.4 preserves Schema v0.2, the v0.2 visible-language publication gate, and all v0.4.3 trust, direction, unsupported, attribution, and world-language corrections.
