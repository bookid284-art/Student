# The Inner Room

Xu Zhixia is a boarding science student in the last year of high school. Xu Guifang, a single mother, calls every night for the scores, locks the phone, and has a camera in the dorm room. Classmates mock appearance, state of mind, and grades, demand to be treated, and refuse to share a desk. Teacher Liang thinks he is bright and cannot forgive the scores.

Under that pressure he hallucinates a second set of rooms, where the malice has a body. There is no combat. Earplugs, a lens cloth, a bottle of soda, and a light that has not gone out are only for stalling and slipping past. Mr. Zhou, retired from the infirmary, explains the two doors. One is opened by hand, and an inner light leads out of the rooms. The other is shut from inside a closet, and he stays.

The camera is over the right shoulder and stays close. Field of view is narrow. The body fills the bottom of the frame, and the view does not show behind him until he turns. Fear pulls that camera in. It does not open a wider shot, and it does not add an ending. The body is the same uniformed stand-in used in the light room, without a face. The figure in the light is still the first time that face is there to be looked at. The first time a room is entered, code sweeps a camera around it. Tab skips that look. Hold Q to lean left and E to lean right. Leaning moves the camera, not the body, so a ghost still judges where he is standing.

The clear is six chapters. Each one ends by stamping a seal before the next door accepts you. Every room has a red ink pad. Press G there to mark your place. Rings have to be turned while someone is walking toward you. Each set lasts eighty seconds. If the rings are still wrong when the time runs out, you return to the last ink pad. That return is not a third ending. Two of the rings are on the old path: the coin box will not open until the dorm rings match digits split across the antechamber, the laundry, and the south walk; the office will not take the paper until the hall rings match digits split across the classroom, the call room, and the shop. The south walk, the tool room, the east porch, the bike shed, and the old reading room are side exploration. Four night-study pages can be left on a table there. Leaving them does not change either door. Read, double back, fail a few rings, and walk the side wing, and one clear is meant to take about ten hours. Skipping the sentences and the side rooms makes it shorter. This environment has no Unreal Editor, so that length was not timed in Play.

## Six chapters

1. Missed calls. The phone, three transcripts, the duty logs, and the lens cloth. The seal goes on the antechamber board before the hall opens.
2. The empty seat. Jiang Wan's four notes and three class pages. After sitting, stamp the seal in the duty room.
3. Treating. The coin opens the shop, three receipts sit in the back room, and the ledger is pinned in the hall. Stamp the seal in the call room before the office takes the paper.
4. The unforgiven score. Three margin notes, Liang's letter, and three meeting pages. Stamp the seal back on the empty seat before the long board opens.
5. The board. Four notices, then four more. Return the seal to the covered camera before the infirmary opens.
6. Two doors. Three old charts, then Zhou in two conversations. Look at yourself in the light, then throw the switch or shut the closet from inside.

Three essays can still be left on the bench. Doing so adds a line to the leaving ending. Either ending can be reached without them.

The original six rings are in the call room, classroom, shop, office, long board, and infirmary. The coin box and the hall add two more on the main path, and the bike shed and the tool room add two that are not required to finish. Digits are written on walls, not on the wheels. Press G to turn the three wheels, then confirm. Someone walks the room while you do it. Eighty seconds, then you are sent back to the last ink pad. Hold Q or E against a wall to lean the camera out and read the digits.

Jiang Wan's notes are not a romance. She almost sat there, then did not. The coin only opens the shop. Lin Xiaoman has already graduated; returning her pencil case, bus pass, and timetable does not rescue her. A second bottle is in the shop, and a spare cell is in the infirmary. The battery feeds the small light used to make figures step back.

## Play

Open `Student.uproject` with the Unreal Engine 5.7 install already associated with this project, then press Play. The rooms are built in code above the template level. No extra assets need to be downloaded.

- WASD move, mouse look
- Shift sprint, Ctrl crouch
- Hold Q to lean left, hold E to lean right
- G interact, F the small light, Tab close a note or skip the first look around a room
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
- The uniformed figure in the lit room is the static mesh from `Tools/Protagonist/generate.py`. Until that FBX is imported, the same spot is built from engine boxes in the same colors. The body that walks is that same stand-in, seen from behind, with no eyes and no mouth. Import steps are in `Tools/Protagonist/README.md`.
- Gameplay code lives in `Source/Student/NightSchool/`.
- `UStudentInteractionComponent` still traces from the camera. The camera is no longer in the head.

On Play, the game instance starts the story. The template map's game mode does not need to be reassigned.
