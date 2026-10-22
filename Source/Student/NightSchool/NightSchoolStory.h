#pragma once

#include "CoreMinimal.h"
#include "NightSchoolTypes.h"

/** 全部玩家可见中文。世界里的牌子只放短标题，长文由界面展开。 */
namespace NightSchoolStory
{
	const TCHAR* NoticeTitle();
	const TCHAR* NoticeBody();
	const TCHAR* SignInTitle();
	const TCHAR* SignInBody();
	const TCHAR* EssayLinTitle();
	const TCHAR* EssayLinBody();
	const TCHAR* EssayZhaoTitle();
	const TCHAR* EssayZhaoBody();
	const TCHAR* EssayZhouTitle();
	const TCHAR* EssayZhouBody();
	const TCHAR* KeyTitle();
	const TCHAR* KeyBody();
	const TCHAR* ConfessionTitle();
	const TCHAR* ConfessionBody();
	const TCHAR* CandleTitle();
	const TCHAR* CandleBody();
	const TCHAR* ScarfTitle();
	const TCHAR* ScarfBody();
	const TCHAR* BatteryTitle();
	const TCHAR* BatteryBody();
	const TCHAR* NameTrueTitle();
	const TCHAR* NameTrueBody();
	const TCHAR* NameWrongTitle();
	const TCHAR* NameWrongBody();
	const TCHAR* ScarfHookTitle();
	const TCHAR* ScarfHookBody();
	const TCHAR* RegisterTitle();
	const TCHAR* RegisterBody();
	const TCHAR* LastPaperTitle();
	const TCHAR* LastPaperBody();

	const TCHAR* LockedOffice();
	const TCHAR* LockedAuditorium();
	const TCHAR* LockedBoiler();
	const TCHAR* LockedGarden();
	const TCHAR* NeedPublication();
	const TCHAR* NeedBurn();
	const TCHAR* NeedStay();
	const TCHAR* NeedReply();

	const TCHAR* EndingTitle(ENightEnding Ending, bool bFromFear);
	const TCHAR* EndingBody(ENightEnding Ending, bool bFromFear);

	const TCHAR* NameLabel(ENightName Name);
}
