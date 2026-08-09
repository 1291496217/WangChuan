# Day7 MR15 Human Semantic Drift Audit v0.1

ExperimentID: `Week8.Day7.Drift.MR15.001`. Five independent calls were audited manually after local validation. The conclusion is **stable semantic behavior with minor non-blocking taxonomy and emphasis variation**.

## Per-repeat audit

| Repeat | RunID | Formal | Fragment set / roles | Unsupported | Thought | World language | Overall |
|---:|---|---|---|---|---|---|---|
| 1 | `20260809T151629404116Z_5e37123b` | PASS | stable 5 / stable directions | empty | partial | PASS (machine) / PASS (human) | acceptable |
| 2 | `20260809T151640260016Z_0777a120` | PASS | stable 5 / stable directions | empty | clear | PASS (machine) / PASS (human) | acceptable |
| 3 | `20260809T151652137996Z_8257c248` | PASS | stable 5 / stable directions | empty | partial | PASS (machine) / PASS (human) | acceptable |
| 4 | `20260809T151702933671Z_6e8f219b` | PASS | stable 5 / stable directions | empty | clear | PASS (machine) / PASS (human) | acceptable |
| 5 | `20260809T151712312505Z_7b549a24` | PASS | stable 5 / stable directions | empty | partial | PASS (machine) / PASS (human) | acceptable |

## Aggregate findings

- **formal_choice_integrity**: PASS 5/5
- **fragment_set_stability**: PASS 5/5 identical unordered set
- **fragment_role_stability**: PASS 5/5 expected directions
- **core_story_semantic_integrity**: PASS 5/5: unauthorized last-medicine removal, genuine child benefit, real resource cost, non-family aid/long-term honesty and non-erased responsibility remain intact
- **used_but_not_selected_stability**: PASS 5/5 Outcome02 recalled; Thought01 absent
- **unsupported_stability**: PASS 5/5 empty
- **personality_stability**: PASS 5/5 plausible/contextual
- **thought_stability**: CONDITIONAL: clear 2/5, partial 3/5; no unjustified Thought01 mapping
- **moral_reasoning_stability**: CONDITIONAL: strong 3/5, adequate 2/5; formal choice unchanged
- **disposition_stability**: PASS 5/5 strong
- **rhetoric_stability**: PASS 5/5 adequate with natural sentence variation
- **judge_response_diversity**: 5 unique responses; diversity is surface/prose variation, not semantic drift
- **archive_summary_diversity**: 5 unique summaries; all preserve the same decision and evidence boundary
- **security_commentary_residue**: PASS 0/5
- **action_claim_taxonomy**: CONDITIONAL: three-year integrity/context is repeatedly classified as an action claim; track for future taxonomy refinement
- **overall**: stable_semantic_minor_nonblocking_variation

## Full visible AI replies

### Repeat 1 — `20260809T151629404116Z_5e37123b`

**judge_response**

呈文所陈，擅取之责与救人之善并置，判善多于恶，荐生，并记明擅取责任。所引三年诚信与救助非亲故，皆可支撑救人动机；然善果不能抹去资源代价，此点已明。惟呈文未及取药前之思虑，若补此节，则论证更全。

**archive_summary**

呈文人主张亡魂擅取有责，但救助非亲故且孩子好转，三年诚信支持紧急选择，善果更实，判善多于恶，荐生并记责。论证兼顾代价与善果，惟未及取药前之预见。

Human note: Wording is more expansive and asks for the unasserted foresight detail to be added; this is a style/coverage observation, not a formal or publication failure.

### Repeat 2 — `20260809T151640260016Z_0777a120`

**judge_response**

呈文所陈，擅取之责与救人之善并置，未以善果掩过，亦未因旧日清白而免其责。三年诚信与应诺送药，皆可佐证救人动机之实。然药铺缺药致他人发作一节，虽未明言，亦在善果不能抹去资源代价一语之中。判善多于恶，荐生，并记擅取之责，处置与论证相符。

**archive_summary**

呈文人主张亡魂擅取有责，但救助非亲非故的孩子且三年诚信，救人动机较重；善果不能抹去资源代价，故判善多于恶，荐生并记责。论证兼顾责任与善果，处置一致。

Human note: Most explicit resource-cost integration; thought/action distinction is clear and the response remains world-internal.

### Repeat 3 — `20260809T151652137996Z_8257c248`

**judge_response**

呈文所陈，擅取之过与救人之实并见。你既认越界之责，又举非亲救助与三年诚信为凭，判善多于恶，荐生并记责，处置与论证相合。然资源代价一节，你虽言善果不能抹去，却未深究另一病人缺药之苦，此点尚可再酌。

**archive_summary**

呈文人主张亡魂取药有过，但救助非亲且三年诚信，救人动机较重，判善多于恶，荐生并记责。已承认资源代价，但未充分展开。

Human note: Thought/action distinction is partial and the response asks for deeper treatment of the other patient's cost; no unsupported claim is created.

### Repeat 4 — `20260809T151702933671Z_6e8f219b`

**judge_response**

呈文所陈，擅取之责与救人之实俱在。你既认越界，又举非亲救助与三年诚信，善果虽不能抹去资源代价，但救人动机较重，判善多于恶，荐生并记明擅取责任，处置与论证相符。

**archive_summary**

呈文人主张亡魂取药有责，但救助非亲且三年诚信，善果更实，判善多于恶，荐生并记明擅取责任。材料对应完整，矛盾已整合。

Human note: Most compact complete response; all five expected fragments are mapped with stable roles and no security residue.

### Repeat 5 — `20260809T151712312505Z_7b549a24`

**judge_response**

呈文所陈，擅取之责与救人之实并见。善果虽实，不能掩去资源代价，此点已明。三年诚信与救助非亲故，足见动机之重。判善多于恶，荐生，并记擅取之责。

**archive_summary**

呈文人主张亡魂取药有越界之责，但救助非亲故且孩子好转，三年诚信支持紧急选择。善果不能抹去资源代价，整体救人动机较重。判善多于恶，荐生，记明擅取责任。

Human note: Compact stable response; same formal choice and evidence boundary with natural wording variation.

## Audit boundary

No deterministic score is inferred from these labels. The human audit separates semantic integrity from natural prose diversity and preserves the raw/validated envelopes as the source record.
