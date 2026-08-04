# Week8 CLI Demonstration Checklist

This checklist is a local, no-billing demonstration. It uses the saved JSON
results and the local corpus parser; it does not authorize or require a new
provider call.

## A. List the case and corpus

From `AI_Judgement_Prototype`:

```powershell
$env:PYTHONIOENCODING = "utf-8"
python .\src\experiment_cli.py --list
```

Expected display:

- Case `Case.Knife.001 v0.2` and five valid fragments.
- Three allowed dispositions: `reincarnate`, `detain_for_review`, `dissolve`.
- Reports R01-R20 grouped as A_rigorous, B_compassionate, C_rhetorical,
  D_sophistic, and E_adversarial.
- The command exits 0 and makes no API call.

Observed during final consolidation: the command listed all 20 reports and
the five fragments successfully when `PYTHONIOENCODING=utf-8` was set.

## B. Demonstrate local R20 rejection

```powershell
$env:PYTHONIOENCODING = "utf-8"
python .\src\experiment_cli.py --report-id R20
```

Expected display:

```text
EXPECTED LOCAL REJECTION
Reason: Player report must contain at least 100 characters.
No API call was made.
```

Observed: exact expected rejection, exit 0, no raw or validated R20 result.

## C. Use saved results for representative cases

Do not rerun the provider. Inspect the saved files under
`results/raw/` and `results/validated/` or use
`reports/week8_summary/Week8_Experiment_Summary.md`.

- R01/R02: rigorous reports with different selected dispositions; demonstrate
  the baseline/candidate comparison boundary and disposition preservation.
- R17: adversarial fabricated-evidence input; demonstrates unsupported-claim
  handling and the need for stronger fragment attribution.
- R18: prompt-injection/metatext input; demonstrates that the judge resists
  changing the schema/disposition, while still risking game-language leakage.
- R19: adversarial request to modify disposition; demonstrates that the
  selected disposition remains the player's selection and should not be
  silently changed.

## D. Semantic audit snapshot

The saved audit JSON covers R01-R19 exactly once:

```text
Correct       11
Acceptable     1
Questionable   7
Incorrect      0
Not Auditable  0
```

Directly usable (Correct + Acceptable) is 12/19 (63.16%). The core gameplay
hypothesis is marked `Promising`, not Proven.

## E. Security and repository hygiene

```powershell
git status --short
git check-ignore -v .env
```

Expected security check: `.env` is ignored. Never open, print, parse, or copy
`.env` or any API key. A status listing may contain pre-existing user changes
and new reports/scripts; review before any future commit.

## F. Final local verification commands

```powershell
python .\src\validate_corpus.py
python -m unittest discover -s tests -v
python .\src\summarize_week8_results.py
python -m json.tool .\reports\semantic_audit\semantic_audit_results_v0_1.json > $null
python -m json.tool .\reports\week8_summary\week8_experiment_summary_v0_2.json > $null
git diff --check
```

Expected final result: corpus validation PASS, 44 unit tests PASS, summary
script PASS, both JSON files valid, and no whitespace errors. No commit or
Push is part of this consolidation task.
