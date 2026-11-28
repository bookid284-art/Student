#include "NightSchool/NightSchoolDirector.h"

#include "Components/AudioComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SpotLight.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NightSchool/NightSchoolCharacter.h"
#include "NightSchool/NightSchoolLayout.h"
#include "NightSchool/NightSchoolProp.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundWave.h"

namespace
{
	const FLinearColor DarkTimber(0.102f, 0.058f, 0.029f);
	const FLinearColor WornEdge(0.15f, 0.095f, 0.042f);
	constexpr float OldTubeIntensity = 6.4f;
	const FLinearColor GreenIron(0.054f, 0.083f, 0.064f);
	const FLinearColor Iron(0.038f, 0.045f, 0.046f);
	const FLinearColor Tarnish(0.16f, 0.112f, 0.049f);
	const FLinearColor DirtyLinen(0.26f, 0.255f, 0.188f);
	const FLinearColor DustCloth(0.085f, 0.112f, 0.099f);
}

void ANightSchoolDirector::BuildCampusDressing()
{
	auto Detail = [this](const FVector& Position, const FVector& Size, const FLinearColor& Color)
	{
		return AddBox(W(Position.X, Position.Y, Position.Z), Size, Color, true, false);
	};

	// Framed glass slot and exposed conduit. Keep every decorative piece off the door collision channel.
	Detail(FVector(0.f, -18.f, 266.f), FVector(138.f, 8.f, 32.f), DarkTimber);
	Detail(FVector(0.f, -23.f, 266.f), FVector(126.f, 3.f, 21.f), FLinearColor(0.048f, 0.097f, 0.096f));
	for (int32 Index = -3; Index <= 3; ++Index)
	{
		Detail(FVector(Index * 17.5f, -26.f, 266.f), FVector(2.2f, 3.f, 22.f), Iron);
	}
	Detail(FVector(0.f, -25.f, 266.f), FVector(127.f, 3.f, 2.3f), Iron);
	for (float Edge : {-1.f, 1.f})
	{
		Detail(FVector(Edge * 72.5f, -20.f, 146.f), FVector(6.f, 10.f, 211.f), DarkTimber);
	}
	AddCylinder(W(-186.f, -21.f, 287.f), 1.7f, 213.f, FRotator(90.f, 0.f, 0.f), Iron, false);
	AddCylinder(W(-292.f, -21.f, 237.f), 1.7f, 100.f, FRotator::ZeroRotator, Iron, false);
	Detail(FVector(-292.f, -22.f, 189.f), FVector(14.f, 8.f, 18.f), GreenIron);

	// The original interactable still owns collision and slides; the finish follows its root.
	if (ANightSchoolProp* Door = FindProp(ENightPropId::HallDoor))
	{
		auto DoorTrim = [this, Door](const FVector& Relative, const FVector& Size, const FLinearColor& Color)
		{
			if (UStaticMeshComponent* Panel = AddBox(Door->GetActorLocation() + Relative, Size, Color, true, false))
			{
				Panel->AttachToComponent(Door->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
			}
		};
		for (float Face : {-1.f, 1.f})
		{
			const float Y = 9.4f * Face;
			for (int32 Index = -3; Index <= 3; ++Index)
			{
				DoorTrim(FVector(Index * 17.f, Y, 1.f), FVector(15.5f, 2.8f, 188.f),
					FMath::Lerp(DarkTimber, WornEdge, 0.28f + 0.075f * Index));
			}
			for (float Z : {-88.f, 78.f})
			{
				DoorTrim(FVector(0.f, Y + Face * 1.3f, Z), FVector(124.f, 3.f, 8.f), DarkTimber);
			}
		}
	}

	auto DormDetail = [this](const FVector& Position, const FVector& Size, const FLinearColor& Color,
		bool bCollision = false)
	{
		return AddBox(NightSchool::Dorm(Position.X, Position.Y, Position.Z), Size, Color, true, bCollision);
	};

	// The bunk and lockers are anchored to the upper dorm's floor and walls.
	for (float X : {-949.f, -771.f})
	{
		for (float Y : {-1.f, 1.f})
		{
			AddCylinder(NightSchool::Dorm(X, 600.f + Y * 39.f, 132.f), 2.8f, 184.f, FRotator::ZeroRotator, GreenIron, false);
		}
	}
	for (float Z : {69.f, 160.f, 196.f})
	{
		for (float Y : {561.f, 639.f})
		{
			DormDetail(FVector(-860.f, Y, Z), FVector(178.f, 4.f, 4.f), GreenIron);
		}
	}
	DormDetail(FVector(-860.f, 600.f, 163.f), FVector(170.f, 76.f, 7.f), DarkTimber, true);
	DormDetail(FVector(-860.f, 600.f, 172.f), FVector(163.f, 73.f, 12.f), DirtyLinen);
	DormDetail(FVector(-917.f, 601.f, 182.f), FVector(38.f, 44.f, 8.f), DirtyLinen * 1.18f);
	DormDetail(FVector(-820.f, 600.f, 180.f), FVector(78.f, 68.f, 5.f), DustCloth);
	for (float X : {-806.f, -777.f})
	{
		AddCylinder(NightSchool::Dorm(X, 553.f, 115.f), 2.3f, 150.f, FRotator::ZeroRotator, GreenIron, false);
	}
	for (float Z : {68.f, 90.f, 112.f, 134.f, 156.f, 178.f})
	{
		DormDetail(FVector(-791.5f, 553.f, Z), FVector(29.f, 4.6f, 4.6f), GreenIron);
	}

	// The high window is blanked with dark glass; no view or passage through the exterior wall.
	DormDetail(FVector(-945.f, 784.f, 226.f), FVector(193.f, 8.f, 100.f), DarkTimber);
	for (int32 Row = 0; Row < 2; ++Row)
	{
		for (int32 Column = 0; Column < 4; ++Column)
		{
			const float Patina = 0.67f + 0.15f * ((Column + Row * 3) % 4);
			DormDetail(FVector(-1012.5f + Column * 45.f, 778.f, 204.f + Row * 43.f),
				FVector(41.f, 5.f, 39.f), FLinearColor(0.059f, 0.093f, 0.10f) * Patina);
		}
	}
	DormDetail(FVector(-945.f, 777.f, 175.f), FVector(201.f, 25.f, 7.f), WornEdge);

	// Ventilated lockers imply a shared dorm, with flaking fronts and individual number plates.
	for (int32 Index = 0; Index < 3; ++Index)
	{
		const float Y = 366.f + Index * 39.f;
		DormDetail(FVector(-1066.f, Y, 114.5f), FVector(43.f, 37.f, 149.f), GreenIron, true);
		DormDetail(FVector(-1043.f, Y, 115.5f), FVector(4.f, 34.f, 144.f), FLinearColor(0.076f, 0.094f, 0.064f));
		for (float Z : {157.f, 162.f, 167.f})
		{
			DormDetail(FVector(-1040.f, Y, Z), FVector(2.f, 21.f, 1.4f), Iron);
		}
		DormDetail(FVector(-1040.f, Y - 7.f, 144.f), FVector(2.4f, 8.f, 6.f), DirtyLinen);
		DormDetail(FVector(-1038.f, Y + 9.f, 121.f), FVector(4.f, 5.f, 14.f), Tarnish);
	}
}

void ANightSchoolDirector::BuildCampusLighting()
{
	auto Spot = [this](const FVector& From, const FVector& Target, const FLinearColor& Tint,
		float Intensity, float Radius, float Inner, float Outer, bool bShadow) -> USpotLightComponent*
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ASpotLight* Actor = GetWorld()->SpawnActor<ASpotLight>(ASpotLight::StaticClass(), FTransform(From), Params);
		if (!Actor)
		{
			return nullptr;
		}
		Actor->Tags.Add(TEXT("NightSchool"));
		Actor->SetMobility(EComponentMobility::Movable);
		// ASpotLight starts with a downward-rotated root. Aim the registered component in world space.
		Actor->SetActorRotation((Target - From).Rotation());
		USpotLightComponent* Light = Actor->SpotLightComponent;
		Light->SetUseInverseSquaredFalloff(false);
		Light->SetIntensity(Intensity);
		Light->SetAttenuationRadius(Radius);
		Light->SetInnerConeAngle(Inner);
		Light->SetOuterConeAngle(Outer);
		Light->SetLightColor(Tint);
		Light->SetCastShadows(bShadow);
		return Light;
	};

	// Pools of light pick out the counter and stair opening. Wall spill stays blue and weak.
	UnsteadyTubeLight = Spot(W(2.f, -273.f, 281.f), W(-44.f, -158.f, 65.f),
		FLinearColor(.53f, .69f, .84f), OldTubeIntensity, 335.f, 17.f, 42.f, true);
	Spot(W(-92.f, -456.f, 258.f), W(-174.f, -548.f, 84.f),
		FLinearColor(.55f, .69f, .82f), 4.4f, 280.f, 19.f, 34.f, true);
	Spot(W(-939.f, -385.f, 262.f), W(-1094.f, -489.f, 105.f),
		FLinearColor(.48f, .64f, .84f), 7.6f, 325.f, 22.f, 39.f, true);
	Spot(W(159.f, -79.f, 263.f), W(181.f, -16.f, 161.f),
		FLinearColor(.54f, .67f, .81f), 1.9f, 190.f, 17.f, 34.f, true);
	Spot(W(-362.f, -414.f, 268.f), W(-281.f, -578.f, 166.f),
		FLinearColor(.48f, .60f, .76f), 2.9f, 255.f, 18.f, 45.f, true);

	using namespace NightSchool;
	Spot(Dorm(-911.f, 312.f, 278.f), Dorm(-970.f, 204.f, 108.f),
		FLinearColor(.58f, .74f, .89f), 7.4f, 330.f, 15.f, 39.f, true);
	Spot(Dorm(-682.f, 419.f, 278.f), Dorm(-897.f, 603.f, 124.f),
		FLinearColor(.48f, .67f, .85f), 5.1f, 375.f, 19.f, 42.f, true);

	for (int32 Floor : {0, 1})
	{
		const FVector From = AtFloor(-1328.f, -321.f, 278.f, Floor);
		Spot(From, AtFloor(StairEastX, -281.f, 105.f, Floor), FLinearColor(.55f, .7f, .88f), 5.6f, 350.f, 19.f, 47.f, true);
		Spot(AtFloor(StairWestX, -53.f, 449.f, Floor), AtFloor(StairWestX, -115.f, 226.f, Floor),
			FLinearColor(.48f, .63f, .85f), 6.8f, 359.f, 23.f, 45.f, true);
	}
	Spot(AtFloor(-1900.f, -440.f, 277.f, 2), AtFloor(-1812.f, -90.f, 120.f, 2),
		FLinearColor(.49f, .67f, .84f), 6.8f, 465.f, 19.f, 48.f, true);
	for (float X : {AnnexRoomDoorX, ReadingRoomDoorX})
	{
		Spot(AtFloor(X, -31.f, 281.f, 2), AtFloor(X, -5.f, 127.f, 2),
			FLinearColor(.54f, .67f, .84f), 5.6f, 230.f, 18.f, 43.f, true);
	}
	Spot(Annex(1510.f, 2425.f, 278.f), Annex(1570.f, 2665.f, 153.f),
		FLinearColor(.54f, .69f, .87f), 8.f, 425.f, 20.f, 52.f, true);
}

void ANightSchoolDirector::BuildCampusAudio()
{
	CampusCueDelay = 6.4f;
	CampusSoundRandom.Initialize(FMath::Rand());
	if (CampusAir)
	{
		UAudioComponent* Air = NewObject<UAudioComponent>(this, TEXT("CorridorAir"));
		Air->bAutoActivate = false;
		Air->SetupAttachment(GetRootComponent());
		Air->SetSound(CampusAir);
		Air->SetVolumeMultiplier(0.54f);
		Air->RegisterComponent();
		Air->Play();
	}

	CampusEchoAttenuation = NewObject<USoundAttenuation>(this);
	FSoundAttenuationSettings& Echo = CampusEchoAttenuation->Attenuation;
	Echo.bAttenuate = true;
	Echo.bSpatialize = true;
	Echo.AttenuationShapeExtents = FVector(110.f, 0.f, 0.f);
	Echo.FalloffDistance = 1150.f;
	Echo.bEnableOcclusion = true;
	Echo.OcclusionVolumeAttenuation = 0.72f;
	Echo.OcclusionLowPassFilterFrequency = 1700.f;
	Echo.OcclusionInterpolationTime = 0.25f;
	if (!CampusBallast)
	{
		return;
	}
	for (const FVector& Origin : {W(0.f, -280.f, 294.f), W(-700.f, 420.f, 294.f), W(0.f, 500.f, 294.f)})
	{
		UAudioComponent* Ballast = NewObject<UAudioComponent>(this);
		Ballast->bAutoActivate = false;
		Ballast->SetupAttachment(GetRootComponent());
		Ballast->SetSound(CampusBallast);
		Ballast->bOverrideAttenuation = true;
		Ballast->AttenuationOverrides.bAttenuate = true;
		Ballast->AttenuationOverrides.bSpatialize = true;
		Ballast->AttenuationOverrides.AttenuationShapeExtents = FVector(75.f, 0.f, 0.f);
		Ballast->AttenuationOverrides.FalloffDistance = 455.f;
		Ballast->SetVolumeMultiplier(0.55f);
		Ballast->RegisterComponent();
		Ballast->SetWorldLocation(Origin);
		Ballast->Play(CampusSoundRandom.FRandRange(0.f, 3.5f));
	}
}

void ANightSchoolDirector::TickCampus(float Time, float DeltaSeconds)
{
	if (UnsteadyTubeLight)
	{
		const float Cycle = FMath::Fmod(Time + 2.1f, 12.8f);
		const float Dip = FMath::Clamp(1.f - FMath::Abs(Cycle - 9.2f) / 0.34f, 0.f, 1.f);
		UnsteadyTubeLight->SetIntensity(OldTubeIntensity * (0.94f + 0.06f * FMath::Sin(Time * 2.3f) - 0.38f * Dip));
	}
	if (IsEnding())
	{
		return;
	}
	ANightSchoolCharacter* Current = Player();
	if (!Current)
	{
		return;
	}
	CampusCueDelay -= DeltaSeconds;
	if (CampusCueDelay > 0.f || bPreviewing || bNoteOpen)
	{
		return;
	}

	const FVector Listener = Current->GetActorLocation();
	const FVector Sources[] = {W(126.f, 125.f, 110.f), W(-1009.f, 644.f, 123.f), W(800.f, 670.f, 72.f),
		W(1680.f, 514.f, 98.f), W(570.f, 1080.f, 80.f), W(1700.f, 2060.f, 122.f), W(-480.f, -1130.f, 77.f)};
	const FVector* Closest = nullptr;
	float BestDistance = FMath::Square(1100.f);
	for (const FVector& Location : Sources)
	{
		const float Distance = FVector::DistSquared(Listener, Location);
		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			Closest = &Location;
		}
	}
	CampusCueDelay = CampusSoundRandom.FRandRange(14.f, 26.f);
	USoundWave* Cue = bNextCampusKnock ? CampusKnock.Get() : CampusScrape.Get();
	bNextCampusKnock = !bNextCampusKnock;
	if (Closest && Cue)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Cue, *Closest, 0.86f,
			CampusSoundRandom.FRandRange(0.91f, 1.04f), 0.f, CampusEchoAttenuation);
	}
}
