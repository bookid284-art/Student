#include "NightSchool/NightSchoolDirector.h"

#include "Components/StaticMeshComponent.h"

void ANightSchoolDirector::BuildServiceRooms()
{
	const FLinearColor Wood(.145f, .098f, .06f);
	const FLinearColor Iron(.056f, .073f, .066f);
	const FLinearColor Linen(.35f, .34f, .28f);
	const FLinearColor Cabinet(.16f, .189f, .165f);
	const FLinearColor Metal(.29f, .31f, .295f);
	auto Box = [this](const FVector& P, const FVector& Size, const FLinearColor& Color, bool bSolid = false)
	{
		return AddBox(W(P.X, P.Y, P.Z), Size, Color, true, bSolid);
	};
	auto Pipe = [this](const FVector& A, const FVector& B, float Radius, const FLinearColor& Color)
	{
		const FVector Center = (A + B) * .5f;
		const FVector Span = B - A;
		const FRotator Rot = FQuat::FindBetweenNormals(FVector::UpVector, Span.GetSafeNormal()).Rotator();
		AddCylinder(W(Center.X, Center.Y, Center.Z), Radius, Span.Size(), Rot, Color, false);
	};

	// The route between the foyer and the telephone room stays clear of the duty staff's furniture.
	AddWorktable(W(-835.f, -1240.f, 116.f), 162.f, 66.f, Wood);
	Box(FVector(-886.f, -1241.f, 92.f), FVector(47.f, 63.f, 34.f), Wood, true);
	Box(FVector(-886.f, -1208.f, 96.f), FVector(41.f, 4.f, 21.f), Wood * .82f);
	Box(FVector(-886.f, -1208.f, 81.f), FVector(41.f, 4.f, 7.f), Wood * .91f);
	Pipe(FVector(-892.f, -1202.f, 81.f), FVector(-880.f, -1202.f, 81.f), 1.6f, Metal);

	auto Chair = [&Box, &Pipe, &Iron, &Wood](const FVector2D& Position, float Yaw)
	{
		const bool bSideways = Yaw != 0.f;
		auto SeatPart = [&](float X, float Y, float Z)
		{
			return FVector(Position, Z) + FVector(X, Y, 0.f).RotateAngleAxis(Yaw, FVector::UpVector);
		};
		Box(SeatPart(0, 0, 84.f), FVector(44.f, 44.f, 6.f), Wood, true);
		for (float X : {-17.f, 17.f})
		{
			Pipe(SeatPart(X, -17.f, 40.f), SeatPart(X, -17.f, 83.f), 2.6f, Iron);
			Pipe(SeatPart(X, 17.f, 40.f), SeatPart(X, 17.f, 124.f), 2.6f, Iron);
		}
		Box(SeatPart(0, 17.f, 116.f), bSideways ? FVector(5.f, 44.f, 20.f) : FVector(44.f, 5.f, 20.f),
			Wood, true);
	};
	Chair(FVector2D(-850.f, -1157.f), 0.f);
	Chair(FVector2D(-467.f, -803.f), -90.f);

	Box(FVector(-947.f, -665.f, 121.f), FVector(121.f, 64.f, 162.f), Cabinet, true);
	for (float X : {-976.f, -918.f})
	{
		Box(FVector(X, -700.f, 121.f), FVector(55.f, 6.f, 150.f), Cabinet * .91f);
		for (float Z : {173.f, 178.f, 183.f})
		{
			Box(FVector(X, -703.5f, Z), FVector(32.f, 1.8f, 2.4f), Iron);
		}
	}
	Pipe(FVector(-959.f, -707.f, 104.f), FVector(-959.f, -707.f, 121.f), 1.5f, Metal);
	Pipe(FVector(-931.f, -707.f, 104.f), FVector(-931.f, -707.f, 121.f), 1.5f, Metal);
	Box(FVector(-946.f, -703.5f, 145.f), FVector(43.f, 3.f, 29.f), Wood);

	constexpr float BedX = -465.f;
	constexpr float BedY = -1236.f;
	Box(FVector(BedX, BedY, 77.f), FVector(90.f, 201.f, 9.f), Iron, true);
	Box(FVector(BedX, BedY, 88.f), FVector(86.f, 196.f, 13.f), Linen, true);
	Box(FVector(BedX, BedY - 68.f, 100.f), FVector(57.f, 38.f, 11.f), Linen * 1.27f);
	Box(FVector(BedX, BedY + 31.f, 97.f), FVector(85.f, 127.f, 6.f), FLinearColor(.12f, .163f, .14f));
	Box(FVector(BedX - 41.f, BedY + 31.f, 86.f), FVector(5.f, 123.f, 20.f), FLinearColor(.10f, .14f, .12f));
	for (float X : {BedX - 40.f, BedX + 40.f})
	{
		Pipe(FVector(X, BedY - 98.f, 40.f), FVector(X, BedY - 98.f, 123.f), 2.9f, Iron);
		Pipe(FVector(X, BedY + 98.f, 40.f), FVector(X, BedY + 98.f, 104.f), 2.9f, Iron);
	}
	for (float Y : {BedY - 98.f, BedY + 98.f})
	{
		Pipe(FVector(BedX - 40.f, Y, 102.f), FVector(BedX + 40.f, Y, 102.f), 2.8f, Iron);
	}
	Pipe(FVector(BedX - 40.f, BedY - 98.f, 121.f), FVector(BedX + 40.f, BedY - 98.f, 121.f), 2.9f, Iron);

	// The laundry equipment lines the east wall, leaving both north and south doorways open.
	const FLinearColor Enamel(.40f, .427f, .392f);
	const FLinearColor Rubber(.033f, .043f, .043f);
	const FLinearColor Glass(.045f, .09f, .106f);
	for (float Y : {-1060.f, -1198.f})
	{
		Box(FVector(231.f, Y, 88.f), FVector(68.f, 69.f, 96.f), Enamel, true);
		Box(FVector(231.f, Y, 47.f), FVector(71.f, 70.f, 12.f), Iron);
		Box(FVector(195.5f, Y, 124.f), FVector(3.f, 62.f, 16.f), Cabinet);
		Box(FVector(193.5f, Y + 17.f, 124.5f), FVector(2.f, 21.f, 7.f), Rubber);
		for (float Offset : {-19.f, -8.f})
		{
			Pipe(FVector(190.f, Y + Offset, 124.f), FVector(195.f, Y + Offset, 124.f), 3.f, Metal);
		}
		Pipe(FVector(192.f, Y, 85.f), FVector(198.f, Y, 85.f), 26.f, Rubber);
		Pipe(FVector(190.f, Y, 85.f), FVector(192.4f, Y, 85.f), 23.f, Metal);
		Pipe(FVector(187.5f, Y, 85.f), FVector(190.4f, Y, 85.f), 20.f, Glass);
		Pipe(FVector(186.f, Y + 21.5f, 81.f), FVector(186.f, Y + 21.5f, 91.f), 1.7f, Cabinet);
	}

	constexpr float SinkY = -807.f;
	Box(FVector(231.f, SinkY, 113.f), FVector(73.f, 103.f, 9.f), Enamel, true);
	for (float Y : {SinkY - 48.f, SinkY + 48.f})
	{
		Box(FVector(231.f, Y, 121.f), FVector(73.f, 8.f, 13.f), Enamel, true);
		for (float X : {201.f, 258.f})
		{
			Pipe(FVector(X, Y, 40.f), FVector(X, Y, 116.f), 3.4f, Iron);
		}
	}
	Box(FVector(199.f, SinkY, 121.f), FVector(9.f, 90.f, 13.f), Enamel, true);
	Box(FVector(259.f, SinkY, 124.f), FVector(17.f, 90.f, 19.f), Enamel, true);
	Box(FVector(263.f, SinkY, 141.f), FVector(9.f, 105.f, 17.f), Enamel);
	for (float Y : {SinkY - 26.f, SinkY + 26.f})
	{
		Pipe(FVector(254.f, Y, 132.f), FVector(254.f, Y, 153.f), 2.4f, Metal);
		Pipe(FVector(255.f, Y, 152.f), FVector(238.f, Y, 152.f), 2.4f, Metal);
		Pipe(FVector(239.f, Y, 153.f), FVector(239.f, Y, 145.f), 2.5f, Metal);
		Pipe(FVector(254.f, Y - 6.f, 155.f), FVector(254.f, Y + 6.f, 155.f), 1.55f, Metal);
		Pipe(FVector(232.f, Y, 117.5f), FVector(232.f, Y, 119.f), 3.1f, Rubber);
	}
	Pipe(FVector(232.f, SinkY, 109.f), FVector(232.f, SinkY, 68.f), 4.2f, Iron);
	Pipe(FVector(230.f, SinkY, 69.f), FVector(274.f, SinkY, 69.f), 4.2f, Iron);

	Pipe(FVector(-110.f, -1356.f, 169.f), FVector(179.f, -1356.f, 169.f), 1.05f, Linen * .72f);
	for (float X : {-110.f, 179.f})
	{
		Pipe(FVector(X, -1355.f, 169.f), FVector(X, -1387.f, 169.f), 1.9f, Iron);
	}
	for (float X : {-83.f, -67.f})
	{
		Box(FVector(X, -1355.f, 168.f), FVector(2.5f, 7.5f, 6.f), Wood);
	}
}
