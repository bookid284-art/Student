# The Recitation

On New Year's Eve, Chen Wangshu, a Chinese literature student at a provincial teachers' college, enters an old teaching building that is about to be demolished. The building once housed Wangjiang Night School in 1967. A register was ordered to be destroyed, but the names inside remain. As the player explores the abandoned classrooms, they uncover essays, a key, margin notes, a blackboard, a red candle, and a register, and decide where each name belongs.

This is not a combat game. It is a first-person narrative horror experience focused on reading, memory, and the ethics of recording other people's lives. The story unfolds through environmental storytelling: the player examines objects, leaves traces, and faces a difficult question—when you write down another person's name, are you preserving them, or erasing them?

The experience is built in Unreal Engine 5 and is centered on a handcrafted night-school space created at runtime. The player is not fighting enemies; instead, they survive the night by reading carefully, using light, and managing fear.

## Gameplay

Open `Student.uproject` with Unreal Engine 5.7 and press Play. The night school is generated above the template level, so no additional setup or asset downloads are required.

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
- Crouch to break line of sight and avoid being noticed.
- Encountering one of the figures causes you to forget a remembered name, but reading the relevant page again can restore it.
- When fear overwhelms you, the story can collapse into a nameless ending.

The game uses a custom world-building flow: when the game starts, a `UNightSchoolGameInstance` creates the setting and spawns the night school without replacing the template level's GameMode.

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
- `Source/Student/NightSchool/`: custom horror narrative systems
- `Content/NightSchool/`: project-specific art, UI, and story assets
- `Config/`: project configuration files
- `Student.uproject`: project entry point

### Main gameplay systems

The custom game logic lives in the `NightSchool` module under `Source/Student/NightSchool/` and includes systems such as:

- `UNightSchoolGameInstance`: builds the experience when the level loads
- `ANightSchoolDirector`: manages the world, story progression, endings, objects, and fear state
- `ANightSchoolCharacter`: custom avatar logic used for the playable night-school experience
- `ANightSchoolProp`: interactivity for story items and props
- `ANightSchoolApparition`: ghost-like figures that stalk the school
- `ANightSchoolStory` and related types: story logic, state, and ending conditions

The base first-person movement and interaction systems remain in the broader project, while the night-school story logic is layered on top of them.

## Technical details

- The environment is built from engine primitives such as cubes, spheres, and cylinders.
- Many props and visual elements are generated at runtime rather than imported as full static scenes.
- The "miswritten" figures use the existing project Mannequin skeleton and animation assets.
- Chinese text uses the bundled `Droid Sans Fallback` font (Apache 2.0), located under `Content/NightSchool/Fonts/`.
- The game instance starts the story automatically when the level loads; no custom GameMode change is required.

## Notes

This project is a compact Unreal Engine horror game prototype that blends exploration, environmental storytelling, and choice-driven endings. It is designed as a playable narrative experiment rather than a large-scale production project.

## License and attribution

The project includes font resources under the Apache 2.0 license, and the game uses standard Unreal Engine assets and generated project content. Please check the repository files themselves for licensing details on any third-party content.
