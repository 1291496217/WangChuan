# 《忘川河畔》Week9 Day1 Advanced Modification Progress

**Project:** WangChuan / 《忘川河畔》
**Engine:** Unreal Engine 5.8
**Development Branch:** `feature/ai-first-prototype`
**Stage:** `Week9Day1AdvancedModification`
**Theme:** Combat Teaching Layout Refinement
**Date:** 2026-08-10
**Status:** Completed
**Human Review:** PASS
**Second Technical Review:** PASS
**Implementation Note:** Codex initial greybox required substantial manual refinement by the developer before final acceptance
**Commit / Push:** Handled by separate Git step

---

# 1. Modification Goal

Before implementing the formal combat tutorial, the Tutorial Dungeon combat section was expanded from one Combat Room into three sequential teaching spaces.

The design goal was:

```text
Combat Room 01
Light / Heavy teaching
↓
Downhill Bridge
↓
Combat Room 02
Lock-On teaching
↓
Transition / Short Alcove
↓
Uphill Passage
↓
Combat Room 03
Future Dodge / Roll teaching
↓
Return to Puzzle
```

The modification was intended to create three distinct future combat-learning stages without returning to the complex Branch / Loop structure used by the Week8 Memory Maze prototype.

The central rule remained:

```text
More teaching rooms
≠
More route ambiguity
```

The final Tutorial Dungeon still uses one mandatory main route with one short optional dead-end Alcove.

---

# 2. Starting Baseline

Before this modification, Week9 had already completed:

```text
Week9 Day1
→ simplified Tutorial Dungeon route

Week9 Day1 Advanced
→ Tutorial Fragment system
→ Tutorial HUD
→ Fragment reading UI
→ Move / Look Hint
→ Interact Hint
```

The active map remained:

```text
/Game/WangChuan/Maps/TutorialDungeon_Prologue_v01
```

Previous combat section:

```text
TD_CombatRoom
→ direct transition
→ TD_LanternPuzzle
```

This single Combat Room was considered insufficient for the existing player combat mechanics:

```text
Light Attack
Heavy Attack
Lock-On
```

A third room was also reserved for a possible future Dodge / Roll tutorial.

---

# 3. User-Approved New Topology

The approved structure became:

```text
TD_Entrance
↓
TD_Fragment01_Slot
↓
Combat Room 01
Future Light / Heavy Tutorial
↓
TD_Fragment02_Slot
↓
Downhill Bridge
↓
Combat Room 02
Future Lock-On Tutorial
↓
Transition
├─ Short Side Alcove
└─ Uphill Passage
      ↓
Combat Room 03
Future Dodge / Roll Tutorial
↓
Combat-to-Puzzle Connector
↓
TD_LanternPuzzle
↓
TD_Fragment03_Slot
↓
TD_BossGate
↓
TD_BossArena
↓
TD_ExitGate
```

The old direct connection:

```text
Combat Room 01
→
Puzzle
```

was removed as a valid route.

---

# 4. Scope Boundary

This modification only prepared spatial gameplay structure.

Not implemented:

```text
Light Attack tutorial logic
Heavy Attack tutorial logic
Lock-On tutorial logic
Dodge / Roll system
Dodge tutorial logic

Enemy placement
Enemy spawn logic
Enemy defeat binding
Fragment02 enemy binding

Combat doors
Room locking
Encounter Controller

Lantern puzzle integration changes
Fragment03 puzzle binding

Boss logic
Exit transition
Land Temple transition

Ghost AI tuning
Persistence changes
AI Judgement changes
```

Real AI calls:

```text
0
```

No gameplay C++ or Tutorial UI system was modified as part of the final user refinement.

---

# 5. Codex Initial Implementation Review

Codex initially produced the requested broad topology:

```text
CR01
→ Downhill
→ CR02
→ Transition / Alcove
→ Uphill
→ CR03
→ Puzzle
```

However, subsequent developer play inspection found that the first version remained closer to a route-layout sketch than a reliable playable greybox.

Important issues included:

- bridgehead floor / wall seams;
- insufficient containment around the Downhill Bridge;
- exposed CR02 entry geometry;
- very thin CR02 floor support around elevation differences;
- open CR02 exit / turn geometry;
- gaps around the Transition / Alcove boundary;
- incomplete support around the lower uphill turn;
- an overly open Uphill Passage side;
- insufficient CR03 entry containment;
- a short CR03 → Puzzle connector;
- an unnecessarily complicated old-route closure using additional closure Actors.

The original automated/result conclusion that the route was fully ready was therefore too optimistic.

The developer manually refined the map before final acceptance.

---

# 6. Manual Refinement — CR01 / Bridgehead

The transition from Combat Room 01 into the new bridge route was tightened.

Key adjustments included:

```text
MOD_Bridgehead_Floor
→ moved +50 cm in Y

MOD_Bridgehead_Wall_W
→ moved +70 cm in X
```

These changes improved the physical overlap between:

```text
CR01 exit
→ bridgehead buffer
→ Downhill Bridge
```

and removed visible / traversable seams around the connection.

---

# 7. Downhill Bridge Refinement

The Downhill Bridge was significantly reworked.

Final bridge behavior was simplified into a clearer mostly straight descent.

Recorded final bridge transform included:

```text
MOD_DownhillBridge_Deck

Location:
(2990, -2350, -155)

Roll:
approximately -7.26°

Scale:
(5.5, 18.7683, 0.2)
```

Compared with the earlier Codex version, the final bridge:

- removed the unnecessary lateral directional complexity;
- used a gentler and clearer descent;
- improved the upper and lower landing connection;
- strengthened both side boundaries.

Bridge rails were also enlarged and repositioned.

Example:

```text
MOD_DownhillBridge_Rail_E

Scale:
approximately (2, 19, 5)
```

The result is a safer Tutorial traversal space rather than a narrow platforming challenge.

---

# 8. Combat Room 02 Entry Refinement

Combat Room 02 is intended for the future Lock-On tutorial.

Its entry geometry was strengthened so the player transitions cleanly from the bridge into the room.

Important refinements included:

```text
MOD_CR02_EntryWall_B
→ extended significantly

MOD_CR02_EntryWall_Lintel
→ repositioned and narrowed
```

Final recorded example:

```text
MOD_CR02_EntryWall_B

Location:
(3935, -3250, -150)

Scale:
(15, 1, 4)
```

This constrains the intended doorway more clearly and prevents the entry from exposing a large unintended external opening.

---

# 9. Combat Room 02 Floor Refinement

The original CR02 floor was too thin for a room located at a lower elevation.

It was increased from a shallow greybox floor into a substantially thicker support volume.

Recorded change:

```text
Old:
Scale approximately (24, 20, 0.2)

Final:
Scale approximately (24, 23, 2)
```

This increased:

- Y coverage;
- vertical thickness;
- reliability around elevation boundaries.

The room now reads and behaves more like a physically supported lower combat space.

---

# 10. Combat Room 02 East / Exit Refinement

The original CR02 east side and exit turn contained some of the most serious gaps.

Manual changes included:

```text
MOD_CR02_Wall_E
→ extended and raised

MOD_CR02_ExitWall_B
→ enlarged substantially

MOD_CR02_ExitWall_A
→ repositioned

MOD_CR02_ExitWall_Lintel
→ widened, repositioned, and angled
```

Final example:

```text
MOD_CR02_ExitWall_B

Location:
(4495, -5700, -150)

Scale:
(15, 5, 8)
```

These changes closed the exposed turn and created a more deliberate transition toward the Alcove / Uphill section.

---

# 11. Relocated Short Side Alcove

The Tutorial's single optional Alcove was moved between Combat Room 02 and Combat Room 03 as planned.

It remains:

```text
short
optional
dead-end
no second exit
no Puzzle connection
no combat connection
```

The northern boundary was thickened:

```text
MOD_Alcove_NorthWall

Scale:
(7, 2, 4)
```

A new fill Actor was also added around the transition:

```text
MOD_Transition_SouthWall2
```

This removed open edges between the mandatory route and the optional pocket.

The Alcove remains the only optional route in the Tutorial Dungeon.

---

# 12. Uphill Transition Refinement

The route from CR02 toward CR03 was one of the largest manual refinement areas.

The original implementation left the lower turn and side boundaries too open.

A new lower transition support was added:

```text
MOD_Uphill_Transation
```

Recorded transform:

```text
Location:
(4560, -5460, -390)

Scale:
(5, 2, 4)
```

The Actor label currently preserves the existing `Transation` spelling.

This is an editor-label typo only and is not a gameplay issue.

---

# 13. Uphill Passage Rework

The main Uphill Passage was narrowed and reorganized into a more controlled route.

Recorded main deck:

```text
MOD_Uphill_Deck

Location:
(4500, -4335, -175)

Scale:
(3, 22.7706, 0.2)
```

The original wider open side was filled with a second deck/support volume:

```text
MOD_Uphill_Deck2
```

Recorded:

```text
Location:
(4830, -4335, 15)

Scale:
(3, 22.7706, 0.2)
```

The two side boundaries were also repositioned / repurposed:

```text
MOD_Uphill_Rail_E
MOD_Uphill_Rail_W
```

The Top Landing was raised slightly:

```text
Z:
-10
→
10
```

The final space is a controlled uphill passage rather than a broad exposed ramp with fall-through openings.

---

# 14. Combat Room 03 Entry Refinement

Combat Room 03 is reserved for a future Dodge / Roll tutorial.

The uphill entry previously had an insufficient outer boundary.

The developer thickened:

```text
MOD_CR03_EntryWall_B
```

from approximately:

```text
X Scale:
1
```

to:

```text
X Scale:
5
```

This closes the vulnerable outside edge when the player turns into CR03.

---

# 15. Combat Room 03 → Puzzle Connector

The final combat-teaching room remains roughly parallel to the main Tutorial body and returns the player toward the Puzzle section.

The connector floor was lengthened:

```text
MOD_CombatToPuzzle_Floor

Old Scale:
(7, 6, 0.2)

Final Scale:
(7, 8, 0.2)
```

This improves physical overlap between:

```text
CR03
→
Puzzle
```

and makes the return to the main Tutorial flow more reliable.

---

# 16. Old CR01 → Puzzle Route Closure

The old direct route from Combat Room 01 to the Puzzle is no longer a valid shortcut.

The initial Codex version used two additional closure Actors:

```text
MOD_Old_CR01_EastExit_Closure
MOD_Old_Puzzle_WestEntry_Closure
```

The final user-refined version removed that approach.

Instead, existing doorway / wall Actors were expanded into solid walls:

```text
TD_CombatRoom_Door_E
TD_LanternPuzzle_Door_W
```

Recorded scales:

```text
TD_CombatRoom_Door_E:
(1, 18, 4)

TD_LanternPuzzle_Door_W:
(1, 16, 4)
```

This creates a cleaner and simpler physical closure.

The old shortcut remains closed.

---

# 17. Final Actor / Geometry State

Original Codex audit:

```text
138 Actors
```

Final user-refined saved map:

```text
135 Actors
```

Current `MOD_` Actors:

```text
58
```

Three explicitly new filler Actors were identified:

```text
MOD_Transition_SouthWall2
MOD_Uphill_Deck2
MOD_Uphill_Transation
```

The actor-count change is a net result of:

```text
new actors
+
deleted actors
+
reused actors
+
renamed / repurposed actors
```

and should not be interpreted as a simple three-Actor deletion.

---

# 18. Tutorial Fragment State Preservation

The combat-layout refinement preserved the Tutorial Fragment system.

Final recorded states:

## Fragment01

```text
Location:
(1320, 0, 0)

StartAvailable:
true
```

## Fragment02

```text
Location:
(3000, -1250, 0)

StartAvailable:
false
```

Fragment02 remains positioned after Combat Room 01 and near the new bridgehead route.

It remains prepared for the future:

```text
Enemy Defeat
→ Fragment02 Available
```

No Enemy Defeat binding was added during this modification.

## Fragment03

```text
Location:
(6500, -300, 0)

StartAvailable:
false
```

No Puzzle Complete binding was added during this modification.

---

# 19. Tutorial Bootstrap / UI Preservation

Tutorial Bootstrap remained at:

```text
(350, 250, 0)
```

No modification was made to:

```text
ATutorialMemoryFragment
Tutorial HUD
Tutorial Fragment UI
Tutorial Bootstrap logic
AWCCharacter Tutorial state
```

The Day1 Advanced Tutorial onboarding architecture remains intact.

---

# 20. Navigation State

The user rebuilt and saved:

```text
RecastNavMesh-Default
```

Recorded current Nav data bounds:

```text
Bounds Origin:
approximately (5928, -2470, -20)

Extent:
approximately (5928, 3458, 320)
```

`NavMeshBounds_TutorialDungeon` remained at:

```text
Location:
(5500, -2600, -210)

Scale:
(65, 42, 2.4)
```

The uploaded structural review screenshot records the NavMesh Bounds and map relationship, but does not display the green Recast overlay.

Therefore:

```text
Uploaded screenshot alone
≠
proof of full Nav connectivity
```

However, the developer completed the final manual Editor / PIE / navigation review and reported the refined version as passing.

Final acceptance therefore relies on:

```text
saved rebuilt Recast data
+
developer's final manual validation
```

rather than the screenshot alone.

---

# 21. Final Human Validation

After the Codex version proved insufficient, the developer manually refined the level and reran the practical review.

The final user-refined version passed human review.

Validated areas included:

```text
CR01 retained
CR01 left exit
old CR01 → Puzzle route closure
Fragment02 bridgehead position
Downhill Bridge traversal
CR02 spatial usability
CR02 exit containment
relocated Alcove
Uphill traversal
CR03 entry
CR03 → Puzzle return
full Tutorial route
navigation / collision behavior
Fragment state preservation
no saved Enemy
```

Human review:

```text
PASS
```

---

# 22. Second Technical Review

A second technical review was performed against the user-refined Result Package.

Reviewed material:

```text
Week9Day1AdvancedModification_UserRefinement_Result.md

10 final user-refinement screenshots

recorded Actor transform / scale comparisons

saved RecastNavMesh state

Tutorial Fragment states

old-route closure state

scope-boundary declaration
```

The review confirmed that the developer's changes were not merely visual polish.

They corrected meaningful greybox playability issues involving:

```text
floor support
route containment
elevation transitions
bridge boundaries
turn boundaries
connector overlap
old-route closure
```

The final screenshots are consistent with the structural changes described in the Result.

Second technical review found no blocking issue.

Result:

```text
PASS
```

---

# 23. Known Deferred Issue — No Dungeon Ceiling Yet

The Tutorial Dungeon currently has no ceiling / roof.

This is intentional for the present greybox stage.

Reason:

```text
The internal dungeon lighting scheme has not yet been designed.
```

Adding a ceiling now could make the interior substantially darker and reduce testing visibility before appropriate dungeon light sources are established.

Current decision:

```text
Do not add the ceiling yet.
```

Future task:

```text
Design internal dungeon lighting
→ validate brightness / atmosphere
→ then add ceiling / roof coverage
```

This is a known deferred environment / lighting task, not a blocker for the current combat-teaching greybox.

---

# 24. Other Minor Notes

A small unrelated final map adjustment was also recorded:

```text
TD_ExitGate_Frame_W_Lintel

Z:
300
→
320
```

This does not materially affect the combat-teaching topology.

The current editor Actor label:

```text
MOD_Uphill_Transation
```

contains a spelling typo.

Since it is only a development Actor label and does not affect runtime behavior, it is not a blocker.

It may be renamed later during greybox cleanup.

---

# 25. Final Scope Protection

Confirmed:

```text
Enemy Actors Saved:
0

Light / Heavy Tutorial Logic:
NONE

Lock-On Tutorial Logic:
NONE

Dodge Tutorial Logic:
NONE

Combat Gate Logic:
NONE

Fragment02 Enemy Binding:
NONE

Fragment03 Puzzle Binding:
NONE

Ghost AI Changes:
NONE

Lantern Logic Changes:
NONE

Boss Logic:
NONE

Exit Transition Changes:
NONE

Persistence Changes:
NONE

AI Judgement Changes:
NONE

Real AI Calls:
0
```

The final change remains a spatial-layout refinement.

---

# 26. Final Combat Teaching Spatial Roles

## Combat Room 01

Future role:

```text
Light Attack
Heavy Attack
```

Current status:

```text
Spatially Ready
```

## Combat Room 02

Future role:

```text
Lock-On
Strafe / circle movement
```

Current status:

```text
Spatially Ready
```

## Combat Room 03

Future role:

```text
Dodge / Roll
```

Current status:

```text
Spatially Reserved
No tutorial logic yet
```

---

# 27. Final Result

The final Tutorial combat-teaching section now provides:

```text
CR01
↓
Fragment02 reward position
↓
clear downhill traversal
↓
CR02
↓
short optional Alcove
↓
controlled uphill traversal
↓
CR03
↓
short Puzzle return
```

The previous shortcut:

```text
CR01
→ Puzzle
```

is physically closed.

The result remains a single mandatory Tutorial route rather than a Branch / Loop maze.

---

# 28. Completion Summary

Completed:

- three-stage Combat Teaching spatial structure;
- retained CR01;
- left-side CR01 exit;
- Fragment02 bridgehead position;
- safer Downhill Bridge;
- lower CR02;
- strengthened CR02 floor;
- repaired CR02 entry / exit containment;
- relocated Short Side Alcove;
- repaired transition boundaries;
- controlled Uphill Passage;
- additional uphill support deck;
- stronger CR03 entry;
- extended CR03 → Puzzle connector;
- simplified physical closure of the old CR01 → Puzzle route;
- rebuilt Recast navigation data;
- preserved Tutorial Fragment states;
- preserved Tutorial HUD / Bootstrap;
- no Enemy or combat logic added;
- developer manual playability refinement;
- final human review;
- second technical review.

---

# 29. Final Gate

```text
Codex Initial Layout:
Required Manual Correction

Developer User Refinement:
Completed

Human Editor / PIE Review:
PASS

Second Technical Review:
PASS
```

Therefore:

```text
WEEK9 DAY1 ADVANCED MODIFICATION:
COMPLETE
```

The map is ready for the next combat-teaching implementation stage:

```text
Combat Room 01
→ Light / Heavy tutorial
→ Enemy Defeat
→ Fragment02 availability
```
