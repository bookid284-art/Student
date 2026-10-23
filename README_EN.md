# Huike / The Recitation

On New Year's Eve, Chinese-literature student Chen Wangshu enters a teaching building scheduled for demolition. In 1967 it housed the Wangjiang night school. The register was ordered burned. Nineteen-year-old Lin Qiuhe never finished teaching her mother how to sign her own name.

This is not a combat game. You read essays, a key, a margin note, a blackboard, and the register, then decide where those names go. All six endings leave the same kind of question: when you record someone, do you also turn her into something else?

## Play

Open `Student.uproject` with the Unreal Engine 5.7 install already associated with this project, then press Play. The night school is built in code above the template level. No extra assets or tools need to be downloaded.

- WASD move, mouse look
- Shift sprint, Ctrl crouch
- E interact, F flashlight, Q close a document
- R on an ending screen restarts the night

The flashlight pushes the miswritten figures back. Crouching makes them lose you. A touch erases one remembered name; rereading writes it back. If composure empties, the ending is Nameless.

## Endings

| Choice | Requires | The question |
| --- | --- | --- |
| Publish | Register, three students, your own name | Does a memorial turn her into a ghost again? |
| Burn | Register and the red candle | Who does silence protect? |
| Stay | Those names, and the red scarf returned to the front row | If the lesson needs someone to remain, is that person still living their own life? |
| Reply | Register, unfinished note, the professor's confession | Does the haunting leave once the living finish the class? |
| Half a page | Read something, then leave by the front gate | Will you leave her name misspelled because it reads better? |
| Nameless | Leave without reading anyone, or let fear erase memory | The recorder can be erased too |

## Code and art

- Rooms are built from engine basic shapes. Portraits and the register cover are drawn at runtime.
- The miswritten figures use the Mannequin already in this project.
- Chinese text uses the bundled Droid Sans Fallback font (Apache 2.0, `Content/NightSchool/Fonts/`).
- Gameplay code lives in `Source/Student/NightSchool/`.
- The existing first-person movement and `UStudentInteractionComponent` are what the night uses.

On Play, the game instance starts the story. The template map's game mode does not need to be reassigned.
