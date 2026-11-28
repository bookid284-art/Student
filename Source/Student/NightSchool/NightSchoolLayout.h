#pragma once

#include "CoreMinimal.h"

/** 里屋抬到模板关卡上方，避免和第一人称示例几何叠在一起。 */
namespace NightSchool
{
	inline constexpr float DormFacadeX = 300.f;
	inline constexpr float DormEntranceY = -310.f;
	inline constexpr float FloorTop = 40.f;
	inline constexpr float FloorRise = 350.f;
	inline constexpr float StairEastX = -1220.f;
	inline constexpr float StairWestX = -1490.f;
	inline constexpr float StairSplitX = -1355.f;
	inline constexpr float StairWestEdge = -1620.f;
	inline constexpr float StairEastEdge = -1090.f;
	inline constexpr float StairStartY = -430.f;
	inline constexpr float StairTurnY = -160.f;
	inline constexpr float StairNorth = -22.f;
	inline constexpr float StairLandingY = -510.f;
	inline constexpr float StairEntryY = -490.f;
	inline constexpr float DormRoomDoorX = -700.f;
	inline constexpr float AnnexRoomDoorX = -1870.f;
	inline constexpr float ReadingRoomDoorX = -720.f;

	inline FVector Origin()
	{
		return FVector(0.f, 0.f, 20000.f);
	}

	inline FVector AtFloor(float X, float Y, float Z, int32 Floor)
	{
		return Origin() + FVector(X, Y, Z + Floor * FloorRise);
	}

	inline FVector Dorm(float X, float Y, float Z) { return AtFloor(X, Y, Z, 1); }
	inline FVector Annex(float X, float Y, float Z) { return AtFloor(-2230.f + (X - 900.f) * .83f, Y - 1900.f, Z, 2); }
	inline FVector Reading(float X, float Y, float Z) { return AtFloor(X - 3920.f, Y - 800.f, Z, 2); }

	inline int32 FloorAt(const FVector& Point)
	{
		return FMath::Clamp(FMath::FloorToInt((Point.Z - Origin().Z - FloorTop + 3.f) / FloorRise), 0, 2);
	}

	inline const TCHAR* FloorName(int32 Floor)
	{
		const TCHAR* Names[] = {TEXT("一楼"), TEXT("二楼"), TEXT("三楼")};
		return Names[FMath::Clamp(Floor, 0, 2)];
	}

	inline FVector ApproachGroundStairs(const FVector& Local)
	{
		auto Through = [](float X, float Y) { return AtFloor(X, Y, 138.f, 0); };
		if (Local.Y < -1400.f)
		{
			return Local.X < -480.f ? Through(-442.f, -2200.f) : Through(-200.f, -1365.f);
		}
		if (Local.Y < -600.f)
		{
			if (Local.X > DormFacadeX) { return Through(DormFacadeX - 23.f, DormEntranceY); }
			if (Local.X < -1100.f) { return Through(-1065.f, -1000.f); }
			if (Local.X > -350.f) { return Through(-387.f, -1000.f); }
			return Through(-700.f, -564.f);
		}
		if (Local.Y < 0.f)
		{
			return Local.X > DormFacadeX ? Through(DormFacadeX - 23.f, DormEntranceY)
				: Through(StairEastEdge - 55.f, StairEntryY);
		}
		if (Local.Y >= 1400.f && Local.X >= 900.f)
		{
			return Through(1200.f, 1365.f);
		}
		if (Local.Y >= 800.f && Local.X >= 300.f)
		{
			if (Local.X >= 1600.f) { return Through(1565.f, 1100.f); }
			if (Local.X >= 900.f) { return Through(865.f, 1050.f); }
			return Local.Y >= 1400.f ? Through(600.f, 1365.f) : Through(600.f, 765.f);
		}
		if (Local.X >= 300.f)
		{
			if (Local.X >= 2900.f) { return Through(2865.f, 400.f); }
			if (Local.X >= 2320.f) { return Through(2285.f, 400.f); }
			if (Local.X >= 2000.f) { return Through(1965.f, 400.f); }
			if (Local.X >= 1200.f) { return Through(1165.f, 400.f); }
			return Through(265.f, 400.f);
		}
		if (Local.X < -300.f)
		{
			return Local.Y >= 1400.f ? Through(-265.f, 1500.f)
				: Through(-700.f, Local.Y >= 800.f ? 763.f : -36.f);
		}
		return Through(0.f, Local.Y >= 1000.f ? 965.f : -36.f);
	}

	inline TOptional<FVector> StairWaypoint(const FVector& Feet, int32 DestinationFloor)
	{
		const FVector Local = Feet - Origin();
		const int32 Floor = FloorAt(Feet);
		const bool bInFlight = Local.X > StairWestEdge && Local.X < StairEastEdge
			&& Local.Y > StairStartY && Local.Y < -22.f;
		if (bInFlight)
		{
			const float Progress = FMath::Clamp((Local.Y - StairStartY) / (StairTurnY - StairStartY), 0.0, 1.0);
			const float HeightOnFlight = FloorRise * .5f * (Local.X > StairSplitX ? Progress : 2.f - Progress);
			const int32 Flight = FMath::Clamp(FMath::RoundToInt((Local.Z - FloorTop - HeightOnFlight) / FloorRise), 0, 1);
			const bool bUp = DestinationFloor > Flight;
			const float StartX = bUp ? StairEastX : StairWestX;
			const float EndX = bUp ? StairWestX : StairEastX;
			const bool bAtStartSide = (Local.X > StairSplitX) == bUp;
			if (!bAtStartSide)
			{
				if (Local.Y >= StairTurnY + 46.f && FMath::Abs(Local.X - EndX) > 56.f)
				{
					return AtFloor(EndX, -90.f, 133.f + FloorRise * .5f, Flight);
				}
				return AtFloor(EndX, StairLandingY, 133.f, Flight + (bUp ? 1 : 0));
			}
			if (Local.Y < StairTurnY + 46.f)
			{
				return AtFloor(StartX, -90.f, 133.f + FloorRise * .5f, Flight);
			}
			return AtFloor(EndX, -90.f, 133.f + FloorRise * .5f, Flight);
		}
		if (DestinationFloor == Floor)
		{
			return {};
		}
		if (Floor == 0 && (Local.X >= StairEastEdge - 24.f || Local.Y < -590.f || Local.Y >= StairNorth))
		{
			return ApproachGroundStairs(Local);
		}
		if (Floor > 0 && Local.Y > StairStartY - 39.f)
		{
			return AtFloor(Local.X, StairLandingY, 142.f, Floor);
		}
		const bool bUp = DestinationFloor > Floor;
		const float FlightX = bUp ? StairEastX : StairWestX;
		// Approach the flight via its landing, keeping the waypoint clear of the stairwell wall.
		if (FMath::Abs(Local.X - FlightX) > 74.f || Local.Y < StairLandingY - 65.f || Local.Y > StairStartY + 4.f)
		{
			return AtFloor(FlightX, StairLandingY, 145.f, Floor);
		}
		return AtFloor(FlightX, -363.f, bUp ? 190.f : 90.f, Floor);
	}

	inline TOptional<FVector> RouteWaypoint(const FVector& Feet, const FVector& Destination)
	{
		const FVector Local = Feet - Origin();
		const FVector Goal = Destination - Origin();
		const int32 Floor = FloorAt(Feet);
		const int32 GoalFloor = FloorAt(Destination);
		const bool bSameFloor = Floor == GoalFloor;
		const bool bPlayerWest = Floor == 2 && Local.X < -1100.f;
		const bool bGoalWest = GoalFloor == 2 && Goal.X < -1100.f;
		const bool bInUpperRoom = Floor > 0 && Local.Y >= 44.f;
		const bool bGoalInUpperRoom = GoalFloor > 0 && Goal.Y >= -12.f;
		if (bInUpperRoom && (!bSameFloor || !bGoalInUpperRoom || bPlayerWest != bGoalWest))
		{
			const float DoorX = Floor == 1 ? DormRoomDoorX : bPlayerWest ? AnnexRoomDoorX : ReadingRoomDoorX;
			const float TurnY = FMath::Abs(Local.X - DoorX) > 40.f ? 106.f : -76.f;
			return AtFloor(DoorX, TurnY, 141.f, Floor);
		}
		if (const TOptional<FVector> Stair = StairWaypoint(Feet, GoalFloor); Stair.IsSet())
		{
			return Stair;
		}
		if (!bInUpperRoom && bSameFloor && bGoalInUpperRoom)
		{
			const float LobbyX = bGoalWest ? StairWestEdge - 88.f : StairEastEdge + 86.f;
			const bool bAcrossStairwell = bGoalWest ? Local.X > StairWestEdge - 52.f : Local.X < StairEastEdge + 52.f;
			if (bAcrossStairwell)
			{
				if ((Local.X < StairWestEdge || Local.X > StairEastEdge) && Local.Y > StairStartY - 39.f)
				{
					return AtFloor(Local.X, StairLandingY, 142.f, Floor);
				}
				return AtFloor(LobbyX, StairLandingY, 142.f, Floor);
			}
			const float DoorX = Floor == 1 ? DormRoomDoorX : bGoalWest ? AnnexRoomDoorX : ReadingRoomDoorX;
			if (FMath::Abs(Local.X - DoorX) > 40.f)
			{
				return AtFloor(DoorX, -105.f, 142.f, Floor);
			}
			if (Goal.Y > 26.f)
			{
				return AtFloor(DoorX, 71.f, 141.f, Floor);
			}
		}
		if (Floor == 0 && bSameFloor && Local.Y < StairNorth && Local.Y > -590.f
			&& Local.X > StairWestEdge && Local.X < StairEastEdge - 15.f)
		{
			return AtFloor(StairEastEdge + 83.f, StairEntryY, 144.f, 0);
		}
		if (Floor == 0 && bSameFloor && Local.Y < 0.f && Local.Y > -600.f
			&& Local.X < DormFacadeX && Goal.Y < -620.f && Goal.X < DormFacadeX)
		{
			return AtFloor(-700.f, -641.f, 141.f, 0);
		}
		return {};
	}

	/** 胶囊半高 88，地面顶面相对高度 40。出生在宿舍楼外，面朝入口。 */
	inline FVector PlayerSpawn()
	{
		return Origin() + FVector(1020.f, -420.f, 132.f);
	}

	inline FRotator PlayerFacing()
	{
		return FRotator(0.f, 171.f, 0.f);
	}
}
