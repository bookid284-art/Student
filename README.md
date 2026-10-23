# The Recitation

A first-person narrative horror experience built in Unreal Engine 5.

On New Year's Eve, Chen Wangshu—a Chinese literature student at a provincial teacher's college—returns to an old teaching building scheduled for demolition. Inside, the past refuses to stay buried. The building once housed the Wangjiang night school, a chapter of history that was meant to be burned away, but its names, notes, objects, and silences still echo through the halls.

The Recitation is a short-form horror game about reading, memory, and the moral burden of preserving other people's lives in the form of records. It is not a combat game. There are no enemies to defeat in the traditional sense. Instead, the player moves through a haunted school, interprets fragments of lost history, and decides what should be remembered, what should be forgotten, and what should never be written down again.

## Story Premise

At the center of the experience is a simple but disturbing question: when you record a life, do you rescue it—or do you trap it?

As the night unfolds, the player discovers essays, names, margins, blackboards, the school register, and personal belongings that reveal the hidden story of the Wangjiang night school. Each discovered clue and each choice of what to recover or leave behind alters the ending. Memory is unreliable. Fear distorts truth. The school remembers differently from the people who once lived within it.

## Key Features

- First-person exploration in a handcrafted night-school environment
- Narrative-driven horror centered on reading, investigation, and memory
- Dynamic runtime generation of the school space without a custom level setup
- A flashlight mechanic that pushes back "miswritten" apparitions
- Stealth-like avoidance through crouching and line-of-sight disruption
- Multiple endings shaped by evidence, memory, and player decisions
- Built around a custom story system designed for procedural reconstruction of the nightmare

## Gameplay

Open `Student.uproject` in Unreal Engine 5.7 and press Play.

The night school is generated above the template level at runtime, so no extra assets, downloads, or custom map conversion are required.

### Controls

- W/A/S/D — Move
- Mouse — Look around
- Shift — Sprint
- Ctrl — Crouch
- E — Interact
- F — Toggle flashlight
- Q — Close a document
- R — Restart the night from an ending screen

### Core Mechanics

- Read documents, names, notes, and classroom artifacts to uncover the story.
- Use the flashlight to drive back the "miswritten" figures.
- Crouch to break line of sight and avoid being noticed.
- Encountering a figure may cause you to forget a remembered name; rereading the relevant page can restore it.
- Fear can erode your sense of self and push the story toward a nameless ending.
- The experience is assembled through `UNightSchoolGameInstance`, which creates the nighttime school without replacing the base template level's `GameMode`.

## Endings

The project contains six possible endings, each shaped by what the player reads, remembers, and recovers.

| Ending | Requirement | Core Question |
| --- | --- | --- |
| Publish | Register, three students, and your own name | Does a memorial turn a person into a ghost again? |
| Burn | Register and the red candle | Who does silence protect? |
| Stay | The names above, plus the red scarf returned to the front row | If a lesson must leave someone behind, is that person still themselves? |
| Reply | Register, unfinished draft, and a read note | Does the haunting leave once the living finish the class? |
| Half a Page | Read a little, then leave through the front gate | Will you leave her name misspelled because it reads better that way? |
| Nameless | Leave without reading anyone, or let fear erase memory | The recorder can also be erased |

## Project Structure

- `Source/Student/` — core Unreal project code and gameplay systems
- `Source/Student/NightSchool/` — custom horror narrative logic, world generation, and story systems
- `Content/NightSchool/` — project-specific art, UI, and story assets
- `Config/` — project configuration files
- `Student.uproject` — entry point for the project

### Main Systems

The gameplay logic is primarily organized under `Source/Student/NightSchool/` and includes:

- `UNightSchoolGameInstance` — builds the experience when the level loads
- `ANightSchoolDirector` — world management, narrative progression, endings, objects, and fear state
- `ANightSchoolCharacter` — character logic for the playable night-school experience
- `ANightSchoolProp` — interactive story objects and environmental props
- `ANightSchoolApparition` — ghostlike figures that stalk the hallways
- `ANightSchoolStory` and related types — story data, state management, and ending conditions

The base first-person movement and interaction frameworks remain in the broader Unreal project, while the custom night-school narrative layer is layered on top.

## Technical Notes

- The environment is built from engine primitives such as cubes, spheres, and cylinders.
- Many props and visual elements are generated at runtime rather than imported as complete static scenes.
- The "miswritten" figures use the existing project Mannequin skeleton and animation assets.
- Chinese text uses the bundled `Droid Sans Fallback` font under `Content/NightSchool/Fonts/` (Apache 2.0).
- The experience starts automatically when the level loads; there is no need to reassign the template `GameMode`.

## Development Philosophy

The Recitation is a compact Unreal Engine horror prototype designed to explore how a game can deliver dread through interpretation rather than combat. It blends environmental storytelling, memory-based progression, and morally charged endings into a short playable experience that emphasizes atmosphere, context, and the act of recording a life.

## License and Attribution

The project includes font resources under the Apache 2.0 license, and the game uses standard Unreal Engine assets and generated content. Please check the repository files themselves for additional licensing and attribution details where applicable.

## Status

Prototype / Narrative Horror Experiment
