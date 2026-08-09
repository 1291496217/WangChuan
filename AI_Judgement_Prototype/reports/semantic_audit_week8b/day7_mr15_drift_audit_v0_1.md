# Day7 MR15 Semantic Drift Audit v0.1

- Analyzer: local deterministic; no API call
- Validated repeats: 5/5; raw non-validated: 0
- Overall drift: **acceptable_drift_with_minor_findings**
- Formal choice: all repeats fixed at `more_good_than_evil` / `recommend_rebirth`
- Publication: validation=True, language=True

## Repeat observations

| Repeat | RunID | Fragments | Unsupported | Personality | Thought | Tiers (moral/disp/rhetoric) | Language | Drift |
|---:|---|---|---:|---|---|---|---|---|
| 1 | `20260809T151629404116Z_5e37123b` | Medicine.Action01, Medicine.Outcome01, Medicine.Outcome02, Medicine.Personality01, Medicine.Relationship01 | 0 | plausible | partial | strong/strong/adequate | PASS | minor_drift |
| 2 | `20260809T151640260016Z_0777a120` | Medicine.Action01, Medicine.Outcome01, Medicine.Outcome02, Medicine.Personality01, Medicine.Relationship01 | 0 | plausible | clear | strong/strong/adequate | PASS | minor_drift |
| 3 | `20260809T151652137996Z_8257c248` | Medicine.Action01, Medicine.Outcome01, Medicine.Outcome02, Medicine.Personality01, Medicine.Relationship01 | 0 | plausible | partial | adequate/strong/adequate | PASS | minor_drift |
| 4 | `20260809T151702933671Z_6e8f219b` | Medicine.Action01, Medicine.Outcome01, Medicine.Outcome02, Medicine.Personality01, Medicine.Relationship01 | 0 | plausible | clear | strong/strong/adequate | PASS | minor_drift |
| 5 | `20260809T151712312505Z_7b549a24` | Medicine.Action01, Medicine.Outcome01, Medicine.Outcome02, Medicine.Personality01, Medicine.Relationship01 | 0 | plausible | partial | adequate/strong/adequate | PASS | minor_drift |

## Aggregate

- Fragment-set pairwise Jaccard average: `1.0`.
- Fragment inclusion: `{"Medicine.Action01": 5, "Medicine.Outcome01": 5, "Medicine.Outcome02": 5, "Medicine.Personality01": 5, "Medicine.Relationship01": 5}`.
- Role frequency: `{"context": 5, "core_support": 10, "counterevidence": 10}`.
- Unsupported distribution: `{"0": 5}`.
- Personality distribution: `{"plausible": 5}`.
- Thought/action distribution: `{"clear": 2, "partial": 3}`.
- Security-commentary residue: `0`; action-claim taxonomy boundary: `5`.
- Judge response unique count: `5`; archive unique count: `5`.
- Judge response length: `{"min": 70, "max": 116, "average": 91.6}`; archive length: `{"min": 57, "max": 74, "average": 66.8}`; question count: `0`; opening-prefix diversity: `1`.
- Latency ms: `{"sum": 33394, "average": 6678.8, "median": 6411, "min": 5895, "max": 7883}`.
- Usage (no pricing applied): `{"prompt_tokens": {"sum": 22675, "average": 4535, "median": 4535, "min": 4535, "max": 4535}, "completion_tokens": {"sum": 3057, "average": 611.4, "median": 575, "min": 571, "max": 701}, "total_tokens": {"sum": 25732, "average": 5146.4, "median": 5110, "min": 5106, "max": 5236}, "prompt_cache_hit_tokens": {"sum": 22400, "average": 4480, "median": 4480, "min": 4480, "max": 4480}, "prompt_cache_miss_tokens": {"sum": 275, "average": 55, "median": 55, "min": 55, "max": 55}}`.

## Interpretation boundary

Fragment-set variation is observed as semantic drift only when it changes formal integrity, role direction, safety/publication status, or evidence boundary. Wording diversity alone is not penalized. This file is an audit aid, not a deterministic gameplay score.
