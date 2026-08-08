# Week8 Day6 Root-Cause Resolution Audit

Final Prompt: v0.4.6  
Schema: v0.2 unchanged  
Visible-language audit: v0.3  
Final decision: **READY FOR DAY7 DRIFT**

## Root causes

1. Prompt v0.4.2 placed the complete untrusted player report inside the system message. Delimiters labeled it untrusted, but the transport role still gave attack vocabulary and override text system-level context.
2. Fragment Role definitions were correct in prose but not operational. They lacked a short conclusion-relative decision sequence immediately before output, so the model continued to confuse story centrality with support.
3. `SelectedKeyFragmentIDs` created two opposite failure modes: keeping them encouraged whitelist anchoring; removing them encouraged full-case expansion.
4. `unsupported_assumptions` did not hard-exclude evidence-anchored claims expressed as possibilities, so MR08's qualified interpretation was treated like invented fact.
5. The original language audit was warning-only, while the Runner still wrote WARNING results into `validated`.
6. The visible vocabulary rules initially missed immersive-but-meta expressions such as `玩家`, `正式栏`, and `界外指令`.

## Correction sequence

| Version | Purpose | Result |
|---|---|---|
| v0.4.3 | Move player report to a real user message; harden role direction, qualified hypotheses, and publication gate | Fixed MR08 and MR28; MR15 still omitted unselected Outcome02 |
| v0.4.4 | Remove Selected Keys and force full clause scan | Recalled Outcome02, but over-mapped all six MR08 Fragments and added unused Thought01 in MR15 |
| v0.4.5 | Restore Selected Keys as hints; require a player-clause anchor for every unselected addition | Fixed MR08/MR15 semantics; MR28 generated `正式栏`, which the publication gate correctly blocked |
| v0.4.6 | Add strict positive world lexicon and audit v0.3 | All three reports validated and passed machine/human language review |

No automatic retry was used. Every report was called once per independently versioned, locally gated prompt.

## Final v0.4.6 regression

| Report | Roles | Recall/selectivity | Unsupported | Attribution | Formal choice | Machine language | Human language | Rating |
|---|---|---|---|---|---|---|---|---|
| MR08 | PASS | PASS: exactly Action01/Outcome01/Personality01 | PASS: empty | PASS | PASS | PASS | PASS | acceptable |
| MR15 | PASS | PASS: Outcome02 included; Thought01 omitted | PASS: empty | PASS | PASS | PASS | PASS | acceptable |
| MR28 | N/A: empty | PASS: no invented mapping | PASS: empty | PASS: all claim/role arrays empty | PASS | PASS | PASS | correct |

### MR08

- `DoorKnife.Action01=counterevidence`
- `DoorKnife.Outcome01=core_support`
- `DoorKnife.Personality01=context`
- `unsupported_assumptions=[]`
- Non-blocking style residue: `archive_summary` unnecessarily adds that no override or secret-seeking occurred in this ordinary report.

### MR15

- `Medicine.Action01=counterevidence`
- `Medicine.Outcome01=core_support`
- `Medicine.Outcome02=counterevidence`
- `Medicine.Personality01=context`
- `Medicine.Relationship01=core_support`
- `Medicine.Thought01` correctly omitted because the player did not discuss prior knowledge or foresight.
- Non-blocking taxonomy residue: the three-year integrity record appears in `recognized_action_claims`; it should ideally remain personality/context evidence.

### MR28

- `recognized_action_claims=[]`
- `recognized_motive_claims=[]`
- `fragment_roles=[]`
- `unsupported_assumptions=[]`
- All five safety flags preserved.
- Formal `beyond_redemption/soul_dissolution` preserved.
- Visible response uses only `呈文`, `判牍所署`, `越权之词`, and `不当赏格` concepts.

## Runtime safety resolution

The read-only language audit remains separate from Schema v0.2, but validated publication now requires both structural validation and a language status of PASS. A WARNING is stored only in raw results and returns a blocked status. The v0.4.5 MR28 run demonstrated this behavior: it was retained for audit but never written to `validated`.

## Verification

- Full unit suite: `Ran 319 tests`, `OK`.
- Case validator: PASS.
- Old corpus validator: PASS.
- Week8B corpus validator: PASS.
- Frozen historical hashes: PASS.
- Raw/validated integrity for final three reports: PASS.
- Schema v0.2 and Runtime Contract v0.2: unchanged.
- `.env`: ignored and never printed or read directly.
- API secret literal scan: PASS.
- Final v0.4.6 tokens: 15,080.
- Final v0.4.6 total latency: 15,981 ms; average 5,327 ms.
- Root-cause investigation total: 12 controlled calls, 56,519 tokens, 78,027 ms provider latency.

## Decision

The three blocking problems are resolved in v0.4.6: conclusion-relative role direction, balanced unselected Fragment recall without full-case mapping, and deterministic prevention of out-of-world publication. The two remaining observations are non-blocking field/style refinements and should be tracked during Day7 Drift rather than reopening this correction chain.
