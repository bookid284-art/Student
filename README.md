# The Recitation

On New Year's Eve, Chen Wangshu, a Chinese literature student at a provincial teacher's college, enters an old teaching building scheduled for demolition. The building once housed the Wangjiang night school, a place that is remembered in fragments, rumors, and erased names. What remains is not a battlefield, but a memory space—one where the past keeps repeating itself through paper, objects, and unfinished lessons.

This is not a combat game. It is a first-person narrative horror experience focused on reading, memory, and the ethics of recording other people's lives. The story unfolds through environmental storytelling, document fragments, object interactions, and a slowly collapsing sense of certainty. The player does not shoot their way through the nightmare—they survive by deciphering what was omitted, remembering what was lost, and deciding what should be preserved.

The experience is built in Unreal Engine 5 and centered on a handcrafted night-school space generated at runtime. The player explores a school suspended in a repeating night, where each discovered document, recovered object, or remembered name can change the ending.

## Gameplay

Open `Student.uproject` with Unreal Engine 5.7 and press Play. The night school is constructed above the template level at runtime, so no additional setup, asset downloads, or custom level conversion are required.

### Controls

- W/A/S/D: move
- Mouse: look around
- Shift: sprint
- Ctrl: crouch
- E: interact
- F: flashlight
- Q: close a document
- R: restart the night from an ending screen

### Core mechanics

- Read documents, names, notes, and classroom artifacts to uncover the story.
- Use the flashlight to push back the "miswritten" apparitions.
- Crouch to break line of sight and avoid detection.
- Encountering one of the figures causes you to forget a remembered name; reading the relevant page again can restore it.
- When fear overwhelms you, the story can collapse into a nameless ending.
- The game's setting is built dynamically by `UNightSchoolGameInstance`, which spawns the night school without replacing the template level's `GameMode`.

## Endings

There are six possible endings, each driven by what the player reads, what they remember, and which objects they recover or return.

| Ending | Requirements | Core question |
| --- | --- | --- |
| Publish | The register, three students, and your own name are all present | Does a memorial turn a person into a ghost again? |
| Burn | The register and the red candle | Who does silence protect? |
| Stay | The names above, plus the red scarf returned to the front row | If a lesson must leave someone behind, is that person still themselves? |
| Reply | The register, unfinished draft, and a read note | Does the haunting leave once the living finish the class? |
| Half a Page | Read a little, then leave through the front gate | Will you leave her name misspelled because it reads better that way? |
| Nameless | Leave without reading anyone, or let fear erase memory | The recorder can also be erased |

## Project structure

- `Source/Student/`: core Unreal project code and gameplay systems
- `Source/Student/NightSchool/`: custom horror narrative systems and runtime world logic
- `Content/NightSchool/`: project-specific art, UI, and story assets
- `Config/`: project configuration files
- `Student.uproject`: project entry point

### Main gameplay systems

The custom game logic is organized under `Source/Student/NightSchool/` and includes systems such as:

- `UNightSchoolGameInstance`: builds the experience when the level loads
- `ANightSchoolDirector`: manages the world, story progression, endings, interactable objects, and fear state
- `ANightSchoolCharacter`: custom avatar logic for the playable night-school experience
- `ANightSchoolProp`: interactivity for story items and environment props
- `ANightSchoolApparition`: ghost-like figures that stalk the school
- `ANightSchoolStory` and related types: story state, narrative logic, and ending conditions

The base first-person movement and general interaction systems remain in the broader Unreal project, while the night-school narrative layer is built on top of them.

## Technical details

- The environment is built from engine primitives such as cubes, spheres, and cylinders.
- Many props and visual elements are generated at runtime rather than imported as full static scenes.
- The "miswritten" figures use the existing project Mannequin skeleton and animation assets.
- Chinese text uses the bundled `Droid Sans Fallback` font (Apache 2.0), located under `Content/NightSchool/Fonts/`.
- The game instance starts the story automatically when the level loads; no custom `GameMode` reassignment is required.

## Notes

This project is a compact Unreal Engine horror game prototype that blends exploration, environmental storytelling, and choice-driven endings. It is designed as a playable narrative experiment rather than a large commercial production, but it contains a complete loop, multiple endings, and a custom systems framework for building haunted-space interactions.

## License and attribution

The project includes font resources under the Apache 2.0 license, and the game uses standard Unreal Engine assets and generated project content. Please check the repository files themselves for license and attribution details where applicable.
