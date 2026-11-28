#include "NightSchool/NightSchoolDirector.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SpotLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Math/Box2D.h"
#include "Math/RotationMatrix.h"
#include "NightSchool/NightSchoolArt.h"
#include "NightSchool/NightSchoolLayout.h"
#include "NightSchool/NightSchoolProp.h"

namespace
{
	constexpr float Facade = NightSchool::DormFacadeX;
	constexpr float Entrance = NightSchool::DormEntranceY;
	const FVector StreetLamp(1000.f, -588.f, 432.f);
	const FVector FarLamp(1770.f, -945.f, 396.f);
	const FVector EntryLamp(356.f, Entrance, 262.f);
	constexpr float SignInY = -476.f;
	constexpr float AnnouncementY = -136.f;
	const FLinearColor Iron(0.044f, 0.057f, 0.049f);
	const FLinearColor Soot(0.032f, 0.036f, 0.037f);
	const FLinearColor Concrete(0.23f, 0.22f, 0.18f);
	const FLinearColor DoorPaint(0.064f, 0.105f, 0.087f);
	const FLinearColor GreenGlass(0.025f, 0.045f, 0.05f);
	constexpr float SoilTop = 41.8f;
	constexpr float CurbWidth = 7.f;
	constexpr float SouthWallY = -1310.f;
	constexpr float SouthWallWest = 310.f;
	constexpr float SouthWallEast = 2254.f;

	struct FTreePlanting
	{
		FVector2D Position;
		FVector2D BedHalfSize;
		float Yaw;
		float Scale;
	};

	const FTreePlanting Trees[] = {
		{{952.f, -1015.f}, {90.f, 86.f}, 29.f, 1.02f},
		{{1680.f, -1152.f}, {81.f, 80.f}, 92.f, 0.82f},
		{{575.f, -944.f}, {78.f, 75.f}, -33.f, 0.77f},
		{{1120.f, -111.f}, {75.f, 60.f}, 162.f, 0.87f},
		{{2050.f, -805.f}, {84.f, 81.f}, 233.f, 0.88f}
	};

	void CutTreeBed(TArray<FBox2D>& Paving, const FBox2D& Bed)
	{
		TArray<FBox2D> Pieces;
		for (const FBox2D& Slab : Paving)
		{
			const FVector2D Low(FMath::Max(Slab.Min.X, Bed.Min.X), FMath::Max(Slab.Min.Y, Bed.Min.Y));
			const FVector2D High(FMath::Min(Slab.Max.X, Bed.Max.X), FMath::Min(Slab.Max.Y, Bed.Max.Y));
			if (Low.X >= High.X || Low.Y >= High.Y)
			{
				Pieces.Add(Slab);
				continue;
			}
			auto AddPiece = [&Pieces](const FVector2D& Min, const FVector2D& Max)
			{
				if (Max.X > Min.X && Max.Y > Min.Y)
				{
					Pieces.Emplace(Min, Max);
				}
			};
			AddPiece(Slab.Min, FVector2D(Low.X, Slab.Max.Y));
			AddPiece(FVector2D(High.X, Slab.Min.Y), Slab.Max);
			AddPiece(FVector2D(Low.X, Slab.Min.Y), FVector2D(High.X, Low.Y));
			AddPiece(FVector2D(Low.X, High.Y), FVector2D(High.X, Slab.Max.Y));
		}
		Paving = MoveTemp(Pieces);
	}
}

void ANightSchoolDirector::BuildDormExterior()
{
	auto Block = [this](const FVector& Position, const FVector& Size, const FLinearColor& Color,
		bool bCollision = false, UMaterialInterface* Material = nullptr)
	{
		return AddBox(W(Position.X, Position.Y, Position.Z), Size, Color, true, bCollision, Material);
	};
	if (DormNightSky)
	{
		if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
		{
			UStaticMeshComponent* Sky = NewObject<UStaticMeshComponent>(this, TEXT("DormNightSky"));
			Sky->SetupAttachment(GetRootComponent());
			Sky->SetStaticMesh(Sphere);
			Sky->SetMaterial(0, DormNightSky);
			Sky->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Sky->SetCastShadow(false);
			Sky->SetVisibleInRayTracing(false);
			Sky->RegisterComponent();
			Sky->SetWorldLocation(W(600.f, 400.f, 40.f));
			Sky->SetWorldScale3D(FVector(210.f));
		}
	}

	// The court extends east of the old foyer. Its entrance is the only opening in the building façade.
	TArray<FBox2D> Paving{FBox2D(FVector2D(310.f, -1332.f), FVector2D(2250.f, -2.f))};
	if (DormCourtyardTree)
	{
		for (const FTreePlanting& Tree : Trees)
		{
			CutTreeBed(Paving, FBox2D(Tree.Position - Tree.BedHalfSize, Tree.Position + Tree.BedHalfSize));
		}
	}
	for (const FBox2D& Slab : Paving)
	{
		UStaticMeshComponent* Pavers = Block(FVector(Slab.GetCenter(), 41.2f), FVector(Slab.GetSize(), 2.4f),
			Soot, false, DormExteriorTile);
		if (Pavers)
		{
			Pavers->ComponentTags.Add(TEXT("DormCourtyardPavers"));
		}
	}
	Block(FVector(1120.f, Entrance, 43.f), FVector(1595.f, 238.f, 2.f), Concrete, false, DormExteriorTile);
	Block(FVector(326.f, Entrance, 44.f), FVector(44.f, 196.f, 8.f), Concrete, true);
	for (float Side : {-1.f, 1.f})
	{
		Block(FVector(1130.f, Entrance + Side * 129.f, 47.f), FVector(1580.f, 11.f, 14.f), Soot);
	}

	Block(FVector(314.f, -663.f, 524.f), FVector(29.f, 1330.f, 416.f), Concrete, true, DormExteriorTile);
	Block(FVector(314.f, -663.f, 898.f), FVector(29.f, 1330.f, 336.f), Concrete, true, DormExteriorTile);
	Block(FVector(317.f, -506.f, 150.f), FVector(10.f, 190.f, 220.f), Concrete, false, DormExteriorTile);
	Block(FVector(317.f, -105.f, 150.f), FVector(10.f, 208.f, 220.f), Concrete, false, DormExteriorTile);
	Block(FVector(317.f, -964.f, 180.f), FVector(22.f, 728.f, 280.f), Concrete, true, DormExteriorTile);
	Block(FVector(326.f, -620.f, 327.f), FVector(42.f, 1250.f, 12.f), Soot);
	Block(FVector(329.f, -620.f, 731.f), FVector(48.f, 1250.f, 18.f), Soot);
	Block(FVector(329.f, -620.f, 1063.f), FVector(48.f, 1250.f, 18.f), Soot);
	Block(FVector(1267.f, 3.5f, 457.f), FVector(1882.f, 21.f, 281.f), Concrete, true, DormExteriorTile);
	Block(FVector(1288.f, -6.f, 594.f), FVector(1905.f, 28.f, 14.f), Soot);

	// Deep, dark balconies and repetitive school windows read from the streetlamp without lighting the rooms.
	for (float Level : {0.f, 350.f})
	{
		for (float Y : {-1072.f, -855.f, -638.f, -420.f, -200.f})
		{
			Block(FVector(333.f, Y, 529.f + Level), FVector(12.f, 140.f, 137.f), Soot);
			Block(FVector(341.f, Y, 527.f + Level), FVector(8.f, 122.f, 118.f), GreenGlass);
			Block(FVector(346.f, Y, 532.f + Level), FVector(5.f, 132.f, 5.f), Iron);
			Block(FVector(346.f, Y, 527.f + Level), FVector(5.f, 4.f, 122.f), Iron);
			Block(FVector(348.f, Y, 457.f + Level), FVector(28.f, 155.f, 8.f), Concrete);
		}
	}
	for (float Y : {-1201.f, -617.f, -29.f})
	{
		AddCylinder(W(339.f, Y, 545.f), 3.6f, 1002.f, FRotator::ZeroRotator, Iron, false);
		for (float Z : {96.f, 309.f, 528.f, 830.f, 1000.f})
		{
			Block(FVector(336.f, Y, Z), FVector(10.f, 13.f, 8.f), Soot);
		}
	}

	// The entrance canopy covers the step only. The player approaches below open sky.
	Block(FVector(367.f, Entrance, 307.f), FVector(122.f, 261.f, 16.f), Soot);
	Block(FVector(426.f, Entrance, 308.f), FVector(9.f, 267.f, 31.f), Concrete);
	Block(FVector(323.f, Entrance, 250.f), FVector(19.f, 196.f, 12.f), Iron);
	for (float Side : {-1.f, 1.f})
	{
		Block(FVector(320.f, Entrance + Side * 98.f, 146.f), FVector(23.f, 12.f, 210.f), Iron);
	}
	if (ANightSchoolProp* Door = SpawnProp(ENightPropId::DormEntrance, W(Facade, Entrance, 145.f),
		FVector(9.f, 92.f, 105.f), DoorPaint, FText::FromString(TEXT("推开宿舍大门")), FVector(0.f, 195.f, 0.f), 0.f, FString()))
	{
		auto DoorDetail = [Door, &Block](const FVector& Offset, const FVector& Size, const FLinearColor& Color)
		{
			const FVector Center = Door->GetActorLocation() - NightSchool::Origin() + Offset;
			if (UStaticMeshComponent* Part = Block(Center, Size, Color))
			{
				Part->AttachToComponent(Door->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
			}
		};
		for (float Side : {-1.f, 1.f})
		{
			const float Face = Side * 11.f;
			DoorDetail(FVector(Face, 0.f, 54.f), FVector(4.f, 157.f, 67.f), GreenGlass);
			for (float Y : {-58.f, -29.f, 0.f, 29.f, 58.f})
			{
				DoorDetail(FVector(Face + Side * 3.f, Y, 54.f), FVector(5.f, 3.3f, 68.f), Iron);
			}
			DoorDetail(FVector(Face + Side * 3.f, 0.f, 49.f), FVector(5.f, 159.f, 4.f), Iron);
			DoorDetail(FVector(Face, 0.f, -48.f), FVector(4.f, 158.f, 84.f), Iron);
		}
	}

	// Printed texture headings keep their exact Chinese forms in both the world and the read view.
	UStaticMesh* Plane = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	auto Plate = [this, &Block, Plane](FName Name, const FVector& Position, float Width, float Height,
		UMaterialInterface* Material)
	{
		Block(Position - FVector(6.5f, 0.f, 0.f), FVector(7.f, Width + 9.f, Height + 9.f), Iron, true);
		if (!Plane || !Material)
		{
			return;
		}
		UStaticMeshComponent* Print = NewObject<UStaticMeshComponent>(this, Name);
		Print->SetupAttachment(GetRootComponent());
		Print->SetStaticMesh(Plane);
		Print->SetMaterial(0, Material);
		Print->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Print->SetCastShadow(false);
		Print->RegisterComponent();
		const FRotator Rotation = FRotationMatrix::MakeFromZX(FVector::ForwardVector, -FVector::RightVector).Rotator();
		Print->SetWorldLocationAndRotation(W(Position.X, Position.Y, Position.Z), Rotation);
		Print->SetWorldScale3D(FVector(Width / 100.f, Height / 100.f, 1.f));
	};
	Plate(TEXT("DormSignInPrint"), FVector(330.f, SignInY, 171.f), 119.f, 88.f, DormSignIn);
	Plate(TEXT("DormAnnouncementPrint"), FVector(330.f, AnnouncementY, 171.f), 119.f, 88.f, DormAnnouncement);
	Plate(TEXT("DormBuildingPlaque"), FVector(436.f, Entrance, 311.f), 133.f, 27.f, DormEntranceSign);
	SpawnProp(ENightPropId::DormSignIn, W(327.f, SignInY, 171.f), FVector(2.f, 59.f, 44.f), Soot,
		FText::FromString(TEXT("查看宿舍签到表")), FVector::ZeroVector, 0.f, FString());
	SpawnProp(ENightPropId::DormAnnouncement, W(327.f, AnnouncementY, 171.f), FVector(2.f, 59.f, 44.f), Soot,
		FText::FromString(TEXT("阅读宿舍公告")), FVector::ZeroVector, 0.f, FString());

	// Two roadside pools leave the railings, trees, and the far end of the court in shadow.
	for (const FVector& Lamp : {StreetLamp, FarLamp})
	{
		const float PoleY = Lamp.Y - 74.f;
		AddCylinder(W(Lamp.X, PoleY, (Lamp.Z + 40.f) / 2.f), 6.f, Lamp.Z - 40.f, FRotator::ZeroRotator, Iron, false);
		Block(FVector(Lamp.X, PoleY, 59.f), FVector(27.f, 27.f, 37.f), Soot, true);
		Block(FVector(Lamp.X, Lamp.Y - 33.f, Lamp.Z + 12.f), FVector(9.f, 92.f, 8.f), Iron);
		Block(Lamp + FVector(0.f, 0.f, 14.f), FVector(35.f, 47.f, 12.f), Iron);
		AddBox(W(Lamp.X, Lamp.Y, Lamp.Z + 6.f), FVector(26.f, 35.f, 3.f), Concrete, false, false, CampusTube);
	}
	Block(EntryLamp + FVector(-4.f, 0.f, 7.f), FVector(35.f, 47.f, 13.f), Iron);
	for (float Y : {-16.f, 16.f})
	{
		Block(EntryLamp + FVector(-7.f, Y, 25.f), FVector(12.f, 6.f, 27.f), Iron);
	}
	AddBox(W(EntryLamp.X, EntryLamp.Y, EntryLamp.Z), FVector(29.f, 36.f, 3.f), Concrete, false, false, CampusTube);

	// The footing and both returns overlap at the corners; the railing is seated into the masonry.
	const float SouthWallCenter = (SouthWallWest + SouthWallEast) * .5f;
	const float SouthWallLength = SouthWallEast - SouthWallWest;
	Block(FVector(SouthWallCenter, SouthWallY, 106.f), FVector(SouthWallLength, 31.f, 134.f),
		Soot, true, DormExteriorTile);
	Block(FVector(2248.f, -662.f, 107.f), FVector(24.f, 1324.f, 136.f), Soot, true, DormExteriorTile);
	Block(FVector(1308.f, -11.f, 180.f), FVector(2000.f, 18.f, 284.f), Soot, true, DormExteriorTile);
	for (float Y : {-1285.f, -1026.f, -767.f, -508.f, -249.f})
	{
		Block(FVector(2240.f, Y, 179.f), FVector(29.f, 31.f, 250.f), Soot);
	}
	for (int32 Index = 0; Index < 13; ++Index)
	{
		Block(FVector(430.f + 140.f * Index, SouthWallY, 223.f), FVector(8.f, 8.f, 106.f), Iron);
	}
	for (float X : {331.f, 2245.f})
	{
		Block(FVector(X, SouthWallY, 218.f), FVector(30.f, 40.f, 116.f), Soot, true, DormExteriorTile);
	}
	Block(FVector(SouthWallCenter, SouthWallY, 263.f), FVector(SouthWallLength - 16.f, 8.f, 8.f), Iron);
	Block(FVector(SouthWallCenter, SouthWallY, 174.f), FVector(SouthWallLength - 16.f, 8.f, 8.f), Iron);
	if (UStaticMeshComponent* FenceCollision = AddBox(W(SouthWallCenter, SouthWallY, 222.f),
		FVector(SouthWallLength - 16.f, 15.f, 110.f), Iron, false, true))
	{
		FenceCollision->SetHiddenInGame(true);
	}

	if (DormCourtyardTree)
	{
		const FLinearColor Loam(.125f, .079f, .048f);
		const FLinearColor Coping(.16f, .155f, .132f);
		TArray<UInstancedStaticMeshComponent*> LitterMeshes;
		if (UStaticMesh* Pebble = NightSchoolArt::Sphere())
		{
			for (const FLinearColor& Shade : {Loam * .65f, Loam * 1.27f, FLinearColor(.18f, .102f, .049f)})
			{
				UInstancedStaticMeshComponent* Litter = NewObject<UInstancedStaticMeshComponent>(this);
				Litter->SetupAttachment(GetRootComponent());
				Litter->SetStaticMesh(Pebble);
				Litter->SetMaterial(0, NightSchoolArt::MakeColor(this, Shade));
				Litter->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				Litter->SetCastShadow(false);
				Litter->RegisterComponent();
				LitterMeshes.Add(Litter);
			}
		}
		FRandomStream SoilVariation(5741);
		for (const FTreePlanting& Tree : Trees)
		{
			const float X = Tree.Position.X;
			const float Y = Tree.Position.Y;
			const FVector2D BedSize = Tree.BedHalfSize * 2.f;
			if (UStaticMeshComponent* Soil = Block(FVector(X, Y, SoilTop - 1.f), FVector(BedSize, 2.f), Loam, true))
			{
				Soil->ComponentTags.Add(TEXT("DormTreeSoil"));
			}
			for (float Side : {-1.f, 1.f})
			{
				Block(FVector(X + Side * (Tree.BedHalfSize.X - CurbWidth * .5f), Y, 43.5f),
					FVector(CurbWidth, BedSize.Y - 2.f * CurbWidth, 7.f), Coping, true);
				Block(FVector(X, Y + Side * (Tree.BedHalfSize.Y - CurbWidth * .5f), 43.5f),
					FVector(BedSize.X, CurbWidth, 7.f), Coping, true);
			}
			for (UInstancedStaticMeshComponent* Litter : LitterMeshes)
			{
				for (int32 Index = 0; Index < 24; ++Index)
				{
					const float LitterX = X + SoilVariation.FRandRange(-1.f, 1.f) * (Tree.BedHalfSize.X - CurbWidth - 5.f);
					const float LitterY = Y + SoilVariation.FRandRange(-1.f, 1.f) * (Tree.BedHalfSize.Y - CurbWidth - 5.f);
					const float Yaw = SoilVariation.FRandRange(0.f, 360.f);
					const float Width = SoilVariation.FRandRange(2.f, 9.f);
					const float Depth = SoilVariation.FRandRange(1.f, 4.f);
					Litter->AddInstance(FTransform(FRotator(0.f, Yaw, 0.f), W(LitterX, LitterY, SoilTop),
						FVector(Width, Depth, 1.5f) / 100.f), true);
				}
			}
			UStaticMeshComponent* Canopy = NewObject<UStaticMeshComponent>(this);
			Canopy->SetStaticMesh(DormCourtyardTree);
			Canopy->SetupAttachment(GetRootComponent());
			Canopy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Canopy->RegisterComponent();
			Canopy->SetWorldLocationAndRotation(W(X, Y, SoilTop - 4.5f), FRotator(0.f, Tree.Yaw, 0.f));
			Canopy->SetWorldScale3D(FVector(Tree.Scale));
			if (UStaticMeshComponent* TrunkCollision = Block(FVector(X, Y, 119.f),
				FVector(27.f * Tree.Scale, 27.f * Tree.Scale, 158.f), Soot, true))
			{
				TrunkCollision->SetHiddenInGame(true);
				TrunkCollision->SetCastShadow(false);
			}
		}
	}
}

void ANightSchoolDirector::BuildDormExteriorLighting()
{
	auto Spot = [this](const FVector& Source, const FVector& Target, const FLinearColor& Color,
		float Strength, float Radius, float Inner, float Outer, float Falloff, bool bShadows)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ASpotLight* Actor = GetWorld()->SpawnActor<ASpotLight>(ASpotLight::StaticClass(),
			FTransform(W(Source.X, Source.Y, Source.Z)), Params);
		if (!Actor)
		{
			return;
		}
		Actor->Tags.Add(TEXT("NightSchool"));
		Actor->SetMobility(EComponentMobility::Movable);
		Actor->SetActorRotation((Target - Source).Rotation());
		USpotLightComponent* Lamp = Actor->SpotLightComponent;
		Lamp->SetUseInverseSquaredFalloff(false);
		Lamp->SetLightFalloffExponent(Falloff);
		Lamp->SetAttenuationRadius(Radius);
		Lamp->SetIntensity(Strength);
		Lamp->SetInnerConeAngle(Inner);
		Lamp->SetOuterConeAngle(Outer);
		Lamp->SetLightColor(Color);
		Lamp->SetCastShadows(bShadows);
	};
	Spot(StreetLamp, FVector(975.f, -507.f, 45.f), FLinearColor(.87f, .79f, .64f), 10.5f, 830.f, 17.f, 43.f, 4.2f, true);
	Spot(FarLamp, FVector(1770.f, -876.f, 43.f), FLinearColor(.56f, .68f, .60f), 6.f, 674.f, 17.f, 34.f, 4.4f, true);
	Spot(EntryLamp - FVector(0.f, 0.f, 6.f), FVector(356.f, Entrance, 80.f),
		FLinearColor(.72f, .80f, .72f), 7.f, 360.f, 20.f, 52.f, 4.2f, true);
	Spot(FVector(439.f, Entrance, 287.f), FVector(316.f, Entrance, 178.f),
		FLinearColor(.64f, .76f, .63f), 4.5f, 420.f, 27.f, 61.f, 3.2f, false);
	Spot(FVector(870.f, -860.f, 636.f), FVector(314.f, -390.f, 512.f),
		FLinearColor(.48f, .56f, .71f), 4.2f, 1200.f, 37.f, 66.f, 3.3f, false);
	Spot(FVector(1350.f, -651.f, 695.f), FVector(965.f, -756.f, 375.f),
		FLinearColor(.53f, .64f, .76f), 3.8f, 1160.f, 32.f, 62.f, 4.f, true);
}
