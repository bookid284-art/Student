# The Inner Room

Xu Zhixia is a boarding science student in the last year of high school. Xu Guifang, a single mother, calls every night for the scores, locks the phone, and has a camera in the dorm room. Classmates mock appearance, state of mind, and grades, demand to be treated, and refuse to share a desk. Teacher Liang thinks he is bright and cannot forgive the scores.

Under that pressure he hallucinates a second set of rooms, where the malice has a body. There is no combat. Earplugs, a lens cloth, a bottle of soda, and a light that has not gone out are only for stalling and slipping past. Mr. Zhou, retired from the infirmary, explains the two doors. One is opened by hand, and an inner light leads out of the rooms. The other is shut from inside a closet, and he stays.

Play stays in the first-person view already in this project. The first time a room is entered, code sweeps a camera around it. Q skips that look.

The clear is six chapters. Each one ends by stamping a seal before the next door accepts you. Read, double back, and hide, and one clear is meant to take about six hours. Skipping the sentences makes it shorter. This environment has no Unreal Editor, so that length was not timed in Play.

## Six chapters

1. Missed calls. The phone, three transcripts, the duty logs, and the lens cloth. The seal goes on the antechamber board before the hall opens.
2. The empty seat. Jiang Wan's four notes and three class pages. After sitting, stamp the seal in the duty room.
3. Treating. The coin opens the shop, three receipts sit in the back room, and the ledger is pinned in the hall. Stamp the seal in the call room before the office takes the paper.
4. The unforgiven score. Three margin notes, Liang's letter, and three meeting pages. Stamp the seal back on the empty seat before the long board opens.
5. The board. Four notices, then four more. Return the seal to the covered camera before the infirmary opens.
6. Two doors. Three old charts, then Zhou in two conversations. Look at yourself in the light, then throw the switch or shut the closet from inside.

Three essays can still be left on the bench. Doing so adds a line to the leaving ending. Either ending can be reached without them.

Jiang Wan's notes are not a romance. She almost sat there, then did not. The coin only opens the shop. Lin Xiaoman has already graduated; returning her pencil case, bus pass, and timetable does not rescue her. A second bottle is in the shop, and a spare cell is in the infirmary. The battery feeds the small light used to make figures step back.

## Play

Open `Student.uproject` with the Unreal Engine 5.7 install already associated with this project, then press Play. The rooms are built in code above the template level. No extra assets need to be downloaded.

- WASD move, mouse look
- Shift sprint, Ctrl crouch
- E interact, F the small light, Q close a note or skip the first look around a room
- R on an ending screen walks the corridor again

The light makes the faceless classmates step back. Crouching makes them lose you. Soda draws them for about eighteen seconds. None of this causes harm. Full pressure does not end the game. Earplugs shorten their sight, and they have to come off before the switch. The cloth only covers the dorm camera.

## Endings

| Choice | Requires | What remains |
| --- | --- | --- |
| Leave | Zhou's second conversation, sitting at the empty desk, handing in the paper, earplugs off, looking at yourself in the light, and the switch thrown by hand | Morning is ordinary. Mother, teacher, and class are still there. The light does not delete them |
| Shut in | Zhou's second conversation, then closing the closet from the inside | The quiet is real. So is the paper that will never be handed in |

The five seals, eight notices, three old charts, and three returned objects all come before that second conversation. Full pressure still does not create another ending.

## Code and art

- Rooms use engine basic shapes. The cracked mirror and the score sheet are drawn at runtime.
- The faceless classmates use the Mannequin already in this project.
- Chinese text uses the bundled Droid Sans Fallback font (Apache 2.0, `Content/NightSchool/Fonts/`).
- The uniformed figure in the lit room is the static mesh from `Tools/Protagonist/generate.py`. Until that FBX is imported, the same spot is built from engine boxes in the same colors. Import steps are in `Tools/Protagonist/README.md`.
- Gameplay code lives in `Source/Student/NightSchool/`.
- The existing first-person movement and `UStudentInteractionComponent` are what the hallucination uses.

On Play, the game instance starts the story. The template map's game mode does not need to be reassigned.
