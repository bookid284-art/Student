#include "NightSchool/NightSchoolDirector.h"

#include "NightSchool/NightSchoolApparition.h"
#include "NightSchool/NightSchoolProp.h"

namespace
{
	const FLinearColor Plaster(0.16f, 0.145f, 0.125f);
	const FLinearColor DarkWood(0.10f, 0.055f, 0.035f);
	const FLinearColor Ceiling(0.07f, 0.065f, 0.06f);
	const FLinearColor Paper(0.42f, 0.36f, 0.26f);
	const FLinearColor Concrete(0.055f, 0.055f, 0.058f);
	constexpr float WallZ = 180.f;
	constexpr float WallH = 280.f;
	constexpr float Thick = 28.f;
	constexpr float LintelZ = 286.f;
	constexpr float LintelH = 68.f;
}

int32 ANightSchoolDirector::StudyCount() const
{
	return (bStudyA ? 1 : 0) + (bStudyB ? 1 : 0) + (bStudyC ? 1 : 0) + (bStudyD ? 1 : 0);
}

void ANightSchoolDirector::SpawnSouthGhost()
{
	if (bSouthGhost)
	{
		return;
	}
	bSouthGhost = true;
	const FBox Room(W(-440.f, -2500.f, -20.f), W(140.f, -1460.f, 420.f));
	TArray<FVector> Points;
	Points.Add(W(-200.f, -1800.f, 128.f));
	Points.Add(W(0.f, -2100.f, 128.f));
	Points.Add(W(-300.f, -2400.f, 128.f));
	Points.Add(W(40.f, -2300.f, 128.f));
	SpawnGhost(Room, Points, TEXT("夜"));
}

void ANightSchoolDirector::SpawnShedGhost()
{
	if (bShedGhost)
	{
		return;
	}
	bShedGhost = true;
	const FBox Room(W(3000.f, 80.f, -20.f), W(3440.f, 720.f, 420.f));
	TArray<FVector> Points;
	Points.Add(W(3150.f, 300.f, 128.f));
	Points.Add(W(3300.f, 550.f, 128.f));
	Points.Add(W(3050.f, 600.f, 128.f));
	Points.Add(W(3380.f, 250.f, 128.f));
	SpawnGhost(Room, Points, TEXT("车"));
}

void ANightSchoolDirector::SpawnReadGhost()
{
	if (bReadGhost)
	{
		return;
	}
	bReadGhost = true;
	const FBox Room(W(2980.f, 880.f, -20.f), W(3460.f, 1500.f, 420.f));
	TArray<FVector> Points;
	Points.Add(W(3150.f, 1100.f, 128.f));
	Points.Add(W(3350.f, 1250.f, 128.f));
	Points.Add(W(3080.f, 1400.f, 128.f));
	Points.Add(W(3360.f, 1000.f, 128.f));
	SpawnGhost(Room, Points, TEXT("页"));
}

void ANightSchoolDirector::TryOpenReadingRoom()
{
	if (!bPuzzle9)
	{
		return;
	}
	bReadOpen = true;
	if (ANightSchoolProp* Door = FindProp(ENightPropId::ReadDoor))
	{
		Door->OpenDoor();
	}
	SpawnReadGhost();
}

void ANightSchoolDirector::BuildSideMap()
{
	auto WallX = [this](float X, float Y0, float Y1)
	{
		const float Length = FMath::Abs(Y1 - Y0);
		AddBox(W(X, (Y0 + Y1) * 0.5f, WallZ), FVector(Thick, Length, WallH), Plaster, true, true);
	};
	auto WallY = [this](float Y, float X0, float X1)
	{
		const float Length = FMath::Abs(X1 - X0);
		AddBox(W((X0 + X1) * 0.5f, Y, WallZ), FVector(Length, Thick, WallH), Plaster, true, true);
	};

	WallX(-1100.f, -2580.f, -1400.f);
	WallX(180.f, -2580.f, -1400.f);
	WallY(-2580.f, -1100.f, 180.f);
	WallX(-480.f, -2580.f, -2275.f);
	WallX(-480.f, -2125.f, -1400.f);
	AddBox(W(-480.f, -2200.f, LintelZ), FVector(Thick, 150.f, LintelH), Plaster, true, true);

	WallY(0.f, 2320.f, 3520.f);
	WallY(800.f, 2320.f, 2900.f);
	WallY(800.f, 2900.f, 3125.f);
	WallY(800.f, 3275.f, 3520.f);
	AddBox(W(3200.f, 800.f, LintelZ), FVector(150.f, Thick, LintelH), Plaster, true, true);
	WallX(3520.f, 0.f, 1580.f);
	WallX(2900.f, 0.f, 325.f);
	WallX(2900.f, 475.f, 800.f);
	AddBox(W(2900.f, 400.f, LintelZ), FVector(Thick, 150.f, LintelH), Plaster, true, true);
	WallX(2900.f, 800.f, 1580.f);
	WallY(1580.f, 2900.f, 3520.f);

	AddBox(W(-150.f, -1990.f, 314.f), FVector(620.f, 1100.f, 16.f), Ceiling, true, true);
	AddBox(W(-790.f, -1990.f, 314.f), FVector(560.f, 1100.f, 16.f), Ceiling, true, true);
	AddBox(W(2610.f, 400.f, 314.f), FVector(540.f, 720.f, 16.f), Ceiling, true, true);
	AddBox(W(3210.f, 400.f, 314.f), FVector(560.f, 720.f, 16.f), Ceiling, true, true);
	AddBox(W(3210.f, 1190.f, 314.f), FVector(560.f, 700.f, 16.f), Ceiling, true, true);
	AddBox(W(-150.f, -1990.f, 43.f), FVector(580.f, 1040.f, 4.f), Concrete, false, false);
	AddBox(W(-790.f, -1990.f, 43.f), FVector(520.f, 1040.f, 4.f), DarkWood, false, false);
	AddBox(W(2610.f, 400.f, 43.f), FVector(500.f, 680.f, 4.f), Concrete, false, false);
	AddBox(W(3210.f, 400.f, 43.f), FVector(520.f, 680.f, 4.f), DarkWood, false, false);
	AddBox(W(3210.f, 1190.f, 43.f), FVector(520.f, 640.f, 4.f), Concrete, false, false);
	AddBox(W(3300.f, 480.f, 150.f), FVector(16.f, 16.f, 220.f), DarkWood, false, false);
	AddBox(W(3080.f, 220.f, 150.f), FVector(16.f, 16.f, 220.f), DarkWood, false, false);

	const FLinearColor Brass(0.55f, 0.42f, 0.16f);
	SpawnProp(ENightPropId::SouthDoor, W(-200.f, -1400.f, 145.f), FVector(68.f, 8.f, 100.f), DarkWood, FText::FromString(TEXT("推开南甬")), FVector(200.f, 0.f, 0.f), 90.f, TEXT("南甬"));
	SpawnProp(ENightPropId::ToolDoor, W(-480.f, -2200.f, 145.f), FVector(8.f, 68.f, 100.f), DarkWood, FText::FromString(TEXT("进入工具间")), FVector(0.f, 200.f, 0.f), 0.f, TEXT("工具间"));
	SpawnProp(ENightPropId::SideDoor, W(2320.f, 400.f, 145.f), FVector(8.f, 68.f, 100.f), DarkWood, FText::FromString(TEXT("打开东边的门")), FVector(0.f, -200.f, 0.f), 180.f, TEXT("门廊"));
	SpawnProp(ENightPropId::ShedDoor, W(2900.f, 400.f, 145.f), FVector(8.f, 68.f, 100.f), DarkWood, FText::FromString(TEXT("进入车棚")), FVector(0.f, 200.f, 0.f), 180.f, TEXT("车棚"));
	SpawnProp(ENightPropId::ReadDoor, W(3200.f, 800.f, 145.f), FVector(68.f, 8.f, 100.f), DarkWood, FText::FromString(TEXT("进入旧阅览")), FVector(200.f, 0.f, 0.f), -90.f, TEXT("旧阅览"));

	SpawnProp(ENightPropId::SideKey, W(40.f, -2450.f, 96.f), FVector(10.f, 8.f, 4.f), Brass, FText::FromString(TEXT("捡起门房钥匙")), FVector::ZeroVector, 90.f, TEXT("钥匙"));
	SpawnProp(ENightPropId::StudyA, W(-20.f, -2000.f, 110.f), FVector(16.f, 12.f, 3.f), Paper, FText::FromString(TEXT("读夜自习名册")), FVector::ZeroVector, 90.f, TEXT("名册"));
	SpawnProp(ENightPropId::StudyB, W(3250.f, 350.f, 110.f), FVector(16.f, 12.f, 3.f), Paper, FText::FromString(TEXT("读车棚的旧锁")), FVector::ZeroVector, 180.f, TEXT("旧锁"));
	SpawnProp(ENightPropId::StudyC, W(3100.f, 1200.f, 110.f), FVector(16.f, 12.f, 3.f), Paper, FText::FromString(TEXT("读退休前的值班表")), FVector::ZeroVector, -90.f, TEXT("值班表"));
	SpawnProp(ENightPropId::StudyD, W(-600.f, 1250.f, 110.f), FVector(16.f, 12.f, 3.f), Paper, FText::FromString(TEXT("读宿管留下的字")), FVector::ZeroVector, 0.f, TEXT("宿管"));
	SpawnProp(ENightPropId::StudyBench, W(3300.f, 1480.f, 96.f), FVector(22.f, 14.f, 8.f), DarkWood, FText::FromString(TEXT("把四页放上桌子")), FVector::ZeroVector, -90.f, TEXT("阅览桌"));
	SpawnProp(ENightPropId::ToolCabinet, W(-900.f, -2300.f, 110.f), FVector(16.f, 12.f, 18.f), FLinearColor(0.2f, 0.32f, 0.18f), FText::FromString(TEXT("打开工具柜")), FVector::ZeroVector, 0.f, TEXT("电池"));

	AddLantern(-150.f, -2000.f, 230.f, 6.f, false);
	AddLantern(-800.f, -2100.f, 220.f, 5.f, false);
	AddLantern(2600.f, 400.f, 230.f, 6.f, false);
	AddLantern(3200.f, 400.f, 210.f, 5.f, false);
	AddLantern(3200.f, 1200.f, 210.f, 5.f, false);
	AddLabel(W(-200.f, -1500.f, 210.f), 90.f, TEXT("南甬"), 16.f, FColor(220, 200, 160));
	AddLabel(W(2500.f, 620.f, 210.f), 180.f, TEXT("门廊"), 16.f, FColor(220, 200, 160));

	SpawnSouthGhost();
	SpawnShedGhost();
}
