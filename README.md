# The Inner Room

A close third-person psychological horror game built in Unreal Engine 5. The player is a boarding high school student under academic pressure, and as the story unfolds, they begin to encounter a second layer of rooms behind the ordinary campus: a place where dread has form, silence is a weapon, and every clue matters.

This project is a narrative horror experience with exploration, environmental storytelling, and a structured progression system driven by chapter seals, notices, and room-based puzzle logic.

## Story overview

Xu Zhixia is a top-track science student in his final year of high school. His single mother, Xu Guifang, calls every day to check on his grades, and the pressure of school, family expectations, and isolation begins to invade every space in his life. In the dormitory, the ordinary walls are no longer neutral. The hallways hold a hidden logic, and the ghosts of fear and memory begin to move in a pattern of their own.

The game does not rely on combat. Instead, it emphasizes tension, observation, and careful movement: turning the lights, listening for clues, leaning around corners, and choosing how to confront the truth behind the rooms.

## Gameplay

The camera sits on the right shoulder, close and narrow, so the body blocks the lower view and nothing behind him is visible until he turns. There is no face on that body. The only time he looks at a face of himself is the figure already standing in the light. The story is still six chapters, each with a required seal. Two more rings sit on the path that was already there: one on the coin box, one in the hall before the office takes the paper. A south walk, a tool room, and an east wing past the store are side exploration. They do not add a chapter or a third ending. Reading, backtracking, and walking those wings is meant to take about ten hours. Rushing the six doors is shorter. This environment has no Unreal Editor, so that length was not timed in Play.

### Core mechanics
- WASD to move
- Mouse to look around
- Shift to sprint
- Ctrl to crouch
- Q to lean left while near a wall
- E to lean right while near a wall
- G to interact
- F to light the fading glow
- Tab to close text or skip the room-introduction camera
- R to replay the ending sequence

### Survival and traversal
The light can drive back the faceless classmates. Crouching can cause them to lose sight of the player. Soda can distract them for roughly 18 seconds. These actions are not used as attacks; they are tools to help the player escape, read the environment, and survive the pressure of the space.

## Chapters

1. Missed calls. The phone, three call transcripts, duty records, and the lens cloth. The chapter seal is placed in the front hall before the corridor opens.
2. Empty seat. Jiang Wan's notes and class records. After sitting down, return the seal to the duty room.
3. Treating. The coin opens the shop, receipts and records are found in the back room, and the chapter seal is placed before the office accepts the papers.
4. The unforgiven score. Three grading notes, Liang's letter, and several meeting pages. Return the seal to the empty seat before the long wall of notices opens.
5. The board. Four notices and the tall board. Return the seal to the covered lens before the infirmary opens.
6. Two doors. Three old medical records, and the conversation with Zhou in two parts. Look into the light and confront the self behind it, then choose whether to leave or stay hidden.

## Endings

The ending depends on the path taken through the final sequence and the clues gathered across the story.

| Ending | Condition | What remains |
| --- | --- | --- |
| Leave | Finish the final conversation, sit at the empty seat, hand in the paper, keep the earplugs off, look clearly into the light, and turn the switch by hand | Morning remains ordinary, but the truth does not disappear |
| Stay shut in | Finish the final conversation and close the cabinet from within | The silence is real, and the unfinished paper stays behind |

## How to run

Open `Student.uproject` in Unreal Engine 5.7 and press Play. The project already includes the base level used for the experience and does not require extra map setup or asset downloads.

## Project notes

- Rooms are built from engine primitives such as cubes, spheres, and cylinders.
- The cracked mirror and test papers are drawn at runtime.
- The faceless classmates use the existing Mannequin asset in the project.
- Chinese text uses the bundled Droid Sans Fallback font (Apache 2.0, located under `Content/NightSchool/Fonts/`).
- The figure in the light is generated from `Tools/Protagonist/generate.py` as a static mesh. If the FBX is not imported yet, the same location is built from boxes in matching colors.
- Gameplay code lives in `Source/Student/NightSchool/`.
- The played body is the same primitive stand-in, seen from behind, with no face. `UStudentInteractionComponent` still traces from the camera.

When Play starts, the game instance begins automatically. No additional GameMode changes are required for the template map.

## Repository layout notes

- `README_EN.md` contains the English version of the project documentation.
- `Tools/Protagonist/README.md` documents the generation of the static character mesh used in the final light scene.

## License and attribution

This project uses Unreal Engine and the bundled font assets from the project itself. Please refer to the repository files and engine package terms for any additional licensing details.

---

English version of the project README.
