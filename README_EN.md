# The Inner Room

Xu Zhixia is a boarding student in the last year of high school. Xu Guifang, a single mother, calls every night for the scores, locks the phone, and has a camera in the dorm room. Classmates mock appearance, state of mind, and grades, demand to be treated, and refuse to share a desk. Teacher Liang thinks this student is bright and cannot forgive the scores.

Under that pressure he hallucinates a second set of rooms, where the malice has a body. There is no combat. Earplugs, a lens cloth, a bottle of soda, and a light that has not gone out are only for stalling and slipping past. Mr. Zhou, retired from the infirmary, explains the two doors. One is opened by hand, and an inner light leads out of the rooms. The other is shut from inside a closet, and the student stays.

## Play

Open `Student.uproject` with the Unreal Engine 5.7 install already associated with this project, then press Play. The rooms are built in code above the template level. No extra assets need to be downloaded.

- WASD move, mouse look
- Shift sprint, Ctrl crouch
- E interact, F the small light, Q close a note
- R on an ending screen walks the corridor again

The light makes the faceless classmates step back. Crouching makes them lose you. Soda draws them for about eighteen seconds. None of this causes harm. Full pressure does not end the game. Earplugs shorten their sight, and they have to come off before the switch. The cloth only covers the dorm camera.

## Endings

| Choice | Requires | What remains |
| --- | --- | --- |
| Leave | Zhou's guidance, sitting at the empty desk, handing the paper to the office door, earplugs off, and the switch thrown by hand | Morning is ordinary. Mother, teacher, and class are still there. The light does not delete them |
| Shut in | Zhou's guidance, then closing the closet from the inside | The quiet is real. So is the paper that will never be handed in |

## Code and art

- Rooms use engine basic shapes. The cracked mirror and the score sheet are drawn at runtime.
- The faceless classmates use the Mannequin already in this project.
- Chinese text uses the bundled Droid Sans Fallback font (Apache 2.0, `Content/NightSchool/Fonts/`).
- Gameplay code lives in `Source/Student/NightSchool/`.
- The existing first-person movement and `UStudentInteractionComponent` are what the hallucination uses.

On Play, the game instance starts the story. The template map's game mode does not need to be reassigned.
