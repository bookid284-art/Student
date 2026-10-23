# The Inner Room

Xu Zhixia is a boarding student in the last year of high school. Xu Guifang, a single mother, calls every night for the scores, locks the phone, and has a camera in the dorm room. Classmates mock appearance, state of mind, and grades, demand to be treated, and refuse to share a desk. Teacher Liang thinks this student is bright and cannot forgive the scores.

Under that pressure the student hallucinates a second set of rooms, where the malice has a body. There is no combat. Earplugs, a lens cloth, a bottle of soda, and a light that has not gone out are only for stalling and slipping past. Mr. Zhou, retired from the infirmary, explains the two doors. One is opened by hand, and an inner light leads out of the rooms. The other is shut from inside a closet, and the student stays.

The outline above is the spine. The clear also reads what other people wrote down, and returns three objects that do not belong in these rooms. Those steps gate the empty desk, the office, the infirmary, and Zhou's second conversation.

Read, double back, and hide, and one clear is meant to finish within about three to four hours. Skipping the sentences makes it shorter. This environment has no Unreal Editor, so that length was not timed in Play.

## Required side threads

- Warden Wu's three duty logs are in the south duty room. The lens cloth stays in the drawer until they are read. He does not cover the camera.
- Jiang Wan left four notes: by the empty desk, in the hall, under the shop counter, and in a locker. She almost sat there, then did not. All four are required before sitting. This is not a romance.
- The coin in the dorm tin only opens the shop shutter. The treat-ledger has to be pinned on the hall board. The coin is not another round of treating anyone.
- Liang's note opens the grade office, not the infirmary. Three meeting pages are inside, and four notices hang in the north corridor. The infirmary opens after those are read.
- Zhou withholds the two doors at first. Lin Xiaoman has already graduated and is not in the building. Her pencil case, expired bus pass, and timetable go on his tray before he finishes. Returning them does not rescue her.
- Three of the student's own essays can be left on the bench in the lit room. Doing so adds a line to the leaving ending. Leaving them behind still allows either ending. The essays are not a ticket.

A second bottle is in the shop. A spare cell is in the infirmary. The battery feeds the small light used to make figures step back. It is not the light of the ending.

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
| Leave | Zhou's second conversation, sitting at the empty desk, handing the paper to the office door, earplugs off, and the switch thrown by hand | Morning is ordinary. Mother, teacher, and class are still there. The light does not delete them |
| Shut in | Zhou's second conversation, then closing the closet from the inside | The quiet is real. So is the paper that will never be handed in |

The duty logs, four notes, pinned ledger, three meeting pages, four notices, and three returned objects all come before that second conversation. Full pressure still does not create another ending.

## Code and art

- Rooms use engine basic shapes. The cracked mirror and the score sheet are drawn at runtime.
- The faceless classmates use the Mannequin already in this project.
- Chinese text uses the bundled Droid Sans Fallback font (Apache 2.0, `Content/NightSchool/Fonts/`).
- Gameplay code lives in `Source/Student/NightSchool/`.
- The existing first-person movement and `UStudentInteractionComponent` are what the hallucination uses.

On Play, the game instance starts the story. The template map's game mode does not need to be reassigned.
