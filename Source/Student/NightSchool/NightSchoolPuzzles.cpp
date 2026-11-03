#include "NightSchool/NightSchoolDirector.h"

#include "GameFramework/Character.h"
#include "NightSchool/NightSchoolApparition.h"
#include "NightSchool/NightSchoolCharacter.h"
#include "NightSchool/NightSchoolProp.h"

namespace
{
	const int32 PuzzleCode[10][3] = {
		{6, 1, 4},
		{3, 1, 2},
		{4, 0, 8},
		{7, 2, 5},
		{5, 8, 1},
		{1, 7, 0},
		{2, 6, 9},
		{8, 4, 0},
		{9, 3, 5},
		{1, 5, 7},
	};
	const float PuzzleLimit = 80.f;

	bool ParsePuzzleMark(const FString& Mark, int32& OutIndex, int32* OutSlot)
	{
		if (Mark.Len() < 3 || Mark[0] != TEXT('P'))
		{
			return false;
		}
		int32 Cursor = 1;
		if (Mark[Cursor] < TEXT('0') || Mark[Cursor] > TEXT('9'))
		{
			return false;
		}
		int32 Number = 0;
		while (Cursor < Mark.Len() && Mark[Cursor] >= TEXT('0') && Mark[Cursor] <= TEXT('9'))
		{
			Number = Number * 10 + (Mark[Cursor] - TEXT('0'));
			++Cursor;
		}
		const int32 Index = Number - 1;
		if (Index < 0 || Index > 9 || Cursor >= Mark.Len())
		{
			return false;
		}
		OutIndex = Index;
		if (!OutSlot)
		{
			return true;
		}
		const int32 Slot = Mark[Cursor] - TEXT('A');
		if (Slot < 0 || Slot > 2)
		{
			return false;
		}
		*OutSlot = Slot;
		return true;
	}

	bool IsSharedProp(ENightPropId Id)
	{
		return Id == ENightPropId::InkPad || Id == ENightPropId::Dial || Id == ENightPropId::DialConfirm;
	}
}

ANightSchoolProp* ANightSchoolDirector::FindMarked(FName Marker) const
{
	for (ANightSchoolProp* Prop : Props)
	{
		if (Prop && Prop->GetMarker() == Marker)
		{
			return Prop;
		}
	}
	return nullptr;
}

bool ANightSchoolDirector::IsPuzzleDone(int32 Index) const
{
	switch (Index)
	{
	case 0: return bPuzzle1;
	case 1: return bPuzzle2;
	case 2: return bPuzzle3;
	case 3: return bPuzzle4;
	case 4: return bPuzzle5;
	case 5: return bPuzzle6;
	case 6: return bPuzzle7;
	case 7: return bPuzzle8;
	case 8: return bPuzzle9;
	case 9: return bPuzzle10;
	default: return false;
	}
}

void ANightSchoolDirector::SetPuzzleDone(int32 Index)
{
	switch (Index)
	{
	case 0: bPuzzle1 = true; break;
	case 1: bPuzzle2 = true; break;
	case 2: bPuzzle3 = true; break;
	case 3: bPuzzle4 = true; break;
	case 4: bPuzzle5 = true; break;
	case 5: bPuzzle6 = true; break;
	case 6: bPuzzle7 = true; break;
	case 7: bPuzzle8 = true; break;
	case 8: bPuzzle9 = true; break;
	case 9: bPuzzle10 = true; break;
	default: break;
	}
}

bool ANightSchoolDirector::PuzzleReady(int32 Index) const
{
	switch (Index)
	{
	case 0: return CallCount() >= 3;
	case 1: return bSeal1Set;
	case 2: return bShopOpen;
	case 3: return bOfficeOpen && DraftCount() >= 3;
	case 4: return bAnnexOpen;
	case 5: return ChartCount() >= 3 && bItemsReturned;
	case 6: return bPhoneRead;
	case 7: return bHasExam && bSeal3Set;
	case 8: return bInfirmaryOpen;
	case 9: return true;
	default: return false;
	}
}

const TCHAR* ANightSchoolDirector::PuzzleLockedText(int32 Index) const
{
	switch (Index)
	{
	case 0: return TEXT("三页通话还没读完。环先别转。");
	case 1: return TEXT("第一章的章还没按上。教室的环先别转。");
	case 2: return TEXT("卷帘还关着。这组环在小卖部里面。");
	case 3: return TEXT("三页批注还没看完，或者门还关着。钟先别拨。");
	case 4: return TEXT("长榜的门还关着。");
	case 5: return TEXT("病历和托盘还没做完。药柜先别转。");
	case 6: return TEXT("先看过那只手机。铁盒的环先别转。");
	case 7: return TEXT("第三章的章还没盖，或者卷子还没拿。走廊的环先别转。");
	case 8: return TEXT("校医室还没开。车棚的环先别转。甲在宿舍，乙在长榜，丙在校医室。");
	default: return TEXT("现在还转不了。");
	}
}

void ANightSchoolDirector::CaptureCheckpoint(const FString& RoomName, const FVector& Location)
{
	ANightSchoolCharacter* Current = Player();
	if (!Current)
	{
		return;
	}
	Checkpoint.bValid = true;
	Checkpoint.RoomName = RoomName;
	Checkpoint.Location = Location;
	Checkpoint.View = Current->GetController() ? Current->GetController()->GetControlRotation() : FRotator::ZeroRotator;
	Checkpoint.Battery = Current->GetBattery();
	Checkpoint.Stamina = Current->GetStamina();
	Checkpoint.Fear = Fear;
	Checkpoint.bCrouched = Current->IsCrouched();
	Checkpoint.bLightOn = Current->IsFlashlightOn();
	Checkpoint.bPhoneRead = bPhoneRead;
	Checkpoint.bHasEarplugs = bHasEarplugs;
	Checkpoint.bWearingEarplugs = bWearingEarplugs;
	Checkpoint.bHasCloth = bHasCloth;
	Checkpoint.bCameraCovered = bCameraCovered;
	Checkpoint.bHallOpen = bHallOpen;
	Checkpoint.bHasSoda = bHasSoda;
	Checkpoint.bSatAlone = bSatAlone;
	Checkpoint.bHasExam = bHasExam;
	Checkpoint.bExamGiven = bExamGiven;
	Checkpoint.bOfficeOpen = bOfficeOpen;
	Checkpoint.bTeacherRead = bTeacherRead;
	Checkpoint.bInfirmaryOpen = bInfirmaryOpen;
	Checkpoint.bZhouHeard = bZhouHeard;
	Checkpoint.bLightOpen = bLightOpen;
	Checkpoint.bClosetOpen = bClosetOpen;
	Checkpoint.bGhostsSpawned = bGhostsSpawned;
	Checkpoint.bPressureWarned = bPressureWarned;
	Checkpoint.bDutyA = bDutyA;
	Checkpoint.bDutyB = bDutyB;
	Checkpoint.bDutyC = bDutyC;
	Checkpoint.bCoin = bCoin;
	Checkpoint.bCoinSpent = bCoinSpent;
	Checkpoint.bNoteSeat = bNoteSeat;
	Checkpoint.bNoteHall = bNoteHall;
	Checkpoint.bNoteShop = bNoteShop;
	Checkpoint.bNoteLocker = bNoteLocker;
	Checkpoint.bShopOpen = bShopOpen;
	Checkpoint.bLedgerHeld = bLedgerHeld;
	Checkpoint.bLedgerPosted = bLedgerPosted;
	Checkpoint.bGradeOpen = bGradeOpen;
	Checkpoint.bMinSun = bMinSun;
	Checkpoint.bMinLiang = bMinLiang;
	Checkpoint.bMinParent = bMinParent;
	Checkpoint.bNoticeA = bNoticeA;
	Checkpoint.bNoticeB = bNoticeB;
	Checkpoint.bNoticeC = bNoticeC;
	Checkpoint.bNoticeD = bNoticeD;
	Checkpoint.bNoticeE = bNoticeE;
	Checkpoint.bNoticeF = bNoticeF;
	Checkpoint.bNoticeG = bNoticeG;
	Checkpoint.bNoticeH = bNoticeH;
	Checkpoint.bHasPencil = bHasPencil;
	Checkpoint.bHasBus = bHasBus;
	Checkpoint.bHasTime = bHasTime;
	Checkpoint.bItemsReturned = bItemsReturned;
	Checkpoint.bZhouIntro = bZhouIntro;
	Checkpoint.bComputerOpen = bComputerOpen;
	Checkpoint.bEssayDorm = bEssayDorm;
	Checkpoint.bEssayShop = bEssayShop;
	Checkpoint.bEssayGrade = bEssayGrade;
	Checkpoint.bEssaysKept = bEssaysKept;
	Checkpoint.bShopGhost = bShopGhost;
	Checkpoint.bNoticeGhost = bNoticeGhost;
	Checkpoint.bComputerGhost = bComputerGhost;
	Checkpoint.bShopSodaTaken = bShopSodaTaken;
	Checkpoint.bSpareTaken = bSpareTaken;
	Checkpoint.bCallA = bCallA;
	Checkpoint.bCallB = bCallB;
	Checkpoint.bCallC = bCallC;
	Checkpoint.bClassA = bClassA;
	Checkpoint.bClassB = bClassB;
	Checkpoint.bClassC = bClassC;
	Checkpoint.bReceiptA = bReceiptA;
	Checkpoint.bReceiptB = bReceiptB;
	Checkpoint.bReceiptC = bReceiptC;
	Checkpoint.bDraftA = bDraftA;
	Checkpoint.bDraftB = bDraftB;
	Checkpoint.bDraftC = bDraftC;
	Checkpoint.bChartA = bChartA;
	Checkpoint.bChartB = bChartB;
	Checkpoint.bChartC = bChartC;
	Checkpoint.bSeal1Held = bSeal1Held;
	Checkpoint.bSeal1Set = bSeal1Set;
	Checkpoint.bSeal2Held = bSeal2Held;
	Checkpoint.bSeal2Set = bSeal2Set;
	Checkpoint.bSeal3Held = bSeal3Held;
	Checkpoint.bSeal3Set = bSeal3Set;
	Checkpoint.bSeal4Held = bSeal4Held;
	Checkpoint.bSeal4Set = bSeal4Set;
	Checkpoint.bSeal5Held = bSeal5Held;
	Checkpoint.bSeal5Set = bSeal5Set;
	Checkpoint.bStoreOpen = bStoreOpen;
	Checkpoint.bAnnexOpen = bAnnexOpen;
	Checkpoint.bAnnexGhost = bAnnexGhost;
	Checkpoint.bSawSelf = bSawSelf;
	Checkpoint.bPuzzle1 = bPuzzle1;
	Checkpoint.bPuzzle2 = bPuzzle2;
	Checkpoint.bPuzzle3 = bPuzzle3;
	Checkpoint.bPuzzle4 = bPuzzle4;
	Checkpoint.bPuzzle5 = bPuzzle5;
	Checkpoint.bPuzzle6 = bPuzzle6;
	Checkpoint.bPuzzle7 = bPuzzle7;
	Checkpoint.bPuzzle8 = bPuzzle8;
	Checkpoint.bPuzzle9 = bPuzzle9;
	Checkpoint.bPuzzle10 = bPuzzle10;
	Checkpoint.bHasSideKey = bHasSideKey;
	Checkpoint.bSideOpen = bSideOpen;
	Checkpoint.bShedOpen = bShedOpen;
	Checkpoint.bReadOpen = bReadOpen;
	Checkpoint.bStudyA = bStudyA;
	Checkpoint.bStudyB = bStudyB;
	Checkpoint.bStudyC = bStudyC;
	Checkpoint.bStudyD = bStudyD;
	Checkpoint.bStudyKept = bStudyKept;
	Checkpoint.bToolCell = bToolCell;
	Checkpoint.bSouthGhost = bSouthGhost;
	Checkpoint.bShedGhost = bShedGhost;
	Checkpoint.bReadGhost = bReadGhost;
	Checkpoint.Consumed.Reset();
	Checkpoint.OpenDoors.Reset();
	for (ANightSchoolProp* Prop : Props)
	{
		if (!Prop || IsSharedProp(Prop->GetPropId()))
		{
			continue;
		}
		const uint8 Key = static_cast<uint8>(Prop->GetPropId());
		if (Prop->IsDoor())
		{
			if (Prop->IsDoorOpen())
			{
				Checkpoint.OpenDoors.Add(Key);
			}
		}
		else if (Prop->IsConsumed())
		{
			Checkpoint.Consumed.Add(Key);
		}
	}
}

void ANightSchoolDirector::RestoreCheckpoint()
{
	if (!Checkpoint.bValid)
	{
		return;
	}
	bPhoneRead = Checkpoint.bPhoneRead;
	bHasEarplugs = Checkpoint.bHasEarplugs;
	bWearingEarplugs = Checkpoint.bWearingEarplugs;
	bHasCloth = Checkpoint.bHasCloth;
	bCameraCovered = Checkpoint.bCameraCovered;
	bHallOpen = Checkpoint.bHallOpen;
	bHasSoda = Checkpoint.bHasSoda;
	bSatAlone = Checkpoint.bSatAlone;
	bHasExam = Checkpoint.bHasExam;
	bExamGiven = Checkpoint.bExamGiven;
	bOfficeOpen = Checkpoint.bOfficeOpen;
	bTeacherRead = Checkpoint.bTeacherRead;
	bInfirmaryOpen = Checkpoint.bInfirmaryOpen;
	bZhouHeard = Checkpoint.bZhouHeard;
	bLightOpen = Checkpoint.bLightOpen;
	bClosetOpen = Checkpoint.bClosetOpen;
	bGhostsSpawned = Checkpoint.bGhostsSpawned;
	bPressureWarned = Checkpoint.bPressureWarned;
	bDutyA = Checkpoint.bDutyA;
	bDutyB = Checkpoint.bDutyB;
	bDutyC = Checkpoint.bDutyC;
	bCoin = Checkpoint.bCoin;
	bCoinSpent = Checkpoint.bCoinSpent;
	bNoteSeat = Checkpoint.bNoteSeat;
	bNoteHall = Checkpoint.bNoteHall;
	bNoteShop = Checkpoint.bNoteShop;
	bNoteLocker = Checkpoint.bNoteLocker;
	bShopOpen = Checkpoint.bShopOpen;
	bLedgerHeld = Checkpoint.bLedgerHeld;
	bLedgerPosted = Checkpoint.bLedgerPosted;
	bGradeOpen = Checkpoint.bGradeOpen;
	bMinSun = Checkpoint.bMinSun;
	bMinLiang = Checkpoint.bMinLiang;
	bMinParent = Checkpoint.bMinParent;
	bNoticeA = Checkpoint.bNoticeA;
	bNoticeB = Checkpoint.bNoticeB;
	bNoticeC = Checkpoint.bNoticeC;
	bNoticeD = Checkpoint.bNoticeD;
	bNoticeE = Checkpoint.bNoticeE;
	bNoticeF = Checkpoint.bNoticeF;
	bNoticeG = Checkpoint.bNoticeG;
	bNoticeH = Checkpoint.bNoticeH;
	bHasPencil = Checkpoint.bHasPencil;
	bHasBus = Checkpoint.bHasBus;
	bHasTime = Checkpoint.bHasTime;
	bItemsReturned = Checkpoint.bItemsReturned;
	bZhouIntro = Checkpoint.bZhouIntro;
	bComputerOpen = Checkpoint.bComputerOpen;
	bEssayDorm = Checkpoint.bEssayDorm;
	bEssayShop = Checkpoint.bEssayShop;
	bEssayGrade = Checkpoint.bEssayGrade;
	bEssaysKept = Checkpoint.bEssaysKept;
	bShopGhost = Checkpoint.bShopGhost;
	bNoticeGhost = Checkpoint.bNoticeGhost;
	bComputerGhost = Checkpoint.bComputerGhost;
	bShopSodaTaken = Checkpoint.bShopSodaTaken;
	bSpareTaken = Checkpoint.bSpareTaken;
	bCallA = Checkpoint.bCallA;
	bCallB = Checkpoint.bCallB;
	bCallC = Checkpoint.bCallC;
	bClassA = Checkpoint.bClassA;
	bClassB = Checkpoint.bClassB;
	bClassC = Checkpoint.bClassC;
	bReceiptA = Checkpoint.bReceiptA;
	bReceiptB = Checkpoint.bReceiptB;
	bReceiptC = Checkpoint.bReceiptC;
	bDraftA = Checkpoint.bDraftA;
	bDraftB = Checkpoint.bDraftB;
	bDraftC = Checkpoint.bDraftC;
	bChartA = Checkpoint.bChartA;
	bChartB = Checkpoint.bChartB;
	bChartC = Checkpoint.bChartC;
	bSeal1Held = Checkpoint.bSeal1Held;
	bSeal1Set = Checkpoint.bSeal1Set;
	bSeal2Held = Checkpoint.bSeal2Held;
	bSeal2Set = Checkpoint.bSeal2Set;
	bSeal3Held = Checkpoint.bSeal3Held;
	bSeal3Set = Checkpoint.bSeal3Set;
	bSeal4Held = Checkpoint.bSeal4Held;
	bSeal4Set = Checkpoint.bSeal4Set;
	bSeal5Held = Checkpoint.bSeal5Held;
	bSeal5Set = Checkpoint.bSeal5Set;
	bStoreOpen = Checkpoint.bStoreOpen;
	bAnnexOpen = Checkpoint.bAnnexOpen;
	bAnnexGhost = Checkpoint.bAnnexGhost;
	bSawSelf = Checkpoint.bSawSelf;
	bPuzzle1 = Checkpoint.bPuzzle1;
	bPuzzle2 = Checkpoint.bPuzzle2;
	bPuzzle3 = Checkpoint.bPuzzle3;
	bPuzzle4 = Checkpoint.bPuzzle4;
	bPuzzle5 = Checkpoint.bPuzzle5;
	bPuzzle6 = Checkpoint.bPuzzle6;
	bPuzzle7 = Checkpoint.bPuzzle7;
	bPuzzle8 = Checkpoint.bPuzzle8;
	bPuzzle9 = Checkpoint.bPuzzle9;
	bPuzzle10 = Checkpoint.bPuzzle10;
	bHasSideKey = Checkpoint.bHasSideKey;
	bSideOpen = Checkpoint.bSideOpen;
	bShedOpen = Checkpoint.bShedOpen;
	bReadOpen = Checkpoint.bReadOpen;
	bStudyA = Checkpoint.bStudyA;
	bStudyB = Checkpoint.bStudyB;
	bStudyC = Checkpoint.bStudyC;
	bStudyD = Checkpoint.bStudyD;
	bStudyKept = Checkpoint.bStudyKept;
	bToolCell = Checkpoint.bToolCell;
	bSouthGhost = Checkpoint.bSouthGhost;
	bShedGhost = Checkpoint.bShedGhost;
	bReadGhost = Checkpoint.bReadGhost;
	Fear = Checkpoint.Fear;
	FearLock = 0.f;
	LureTime = 0.f;
	bNoteOpen = false;
	ActivePuzzle = -1;
	PuzzleTime = 0.f;
	FMemory::Memzero(Dials, sizeof(Dials));

	for (ANightSchoolProp* Prop : Props)
	{
		if (!Prop)
		{
			continue;
		}
		if (Prop->GetPropId() == ENightPropId::Dial)
		{
			Prop->SetPrompt(FText::FromString(TEXT("转动这环")));
			continue;
		}
		if (IsSharedProp(Prop->GetPropId()))
		{
			continue;
		}
		const uint8 Key = static_cast<uint8>(Prop->GetPropId());
		if (Prop->IsDoor())
		{
			if (Checkpoint.OpenDoors.Contains(Key))
			{
				if (!Prop->IsDoorOpen())
				{
					Prop->OpenDoor();
				}
			}
			else if (Prop->IsDoorOpen())
			{
				Prop->CloseDoor();
			}
		}
		else if (Checkpoint.Consumed.Contains(Key))
		{
			if (!Prop->IsConsumed())
			{
				Prop->Consume();
			}
		}
		else if (Prop->IsConsumed())
		{
			Prop->Revive();
		}
	}

	RefreshEarplugPrompt();
	if (ANightSchoolProp* ZhouProp = FindProp(ENightPropId::Zhou))
	{
		if (bZhouHeard)
		{
			ZhouProp->SetPrompt(FText::FromString(TEXT("周先生已经说完")));
		}
		else if (bItemsReturned)
		{
			ZhouProp->SetPrompt(FText::FromString(TEXT("听周先生把两扇门说完")));
		}
		else if (bZhouIntro)
		{
			ZhouProp->SetPrompt(FText::FromString(TEXT("三样东西还没放上托盘")));
		}
		else
		{
			ZhouProp->SetPrompt(FText::FromString(TEXT("听周先生说")));
		}
	}

	const bool bHall = bGhostsSpawned;
	const bool bShop = bShopGhost;
	const bool bNotice = bNoticeGhost;
	const bool bComputer = bComputerGhost;
	const bool bAnnex = bAnnexGhost;
	const bool bSouth = bSouthGhost;
	const bool bShed = bShedGhost;
	const bool bRead = bReadGhost;
	RebuildGhosts(bHall, bShop, bNotice, bComputer, bAnnex, bSouth, bShed, bRead);

	if (ANightSchoolCharacter* Current = Player())
	{
		Current->SetReading(false);
		Current->ApplyResume(Checkpoint.Location, Checkpoint.View, Checkpoint.Battery, Checkpoint.Stamina, Checkpoint.bCrouched, Checkpoint.bLightOn);
	}
}

void ANightSchoolDirector::EnsureOpeningCheckpoint()
{
	if (Checkpoint.bValid)
	{
		return;
	}
	if (ANightSchoolCharacter* Current = Player())
	{
		CaptureCheckpoint(TEXT("前厅"), Current->GetActorLocation());
	}
}

void ANightSchoolDirector::UseInkPad(ANightSchoolProp* Prop)
{
	if (!Prop)
	{
		return;
	}
	if (ActivePuzzle >= 0)
	{
		ShowStatus(TEXT("这组环还没拨完。印台要等它停。"), 2.8f);
		return;
	}
	ANightSchoolCharacter* Current = Player();
	if (!Current)
	{
		return;
	}
	CaptureCheckpoint(Prop->GetMarker().ToString(), Current->GetActorLocation());
	ShowStatus(FString::Printf(TEXT("%s的印台记下了。解不开的时候，从这里再来。"), *Prop->GetMarker().ToString()), 4.2f);
}

void ANightSchoolDirector::BeginPuzzle(int32 Index)
{
	if (Index < 0 || Index > 9 || IsPuzzleDone(Index) || ActivePuzzle == Index)
	{
		return;
	}
	ActivePuzzle = Index;
	PuzzleTime = PuzzleLimit;
	SpawnPuzzleChaser(Index);
	ShowStatus(TEXT("八十秒。数字写在墙上。它们已经在走。"), 4.2f);
}

void ANightSchoolDirector::FinishPuzzle(int32 Index)
{
	SetPuzzleDone(Index);
	ActivePuzzle = -1;
	PuzzleTime = 0.f;
	ClearPuzzleChaser();
	if (Index == 8)
	{
		TryOpenReadingRoom();
		ShowStatus(TEXT("三环对上了。旧阅览的门开了。这不是第三扇门。"), 4.2f);
		return;
	}
	ShowStatus(TEXT("三环对上了。这一轮的追赶停了。"), 4.2f);
}

void ANightSchoolDirector::FailPuzzle()
{
	ActivePuzzle = -1;
	PuzzleTime = 0.f;
	RestoreCheckpoint();
	ShowStatus(TEXT("八十秒到了。没能拨完，你被退回上一个印台。"), 5.5f);
}

void ANightSchoolDirector::TickPuzzle(float DeltaSeconds)
{
	if (ActivePuzzle < 0 || IsEnding() || bPreviewing)
	{
		return;
	}
	PuzzleTime -= DeltaSeconds;
	if (PuzzleTime <= 0.f)
	{
		FailPuzzle();
	}
}

void ANightSchoolDirector::UseDial(ANightSchoolProp* Prop)
{
	if (!Prop)
	{
		return;
	}
	const FString Mark = Prop->GetMarker().ToString();
	int32 Index = 0;
	int32 Slot = 0;
	if (!ParsePuzzleMark(Mark, Index, &Slot))
	{
		return;
	}
	if (IsPuzzleDone(Index))
	{
		ShowStatus(TEXT("这组环已经对上了。"), 2.4f);
		return;
	}
	if (!PuzzleReady(Index))
	{
		ShowNote(TEXT("环"), PuzzleLockedText(Index));
		return;
	}
	if (ActivePuzzle >= 0 && ActivePuzzle != Index)
	{
		ShowStatus(TEXT("眼前这组还没拨完。"), 2.4f);
		return;
	}
	BeginPuzzle(Index);
	Dials[Index][Slot] = (Dials[Index][Slot] + 1) % 10;
	Prop->SetPrompt(FText::FromString(FString::Printf(TEXT("这环现在是 %d"), Dials[Index][Slot])));
}

void ANightSchoolDirector::UseDialConfirm(ANightSchoolProp* Prop)
{
	if (!Prop)
	{
		return;
	}
	const FString Mark = Prop->GetMarker().ToString();
	int32 Index = 0;
	if (!ParsePuzzleMark(Mark, Index, nullptr))
	{
		return;
	}
	if (IsPuzzleDone(Index))
	{
		ShowStatus(TEXT("这组环已经对上了。"), 2.4f);
		return;
	}
	if (!PuzzleReady(Index))
	{
		ShowNote(TEXT("环"), PuzzleLockedText(Index));
		return;
	}
	if (ActivePuzzle >= 0 && ActivePuzzle != Index)
	{
		ShowStatus(TEXT("眼前这组还没拨完。"), 2.4f);
		return;
	}
	BeginPuzzle(Index);
	const bool bMatch = Dials[Index][0] == PuzzleCode[Index][0]
		&& Dials[Index][1] == PuzzleCode[Index][1]
		&& Dials[Index][2] == PuzzleCode[Index][2];
	if (bMatch)
	{
		FinishPuzzle(Index);
	}
	else
	{
		ShowStatus(FString::Printf(TEXT("现在是 %d、%d、%d。对不上。"), Dials[Index][0], Dials[Index][1], Dials[Index][2]), 2.6f);
	}
}

void ANightSchoolDirector::ClearPuzzleChaser()
{
	if (!PuzzleGhost)
	{
		return;
	}
	Apparitions.Remove(PuzzleGhost);
	PuzzleGhost->Destroy();
	PuzzleGhost = nullptr;
}

void ANightSchoolDirector::SpawnPuzzleChaser(int32 Index)
{
	ClearPuzzleChaser();
	FBox Room(EForceInit::ForceInit);
	TArray<FVector> Points;
	FString Glyph;
	if (Index == 0)
	{
		Room = FBox(W(-2100.f, -1320.f, -20.f), W(-1180.f, -680.f, 420.f));
		Points.Add(W(-1800.f, -1100.f, 128.f));
		Points.Add(W(-1500.f, -900.f, 128.f));
		Points.Add(W(-1900.f, -800.f, 128.f));
		Points.Add(W(-1400.f, -1200.f, 128.f));
		Glyph = TEXT("听");
	}
	else if (Index == 3)
	{
		Room = FBox(W(380.f, 880.f, -20.f), W(860.f, 1340.f, 420.f));
		Points.Add(W(500.f, 1000.f, 128.f));
		Points.Add(W(700.f, 1200.f, 128.f));
		Points.Add(W(450.f, 1250.f, 128.f));
		Points.Add(W(780.f, 950.f, 128.f));
		Glyph = TEXT("恨");
	}
	else if (Index == 5)
	{
		Room = FBox(W(-240.f, 1080.f, -20.f), W(240.f, 1520.f, 420.f));
		Points.Add(W(-100.f, 1200.f, 128.f));
		Points.Add(W(100.f, 1400.f, 128.f));
		Points.Add(W(0.f, 1460.f, 128.f));
		Points.Add(W(-150.f, 1350.f, 128.f));
		Glyph = TEXT("门");
	}
	else if (Index == 6)
	{
		Room = FBox(W(-1020.f, 60.f, -20.f), W(-360.f, 740.f, 420.f));
		Points.Add(W(-800.f, 300.f, 128.f));
		Points.Add(W(-500.f, 500.f, 128.f));
		Points.Add(W(-900.f, 550.f, 128.f));
		Points.Add(W(-480.f, 250.f, 128.f));
		Glyph = TEXT("盒");
	}
	else if (Index == 9)
	{
		Room = FBox(W(-1040.f, -2500.f, -20.f), W(-560.f, -1480.f, 420.f));
		Points.Add(W(-900.f, -1900.f, 128.f));
		Points.Add(W(-700.f, -2300.f, 128.f));
		Points.Add(W(-960.f, -2100.f, 128.f));
		Points.Add(W(-680.f, -1700.f, 128.f));
		Glyph = TEXT("具");
	}
	else
	{
		return;
	}
	PuzzleGhost = SpawnGhost(Room, Points, Glyph);
}

void ANightSchoolDirector::RebuildGhosts(bool bHall, bool bShop, bool bNotice, bool bComputer, bool bAnnex, bool bSouth, bool bShed, bool bRead)
{
	PuzzleGhost = nullptr;
	for (ANightSchoolApparition* Ghost : Apparitions)
	{
		if (Ghost)
		{
			Ghost->Destroy();
		}
	}
	Apparitions.Reset();
	bGhostsSpawned = false;
	bShopGhost = false;
	bNoticeGhost = false;
	bComputerGhost = false;
	bAnnexGhost = false;
	bSouthGhost = false;
	bShedGhost = false;
	bReadGhost = false;
	if (bHall)
	{
		SpawnHallGhosts();
	}
	if (bShop)
	{
		SpawnShopGhost();
	}
	if (bNotice)
	{
		SpawnNoticeGhost();
	}
	if (bComputer)
	{
		SpawnComputerGhost();
	}
	if (bAnnex)
	{
		SpawnAnnexGhost();
	}
	if (bSouth)
	{
		SpawnSouthGhost();
	}
	if (bShed)
	{
		SpawnShedGhost();
	}
	if (bRead)
	{
		SpawnReadGhost();
	}
}

void ANightSchoolDirector::BuildCheckpoints()
{
	const FLinearColor Ink(0.42f, 0.06f, 0.05f);
	const FLinearColor Brass(0.32f, 0.26f, 0.14f);
	auto Pad = [this, Ink](float X, float Y, const TCHAR* Room)
	{
		if (ANightSchoolProp* Prop = SpawnProp(ENightPropId::InkPad, W(X, Y, 96.f), FVector(14.f, 14.f, 10.f), Ink, FText::FromString(TEXT("把进度印在这里")), FVector::ZeroVector, 0.f, TEXT("印台")))
		{
			Prop->SetMarker(FName(Room));
		}
	};
	auto Wheel = [this, Brass](float X, float Y, const TCHAR* Mark, const TCHAR* Label)
	{
		if (ANightSchoolProp* Prop = SpawnProp(ENightPropId::Dial, W(X, Y, 112.f), FVector(10.f, 10.f, 16.f), Brass, FText::FromString(TEXT("转动这环")), FVector::ZeroVector, 0.f, Label))
		{
			Prop->SetMarker(FName(Mark));
		}
	};
	auto Plate = [this](float X, float Y, const TCHAR* Mark)
	{
		if (ANightSchoolProp* Prop = SpawnProp(ENightPropId::DialConfirm, W(X, Y, 100.f), FVector(16.f, 12.f, 8.f), FLinearColor(0.18f, 0.16f, 0.13f), FText::FromString(TEXT("按下确认")), FVector::ZeroVector, 0.f, TEXT("确认")))
		{
			Prop->SetMarker(FName(Mark));
		}
	};

	Pad(-80.f, -450.f, TEXT("前厅"));
	Pad(-600.f, 480.f, TEXT("宿舍"));
	Pad(-500.f, -1100.f, TEXT("值班室"));
	Pad(120.f, -900.f, TEXT("洗衣间"));
	Pad(-2000.f, -1000.f, TEXT("通话间"));
	Pad(-500.f, 1050.f, TEXT("储物柜"));
	Pad(-160.f, 400.f, TEXT("走廊"));
	Pad(400.f, 600.f, TEXT("教室"));
	Pad(1400.f, 150.f, TEXT("小卖部"));
	Pad(2180.f, 400.f, TEXT("后仓"));
	Pad(400.f, 900.f, TEXT("办公室"));
	Pad(1300.f, 900.f, TEXT("年级组"));
	Pad(1850.f, 980.f, TEXT("微机室"));
	Pad(1200.f, 1550.f, TEXT("北廊"));
	Pad(1100.f, 2100.f, TEXT("长榜"));
	Pad(0.f, 1450.f, TEXT("校医室"));
	Pad(400.f, 1550.f, TEXT("光"));
	Pad(-650.f, 1550.f, TEXT("柜子"));

	Wheel(-1900.f, -900.f, TEXT("P1A"), TEXT("甲"));
	Wheel(-1500.f, -1200.f, TEXT("P1B"), TEXT("乙"));
	Wheel(-2000.f, -1100.f, TEXT("P1C"), TEXT("丙"));
	Plate(-1750.f, -1150.f, TEXT("P1OK"));
	AddLabel(W(-1700.f, -1000.f, 168.f), 90.f, TEXT("6   1   4"), 18.f, FColor(220, 190, 150));

	Wheel(550.f, 650.f, TEXT("P2A"), TEXT("甲"));
	Wheel(560.f, 100.f, TEXT("P2B"), TEXT("乙"));
	Wheel(1100.f, 480.f, TEXT("P2C"), TEXT("丙"));
	Plate(760.f, 320.f, TEXT("P2OK"));
	AddLabel(W(760.f, 360.f, 168.f), -90.f, TEXT("3   1   2"), 18.f, FColor(220, 190, 150));

	Wheel(1450.f, 650.f, TEXT("P3A"), TEXT("甲"));
	Wheel(1850.f, 180.f, TEXT("P3B"), TEXT("乙"));
	Wheel(1900.f, 650.f, TEXT("P3C"), TEXT("丙"));
	Plate(1550.f, 450.f, TEXT("P3OK"));
	AddLabel(W(1600.f, 300.f, 168.f), 180.f, TEXT("4   0   8"), 18.f, FColor(220, 190, 150));

	Wheel(420.f, 1250.f, TEXT("P4A"), TEXT("甲"));
	Wheel(800.f, 1000.f, TEXT("P4B"), TEXT("乙"));
	Wheel(820.f, 1180.f, TEXT("P4C"), TEXT("丙"));
	Plate(600.f, 1100.f, TEXT("P4OK"));
	AddLabel(W(620.f, 1080.f, 168.f), -90.f, TEXT("7   2   5"), 18.f, FColor(220, 190, 150));

	Wheel(1300.f, 2400.f, TEXT("P5A"), TEXT("甲"));
	Wheel(1800.f, 2100.f, TEXT("P5B"), TEXT("乙"));
	Wheel(2000.f, 2450.f, TEXT("P5C"), TEXT("丙"));
	Plate(1450.f, 2350.f, TEXT("P5OK"));
	AddLabel(W(1600.f, 2200.f, 168.f), -90.f, TEXT("5   8   1"), 18.f, FColor(220, 190, 150));

	Wheel(-150.f, 1400.f, TEXT("P6A"), TEXT("甲"));
	Wheel(150.f, 1450.f, TEXT("P6B"), TEXT("乙"));
	Wheel(0.f, 1120.f, TEXT("P6C"), TEXT("丙"));
	Plate(-80.f, 1250.f, TEXT("P6OK"));
	AddLabel(W(-40.f, 1220.f, 168.f), 0.f, TEXT("1   7   0"), 18.f, FColor(220, 190, 150));

	Wheel(-900.f, 400.f, TEXT("P7A"), TEXT("甲"));
	Wheel(-450.f, 180.f, TEXT("P7B"), TEXT("乙"));
	Wheel(-420.f, 620.f, TEXT("P7C"), TEXT("丙"));
	Plate(-640.f, 180.f, TEXT("P7OK"));
	AddLabel(W(180.f, -200.f, 168.f), 90.f, TEXT("甲  2"), 18.f, FColor(220, 190, 150));
	AddLabel(W(-80.f, -780.f, 168.f), 0.f, TEXT("乙  6"), 18.f, FColor(220, 190, 150));
	AddLabel(W(-300.f, -2100.f, 168.f), 90.f, TEXT("丙  9"), 18.f, FColor(220, 190, 150));

	Wheel(-80.f, 250.f, TEXT("P8A"), TEXT("甲"));
	Wheel(80.f, 500.f, TEXT("P8B"), TEXT("乙"));
	Wheel(-40.f, 780.f, TEXT("P8C"), TEXT("丙"));
	Plate(40.f, 360.f, TEXT("P8OK"));
	AddLabel(W(450.f, 700.f, 168.f), 180.f, TEXT("甲  8"), 18.f, FColor(220, 190, 150));
	AddLabel(W(-1300.f, -800.f, 168.f), 0.f, TEXT("乙  4"), 18.f, FColor(220, 190, 150));
	AddLabel(W(1350.f, 550.f, 168.f), 180.f, TEXT("丙  0"), 18.f, FColor(220, 190, 150));

	Wheel(3100.f, 200.f, TEXT("P9A"), TEXT("甲"));
	Wheel(3350.f, 600.f, TEXT("P9B"), TEXT("乙"));
	Wheel(3400.f, 180.f, TEXT("P9C"), TEXT("丙"));
	Plate(3180.f, 450.f, TEXT("P9OK"));
	AddLabel(W(-980.f, 500.f, 168.f), 0.f, TEXT("甲  9"), 18.f, FColor(220, 190, 150));
	AddLabel(W(1050.f, 2100.f, 168.f), -90.f, TEXT("乙  3"), 18.f, FColor(220, 190, 150));
	AddLabel(W(220.f, 1200.f, 168.f), 180.f, TEXT("丙  5"), 18.f, FColor(220, 190, 150));
	AddLabel(W(3200.f, 500.f, 168.f), 180.f, TEXT("甲宿舍  乙长榜  丙校医室"), 14.f, FColor(220, 190, 150));

	Wheel(-900.f, -1800.f, TEXT("P10A"), TEXT("甲"));
	Wheel(-700.f, -2400.f, TEXT("P10B"), TEXT("乙"));
	Wheel(-980.f, -2200.f, TEXT("P10C"), TEXT("丙"));
	Plate(-800.f, -2100.f, TEXT("P10OK"));
	AddLabel(W(-200.f, -1800.f, 168.f), 90.f, TEXT("甲  1"), 18.f, FColor(220, 190, 150));
	AddLabel(W(-250.f, -2400.f, 168.f), 90.f, TEXT("乙  5"), 18.f, FColor(220, 190, 150));
	AddLabel(W(-600.f, -1600.f, 168.f), 0.f, TEXT("丙  7"), 18.f, FColor(220, 190, 150));

	Pad(-160.f, -1700.f, TEXT("南甬"));
	Pad(-800.f, -2000.f, TEXT("工具间"));
	Pad(2600.f, 620.f, TEXT("门廊"));
	Pad(3300.f, 250.f, TEXT("车棚"));
	Pad(3400.f, 1100.f, TEXT("旧阅览"));
}
