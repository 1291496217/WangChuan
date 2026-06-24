\# Week 1 Progress Log



\## Project: WangChuan



\### Week 1 Goal



Build the first playable prototype foundation for \*WangChuan\*, including a basic playable map, a custom C++ player character, third-person movement, an interaction framework, memory fragments, and a simple memory journal prototype.



\---



\## Day 1 - Project Setup and Prototype Map



\*\*Date:\*\* 6/18



\* Defined the initial theme and gameplay direction.

\* Set up the development environment with Unreal Engine 5.4 and Visual Studio 2022.

\* Created the `WangChuan` Unreal project.

\* Learned the basic editor workflow, including Content Browser, Levels, Actors, and Play testing.

\* Created a custom project folder structure under `Content/WangChuan` to separate project assets from Unreal template assets.

\* Created `Prototype\_Map` and set it as the default startup map.

\* Built a simple prototype scene using basic geometry:



&#x20; \* Ground

&#x20; \* River

&#x20; \* Bridge

&#x20; \* Rocks

\* Adjusted lighting and placed `PlayerStart`.

\* Successfully tested the map in Play mode.

\* Created the first custom C++ character class as the first step toward moving away from the default template character.



\---



\## Day 2 - Custom Player Character, Camera, and Input



\*\*Date:\*\* 6/19



\* Studied the Epic Third Person template character structure.

\* Created `AWCCharacter` in C++.

\* Created `BP\_PlayerCharacter` as a Blueprint child of `AWCCharacter`.

\* Established the C++ to Blueprint character workflow.

\* Added third-person camera components:



&#x20; \* `CameraBoom` using `USpringArmComponent`

&#x20; \* `FollowCamera` using `UCameraComponent`

\* Configured camera attachment, target arm length, and pawn control rotation.

\* Created `BP\_GameMode`.

\* Set `BP\_PlayerCharacter` as the default pawn class.

\* Configured `Prototype\_Map` to use the custom game mode.

\* Assigned `SKM\_Manny` as the player mesh.

\* Adjusted mesh transform.

\* Assigned `ABP\_Manny` for basic locomotion animation.

\* Connected Enhanced Input:



&#x20; \* `IMC\_Default`

&#x20; \* `IA\_Move`

&#x20; \* `IA\_Look`

&#x20; \* `IA\_Jump`

\* Loaded the input mapping context in `BeginPlay`.

\* Bound movement, camera look, and jump in `SetupPlayerInputComponent`.

\* Assigned input action assets inside `BP\_PlayerCharacter`.

\* Successfully implemented:



&#x20; \* WASD movement

&#x20; \* Mouse look

&#x20; \* Spacebar jump



\*\*Future Polish:\*\*



\* Improve turn-in-place animation.



\---



\## Day 3 - Interaction Prototype



\*\*Date:\*\* 6/20



\* Created the first gameplay object:



&#x20; \* `AInteractionStone` in C++

&#x20; \* `BP\_InteractionStone` in Blueprint

\* Placed the interaction stone in `Prototype\_Map`.

\* Added and configured a static mesh.

\* Created the first gameplay interaction function:



&#x20; \* `Interact()`

\* Added C++ debug output for interaction testing.

\* Added an interaction range using `USphereComponent`.

\* Set the interaction radius to 200 cm.

\* Implemented overlap events:



&#x20; \* `OnPlayerEnter()`

&#x20; \* `OnPlayerExit()`

\* Bound begin overlap and end overlap events.

\* Created `IA\_Interact`.

\* Bound the E key in `IMC\_Default`.

\* Assigned the interact input action in `BP\_PlayerCharacter`.

\* Bound interact input in `SetupPlayerInputComponent`.

\* Successfully tested player-to-world interaction using `CurrentStone`.



\*\*Note:\*\*

This was still a prototype implementation. It was later refactored into a more flexible `CurrentInteractable` system.



\---



\## Day 4 - Interaction Refactor



\*\*Date:\*\* 6/21



\* Created the interaction interface:



&#x20; \* `UInteractable`

&#x20; \* `IInteractable`

\* Refactored `AInteractionStone` to implement `IInteractable`.

\* Overrode `Interact()` through the interface.

\* Added a pure virtual interaction function.

\* Refactored the player character:



&#x20; \* Removed `CurrentStone`

&#x20; \* Added `CurrentInteractable`

\* Reduced direct dependency between the player character and specific interactable object types.

\* Verified that overlap detection, interactable assignment, E key interaction, and `Interact()` all worked correctly after the refactor.



\---



\## Day 5 - Memory Fragment Prototype



\*\*Date:\*\* 6/22



\* Created `AMemoryFragment` in C++.

\* Implemented the memory fragment as an interactable actor.

\* Connected memory fragments to the existing interaction system:



&#x20; \* `IInteractable`

&#x20; \* `Interact()`

&#x20; \* Overlap detection

&#x20; \* Player enter and exit logic

&#x20; \* E key interaction

\* Added memory fragment data:



&#x20; \* `FragmentID`

&#x20; \* `FragmentText`

\* Configured three separate memory fragments through Blueprint.

\* Created a simple collection system using:



&#x20; \* `TSet<int32> CollectedFragments`

&#x20; \* `Add(FragmentID)`

&#x20; \* Fragment count tracking

\* Displayed each fragment's text when collected.

\* Destroyed each fragment after collection.

\* Implemented complete memory unlock after collecting all three fragments.

\* Added the first complete memory narrative: \*\*The Quiet Child\*\*.

\* Connected the world theme to gameplay for the first time.

\* Established the first narrative prototype where a side character's regret reflects the larger theme of the main character's journey.



\---



\## Day 6 - Interaction Prompt System and GitHub Workflow



\*\*Date:\*\* 6/23



\* Extended the interaction interface with:



&#x20; \* `GetInteractionPrompt()`

\* Implemented interaction prompts for:



&#x20; \* `InteractionStone`

&#x20; \* `MemoryFragment`

\* Added player-side prompt functions:



&#x20; \* `CurrentPrompt`

&#x20; \* `ShowInteractionPrompt()`

&#x20; \* `HideInteractionPrompt()`

\* Implemented contextual interaction feedback:



&#x20; \* `\[E] Investigate` near the stone

&#x20; \* `\[E] Collect Memory` near memory fragments

\* Made prompts disappear automatically when leaving the interaction range.

\* Improved system separation:



&#x20; \* Interactable objects only return prompt text.

&#x20; \* Player character handles prompt display.

\* Set up Git and GitHub workflow.

\* Created the first remote repository and pushed the project.

\* Established version control for future rollback, iteration, and portfolio presentation.



\---



\## Day 7 - Memory Journal Prototype and Week 1 Wrap-Up



\*\*Date:\*\* 6/24



\* Created `IA\_Journal`.

\* Bound the J key in `IMC\_Default`.

\* Assigned `JournalAction` in `BP\_PlayerCharacter`.

\* Bound `ShowMemoryJournal()` in `WCCharacter`.

\* Implemented a simple memory journal prototype.

\* Added journal input logic:



&#x20; \* Press J to view memory progress.

&#x20; \* Show `0 / 3` before collecting fragments.

&#x20; \* Show `1 / 3` and `2 / 3` during collection.

&#x20; \* Show `3 / 3` after completion.

&#x20; \* Show unlocked status when all fragments are collected.

\* Added memory review functionality.

\* Allowed the complete memory \*\*The Quiet Child\*\* to be reviewed after unlocking.

\* Created the first full gameplay loop:



```text

Explore the map

→ Discover interactable objects

→ Approach and see interaction prompts

→ Press E to interact

→ Collect memory fragments

→ Unlock the complete memory

→ Press J to review the memory journal

```



\---



\## Current Technical TODOs



\* Some memory text is still hardcoded in C++.

\* Future memory content should be moved into Blueprint variables, DataAssets, or DataTables.

\* The current journal only supports one memory: \*\*The Quiet Child\*\*.

\* The journal should later become a general memory journal system.

\* Future memory data should include:



&#x20; \* `MemoryID`

&#x20; \* `MemoryTitle`

&#x20; \* `FragmentTexts`

&#x20; \* `CompleteText`

&#x20; \* `UnlockState`

\* Adding new NPC or ghost memory lines should not require modifying `WCCharacter`.

\* Future memory content should be data-driven.

\* Formal UI has not been implemented yet.

\* Current debug messages should eventually be replaced with UMG widgets.



\---



\## Week 1 Result



By the end of Week 1, the project includes:



\* A playable prototype map.

\* A custom C++ player character.

\* Third-person movement and camera.

\* Enhanced Input integration.

\* Custom GameMode setup.

\* A reusable interaction interface.

\* Interactable world objects.

\* Context-sensitive interaction prompts.

\* Memory fragment collection.

\* Complete memory unlock logic.

\* A basic memory journal prototype.

\* Git and GitHub version control workflow.



Week 1 successfully established the first playable foundation for \*WangChuan\* and created a small but complete gameplay loop centered on exploration, interaction, memory collection, and narrative discovery.

