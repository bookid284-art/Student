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

	const TCHAR* EndingTitle(ENightEnding Ending);
	const TCHAR* EndingBody(ENightEnding Ending);
}
