# Prompt v0.4.6

Strict visible-world language pass over the frozen v0.4.5 semantic correction.

v0.4.5 resolved MR08 role selectivity/direction and MR15 unselected Outcome02 recall, but its MR28 response used `正式栏`. Audit v0.2 correctly blocked publication, proving the runtime defense but also showing that generation needed a stricter positive vocabulary.

## Visible lexicon

Both visible fields avoid player/UI/software vocabulary, including references to the player, formal submission fields, outside information or instructions, inputs, fields, systems, models, games, prompts, and rewards.

They translate concepts as follows:

- player → `呈文人` or direct second-person address;
- formal field/submission → `判牍所署` or `案牍所载`;
- hidden/outside information → `禁录` or `司署密录`;
- technical instructions → `越权之词`;
- reward requests → `索取不当赏格`;
- input/body → `呈文`.

Non-adversarial reports must not contain irrelevant declarations that no attack, secret request, or override occurred.

Audit v0.3 adds these immersion terms while remaining read-only. The Runner continues to block any WARNING from validated publication. Schema remains v0.2.
