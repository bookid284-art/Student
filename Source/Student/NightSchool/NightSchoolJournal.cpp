#include "NightSchool/NightSchoolJournal.h"

#include "NightSchool/NightSchoolDirector.h"
#include "NightSchool/NightSchoolProp.h"
#include <initializer_list>

namespace
{
	using PropId = ENightPropId;

	struct FJournalRecord
	{
		PropId Prop;
		bool bRead;
	};

	struct FJournalRing
	{
		const TCHAR* Title;
		const TCHAR* Place;
		const TCHAR* Hint;
	};

	constexpr FJournalRing Rings[] = {
		{TEXT("对上通话间的三只环"), TEXT("通话间"), TEXT("先看墙上的数字，再转甲、乙、丙三只环。转动后限时，最后按确认。")},
		{TEXT("对上空位旁的三只环"), TEXT("教室"), TEXT("查看教室墙上的数字，转动三只环并确认。")},
		{TEXT("解开柜台账本旁的环"), TEXT("小卖部"), TEXT("数字在小卖部墙上，三只环沿货架分布。")},
		{TEXT("对上办公室的三只环"), TEXT("办公室"), TEXT("先检查墙上的数字，再拨环和确认。")},
		{TEXT("对上长榜的三只环"), TEXT("三楼长榜展室"), TEXT("先查看三楼展室东墙的数字，再依次转环并确认。")},
		{TEXT("对上药柜的三只环"), TEXT("校医室"), TEXT("把旧物放到托盘后，查看墙上的数字，再转药柜的环。")},
		{TEXT("解开宿舍铁盒"), TEXT("二楼 203"), TEXT("先找一楼前厅的甲、洗衣间的乙、南甬的丙，再上二楼转环。")},
		{TEXT("解开走廊的三只环"), TEXT("走廊"), TEXT("甲在教室、乙在通话间、丙在小卖部。先记数字，再回主走廊确认。")},
		{TEXT("对上车棚的三只环"), TEXT("一楼车棚"), TEXT("甲在二楼 203，乙在三楼长榜，丙在一楼校医室。把数字带回车棚。")},
		{TEXT("对上工具柜的三只环"), TEXT("工具间"), TEXT("先在南甬和工具间墙上找数字。")}
	};

	FName RingId(int32 Number)
	{
		return FName(*FString::Printf(TEXT("Rings%d"), Number));
	}

	FName RingConfirm(int32 Number)
	{
		return FName(*FString::Printf(TEXT("P%dOK"), Number));
	}
}

void ANightSchoolDirector::BuildJournal(TArray<FNightSchoolJournalEntry>& Entries) const
{
	Entries.Reset();
	Entries.Reserve(10);
	const int32 CurrentChapter = GetChapterIndex();
	FNightSchoolJournalEntry* Entry = nullptr;

	auto Chapter = [&](int32 Number, const TCHAR* Title, const TCHAR* Summary)
	{
		Entry = &Entries.AddDefaulted_GetRef();
		Entry->Id = FName(*FString::Printf(TEXT("Chapter%d"), Number));
		Entry->Chapter = Number;
		Entry->bLocked = Number > CurrentChapter;
		Entry->bComplete = Number < CurrentChapter || IsEnding();
		Entry->Title = Entry->bLocked ? TEXT("尚未开启") : Title;
		if (!Entry->bLocked)
		{
			Entry->Summary = Summary;
		}
	};
	auto Side = [&](FName Id, const TCHAR* Title, const TCHAR* Summary, bool bDone)
	{
		Entry = &Entries.AddDefaulted_GetRef();
		Entry->Id = Id;
		Entry->Title = Title;
		Entry->Summary = Summary;
		Entry->bComplete = bDone;
	};
	auto Add = [&](FName Id, const FString& Title, const TCHAR* Place, const TCHAR* Hint,
		bool bDone, const ANightSchoolProp* Target)
	{
		FNightSchoolJournalStep& Step = Entry->Steps.AddDefaulted_GetRef();
		Step.Id = Id;
		Step.Title = Title;
		Step.Place = Place;
		Step.Hint = Hint;
		Step.bDone = bDone;
		if (Target && !bDone)
		{
			Step.Destination = Target->GetActorLocation();
		}
	};
	auto Step = [&](FName Id, const TCHAR* Title, const TCHAR* Place, const TCHAR* Hint, bool bDone, PropId Target)
	{
		Add(Id, Title, Place, Hint, bDone, bDone ? nullptr : FindProp(Target));
	};
	auto DoorOpen = [this](PropId Id)
	{
		const ANightSchoolProp* Door = FindProp(Id);
		return Door && Door->IsDoorOpen();
	};
	auto Records = [&](FName Id, const TCHAR* Title, const TCHAR* Place, const TCHAR* Hint,
		std::initializer_list<FJournalRecord> RecordsToRead, bool bAlreadyReturned = false)
	{
		const int32 Total = static_cast<int32>(RecordsToRead.size());
		int32 Read = bAlreadyReturned ? Total : 0;
		const ANightSchoolProp* Next = nullptr;
		if (!bAlreadyReturned)
		{
			for (const FJournalRecord& Record : RecordsToRead)
			{
				if (Record.bRead)
				{
					++Read;
				}
				else if (!Next)
				{
					Next = FindProp(Record.Prop);
				}
			}
		}
		Add(Id, FString::Printf(TEXT("%s  %d/%d"), Title, Read, Total), Place, Hint, Read == Total, Next);
	};
	auto Ring = [&](int32 Number)
	{
		const bool bDone = IsPuzzleDone(Number - 1);
		const FJournalRing& Task = Rings[Number - 1];
		Add(RingId(Number), Task.Title, Task.Place, Task.Hint, bDone, bDone ? nullptr : FindMarked(RingConfirm(Number)));
	};

	Chapter(1, TEXT("未接"), TEXT("宿舍里的手机还亮着。循着未接来电，弄清床边那个红点是谁留下的。"));
	const bool bEnteredDorm = bPhoneRead || RoomViews.ContainsByPredicate([](const FNightRoomView& Room)
	{
		return Room.bSeen && Room.Id == TEXT("Ante");
	});
	Step(TEXT("Entrance"), TEXT("推开宿舍大门"), TEXT("宿舍楼外"), TEXT("沿路灯走到楼门。门两侧的签到表和公告可以翻看。"),
		bEnteredDorm || DoorOpen(PropId::DormEntrance), PropId::DormEntrance);
	Step(TEXT("Phone"), TEXT("查看未接来电"), TEXT("二楼 203"), TEXT("穿过一楼前厅，走西侧楼梯。到二楼后往东，进标有 203 的宿舍，手机在靠墙的桌上。"), bPhoneRead, PropId::Phone);
	Step(TEXT("CallDoor"), TEXT("进入通话间"), TEXT("一楼值班室西侧"), TEXT("沿楼梯下一楼。回前厅，走南门进值班室，再开西侧通话间的门。"), DoorOpen(PropId::CallDoor), PropId::CallDoor);
	Records(TEXT("Calls"), TEXT("读完通话记录"), TEXT("通话间"), TEXT("找到清晨、锁屏和镜头三页记录。"), {{PropId::CallA, bCallA}, {PropId::CallB, bCallB}, {PropId::CallC, bCallC}});
	Records(TEXT("Duty"), TEXT("读完老吴的值班记录"), TEXT("值班室"), TEXT("搜查值班室的桌面和墙边，找到三页值班记录。"), {{PropId::DutyLogA, bDutyA}, {PropId::DutyLogB, bDutyB}, {PropId::DutyLogC, bDutyC}});
	Ring(1);
	Step(TEXT("Cloth"), TEXT("拿到镜头布"), TEXT("值班室"), TEXT("读完记录并解开通话间的环后，回值班室拿抽屉里的布。"), bHasCloth || bCameraCovered, PropId::LensCloth);
	Step(TEXT("Cover"), TEXT("盖住床边的红点"), TEXT("二楼 203"), TEXT("回二楼 203，用镜头布盖住宿舍北墙上的摄像头。"), bCameraCovered, PropId::DormCamera);
	Step(TEXT("Seal1"), TEXT("盖上第一枚章印"), TEXT("一楼前厅"), TEXT("下楼，到前厅南墙的接待柜，在红印台旁的木板上盖章。"), bSeal1Set, PropId::StampAnte);

	Chapter(2, TEXT("空位"), TEXT("江晚留下了四张条子。把它们和班级日志拼在一起，再回那张空位。"));
	if (!Entry->bLocked)
	{
		Ring(7);
		Step(TEXT("Coin"), TEXT("取出铁盒里的硬币"), TEXT("二楼 203"), TEXT("用这枚硬币打开一楼小卖部的卷帘。"), bCoin || bCoinSpent || bShopOpen, PropId::Coin);
		Step(TEXT("Shop"), TEXT("打开小卖部卷帘"), TEXT("小卖部"), TEXT("从走廊穿过教室，去东边小卖部的投币口。"), bShopOpen, PropId::ShopDoor);
		Records(TEXT("JiangNotes"), TEXT("找齐江晚的条子"), TEXT("一楼各处"), TEXT("检查教室空位、走廊、小卖部柜台和前厅北面储物过道尽头的柜子。"), {{PropId::NoteSeat, bNoteSeat}, {PropId::NoteHall, bNoteHall}, {PropId::NoteShop, bNoteShop}, {PropId::NoteLocker, bNoteLocker}});
		Records(TEXT("ClassLog"), TEXT("读完班级日志"), TEXT("教室"), TEXT("三页日志分散在教室里。"), {{PropId::ClassLogA, bClassA}, {PropId::ClassLogB, bClassB}, {PropId::ClassLogC, bClassC}});
		Ring(2);
		Step(TEXT("Desk"), TEXT("坐到那张空位上"), TEXT("教室"), TEXT("条子、日志和三只环都齐了，再与空位交互。"), bSatAlone, PropId::UnwantedDesk);
		Step(TEXT("Seal2"), TEXT("盖上第二枚章印"), TEXT("值班室"), TEXT("回前厅南边的值班室，在桌上盖章。"), bSeal2Set, PropId::StampDuty);
	}

	Chapter(3, TEXT("请客"), TEXT("小卖部的账本记着一笔笔人情。先从后仓看清来路，再决定把它放在哪里。"));
	if (!Entry->bLocked)
	{
		Step(TEXT("Store"), TEXT("打开后仓的门"), TEXT("小卖部东侧"), TEXT("穿过已打开的卷帘，到小卖部东墙找后仓门。"), bStoreOpen, PropId::StoreDoor);
		Records(TEXT("Receipts"), TEXT("读完后仓收据"), TEXT("后仓"), TEXT("找两张收据和一张背面的手写记录。"), {{PropId::ReceiptA, bReceiptA}, {PropId::ReceiptB, bReceiptB}, {PropId::ReceiptC, bReceiptC}});
		Ring(3);
		Step(TEXT("Ledger"), TEXT("取走柜台下的账本"), TEXT("小卖部"), TEXT("三张收据和环都齐了，再拿账本。"), bLedgerHeld || bLedgerPosted, PropId::Ledger);
		Step(TEXT("PostLedger"), TEXT("把账本钉上公告栏"), TEXT("走廊"), TEXT("回主走廊找布告栏。"), bLedgerPosted, PropId::LedgerBoard);
		Step(TEXT("Seal3"), TEXT("盖上第三枚章印"), TEXT("通话间"), TEXT("回值班室西边，在通话间盖章。"), bSeal3Set, PropId::StampCall);
	}

	Chapter(4, TEXT("恨铁"), TEXT("把旧试卷交到办公室。那里的字条和会议记录会告诉你之后发生了什么。"));
	if (!Entry->bLocked)
	{
		Step(TEXT("Exam"), TEXT("拿起旧试卷"), TEXT("教室"), TEXT("在教室桌上找那张旧试卷。"), bHasExam || bExamGiven, PropId::ExamPaper);
		Ring(8);
		Step(TEXT("DeliverExam"), TEXT("把试卷交到办公室"), TEXT("办公室门口"), TEXT("带试卷到办公室门口交卷。"), bExamGiven && bOfficeOpen, PropId::OfficeDoor);
		Records(TEXT("Drafts"), TEXT("读完改卷批注"), TEXT("办公室"), TEXT("把办公室里三页改卷批注找齐。"), {{PropId::DraftA, bDraftA}, {PropId::DraftB, bDraftB}, {PropId::DraftC, bDraftC}});
		Ring(4);
		Step(TEXT("Teacher"), TEXT("读梁老师的字条"), TEXT("办公室"), TEXT("字条就在办公室的桌上。"), bTeacherRead, PropId::TeacherNote);
		Records(TEXT("Minutes"), TEXT("读完年级组会议记录"), TEXT("年级组"), TEXT("读完字条后，沿办公室通往年级组的门寻找三页会议记录。"), {{PropId::MinutesSun, bMinSun}, {PropId::MinutesLiang, bMinLiang}, {PropId::MinutesParent, bMinParent}});
		Step(TEXT("Seal4"), TEXT("把第四枚章印盖回空位"), TEXT("教室"), TEXT("回教室，在空位旁盖章。盖好后，三楼长榜展室才能打开。"), bSeal4Set, PropId::StampClass);
	}

	Chapter(5, TEXT("榜"), TEXT("北廊和长榜上有两组告示。读完它们，然后把章印送回视线最初落下的地方。"));
	if (!Entry->bLocked)
	{
		Records(TEXT("NorthNotices"), TEXT("读完北廊告示"), TEXT("北廊"), TEXT("从年级组向北，查看墙上并列的四张。"), {{PropId::NoticeA, bNoticeA}, {PropId::NoticeB, bNoticeB}, {PropId::NoticeC, bNoticeC}, {PropId::NoticeD, bNoticeD}});
		Step(TEXT("Annex"), TEXT("推开长榜展室的门"), TEXT("三楼西侧"), TEXT("沿来路回一楼前厅，从西侧楼梯上三楼，长榜在平台西侧。"), bAnnexOpen, PropId::AnnexDoor);
		Records(TEXT("AnnexNotices"), TEXT("读完长榜告示"), TEXT("三楼长榜展室"), TEXT("读完展室里最里面一整排的四张告示。"), {{PropId::NoticeE, bNoticeE}, {PropId::NoticeF, bNoticeF}, {PropId::NoticeG, bNoticeG}, {PropId::NoticeH, bNoticeH}});
		Ring(5);
		Step(TEXT("Seal5"), TEXT("把第五枚章印盖回宿舍"), TEXT("二楼 203"), TEXT("下一层，到 203 摄像头旁的小桌上盖章。"), bSeal5Set, PropId::StampDorm);
	}

	Chapter(6, TEXT("两扇门"), TEXT("周先生在校医室。听完他的话，再由你决定怎么走。"));
	if (!Entry->bLocked)
	{
		Step(TEXT("Infirmary"), TEXT("打开校医室的门"), TEXT("主走廊北端"), TEXT("回主走廊，往北走到校医室。"), bInfirmaryOpen, PropId::InfirmaryDoor);
		Records(TEXT("Charts"), TEXT("读完三页病历"), TEXT("校医室"), TEXT("看完病历，再去和周先生说话。"), {{PropId::ChartA, bChartA}, {PropId::ChartB, bChartB}, {PropId::ChartC, bChartC}});
		Step(TEXT("ZhouIntro"), TEXT("听周先生说话"), TEXT("校医室"), TEXT("周先生会告诉你三件遗留物分别在哪里。"), bZhouIntro, PropId::Zhou);
		Records(TEXT("LostItems"), TEXT("找齐林小满的遗留物"), TEXT("一楼各处"), TEXT("铅笔盒在前厅北面过道尽头的储物柜，月票在微机室，课表在南边的洗衣间。"), {{PropId::PencilCase, bHasPencil}, {PropId::BusPass, bHasBus}, {PropId::Timetable, bHasTime}}, bItemsReturned);
		Step(TEXT("Return"), TEXT("把遗留物放回托盘"), TEXT("校医室"), TEXT("回周先生身后，把三样东西放上托盘。"), bItemsReturned, PropId::ReturnTray);
		Ring(6);
		Step(TEXT("ZhouFinal"), TEXT("听完周先生的话"), TEXT("校医室"), TEXT("再找周先生，他这次会把两扇门都说完。"), bZhouHeard, PropId::Zhou);
		Add(TEXT("ChooseDoor"), TEXT("决定走哪扇门"), TEXT("校医室旁"), bWearingEarplugs
			? TEXT("东边是光，西边是柜子。想拨开关，先回宿舍取下耳塞，再看清光里的人。")
			: TEXT("东边是光，西边是柜子。进光后先看清里面的人，也可以选择走进柜子。"), IsEnding(), nullptr);
	}

	if (bPhoneRead || bEssayDorm)
	{
		Side(TEXT("Essays"), TEXT("我的句子"), TEXT("找齐许知夏自己的三篇作文，把它们留在长凳上。"), bEssaysKept);
		Step(TEXT("EssayDorm"), TEXT("找回《晚饭》"), TEXT("二楼 203"), TEXT("查看宿舍铁架床外侧、床脚边的纸。"), bEssayDorm, PropId::EssayDorm);
		if (bShopOpen || bEssayShop)
		{
			Step(TEXT("EssayShop"), TEXT("找回《同桌》"), TEXT("小卖部"), TEXT("卷帘打开后，去柜台周围找作文。"), bEssayShop, PropId::EssayShop);
		}
		if (bGradeOpen || bEssayGrade)
		{
			Step(TEXT("EssayGrade"), TEXT("找回没交的作文"), TEXT("年级组"), TEXT("留意年级组桌边的作文。"), bEssayGrade, PropId::EssayGrade);
		}
		if (EssayCount() == 3)
		{
			Add(TEXT("EssayBench"), TEXT("把三篇作文放上长凳"), TEXT("光里"), TEXT("收齐作文后，等光门打开，再把它们放在长凳上。"), bEssaysKept,
				bLightOpen && !bEssaysKept ? FindProp(PropId::EssayBench) : nullptr);
		}
	}
	if (DoorOpen(PropId::SouthDoor) || bHasSideKey || bSideOpen || bStudyD)
	{
		Side(TEXT("Study"), TEXT("夜自习的四页"), TEXT("从一楼南甬和车棚开始找记录，收齐后带到三楼旧阅览室。"), bStudyKept);
		Step(TEXT("StudyRoll"), TEXT("读夜自习名册"), TEXT("南甬"), TEXT("从洗衣间南门进去，沿南甬寻找桌上的名册。"), bStudyA, PropId::StudyA);
		Step(TEXT("SideKey"), TEXT("找到门房钥匙"), TEXT("南甬尽头"), TEXT("钥匙开后仓东墙的门。"), bHasSideKey || bSideOpen, PropId::SideKey);
		if (bStoreOpen || bSideOpen)
		{
			Step(TEXT("SideDoor"), TEXT("用钥匙打开东门"), TEXT("后仓"), TEXT("穿过小卖部和后仓，在东墙找门房锁。"), bSideOpen, PropId::SideDoor);
			Step(TEXT("ShedDoor"), TEXT("进入旧车棚"), TEXT("东侧门廊"), TEXT("出东门，沿门廊继续向东。"), bShedOpen, PropId::ShedDoor);
			Step(TEXT("StudyLock"), TEXT("读车棚的旧锁记录"), TEXT("车棚"), TEXT("在车棚里找写着旧锁的纸。"), bStudyB, PropId::StudyB);
		}
		if (bInfirmaryOpen || bReadOpen)
		{
			Ring(9);
			Step(TEXT("ReadingDoor"), TEXT("解锁旧阅览室"), TEXT("三楼东侧"), TEXT("解开一楼车棚的环，三楼旧阅览室的门就会打开。"), bReadOpen, PropId::ReadDoor);
			Step(TEXT("StudyDuty"), TEXT("读退休前的值班表"), TEXT("三楼旧阅览室"), TEXT("回宿舍楼上三楼，在平台东侧的旧阅览室找值班表。"), bStudyC, PropId::StudyC);
		}
		Step(TEXT("StudyKeeper"), TEXT("读宿管留下的字"), TEXT("一楼储物柜"), TEXT("检查前厅北面过道尽头的储物柜。"), bStudyD, PropId::StudyD);
		if (StudyCount() == 4)
		{
			Step(TEXT("StudyBench"), TEXT("把四页放上阅览桌"), TEXT("三楼旧阅览室"), TEXT("回到三楼，把四份纸放在旧阅览室靠里的桌上。"), bStudyKept, PropId::StudyBench);
		}
	}
	if (DoorOpen(PropId::SouthDoor) || bToolCell)
	{
		Side(TEXT("ToolRoom"), TEXT("南甬的备用电池"), TEXT("南甬西边的工具柜里还有一节电池。"), bToolCell);
		Step(TEXT("Tools"), TEXT("打开工具间"), TEXT("南甬西侧"), TEXT("进入南甬后，从西侧的门进入工具间。"), DoorOpen(PropId::ToolDoor), PropId::ToolDoor);
		Ring(10);
		Step(TEXT("ToolCell"), TEXT("取出工具柜的电池"), TEXT("工具间"), TEXT("解开三只环以后打开工具柜。"), bToolCell, PropId::ToolCabinet);
	}
}

TOptional<FNightSchoolJournalStep> ANightSchoolDirector::GetTimedObjective() const
{
	if (ActivePuzzle < 0 || ActivePuzzle >= static_cast<int32>(UE_ARRAY_COUNT(Rings)))
	{
		return {};
	}
	const FJournalRing& Task = Rings[ActivePuzzle];
	FNightSchoolJournalStep Step;
	Step.Id = RingId(ActivePuzzle + 1);
	Step.Title = Task.Title;
	Step.Place = Task.Place;
	Step.Hint = Task.Hint;
	if (const ANightSchoolProp* Target = FindMarked(RingConfirm(ActivePuzzle + 1)))
	{
		Step.Destination = Target->GetActorLocation();
	}
	return Step;
}

void ANightSchoolDirector::GetJournalRooms(TArray<FNightSchoolJournalRoom>& Rooms) const
{
	Rooms.Reset();
	for (const FNightRoomView& Room : RoomViews)
	{
		if (Room.bSeen)
		{
			Rooms.Add({Room.Name, Room.Bounds});
		}
	}
}
