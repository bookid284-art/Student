# The Inner Room

**The Inner Room** is a narrative psychological horror game built with **Unreal Engine 5.7**. It follows Xu Zhixia, a final-year boarding high school science student trapped under the pressure of academic performance, family expectations, and constant surveillance.

As Xu moves through familiar corridors and classrooms, he begins to encounter a second, distorted version of the school. The game combines close third-person exploration, environmental storytelling, puzzle sequences, and chapter-based progression. It has no combat system; fear comes from observation, limited visibility, and the player’s choices.

## Story

Xu Zhixia is pressured to maintain perfect grades. His mother, Xu Guifang, calls every night to check his scores, while the school and his own fears leave him increasingly isolated.

Under that pressure, Xu hallucinates a parallel school in which hidden malice takes physical form. He must collect evidence, mark chapter seals, solve room-based puzzles, and confront the things he has avoided. The final journey leads to one of two endings: leave, or remain hidden.

## Features

- Close right-shoulder, third-person camera
- Six-chapter narrative progression
- Seals, notices, records, and collectable evidence
- Room-based puzzles and optional clues
- Faceless classroom threats without a combat system
- Sprinting, crouching, wall leaning, and limited light mechanics
- Two meaningful endings
- Runtime-generated geometry and environmental details
- A Blender tool for generating the protagonist’s static mesh

## Controls

| Key | Action |
| --- | --- |
| `WASD` | Move |
| `Mouse` | Look |
| `Shift` | Sprint |
| `Ctrl` | Crouch |
| `Q` | Lean left |
| `E` | Lean right |
| `G` | Interact / place or inspect evidence |
| `F` | Use the small light |
| `Tab` | Close a note or skip an introduction shot |
| `R` | Replay the ending corridor |

Crouching can break visual pursuit, and light can make the faceless classmates step back. A bottle of soda can distract them for roughly 18 seconds. The light and throwing mechanics never inflict damage or end the game.

## Chapters

1. **Missed Calls** — Search the phone, call transcripts, duty records, and lens cloth to open the hall.
2. **The Empty Seat** — Investigate Jiang Wan’s notes and class records, then return the seal to the duty room.
3. **Treating** — Use the coin to enter the shop and collect receipts and records for the office.
4. **The Unforgiven Score** — Examine grading notes, Liang’s letter, and meeting records.
5. **The Board** — Search notices and return the seal to the covered camera.
6. **Two Doors** — Review the old records and Zhou’s final conversation, then choose to leave or shut the closet from within.

The chapter structure is driven by game code, seals, interactions, and room state rather than separate downloaded levels.

## Endings

| Ending | Requirement |
| --- | --- |
| Leave | Complete Zhou’s final conversation, sit at the empty desk, submit the paper, remove the earplugs, look at yourself in the light, and throw the switch by hand |
| Shut In | Complete Zhou’s final conversation and close the closet from inside |

Both endings can be reached without collecting every optional essay. Pressure, exhaustion, and clues affect the narrative presentation but do not create additional endings.

## Getting Started

### Requirements

- Unreal Engine **5.7**
- A launcher or editor installation matching `EngineAssociation` in `Student.uproject`
- Visual Studio or a compatible C++ build toolchain for recompiling source changes

### Run the project

1. Clone or download this repository.
2. Open `Student.uproject` in Unreal Engine 5.7.
3. Allow Unreal Engine to generate project files if prompted.
4. Open the project and press **Play**.

The template map and game instance already contain the required setup. The project does not need extra map reassignment or asset downloads.

```bash
git clone https://github.com/bookid284-art/Student.git
cd Student
```

## Project Structure

```text
.
├── Config/                  Engine and input configuration
├── Content/                 Unreal assets, generated content, and fonts
├── Source/Student/          C++ gameplay modules and NightSchool implementation
├── Tools/                   Blender and asset-generation tools
│   ├── Campus/              Campus mesh-generation utility
│   └── Protagonist/         Xu Zhixia static-mesh generator
├── Student.uproject         Unreal Engine project definition
├── README.md                English project overview
└── README_EN.md             Alternate detailed English documentation
```

## Technical Notes

- Gameplay code is located in `Source/Student/NightSchool/`.
- The project uses a right-shoulder camera and a custom interaction trace from `UStudentInteractionComponent`.
- Rooms and environmental props are primarily assembled from Unreal Engine primitive shapes.
- The cracked mirror and score sheet are drawn at runtime.
- The faceless classmates use the mannequin asset bundled with the project.
- Chinese text uses the bundled **Droid Sans Fallback** font under `Content/NightSchool/Fonts/`.
- The final light-scene figure can be generated as a static mesh with `Tools/Protagonist/generate.py`.

### Generate the protagonist mesh

The generator requires Blender 4.x. From the repository root:

```bash
blender --background --python Tools/Protagonist/generate.py
```

The script exports `SM_XuZhixia.fbx` to `Content/NightSchool/Characters/`. Import it as a **Static Mesh**, without skeletal mesh or animation import. See `Tools/Protagonist/README.md` for import and material details.

## Building and Source Changes

To change gameplay code:

1. Open `Student.uproject` or generate a Visual Studio solution.
2. Modify C++ files under `Source/Student/`.
3. Build the project in Unreal Engine or your IDE.
4. Run the project in the editor and verify chapter state and interactions.

Generated assets and local Unreal Engine folders should not be committed unless they are intentionally part of the project.

## Assets and Licensing

- The project is built with Unreal Engine and may use engine-provided assets.
- `Droid Sans Fallback` is bundled in the project and distributed under the Apache 2.0 license.
- No dedicated license file is currently included in the repository. Check the Unreal Engine and third-party asset terms before redistributing derived builds or commercial products.

## Contributing

Contributions should preserve the existing horror pacing, close-camera behavior, and non-combat design. Before submitting a change:

- Build cleanly in Unreal Engine 5.7.
- Test movement, interaction, chapter progression, and both endings.
- Document any new assets, plugins, generated files, or setup requirements.

## Links

- [Project repository](https://github.com/bookid284-art/Student)
- [Protagonist mesh generator](Tools/Protagonist/README.md)
- [Alternate English README](README_EN.md)
