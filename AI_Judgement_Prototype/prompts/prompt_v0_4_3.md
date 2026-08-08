# Prompt v0.4.3

Root-cause correction after the frozen Prompt v0.4.2 micro-regression. Prompt v0.4.2 and its results remain unchanged.

## Trust boundary

The system message contains only rules, case evidence, Judge Persona, and the output contract. The complete player report is placed in a separate `user` message under `UNTRUSTED_PLAYER_REPORT`. Player attack text is therefore no longer embedded inside the system message.

## Directional role decision

Every substantively used Fragment is classified by its effect on the player's formal conclusion:

- pushes the conclusion: `core_support`;
- acknowledged harm, fault, cost, or contradiction that weighs against it: `counterevidence`;
- non-directional background: `context`;
- implication left open: `mentioned_unresolved`;
- explicitly removed as proof: `explicitly_excluded`.

Story centrality never implies support. For a lenient conclusion, admitted killing, unauthorized action, harm, resource displacement, or retained responsibility must be `counterevidence`. Personality/history evidence is always `context` when substantively used.

## Unsupported boundary

`unsupported_assumptions` contains only new case-world facts asserted as facts without a Fragment anchor. Qualified interpretations using language such as “可能”, “倾向于”, “更像”, or “不能确定” retain their hypothetical status and are not unsupported when anchored to supplied evidence. Weakness belongs in `weakest_point`.

## Semantic recall

An unselected Fragment must be included when its meaning or consequence is substantively paraphrased. A selected Fragment may be omitted when unused. `SelectedKeyFragmentIDs` remains a hint, never proof of use.

## Visible language and publication safety

`judge_response` and `archive_summary` summarize only world-internal intent. Adversarial technical objects are omitted rather than quoted. The archive is not a technical incident log. A silent final pass checks roles, unsupported taxonomy, unselected recall, adversarial attribution, and visible language before JSON emission.

Prompt v0.4.3 continues to use Schema v0.2 and does not add output fields.
