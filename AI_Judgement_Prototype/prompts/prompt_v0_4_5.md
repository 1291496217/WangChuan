# Prompt v0.4.5

Final root-cause balancing pass after the frozen v0.4.3 and v0.4.4 regressions.

## Evidence from the two prior runs

- v0.4.3 kept Selected Keys and correctly avoided full-case mapping, but MR15 omitted the unselected Outcome02 despite recognizing its consequence elsewhere.
- v0.4.4 removed Selected Keys and recovered Outcome02, but MR08 mapped all six Case Fragments and MR15 added an unused Thought Fragment.

The problem is therefore not simply “use” or “remove” Selected Keys. The model needs a bounded expansion rule.

## Balanced Selected-Key rule

Selected Keys are candidate starting points, not a whitelist and not proof of use. Selected candidates are retained only when the player's text substantively uses them. Unselected Fragments are added only when a specific player clause uses a unique fact, consequence, cost, or conflict from that Fragment.

## Positive and negative anchors

- “善果不能抹去资源代价” includes the unique Fragment recording an actual third-person resource cost even when it was not selected.
- “可能是保护行动” plus a cited rescue outcome does not import an unmentioned promise, thought history, death wound, or key.
- A resource-cost statement does not import a Thought Fragment about prior knowledge unless the player actually discusses knowledge or foresight.

The trust boundary, directional roles, qualified-hypothesis exclusion, claim-field boundaries, world-language rules, audit v0.2, publication gate, and Schema v0.2 remain unchanged.
