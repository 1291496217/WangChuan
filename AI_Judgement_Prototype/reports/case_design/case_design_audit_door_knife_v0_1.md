# Case.DoorKnife.001 — Case Design & Information Budget Audit v0.1

**Audit scope:** Day3 design-only review; no AI call, no new Corpus, no Prompt/Schema change.
**Source:** `cases/case_door_knife_001.json` and `cases/case_door_knife_001_notes.md`
**Design verdict:** Suitable for Day4 structural validation, with explicit risks around `Action01`/`Death01` weight and `ordinary_transfer` safety bias.

## 1. Audit Boundary

The six Fragments are local facts, not a complete biography or hidden answer. The audit checks whether multiple evidence-bounded readings can be written without inventing a child identity, a specific abuse history, a pursuer, a blood relationship, or a canonical moral judgement.

`DispositionSupportTags` describe possible material support only. They are not scores, truth labels, automatic punishment, or instructions that AI must follow.

## 2. Fragment Budget Table

| FragmentID | SemanticType | SourceType | Weight | Primary fact | Independent value | Main risk | Disposition relevance |
|---|---|---|---:|---|---|---|---|
| `DoorKnife.Action01` | action | objective_trace | 3 | A knife attack caused the owner's immediate death. | Establishes direct responsibility and the central event. | May overpower all contextual facts. | `ordinary_transfer`, `send_to_prison` |
| `DoorKnife.Personality01` | personality | others_testimony | 2 | Neighbours describe a quiet, yielding, timid person. | Creates a public-persona/action conflict. | May be treated as proof of innocence or deception. | All three base dispositions |
| `DoorKnife.Thought01` | thought | soul_self_knowledge | 2 | The soul repeatedly imagined the owner dead. | Changes motive interpretation. | May be upgraded into certain premeditation. | `ordinary_transfer`, `send_to_prison` |
| `DoorKnife.Outcome01` | outcome | objective_trace | 3 | The child escaped after the owner fell. | Supplies an observable protective result. | May automatically wash away responsibility. | All three base dispositions |
| `DoorKnife.Relationship01` | relationship | others_testimony | 2 | A child reports a promise not to let the person enter again. | Connects promise, door, child, and conflict. | May trigger invented blood relation or abuse. | All three base dispositions |
| `DoorKnife.Death01` | death | objective_trace | 3 | The soul died outside a temple with a back injury and the rear-door key. | Supplies unresolved post-event context without confirming pursuit, escape, retaliation, or sacrifice. | May trigger invented pursuer or heroic sacrifice. | `ordinary_transfer`, `send_to_prison` |

Weight measures narrative information load only. It is not a guilt or reliability score.

## 3. Interpretation Coverage

### A — The disguised wrongdoer

- **InterpretationID:** `doorknife.disguised_wrongdoer`
- **WorkingTitle:** Quietness as a mask
- **UsedFragmentIDs:** Action01, Personality01, Thought01, Outcome01, Relationship01, Death01
- **FragmentRoleSummary:** Action establishes the killing; Thought supports a hostile reading; Personality is treated as neighbour testimony that may be misleading; Outcome and Relationship are acknowledged as a child escaping but not as a moral eraser; Death is treated as an unresolved post-event cost.
- **PersonalityActionRelation:** The timid public image conflicts with the confirmed attack; conflict does not prove a long-term disguise.
- **ThoughtMotiveUse:** Repeated death imagery can support hostile intent, but cannot alone prove a plan.
- **ConfirmedFacts:** All six local facts remain intact.
- **ExplicitInferences:** The player may infer concealed hostility or a long-running grievance.
- **RejectedOrUnresolvedClaims:** No invented owner crimes, child identity, pursuer, or escape sequence.
- **MoralJudgementID:** `more_evil_than_good`.
- **ExtremeAlternative:** `beyond_redemption` may be tested later, but this interpretation does not independently prove that extreme judgement.
- **DispositionID:** `send_to_prison`; `soul_dissolution` is an extreme test only and is not required by the Case.
- **DispositionRationale:** The confirmed killing plus repeated hostile thought can support detention for responsibility review, but the material does not supply a complete basis for automatic soul dissolution.
- **StrongestPoint:** Does not deny the central act while using thought and public persona to build a coherent adverse reading.
- **WeakestPoint:** It risks treating neighbour testimony and thought as more certain than they are.
- **Counterevidence:** Child escape, promise, key, and back injury support a protective or mixed reading.
- **WouldThisBecomeHiddenTruth:** `false`; it is one admissible organization of the facts.

### B — The forced protective counterattack

- **InterpretationID:** `doorknife.forced_protector`
- **WorkingTitle:** The door that had to open
- **UsedFragmentIDs:** Action01, Personality01, Outcome01, Relationship01, Death01, with Thought01 acknowledged as counterevidence.
- **FragmentRoleSummary:** Outcome and Relationship form the primary protection route; Death supplies unresolved aftermath context; Action remains a killing with responsibility; Personality supplies a possible fear/pressure context; Thought prevents a simplistic innocent reading.
- **PersonalityActionRelation:** A yielding neighbour profile can be read as long-term fear or misunderstanding, not objective goodness.
- **ThoughtMotiveUse:** Repeated imagined death may be fear, hatred, or mixed pressure; it must not be silently converted into either innocence or premeditation.
- **ConfirmedFacts:** All six local facts remain intact.
- **ExplicitInferences:** The killing may have been intended to stop access to the child or to enable escape.
- **RejectedOrUnresolvedClaims:** The owner is not declared an abuser; the child is not assigned a family identity; the back injury is not assigned to a pursuer.
- **MoralJudgementID:** `more_good_than_evil`.
- **DispositionID:** `recommend_rebirth` (with `ordinary_transfer` as a cautious alternative).
- **DispositionRationale:** The child escaping and the promise about entry provide concrete protective-result support. Death01 may add unresolved aftermath context, but it is not direct evidence of a protective motive. The killing remains a responsibility that must be addressed.
- **StrongestPoint:** Makes the beneficial outcome matter without deleting the violent act.
- **WeakestPoint:** The protective motive is inferred rather than directly stated by the Case.
- **Counterevidence:** Thought01 and Action01 can support premeditation or private hatred.
- **WouldThisBecomeHiddenTruth:** `false`.

### C — Mixed-merit revenge

- **InterpretationID:** `doorknife.mixed_revenge`
- **WorkingTitle:** A promise, a death, and a debt of responsibility
- **UsedFragmentIDs:** All six Fragments, with explicit distinction between fact, inference, and unresolved claim.
- **FragmentRoleSummary:** Action and Thought establish serious fault; Outcome and Relationship establish a possible protective result; Personality and Death keep motive and aftermath unsettled.
- **PersonalityActionRelation:** The public persona is evidence of social perception, not a moral verdict.
- **ThoughtMotiveUse:** Repeated imagining supports a long-held conflict that may contain fear, resentment, or revenge.
- **ConfirmedFacts:** All six local facts remain intact.
- **ExplicitInferences:** Protection and private grievance can coexist.
- **RejectedOrUnresolvedClaims:** No claim that the killing was purely selfless, purely revenge, or legally justified.
- **MoralJudgementID:** `mixed_merit_and_fault`.
- **DispositionID:** `ordinary_transfer`, with `send_to_prison` defensible if responsibility is weighted more heavily.
- **DispositionRationale:** The evidence contains both a concrete beneficial result and a confirmed lethal act; an intermediate disposition preserves that conflict better than an automatic extreme.
- **StrongestPoint:** Gives independent work to Action, Outcome, Thought, and Relationship instead of collapsing them into one route.
- **WeakestPoint:** It may feel cautious if the player expects the outcome to decide the case.
- **Counterevidence:** A strict responsibility reading can argue that the result does not excuse the act.
- **WouldThisBecomeHiddenTruth:** `false`.

### D — Forceful rhetoric within the evidence boundary

- **InterpretationID:** `doorknife.boundary_strict_rhetoric`
- **WorkingTitle:** The clerk refuses to let a good result erase a bad act
- **UsedFragmentIDs:** Action01, Thought01, Outcome01, Relationship01, plus Personality01 and Death01 as counterevidence.
- **FragmentRoleSummary:** Uses sharp language and a strong position, but labels every motive and relationship as inference.
- **PersonalityActionRelation:** Neighbour testimony cannot outweigh the objective attack, but it can explain why the action is surprising.
- **ThoughtMotiveUse:** The repeated image is a serious question, not a verdict.
- **ConfirmedFacts:** Action, Outcome, Death and the reported statements remain literal facts.
- **ExplicitInferences:** A harsh judge may infer responsibility without inventing facts.
- **RejectedOrUnresolvedClaims:** No new witness, prior case, bloodline, owner crime, or pursuer.
- **MoralJudgementID:** `more_evil_than_good`.
- **DispositionID:** `send_to_prison`.
- **AlternativeReading:** The same rhetorical discipline could support `mixed_merit_and_fault` / `ordinary_transfer`, but this audit path records one primary judgement and disposition.
- **DispositionRationale:** A strong rhetorical stance may favour responsibility, while still admitting that the protective result creates unresolved counterweight.
- **StrongestPoint:** Tests whether rhetorical force can coexist with explicit uncertainty.
- **WeakestPoint:** Strong language can make weak inferences sound like facts; this is a deliberate audit risk.
- **Counterevidence:** Relationship01, Outcome01, Personality01, and Death01 resist a one-directional condemnation.
- **WouldThisBecomeHiddenTruth:** `false`.

## 4. Coverage Check

- Primary moral judgements covered: `more_good_than_evil`, `mixed_merit_and_fault`, and `more_evil_than_good`.
- `beyond_redemption` remains an extreme test input, not a fourth independently supported interpretation.
- Base dispositions with material support: `recommend_rebirth`, `ordinary_transfer`, `send_to_prison`.
- Personality-as-disguise route: Interpretation A.
- Personality-as-pressure/misunderstanding route: Interpretation B.
- Thought-as-planning route: Interpretation A.
- Thought-as-fear/struggle route: Interpretation B.
- Explicit rejection of “good outcome automatically proves good motive”: Interpretations A, C, and D.
- Explicit coexistence of responsibility and protection: Interpretations B and C.

The four readings therefore cover at least three moral directions and at least two base dispositions without requiring all six Fragments in every reading.

## 5. Information Budget Questions

1. **What local action is confirmed?** The knife attack and immediate death.
2. **What result is confirmed?** The child escapes after the owner falls.
3. **How can personality alter interpretation?** It creates a public-persona/action conflict, but is only neighbour testimony.
4. **How can thought alter motive interpretation?** It can support planning, hatred, fear, or mixed pressure; it does not determine which.
5. **Which content is only others' evaluation?** `Personality01` and the quoted `Relationship01` are testimony, not omniscient labels.
6. **Which Fragments support each other?** Action–Outcome form event/result; Relationship–Outcome form promise/escape; Thought–Action form motive/action tension; Death–Relationship form post-event key/door context.
7. **Which Fragments conflict?** Personality conflicts with Action; Outcome can support protection while Action supports responsibility; Thought can support either planning or fear.
8. **What creates the largest reversal?** The combination of Outcome01 and Relationship01 can move a reading from condemnation toward protection without denying Action01.
9. **How do three readings stand?** A uses Action+Thought, B uses Outcome+Relationship+Death, C explicitly balances both, and D tests rhetoric against the same boundaries.
10. **Rebirth material support?** Outcome01, Relationship01, and Personality01 provide the direct support route. Death01 may be contextual counterevidence or aftermath, but is not tagged as direct rebirth support. Action01 and Thought01 must be confronted.
11. **Ordinary-transfer support?** Action01, Thought01, Outcome01, and Death01 together support mixed merit and fault.
12. **Prison support?** Action01 and Thought01 provide responsibility evidence; Outcome01 is a counterweight, not an eraser.
13. **Should soul dissolution enter later Corpus?** Only as an extreme boundary test; it lacks equal positive support here.
14. **Is there an obvious safety disposition?** `ordinary_transfer` is the likely safety bias because it can acknowledge both sides without committing to either extreme.
15. **Are Action01 and Death01 the only effective facts?** No. They are high-weight anchors, but Outcome01 and Relationship01 independently alter responsibility interpretation.
16. **Does Personality change interpretation?** Yes, by creating a credible conflict between public reputation and confirmed action, not by proving character.
17. **Does Thought have a non-planning role?** Yes: fear, resentment, helpless fantasy, or a mixed unresolved motive.
18. **Does Outcome provide a result rather than an answer?** Yes. The child escapes; the Case does not state why the killing occurred.
19. **Does Relationship have independent value?** Yes. It links the child, entry restriction, and a prior promise without defining identity or motive.
20. **Does removing any one Fragment leave meaning?** Yes, but the interpretation space changes; see ablation below.
21. **Does Death01 reveal a hidden answer?** No. It gives aftermath, injury, location, and a key, not a pursuer or sacrifice story.
22. **Does the full set over-point to protecting the child?** Somewhat. Outcome01 and Relationship01 form the direct protection cluster, while Death01 can easily be over-read as sacrifice or pursuit. This is the main information-density risk.
23. **Is there a complete hidden biography?** No. Multiple key motives and relationships remain intentionally unknown.
24. **Do at least two base dispositions have real material support?** Yes. All three base dispositions have explicit, non-empty support routes.

## 6. Ablation Audit

| Removed Fragment | Remaining judgement space | Clearly weakened reading | At least two readings remain? | Non-replaceable information | Load concern |
|---|---|---|---|---|---|
| `Action01` | Motive, public persona, promise, outcome, and aftermath remain, but no direct killing fact. | Prison and disguised-wrongdoer readings weaken sharply. | Yes: protection, mixed, and uncertainty. | Yes; it is the central action anchor. | Carries too much responsibility information. |
| `Personality01` | Action, thought, outcome, promise, and death still support responsibility/protection conflict. | The public-persona reversal weakens. | Yes. | No; the case remains interpretable. | Its low weight should not be treated as moral evidence. |
| `Thought01` | Action and outcome/relationship still support prison, protection, or mixed readings. | Premeditation/long-term grievance route weakens. | Yes. | No; motive becomes more open. | Removing it may reduce deterministic guilt. |
| `Outcome01` | Action, thought, relationship, personality, and death remain, but the concrete beneficial result disappears. | Rebirth/protection route weakens most. | Yes, but rebirth becomes harder to defend. | Nearly; it is the strongest counterweight to Action01. | High weight may create protection bias when retained. |
| `Relationship01` | Outcome still shows escape; action/thought/death support responsibility. | The promise-based protection route weakens. | Yes. | No; Outcome retains an independent result. | Avoid filling the child's identity from its quotation. |
| `Death01` | Action, thought, personality, outcome, and promise remain. | Post-killing cost/pursuit reading weakens. | Yes. | No, but aftermath becomes generic. | Key and injury invite invented pursuit, escape, retaliation, or sacrifice narratives. |

Every ablation leaves at least two broad readings, but removing `Action01` or `Outcome01` changes the case's centre of gravity. This is acceptable for a design audit; it is not proof that every future random composition will be balanced.


## 7. Runtime Metadata Boundary

The following fields are design/audit metadata rather than additional case facts:

```text
interpretation_hooks
disposition_support_tags
information_weight
relation_tags
```

For later Prompt/Corpus work:

- do not send `interpretation_hooks` to the model;
- do not send `disposition_support_tags` to the model;
- do not convert `information_weight` into guilt, reliability, reward, or moral score;
- keep relation tags semantically neutral;
- use these fields for local validation and design audit only unless a later version explicitly defines a safe runtime contract.

Otherwise the design metadata would become a hidden-answer channel.

## 8. Audit Verdict

`Case.DoorKnife.001` meets the Day3 design threshold:

- six short, typed, weighted Fragments are present;
- no complete hidden truth is encoded;
- at least four materially different, evidence-bounded readings are possible;
- three primary moral directions and three base dispositions have support routes;
- `beyond_redemption` remains an extreme test rather than a claimed fourth supported route;
- personality, thought, result, relationship, and death retain separate semantic roles;
- information budget and ablation risks are documented.

**Day4 handoff:** build structural validation around these IDs and enum values, verify relation/disposition references, and test that the old Knife baseline remains unchanged. Do not add AI calls or Prompt v0.4 yet.
