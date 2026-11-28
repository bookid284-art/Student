#include "NightSchool/NightSchoolDirector.h"

#include "Components/StaticMeshComponent.h"
#include "NightSchool/NightSchoolLayout.h"

namespace
{
	const FLinearColor Concrete(.075f, .083f, .09f);
	const FLinearColor Plaster(.105f, .114f, .115f);
	const FLinearColor Iron(.05f, .065f, .068f);
	const FLinearColor StepEdge(.19f, .21f, .21f);
	const FLinearColor Cabinet(.105f, .116f, .104f);
	const FLinearColor PaintedWood(.105f, .078f, .052f);
	const FColor Sign(183, 198, 184);
}

void ANightSchoolDirector::AddWorktable(const FVector& Top, float Width, float Depth, const FLinearColor& Color)
{
	constexpr float Leg = 7.f;
	const FVector Floor(Top.X, Top.Y, NightSchool::AtFloor(0.f, 0.f, NightSchool::FloorTop, NightSchool::FloorAt(Top)).Z);
	const float LegHeight = Top.Z - Floor.Z - 7.f;
	AddBox(Top - FVector(0.0, 0.0, 4.0), FVector(Width, Depth, 8.f), Color, true, true);
	for (float X : {-Width * .5f + 9.f, Width * .5f - 9.f})
	{
		for (float Y : {-Depth * .5f + 8.f, Depth * .5f - 8.f})
		{
			AddBox(Floor + FVector(X, Y, LegHeight * .5f), FVector(Leg, Leg, LegHeight), Iron, true, false);
		}
	}
}

void ANightSchoolDirector::BuildDormFloors()
{
	using namespace NightSchool;
	constexpr float LobbyWest = -2240.f;
	constexpr float LobbyEast = -300.f;
	constexpr float LobbySouth = -600.f;
	constexpr float WallWidth = 24.f;
	constexpr float CeilingHeight = 306.f;
	constexpr float DoorHalfWidth = 85.f;

	auto Deck = [&](int32 Floor, float X0, float X1, float Y0, float Y1, float Top, float Depth = 22.f)
	{
		AddBox(AtFloor((X0 + X1) * .5f, (Y0 + Y1) * .5f, Top - Depth * .5f, Floor),
			FVector(X1 - X0, Y1 - Y0, Depth), Concrete, true, true, WornTerrazzo);
	};
	auto WallX = [&](int32 Floor, float X, float Y0, float Y1)
	{
		AddBox(AtFloor(X, (Y0 + Y1) * .5f, 175.f, Floor), FVector(WallWidth, Y1 - Y0, 270.f),
			Plaster, true, true, DampPlaster);
	};
	auto WallY = [&](int32 Floor, float Y, float X0, float X1)
	{
		AddBox(AtFloor((X0 + X1) * .5f, Y, 175.f, Floor), FVector(X1 - X0, WallWidth, 270.f),
			Plaster, true, true, DampPlaster);
	};
	auto Header = [&](int32 Floor, float X, float Y)
	{
		AddBox(AtFloor(X, Y, 274.f, Floor), FVector(DoorHalfWidth * 2.f, WallWidth, 72.f),
			Plaster, true, true, DampPlaster);
		for (float Offset : {-1.f, 1.f})
		{
			AddBox(AtFloor(X + Offset * (DoorHalfWidth - 3.f), Y, 142.f, Floor), FVector(6.f, 35.f, 204.f), Iron, true, true);
		}
		AddBox(AtFloor(X, Y, 243.f, Floor), FVector(DoorHalfWidth * 2.f, 35.f, 8.f), Iron, true, true);
	};

	// The stair enclosure spans both half-landings and the floor-to-floor bands.
	const float ShaftTop = 2.f * FloorRise + CeilingHeight + 2.f;
	const float ShaftHeight = ShaftTop - FloorTop;
	const float ShaftCenter = (ShaftTop + FloorTop) * .5f;
	for (float X : {StairWestEdge, StairEastEdge})
	{
		AddBox(W(X, (StairStartY + StairNorth) * .5f, ShaftCenter),
			FVector(WallWidth, StairNorth - StairStartY + WallWidth, ShaftHeight), Plaster, true, true, DampPlaster);
	}
	AddBox(W((StairWestEdge + StairEastEdge) * .5f, StairNorth, ShaftCenter),
		FVector(StairEastEdge - StairWestEdge, WallWidth, ShaftHeight), Plaster, true, true, DampPlaster);
	WallX(0, StairWestEdge, LobbySouth, StairStartY);
	Deck(0, StairWestEdge, StairEastEdge, LobbySouth, StairStartY, FloorTop + 1.f);
	for (int32 Floor : {1, 2})
	{
		Deck(Floor, LobbyWest, StairWestEdge, LobbySouth, 12.f, FloorTop);
		Deck(Floor, StairEastEdge, LobbyEast, LobbySouth, 12.f, FloorTop);
		Deck(Floor, StairWestEdge, StairEastEdge, LobbySouth, StairStartY, FloorTop);
		WallX(Floor, LobbyWest, LobbySouth, 0.f);
		WallX(Floor, LobbyEast, LobbySouth, 0.f);
		WallY(Floor, LobbySouth, LobbyWest, LobbyEast);
		if (Floor == 2)
		{
			WallY(Floor, StairStartY, StairSplitX, StairEastEdge);
		}
	}

	constexpr int32 StepsPerFlight = 10;
	constexpr float StepHeight = FloorRise / (StepsPerFlight * 2);
	constexpr float Tread = (StairTurnY - StairStartY) / StepsPerFlight;
	for (int32 Floor : {0, 1})
	{
		Deck(Floor, StairWestEdge, StairEastEdge, StairTurnY, StairNorth,
			FloorTop + FloorRise * .5f);
		AddBox(AtFloor(-1340.f, -321.f, 278.f, Floor), FVector(8.f, 39.f, 13.f), Iron, true, false);
		AddBox(AtFloor(-1335.f, -321.f, 278.f, Floor), FVector(2.f, 31.f, 7.f),
			FLinearColor(.29f, .37f, .46f), false, false, CampusTube);
		AddBox(AtFloor(StairWestX, -40.5f, 449.f, Floor), FVector(39.f, 13.f, 13.f), Iron, true, false);
		AddBox(AtFloor(StairWestX, -47.8f, 449.f, Floor), FVector(32.f, 2.f, 7.f),
			FLinearColor(.29f, .37f, .46f), false, false, CampusTube);
		for (int32 Step = 0; Step < StepsPerFlight; ++Step)
		{
			for (bool bSecondFlight : {false, true})
			{
				const float X = bSecondFlight ? StairWestX : StairEastX;
				const float Y = bSecondFlight ? StairTurnY - (Step + .5f) * Tread : StairStartY + (Step + .5f) * Tread;
				const float Top = FloorTop + (Step + 1 + (bSecondFlight ? StepsPerFlight : 0)) * StepHeight;
				AddBox(AtFloor(X, Y, Top - 9.5f, Floor), FVector(244.f, Tread + .15f, 19.f),
					Concrete, true, true, WornTerrazzo);
				const float Nose = Y + (bSecondFlight ? 1.f : -1.f) * (Tread * .5f - 2.f);
				AddBox(AtFloor(X, Nose, Top - .8f, Floor), FVector(243.f, 3.5f, 1.6f), StepEdge, false, false);
			}
		}
	}
	// Solid central spine, separated from the common landing at each end.
	AddBox(W(StairSplitX, (StairStartY + StairTurnY) * .5f, ShaftCenter),
		FVector(22.f, StairTurnY - StairStartY, ShaftHeight), Plaster, true, true, DampPlaster);
	const float ServiceTop = FloorTop + FloorRise - 19.f;
	AddBox(W((StairWestEdge + StairSplitX) * .5f, StairStartY - 9.f, (ServiceTop + FloorTop) * .5f),
		FVector(StairSplitX - StairWestEdge + 2.f, 20.f, ServiceTop - FloorTop), Plaster, true, true, DampPlaster);
	AddBox(W(StairWestX, StairStartY - 22.f, 143.f), FVector(96.f, 7.f, 206.f), Iron, true, true);
	AddLabel(W(StairWestX, StairStartY - 26.f, 150.f), -90.f, TEXT("设备间"), 16.f, Sign);
	AddLabel(W(StairEastEdge + 8.f, StairEntryY, 258.f), 0.f, TEXT("宿舍楼梯  ↑"), 17.f, Sign);
	for (int32 Floor : {1, 2})
	{
		AddLabel(AtFloor(StairSplitX, LobbySouth + 14.f, 217.f, Floor), 90.f, FloorName(Floor), 24.f, Sign);
		AddLantern(-1010.f, -504.f, 234.f, 5.f, false, false, Floor);
		AddLantern(-1790.f, -340.f, 234.f, 4.f, false, false, Floor);
	}

	// 2F dorm room. Its entrance and all gameplay objects keep the same orientation.
	Deck(1, -1112.f, -288.f, 0.f, 812.f, FloorTop);
	Deck(1, -1112.f, -288.f, -612.f, 812.f, CeilingHeight + 16.f, 16.f);
	Deck(1, LobbyWest, StairWestEdge, LobbySouth, 0.f, CeilingHeight + 16.f, 16.f);
	WallX(1, -1100.f, 0.f, 800.f);
	WallX(1, -300.f, 0.f, 800.f);
	WallY(1, 800.f, -1100.f, -300.f);
	WallY(1, 0.f, LobbyWest, DormRoomDoorX - DoorHalfWidth);
	WallY(1, 0.f, DormRoomDoorX + DoorHalfWidth, LobbyEast);
	Header(1, DormRoomDoorX, 0.f);
	AddLabel(AtFloor(DormRoomDoorX, -20.f, 267.f, 1), -90.f, TEXT("203 宿舍"), 16.f, Sign);
	AddWorktable(Dorm(-520.f, 360.f, 94.f), 80.f, 46.f, PaintedWood);

	// 3F hall, the long board to the west, and the old reading room to the east.
	Deck(2, LobbyWest, LobbyEast, LobbySouth, 0.f, CeilingHeight + 16.f, 16.f);
	Deck(2, -2242.f, -1139.f, 0.f, 812.f, FloorTop);
	Deck(2, -2242.f, -1139.f, StairNorth, 812.f, CeilingHeight + 16.f, 16.f);
	Deck(2, -1032.f, -388.f, 0.f, 792.f, FloorTop);
	Deck(2, -1032.f, -388.f, StairNorth, 792.f, CeilingHeight + 16.f, 16.f);
	WallX(2, -2230.f, 0.f, 800.f);
	WallX(2, -1151.f, 0.f, 800.f);
	WallX(2, -1020.f, 0.f, 780.f);
	WallX(2, -400.f, 0.f, 780.f);
	WallY(2, 800.f, -2230.f, -1151.f);
	WallY(2, 780.f, -1020.f, -400.f);
	WallY(2, 0.f, LobbyWest, AnnexRoomDoorX - DoorHalfWidth);
	WallY(2, 0.f, AnnexRoomDoorX + DoorHalfWidth, ReadingRoomDoorX - DoorHalfWidth);
	WallY(2, 0.f, ReadingRoomDoorX + DoorHalfWidth, LobbyEast);
	Header(2, AnnexRoomDoorX, 0.f);
	Header(2, ReadingRoomDoorX, 0.f);
	auto RoomSign = [&](float X, const TCHAR* Name)
	{
		AddBox(AtFloor(X, -16.f, 269.f, 2), FVector(132.f, 8.f, 23.f), Iron, true, false);
		AddLabel(AtFloor(X, -21.f, 269.f, 2), -90.f, Name, 14.f, Sign);
		AddBox(AtFloor(X, -19.f, 290.f, 2), FVector(78.f, 14.f, 10.f), Iron, true, false);
		AddBox(AtFloor(X, -20.f, 284.f, 2), FVector(65.f, 7.f, 2.f), Plaster, false, false, CampusTube);
	};
	RoomSign(AnnexRoomDoorX, TEXT("长榜展室"));
	RoomSign(ReadingRoomDoorX, TEXT("旧阅览室"));
	AddLantern(-1820.f, 405.f, 232.f, 6.f, false, false, 2);
	AddLantern(-725.f, 355.f, 232.f, 5.f, false, false, 2);

	AddBox(Annex(1550.f, 2681.f, 167.f), FVector(895.f, 14.f, 118.f), Cabinet, true, true);
	AddBox(Annex(1550.f, 2672.f, 109.f), FVector(928.f, 20.f, 7.f), Iron, true, false);
	AddWorktable(Reading(3100.f, 1200.f, 107.f), 95.f, 81.f, PaintedWood);
	AddWorktable(Reading(3300.f, 1480.f, 88.f), 104.f, 72.f, PaintedWood);
	for (float Y : {250.f, 440.f, 630.f})
	{
		AddBox(AtFloor(-425.f, Y, 129.f, 2), FVector(41.f, 172.f, 178.f), Cabinet, true, true);
		for (float Z : {80.f, 124.f, 167.f, 208.f})
		{
			AddBox(AtFloor(-443.f, Y, Z, 2), FVector(15.f, 163.f, 5.f), Iron, true, false);
		}
	}
}
