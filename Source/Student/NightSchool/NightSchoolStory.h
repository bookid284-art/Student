#pragma once

#include "CoreMinimal.h"
#include "NightSchoolTypes.h"

/** 玩家能读到的中文。牌子只放短标题，长文由界面展开。 */
namespace NightSchoolStory
{
	const TCHAR* PhoneTitle();
	const TCHAR* PhoneBody();
	const TCHAR* EarplugTitle();
	const TCHAR* EarplugBody();
	const TCHAR* ClothTitle();
	const TCHAR* ClothBody();
	const TCHAR* CameraBody();
	const TCHAR* DeskTitle();
	const TCHAR* DeskBody();
	const TCHAR* ExamTitle();
	const TCHAR* ExamBody();
	const TCHAR* HideScoreTitle();
	const TCHAR* HideScoreBody();
	const TCHAR* HideScoreEmpty();
	const TCHAR* TeacherTitle();
	const TCHAR* TeacherBody();
	const TCHAR* ZhouTitle();
	const TCHAR* ZhouBody();
	const TCHAR* FinalCameraTitle();
	const TCHAR* FinalCameraBody();

	const TCHAR* LockedHall();
	const TCHAR* LockedOffice();
	const TCHAR* LockedInfirmary();
	const TCHAR* LockedLight();
	const TCHAR* LockedCloset();
	const TCHAR* NeedZhou();
	const TCHAR* NeedEarplugsOff();
	const TCHAR* NeedSit();
	const TCHAR* NeedExam();
	const TCHAR* SodaNeed();
	const TCHAR* SodaEarly();

	const TCHAR* DutyTitleA();
	const TCHAR* DutyBodyA();
	const TCHAR* DutyTitleB();
	const TCHAR* DutyBodyB();
	const TCHAR* DutyTitleC();
	const TCHAR* DutyBodyC();
	const TCHAR* ClothEarly();
	const TCHAR* CoinTitle();
	const TCHAR* CoinBody();
	const TCHAR* NoteSeatTitle();
	const TCHAR* NoteSeatBody();
	const TCHAR* NoteHallTitle();
	const TCHAR* NoteHallBody();
	const TCHAR* NoteShopTitle();
	const TCHAR* NoteShopBody();
	const TCHAR* NoteLockerTitle();
	const TCHAR* NoteLockerBody();
	const TCHAR* DeskEarly();
	const TCHAR* LedgerTitle();
	const TCHAR* LedgerBody();
	const TCHAR* LedgerPosted();
	const TCHAR* MinutesSunTitle();
	const TCHAR* MinutesSunBody();
	const TCHAR* MinutesLiangTitle();
	const TCHAR* MinutesLiangBody();
	const TCHAR* MinutesParentTitle();
	const TCHAR* MinutesParentBody();
	const TCHAR* NoticeTitleA();
	const TCHAR* NoticeBodyA();
	const TCHAR* NoticeTitleB();
	const TCHAR* NoticeBodyB();
	const TCHAR* NoticeTitleC();
	const TCHAR* NoticeBodyC();
	const TCHAR* NoticeTitleD();
	const TCHAR* NoticeBodyD();
	const TCHAR* LockerHint();
	const TCHAR* PencilTitle();
	const TCHAR* PencilBody();
	const TCHAR* BusTitle();
	const TCHAR* BusBody();
	const TCHAR* TimeTitle();
	const TCHAR* TimeBody();
	const TCHAR* ZhouIntroTitle();
	const TCHAR* ZhouIntroBody();
	const TCHAR* TrayMissing();
	const TCHAR* TrayDone();
	const TCHAR* EssayTitleA();
	const TCHAR* EssayBodyA();
	const TCHAR* EssayTitleB();
	const TCHAR* EssayBodyB();
	const TCHAR* EssayTitleC();
	const TCHAR* EssayBodyC();
	const TCHAR* EssayBenchBody();
	const TCHAR* EssayCoda();
	const TCHAR* ShopLocked();
	const TCHAR* GradeLocked();
	const TCHAR* ComputerLocked();

	const TCHAR* EndingTitle(ENightEnding Ending);
	const TCHAR* EndingBody(ENightEnding Ending);
}
