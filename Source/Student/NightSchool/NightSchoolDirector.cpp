#include "NightSchool/NightSchoolDirector.h"

#include "Animation/AnimationAsset.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/AudioComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/EngineTypes.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PointLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "NightSchool/NightSchoolApparition.h"
#include "NightSchool/NightSchoolArt.h"
#include "NightSchool/NightSchoolCharacter.h"
#include "NightSchool/NightSchoolLayout.h"
#include "NightSchool/NightSchoolProp.h"
#include "NightSchool/NightSchoolStory.h"
#include "Sound/SoundWaveProcedural.h"
#include "Student.h"

namespace
{
	const FLinearColor Plaster(0.16f, 0.145f, 0.125f);
	const FLinearColor Wood(0.20f, 0.11f, 0.055f);
	const FLinearColor DarkWood(0.10f, 0.055f, 0.035f);
	const FLinearColor Concrete(0.055f, 0.055f, 0.058f);
	const FLinearColor Night(0.012f, 0.015f, 0.028f);
	const FLinearColor Paper(0.42f, 0.36f, 0.26f);
	const FLinearColor Red(0.38f, 0.025f, 0.02f);
	const FLinearColor Rust(0.28f, 0.09f, 0.04f);
	constexpr float WallZ = 180.f;
	constexpr float WallH = 280.f;
	constexpr float Thick = 28.f;
	constexpr float LintelZ = 286.f;
	constexpr float LintelH = 68.f;
}

ANightSchoolDirector::ANightSchoolDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	Tags.Add(TEXT("NightSchool"));
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

ANightSchoolDirector* ANightSchoolDirector::Find(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	for (TActorIterator<ANightSchoolDirector> It(World); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

FVector ANightSchoolDirector::W(float X, float Y, float Z) const
{
	return NightSchool::Origin() + FVector(X, Y, Z);
}

UStaticMeshComponent* ANightSchoolDirector::AddBox(const FVector& Center, const FVector& Size, const FLinearColor& Color, bool bShadow, bool bCollision)
{
	UStaticMesh* Cube = NightSchoolArt::Cube();
	if (!Cube || Size.GetMin() <= 1.f)
	{
		return nullptr;
	}
	UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(this);
	Mesh->SetStaticMesh(Cube);
	Mesh->SetupAttachment(GetRootComponent());
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	if (bCollision)
	{
		Mesh->SetCollisionObjectType(ECC_WorldStatic);
		Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	}
	Mesh->SetCastShadow(bShadow);
	if (UMaterialInstanceDynamic* Mid = NightSchoolArt::MakeColor(this, Color))
	{
		Mesh->SetMaterial(0, Mid);
	}
	Mesh->RegisterComponent();
	Mesh->SetWorldLocation(Center);
	Mesh->SetWorldScale3D(Size / 100.f);
	return Mesh;
}

void ANightSchoolDirector::AddSphere(const FVector& Center, float Radius, const FLinearColor& Color, bool bCollision)
{
	UStaticMesh* Sphere = NightSchoolArt::Sphere();
	if (!Sphere)
	{
		return;
	}
	UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(this);
	Mesh->SetStaticMesh(Sphere);
	Mesh->SetupAttachment(GetRootComponent());
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	Mesh->SetCastShadow(false);
	if (UMaterialInstanceDynamic* Mid = NightSchoolArt::MakeColor(this, Color))
	{
		Mesh->SetMaterial(0, Mid);
	}
	Mesh->RegisterComponent();
	const float Scale = Radius / 50.f;
	Mesh->SetWorldLocation(Center);
	Mesh->SetWorldScale3D(FVector(Scale));
}

void ANightSchoolDirector::AddCylinder(const FVector& Center, float Radius, float Height, const FRotator& Rotation, const FLinearColor& Color, bool bCollision)
{
	UStaticMesh* Cylinder = NightSchoolArt::Cylinder();
	if (!Cylinder)
	{
		return;
	}
	UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(this);
	Mesh->SetStaticMesh(Cylinder);
	Mesh->SetupAttachment(GetRootComponent());
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	Mesh->SetCastShadow(bCollision);
	if (UMaterialInstanceDynamic* Mid = NightSchoolArt::MakeColor(this, Color))
	{
		Mesh->SetMaterial(0, Mid);
	}
	Mesh->RegisterComponent();
	Mesh->SetWorldLocation(Center);
	Mesh->SetWorldRotation(Rotation);
	Mesh->SetWorldScale3D(FVector(Radius / 50.f, Radius / 50.f, Height / 100.f));
}

void ANightSchoolDirector::AddLabel(const FVector& Location, float Yaw, const FString& Text, float WorldSize, const FColor& Color)
{
	UFont* Font = NightSchoolArt::GetRuntimeFont();
	if (!Font || Text.IsEmpty())
	{
		return;
	}
	UTextRenderComponent* Label = NewObject<UTextRenderComponent>(this);
	Label->SetupAttachment(GetRootComponent());
	Label->SetFont(Font);
	Label->SetText(FText::FromString(Text));
	Label->SetWorldSize(WorldSize);
	Label->SetTextRenderColor(Color);
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetVerticalAlignment(EVRTA_TextCenter);
	Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Label->SetCastShadow(false);
	Label->RegisterComponent();
	Label->SetWorldLocation(Location);
	Label->SetWorldRotation(FRotator(0.f, Yaw, 0.f));
}

void ANightSchoolDirector::AddLantern(float X, float Y, float Z, float Intensity)
{
	const FVector At = W(X, Y, Z);
	AddCylinder(W(X, Y, Z + 55.f), 2.5f, 90.f, FRotator::ZeroRotator, Red, false);
	AddSphere(At, 18.f, FLinearColor(0.45f, 0.03f, 0.02f), false);
	AddSphere(At, 8.f, FLinearColor(1.f, 0.42f, 0.16f), false);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APointLight* Light = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), FTransform(At), Params);
	if (!Light)
	{
		return;
	}
	Light->Tags.Add(TEXT("NightSchool"));
	Light->SetMobility(EComponentMobility::Movable);
	if (UPointLightComponent* Component = Light->PointLightComponent)
	{
		Component->SetMobility(EComponentMobility::Movable);
		Component->bUseInverseSquaredFalloff = false;
		Component->SetIntensity(Intensity);
		Component->SetAttenuationRadius(720.f);
		Component->SetLightColor(FLinearColor(1.f, 0.55f, 0.28f));
		Component->SetCastShadows(false);
	}
	FlickerLights.Add(Light);
	FlickerBase.Add(Intensity);
	FlickerPhase.Add(FMath::FRandRange(1.4f, 3.2f));
	WarmLights.Add(At);
}

void ANightSchoolDirector::AddSprite(const FVector& Location, float Yaw, UTexture2D* Texture, float WidthCm, float HeightCm)
{
	if (!Texture || !GetWorld() || WidthCm <= 1.f)
	{
		return;
	}
	OwnedTextures.Add(Texture);
	UNightSchoolSpriteWidget* Widget = CreateWidget<UNightSchoolSpriteWidget>(GetWorld(), UNightSchoolSpriteWidget::StaticClass());
	if (!Widget)
	{
		return;
	}
	Widget->SetTexture(Texture);
	UWidgetComponent* Comp = NewObject<UWidgetComponent>(this);
	Comp->SetupAttachment(GetRootComponent());
	Comp->SetWidgetSpace(EWidgetSpace::World);
	Comp->SetWidget(Widget);
	Comp->SetDrawSize(FVector2D(512.f, 512.f * HeightCm / WidthCm));
	Comp->SetPivot(FVector2D(0.5f, 0.5f));
	Comp->SetTwoSided(true);
	Comp->SetBlendMode(EWidgetBlendMode::Opaque);
	Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Comp->SetCastShadow(false);
	Comp->SetRelativeScale3D(FVector(WidthCm / 512.f));
	Comp->RegisterComponent();
	Comp->SetWorldLocationAndRotation(Location, FRotator(0.f, Yaw, 0.f));
}

ANightSchoolProp* ANightSchoolDirector::SpawnProp(ENightPropId Id, const FVector& Location, const FVector& Extent, const FLinearColor& Color, const FText& Prompt, const FVector& Slide, float LabelYaw, const FString& Label)
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ANightSchoolProp* Prop = GetWorld()->SpawnActor<ANightSchoolProp>(ANightSchoolProp::StaticClass(), FTransform(FRotator::ZeroRotator, Location), Params);
	if (!Prop)
	{
		return nullptr;
	}
	Prop->Configure(Id, Extent, Color, Prompt, Slide);
	Props.Add(Prop);
	if (!Label.IsEmpty())
	{
		AddLabel(Location + FVector(0.f, 0.f, Extent.Z + 16.f), LabelYaw, Label, 13.f, FColor(235, 214, 176));
	}
	return Prop;
}

ANightSchoolProp* ANightSchoolDirector::FindProp(ENightPropId Id) const
{
	for (ANightSchoolProp* Prop : Props)
	{
		if (Prop && Prop->GetPropId() == Id)
		{
			return Prop;
		}
	}
	return nullptr;
}

void ANightSchoolDirector::SpawnGhost(const FBox& Room, const TArray<FVector>& Points, const FString& Glyph)
{
	if (!GetWorld() || Points.Num() == 0)
	{
		return;
	}
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ANightSchoolApparition* Ghost = GetWorld()->SpawnActor<ANightSchoolApparition>(ANightSchoolApparition::StaticClass(), FTransform(FRotator::ZeroRotator, Points[0]), Params);
	if (!Ghost)
	{
		return;
	}
	Ghost->Configure(Room, Points, Glyph);
	Apparitions.Add(Ghost);
}

void ANightSchoolDirector::BuildWorld()
{
	if (bWorldBuilt || !GetWorld())
	{
		return;
	}
	if (!NightSchoolArt::Cube())
	{
		UE_LOG(LogStudent, Error, TEXT("回课：引擎基础方块没有加载，场景无法搭建。"));
		return;
	}
	BuildShell();
	BuildStructure();
	BuildDressing();
	BuildLights();
	BuildAudio();
	if (ANightSchoolProp* Key = FindProp(ENightPropId::OldKey))
	{
		Key->SetAvailable(false);
	}
	ShowStatus(TEXT("除夕。旧楼明天就拆。导师说夜校不许写。"), 14.f);
	bWorldBuilt = true;
	UE_LOG(LogStudent, Log, TEXT("回课：望江夜校已搭好。"));
}

void ANightSchoolDirector::BuildShell()
{
	AddBox(W(0.f, 900.f, 20.f), FVector(4800.f, 5200.f, 40.f), Concrete, true, true);
	AddBox(W(0.f, 900.f, 860.f), FVector(4800.f, 5200.f, 24.f), Night, false, true);
	AddBox(W(0.f, 3500.f, 450.f), FVector(4800.f, 40.f, 820.f), Night, false, true);
	AddBox(W(0.f, -1700.f, 450.f), FVector(4800.f, 40.f, 820.f), Night, false, true);
	AddBox(W(2400.f, 900.f, 450.f), FVector(40.f, 5200.f, 820.f), Night, false, true);
	AddBox(W(-2400.f, 900.f, 450.f), FVector(40.f, 5200.f, 820.f), Night, false, true);
	AddSphere(W(-900.f, -900.f, 640.f), 70.f, FLinearColor(0.72f, 0.76f, 0.82f), false);

	FRandomStream Stars(19670312);
	for (int32 Index = 0; Index < 36; ++Index)
	{
		const float X = Stars.FRandRange(-1600.f, 1600.f);
		const float Y = Stars.FRandRange(-1200.f, 3000.f);
		if (Y > 0.f && Y < 2000.f && FMath::Abs(X) < 1500.f)
		{
			continue;
		}
		AddSphere(W(X, Y, 790.f), Stars.FRandRange(3.f, 7.f), FLinearColor(0.8f, 0.84f, 0.9f), false);
	}
}

void ANightSchoolDirector::BuildStructure()
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
	auto LintelX = [this](float X, float Y)
	{
		AddBox(W(X, Y, LintelZ), FVector(Thick, 150.f, LintelH), Plaster, true, true);
	};
	auto LintelY = [this](float Y, float X)
	{
		AddBox(W(X, Y, LintelZ), FVector(150.f, Thick, LintelH), Plaster, true, true);
	};

	WallY(0.f, -460.f, -75.f);
	WallY(0.f, 75.f, 460.f);
	LintelY(0.f, 0.f);
	WallY(720.f, -460.f, -75.f);
	WallY(720.f, 75.f, 460.f);
	LintelY(720.f, 0.f);
	WallX(-460.f, 0.f, 250.f);
	WallX(-460.f, 400.f, 720.f);
	LintelX(-460.f, 325.f);
	WallX(460.f, 0.f, 250.f);
	WallX(460.f, 400.f, 720.f);
	LintelX(460.f, 325.f);

	WallX(-1540.f, 20.f, 1060.f);
	WallY(20.f, -1540.f, -460.f);
	WallY(1060.f, -1540.f, -460.f);

	WallX(-460.f, 720.f, 1420.f);
	WallX(460.f, 720.f, 1420.f);
	WallY(1420.f, -460.f, -75.f);
	WallY(1420.f, 75.f, 460.f);
	LintelY(1420.f, 0.f);

	WallX(-460.f, 1420.f, 2040.f);
	WallX(460.f, 1420.f, 2040.f);
	WallY(2040.f, -460.f, -75.f);
	WallY(2040.f, 75.f, 460.f);
	LintelY(2040.f, 0.f);

	WallX(1660.f, 20.f, 1320.f);
	WallY(20.f, 460.f, 1660.f);
	WallY(1320.f, 460.f, 1660.f);

	const FLinearColor Ceiling(0.07f, 0.065f, 0.06f);
	AddBox(W(0.f, 360.f, 314.f), FVector(880.f, 680.f, 16.f), Ceiling, true, true);
	AddBox(W(-1000.f, 540.f, 314.f), FVector(1000.f, 980.f, 16.f), Ceiling, true, true);
	AddBox(W(0.f, 1070.f, 314.f), FVector(880.f, 660.f, 16.f), Ceiling, true, true);
	AddBox(W(1060.f, 670.f, 314.f), FVector(1120.f, 1240.f, 16.f), Ceiling, true, true);
	AddBox(W(0.f, 1730.f, 236.f), FVector(880.f, 580.f, 16.f), FLinearColor(0.04f, 0.035f, 0.03f), true, true);

	AddBox(W(-620.f, 2420.f, 90.f), FVector(24.f, 700.f, 100.f), Plaster, true, true);
	AddBox(W(620.f, 2420.f, 90.f), FVector(24.f, 700.f, 100.f), Plaster, true, true);
	AddBox(W(0.f, 2770.f, 90.f), FVector(1240.f, 24.f, 100.f), Plaster, true, true);

	const FLinearColor FloorWood(0.18f, 0.09f, 0.045f);
	AddBox(W(0.f, 360.f, 43.f), FVector(840.f, 640.f, 4.f), FloorWood, false, false);
	AddBox(W(-1000.f, 540.f, 43.f), FVector(960.f, 920.f, 4.f), FloorWood, false, false);
	AddBox(W(1060.f, 670.f, 43.f), FVector(1080.f, 1160.f, 4.f), DarkWood, false, false);
	AddBox(W(0.f, 2480.f, 44.f), FVector(520.f, 220.f, 3.f), FLinearColor(0.02f, 0.03f, 0.045f), false, false);
}

void ANightSchoolDirector::BuildDressing()
{
	auto Desk = [this](float X, float Y)
	{
		AddBox(W(X, Y, 112.f), FVector(130.f, 70.f, 8.f), Wood, true, false);
		AddBox(W(X - 50.f, Y - 24.f, 76.f), FVector(8.f, 8.f, 72.f), DarkWood, false, false);
		AddBox(W(X + 50.f, Y - 24.f, 76.f), FVector(8.f, 8.f, 72.f), DarkWood, false, false);
		AddBox(W(X - 50.f, Y + 24.f, 76.f), FVector(8.f, 8.f, 72.f), DarkWood, false, false);
		AddBox(W(X + 50.f, Y + 24.f, 76.f), FVector(8.f, 8.f, 72.f), DarkWood, false, false);
	};
	auto Bench = [this](float X, float Y, float Length)
	{
		AddBox(W(X, Y, 78.f), FVector(Length, 36.f, 8.f), DarkWood, false, false);
		AddBox(W(X, Y, 50.f), FVector(Length - 20.f, 8.f, 48.f), DarkWood, false, false);
	};

	AddCylinder(W(-130.f, -40.f, 210.f), 8.f, 220.f, FRotator::ZeroRotator, Red, false);
	AddCylinder(W(130.f, -40.f, 210.f), 8.f, 220.f, FRotator::ZeroRotator, Red, false);
	AddBox(W(-180.f, -520.f, 150.f), FVector(70.f, 6.f, 100.f), FLinearColor(0.35f, 0.08f, 0.06f), false, false);

	for (int32 Arch = 0; Arch <= 8; ++Arch)
	{
		const float Angle = PI * static_cast<float>(Arch) / 8.f;
		AddBox(W(FMath::Cos(Angle) * 180.f, -980.f, 70.f + FMath::Sin(Angle) * 180.f), FVector(36.f, 28.f, 36.f), FLinearColor(0.22f, 0.08f, 0.06f), true, false);
	}

	Desk(-1200.f, 420.f);
	Desk(-980.f, 700.f);
	Desk(-760.f, 900.f);
	AddBox(W(-980.f, 200.f, 100.f), FVector(80.f, 50.f, 90.f), DarkWood, true, false);

	AddBox(W(0.f, 1080.f, 100.f), FVector(160.f, 70.f, 80.f), DarkWood, true, false);
	AddBox(W(0.f, 1320.f, 160.f), FVector(220.f, 16.f, 90.f), Wood, false, false);

	AddBox(W(1600.f, 700.f, 175.f), FVector(12.f, 360.f, 170.f), FLinearColor(0.015f, 0.04f, 0.03f), false, false);
	for (int32 Row = 0; Row < 4; ++Row)
	{
		Bench(900.f, 280.f + Row * 160.f, 280.f);
		Bench(1250.f, 280.f + Row * 160.f, 280.f);
	}
	AddBox(W(620.f, 180.f, 96.f), FVector(80.f, 50.f, 70.f), DarkWood, true, false);

	AddCylinder(W(-80.f, 1720.f, 150.f), 78.f, 200.f, FRotator::ZeroRotator, FLinearColor(0.08f, 0.08f, 0.085f), false);
	AddCylinder(W(40.f, 1860.f, 120.f), 10.f, 180.f, FRotator(0.f, 90.f, 90.f), Rust, false);
	AddSphere(W(-80.f, 1720.f, 70.f), 16.f, FLinearColor(0.7f, 0.22f, 0.05f), false);

	AddBox(W(-200.f, 2300.f, 88.f), FVector(90.f, 60.f, 70.f), Concrete, true, false);
	AddCylinder(W(200.f, 2300.f, 70.f), 36.f, 40.f, FRotator::ZeroRotator, Rust, false);
	AddSphere(W(200.f, 2300.f, 96.f), 12.f, FLinearColor(1.f, 0.35f, 0.08f), false);
	AddBox(W(-200.f, 2100.f, 70.f), FVector(70.f, 70.f, 16.f), Wood, false, false);
	AddBox(W(-200.f, 2100.f, 110.f), FVector(16.f, 70.f, 70.f), Wood, false, false);
	AddBox(W(210.f, 2140.f, 90.f), FVector(36.f, 28.f, 70.f), FLinearColor(0.12f, 0.13f, 0.12f), true, false);

	for (int32 Pole = 0; Pole < 4; ++Pole)
	{
		const float X = -180.f + Pole * 120.f;
		AddCylinder(W(X, 3120.f, 180.f), 6.f, 280.f, FRotator::ZeroRotator, FLinearColor(0.1f, 0.1f, 0.1f), false);
		AddSphere(W(X, 3120.f, 300.f), 16.f, FLinearColor(1.f, 0.62f, 0.22f), false);
	}
}

void ANightSchoolDirector::BuildLights()
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADirectionalLight* Moon = GetWorld()->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FTransform(FRotator(-48.f, -30.f, 0.f), W(0.f, 0.f, 600.f)), Params);
	if (Moon)
	{
		Moon->Tags.Add(TEXT("NightSchool"));
		Moon->SetMobility(EComponentMobility::Movable);
		if (ULightComponent* Light = Moon->GetLightComponent())
		{
			Light->SetMobility(EComponentMobility::Movable);
			Light->SetIntensity(2.2f);
			Light->SetLightColor(FLinearColor(0.55f, 0.64f, 0.85f));
			Light->SetCastShadows(true);
		}
	}

	ASkyLight* Sky = GetWorld()->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), FTransform(W(0.f, 0.f, 500.f)), Params);
	if (Sky)
	{
		Sky->Tags.Add(TEXT("NightSchool"));
		if (USkyLightComponent* Component = Sky->GetLightComponent())
		{
			Component->SetIntensity(0.35f);
			Component->SetLightColor(FLinearColor(0.25f, 0.3f, 0.45f));
			Component->SetMobility(EComponentMobility::Movable);
			Component->RecaptureSky();
		}
	}

	AExponentialHeightFog* Fog = GetWorld()->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), FTransform(W(0.f, 0.f, 80.f)), Params);
	if (Fog)
	{
		Fog->Tags.Add(TEXT("NightSchool"));
		if (UExponentialHeightFogComponent* Component = Fog->GetComponent())
		{
			Component->SetFogDensity(0.004f);
			Component->SetFogHeightFalloff(0.2f);
			Component->SetFogInscatteringColor(FLinearColor(0.03f, 0.035f, 0.05f));
			Component->StartDistance = 300.f;
		}
	}

	APostProcessVolume* Post = GetWorld()->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FTransform(W(0.f, 0.f, 200.f)), Params);
	if (Post)
	{
		Post->Tags.Add(TEXT("NightSchool"));
		Post->bUnbound = true;
		Post->BlendWeight = 1.f;
		Post->Priority = 2.f;
		Post->Settings.bOverride_AutoExposureMethod = true;
		Post->Settings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
		Post->Settings.bOverride_AutoExposureBias = true;
		Post->Settings.AutoExposureBias = 1.05f;
		Post->Settings.bOverride_ColorSaturation = true;
		Post->Settings.ColorSaturation = FVector4(0.48f, 0.44f, 0.4f, 1.f);
		Post->Settings.bOverride_VignetteIntensity = true;
		Post->Settings.VignetteIntensity = 0.72f;
		Post->Settings.bOverride_BloomIntensity = true;
		Post->Settings.BloomIntensity = 0.45f;
		Post->Settings.bOverride_FilmGrainIntensity = true;
		Post->Settings.FilmGrainIntensity = 0.22f;
	}

	AddLantern(0.f, -700.f, 230.f, 16.f);
	AddLantern(0.f, 340.f, 250.f, 18.f);
	AddLantern(-1000.f, 540.f, 250.f, 16.f);
	AddLantern(0.f, 1040.f, 250.f, 15.f);
	AddLantern(700.f, 240.f, 250.f, 14.f);
	AddLantern(0.f, 1560.f, 200.f, 11.f);
	AddLantern(0.f, 2360.f, 180.f, 12.f);

	for (int32 Pole = 0; Pole < 4; ++Pole)
	{
		const FVector At = W(-180.f + Pole * 120.f, 3120.f, 300.f);
		APointLight* WorkLight = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), FTransform(At), Params);
		if (!WorkLight)
		{
			continue;
		}
		WorkLight->Tags.Add(TEXT("NightSchool"));
		WorkLight->SetMobility(EComponentMobility::Movable);
		if (UPointLightComponent* Component = WorkLight->PointLightComponent)
		{
			Component->SetMobility(EComponentMobility::Movable);
			Component->bUseInverseSquaredFalloff = false;
			Component->SetIntensity(30.f);
			Component->SetAttenuationRadius(900.f);
			Component->SetLightColor(FLinearColor(1.f, 0.62f, 0.25f));
			Component->SetCastShadows(false);
		}
	}

	SpawnProp(ENightPropId::FrontDoor, W(0.f, 8.f, 145.f), FVector(68.f, 8.f, 100.f), DarkWood, FText::FromString(TEXT("推开正门")), FVector(190.f, 0.f, 0.f), -90.f, TEXT("正门"));
	SpawnProp(ENightPropId::ExitStreet, W(0.f, -860.f, 120.f), FVector(46.f, 36.f, 70.f), FLinearColor(0.25f, 0.08f, 0.05f), FText::FromString(TEXT("离开工地")), FVector::ZeroVector, -90.f, TEXT("离开"));
	SpawnProp(ENightPropId::Notice, W(-250.f, -460.f, 130.f), FVector(28.f, 6.f, 40.f), Paper, FText::FromString(TEXT("阅读拆迁告示")), FVector::ZeroVector, -90.f, TEXT("告示"));
	SpawnProp(ENightPropId::SignInBook, W(0.f, 250.f, 130.f), FVector(28.f, 22.f, 8.f), Paper, FText::FromString(TEXT("翻开签到簿")), FVector::ZeroVector, -90.f, TEXT("签到簿"));

	SpawnProp(ENightPropId::EssayLin, W(-1200.f, 420.f, 132.f), FVector(36.f, 28.f, 14.f), Paper, FText::FromString(TEXT("阅读林秋禾的作文")), FVector::ZeroVector, 0.f, TEXT("林秋禾"));
	SpawnProp(ENightPropId::EssayZhao, W(-980.f, 700.f, 132.f), FVector(36.f, 28.f, 14.f), Paper, FText::FromString(TEXT("阅读赵铁生的作文")), FVector::ZeroVector, 0.f, TEXT("赵铁生"));
	SpawnProp(ENightPropId::EssayZhou, W(-760.f, 900.f, 132.f), FVector(36.f, 28.f, 14.f), Paper, FText::FromString(TEXT("阅读周棉的作文")), FVector::ZeroVector, 0.f, TEXT("周棉"));
	SpawnProp(ENightPropId::OldKey, W(-980.f, 200.f, 155.f), FVector(10.f, 6.f, 4.f), FLinearColor(0.62f, 0.46f, 0.16f), FText::FromString(TEXT("拿走旧钥匙")), FVector::ZeroVector, 0.f, TEXT("钥匙"));

	SpawnProp(ENightPropId::OfficeDoor, W(0.f, 720.f, 145.f), FVector(68.f, 8.f, 100.f), DarkWood, FText::FromString(TEXT("打开办公室")), FVector(190.f, 0.f, 0.f), -90.f, TEXT("办公室"));
	SpawnProp(ENightPropId::Confession, W(30.f, 1080.f, 150.f), FVector(26.f, 18.f, 5.f), Paper, FText::FromString(TEXT("阅读批注")), FVector::ZeroVector, -90.f, TEXT("批注"));
	SpawnProp(ENightPropId::Candle, W(-190.f, 1240.f, 120.f), FVector(8.f, 8.f, 22.f), Red, FText::FromString(TEXT("拿走红烛")), FVector::ZeroVector, -90.f, TEXT("红烛"));
	SpawnProp(ENightPropId::RedScarf, W(190.f, 1200.f, 120.f), FVector(16.f, 10.f, 8.f), Red, FText::FromString(TEXT("拿走红领巾")), FVector::ZeroVector, -90.f, TEXT("红领巾"));
	SpawnProp(ENightPropId::Battery, W(-230.f, 980.f, 100.f), FVector(10.f, 8.f, 14.f), FLinearColor(0.15f, 0.32f, 0.18f), FText::FromString(TEXT("拿走旧电池")), FVector::ZeroVector, -90.f, TEXT("电池"));

	SpawnProp(ENightPropId::AuditoriumDoor, W(460.f, 325.f, 145.f), FVector(8.f, 68.f, 100.f), DarkWood, FText::FromString(TEXT("进入礼堂")), FVector(0.f, 190.f, 0.f), 180.f, TEXT("礼堂"));
	SpawnProp(ENightPropId::NameTrue, W(1520.f, 560.f, 170.f), FVector(12.f, 40.f, 48.f), Paper, FText::FromString(TEXT("写下陈望舒")), FVector::ZeroVector, 180.f, TEXT("陈望舒"));
	SpawnProp(ENightPropId::NameWrongA, W(1520.f, 700.f, 170.f), FVector(12.f, 40.f, 48.f), FLinearColor(0.28f, 0.24f, 0.2f), FText::FromString(TEXT("写下陈忘书")), FVector::ZeroVector, 180.f, TEXT("陈忘书"));
	SpawnProp(ENightPropId::NameWrongB, W(1520.f, 840.f, 170.f), FVector(12.f, 40.f, 48.f), FLinearColor(0.28f, 0.24f, 0.2f), FText::FromString(TEXT("写下陈望书")), FVector::ZeroVector, 180.f, TEXT("陈望书"));
	SpawnProp(ENightPropId::ScarfHook, W(620.f, 180.f, 140.f), FVector(18.f, 14.f, 10.f), Red, FText::FromString(TEXT("把红领巾放回第一排")), FVector::ZeroVector, 180.f, TEXT("第一排"));

	SpawnProp(ENightPropId::BoilerDoor, W(0.f, 1420.f, 145.f), FVector(68.f, 8.f, 100.f), FLinearColor(0.12f, 0.1f, 0.1f), FText::FromString(TEXT("进入锅炉房")), FVector(190.f, 0.f, 0.f), -90.f, TEXT("锅炉房"));
	SpawnProp(ENightPropId::Register, W(40.f, 1760.f, 120.f), FVector(22.f, 16.f, 8.f), FLinearColor(0.08f, 0.1f, 0.16f), FText::FromString(TEXT("阅读学员名册")), FVector::ZeroVector, -90.f, TEXT("名册"));
	SpawnProp(ENightPropId::LastPaper, W(180.f, 1640.f, 110.f), FVector(20.f, 14.f, 4.f), Paper, FText::FromString(TEXT("捡起没写完的字")), FVector::ZeroVector, -90.f, TEXT("未完稿"));

	SpawnProp(ENightPropId::GardenDoor, W(0.f, 2040.f, 145.f), FVector(68.f, 8.f, 100.f), DarkWood, FText::FromString(TEXT("去后园")), FVector(190.f, 0.f, 0.f), -90.f, TEXT("后园"));
	SpawnProp(ENightPropId::Thesis, W(-200.f, 2300.f, 140.f), FVector(24.f, 18.f, 6.f), Paper, FText::FromString(TEXT("把夜校写成论文发表")), FVector::ZeroVector, -90.f, TEXT("发表"));
	SpawnProp(ENightPropId::Brazier, W(200.f, 2300.f, 120.f), FVector(30.f, 30.f, 16.f), Rust, FText::FromString(TEXT("烧掉名册")), FVector::ZeroVector, -90.f, TEXT("烧掉"));
	SpawnProp(ENightPropId::Chair, W(-200.f, 2100.f, 150.f), FVector(28.f, 28.f, 20.f), Wood, FText::FromString(TEXT("留下来把课上完")), FVector::ZeroVector, -90.f, TEXT("留下"));
	SpawnProp(ENightPropId::Mailbox, W(210.f, 2140.f, 140.f), FVector(16.f, 12.f, 24.f), FLinearColor(0.18f, 0.2f, 0.16f), FText::FromString(TEXT("把字寄还给活人")), FVector::ZeroVector, -90.f, TEXT("寄回"));

	AddSprite(W(0.f, 1388.f, 190.f), -90.f, NightSchoolArt::MakePortrait(this), 52.f, 68.f);
	AddSprite(W(40.f, 1760.f, 150.f), -90.f, NightSchoolArt::MakeRegisterCover(this), 28.f, 36.f);
	AddLabel(W(-70.f, -20.f, 230.f), -90.f, TEXT("识字是为了看见"), 16.f, FColor(150, 28, 24));
	AddLabel(W(70.f, -20.f, 200.f), -90.f, TEXT("下联被撕了"), 12.f, FColor(120, 90, 70));
}

void ANightSchoolDirector::BuildAudio()
{
	DroneWave = NewObject<USoundWaveProcedural>(this);
	DroneWave->SetSampleRate(22050);
	DroneWave->NumChannels = 1;
	DroneWave->Duration = 7200.f;
	DroneWave->SoundGroup = SOUNDGROUP_Ambient;
	DroneWave->bLooping = false;

	DroneAudio = NewObject<UAudioComponent>(this);
	DroneAudio->bAutoActivate = false;
	DroneAudio->SetupAttachment(GetRootComponent());
	DroneAudio->RegisterComponent();
	DroneAudio->SetSound(DroneWave);
	DroneAudio->SetVolumeMultiplier(0.42f);
	DroneAudio->Play();
	QueueDrone(0.4f);
}

void ANightSchoolDirector::QueueDrone(float Seconds)
{
	if (!DroneWave)
	{
		return;
	}
	const int32 SampleRate = 22050;
	const int32 Frames = FMath::Max(1, FMath::RoundToInt(SampleRate * Seconds));
	TArray<uint8> Bytes;
	Bytes.SetNumUninitialized(Frames * sizeof(int16));
	int16* Samples = reinterpret_cast<int16*>(Bytes.GetData());
	const float HeartRate = FMath::Lerp(1.05f, 1.9f, FMath::Clamp(Fear / 100.f, 0.f, 1.f));
	float Cursor = DronePhase;
	for (int32 Frame = 0; Frame < Frames; ++Frame)
	{
		const float Time = Cursor;
		Cursor += 1.f / static_cast<float>(SampleRate);
		if (Cursor > 64.f)
		{
			Cursor -= 64.f;
		}
		float Sample = 0.045f * FMath::Sin(2.f * PI * 55.f * Time);
		Sample += 0.03f * FMath::Sin(2.f * PI * 82.4f * Time);
		Sample += 0.012f * FMath::Sin(2.f * PI * 110.f * Time + 0.4f);
		if (Fear > 28.f)
		{
			const float Beat = FMath::Fmod(HeartPhase + Time * HeartRate, 1.f);
			const float Lub = FMath::Exp(-FMath::Square((Beat - 0.08f) * 28.f));
			const float Dub = 0.65f * FMath::Exp(-FMath::Square((Beat - 0.22f) * 34.f));
			Sample += (Lub + Dub) * FMath::Lerp(0.02f, 0.11f, Fear / 100.f);
		}
		Samples[Frame] = static_cast<int16>(FMath::Clamp(Sample, -1.f, 1.f) * 32000.f);
	}
	DronePhase = Cursor;
	HeartPhase = FMath::Fmod(HeartPhase + Seconds * HeartRate, 1.f);
	DroneWave->QueueAudio(Bytes.GetData(), Bytes.Num());
}

void ANightSchoolDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bWorldBuilt)
	{
		return;
	}
	const float Time = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	for (int32 Index = 0; Index < FlickerLights.Num(); ++Index)
	{
		APointLight* Light = FlickerLights[Index];
		if (!Light || !FlickerBase.IsValidIndex(Index))
		{
			continue;
		}
		if (UPointLightComponent* Component = Light->PointLightComponent)
		{
			const float Wave = 0.84f + 0.16f * FMath::Sin(Time * FlickerPhase[Index] + Index);
			Component->SetIntensity(FlickerBase[Index] * Wave);
		}
	}

	StatusTime = FMath::Max(0.f, StatusTime - DeltaSeconds);
	if (StatusTime <= 0.f)
	{
		StatusLine.Reset();
	}
	FearLock = FMath::Max(0.f, FearLock - DeltaSeconds);
	if (ANightSchoolCharacter* Current = Player())
	{
		if (!IsEnding() && FearLock <= 0.f && IsInWarmLight(Current->GetActorLocation()))
		{
			Fear = FMath::Max(0.f, Fear - DeltaSeconds * 7.f);
		}
	}

	AudioTime += DeltaSeconds;
	if (AudioTime > 0.2f)
	{
		AudioTime = 0.f;
		QueueDrone(0.28f);
	}
}

bool ANightSchoolDirector::IsInWarmLight(const FVector& Location) const
{
	for (const FVector& Lamp : WarmLights)
	{
		if (FVector::DistSquared(Location, Lamp) <= FMath::Square(340.f))
		{
			return true;
		}
	}
	return false;
}

void ANightSchoolDirector::ShowNote(const TCHAR* InTitle, const TCHAR* InBody)
{
	if (IsEnding())
	{
		return;
	}
	NoteTitle = InTitle ? InTitle : TEXT("");
	NoteBody = InBody ? InBody : TEXT("");
	bNoteOpen = true;
	if (ANightSchoolCharacter* Current = Player())
	{
		Current->SetReading(true);
	}
}

void ANightSchoolDirector::ShowStatus(const FString& Line, float Seconds)
{
	StatusLine = Line;
	StatusTime = Seconds;
}

void ANightSchoolDirector::CloseNote()
{
	bNoteOpen = false;
	NoteTitle.Reset();
	NoteBody.Reset();
	if (ANightSchoolCharacter* Current = Player())
	{
		Current->SetReading(false);
	}
}

void ANightSchoolDirector::Remember(ENightName Name)
{
	if (!Names.Contains(Name))
	{
		Names.Add(Name);
		bEverRemembered = true;
	}
	bReadAnyPerson = true;
	RefreshKey();
}

bool ANightSchoolDirector::HasName(ENightName Name) const
{
	return Names.Contains(Name);
}

bool ANightSchoolDirector::HasAllStudents() const
{
	return HasName(ENightName::LinQiuhe) && HasName(ENightName::ZhaoTiesheng) && HasName(ENightName::ZhouMian);
}

void ANightSchoolDirector::ForgetOneName()
{
	const ENightName Order[] = {
		ENightName::ChenWangshu,
		ENightName::ZhouMian,
		ENightName::ZhaoTiesheng,
		ENightName::LinQiuhe
	};
	for (ENightName Name : Order)
	{
		if (Names.Remove(Name) > 0)
		{
			if (Name == ENightName::ChenWangshu)
			{
				bTrueName = false;
			}
			RefreshKey();
			ShowStatus(FString::Printf(TEXT("%s 这个名字从你舌头上掉下去了。"), NightSchoolStory::NameLabel(Name)), 3.2f);
			return;
		}
	}
}

void ANightSchoolDirector::GiveItem(ENightItem Item)
{
	if (!Items.Contains(Item))
	{
		Items.Add(Item);
	}
}

bool ANightSchoolDirector::HasItem(ENightItem Item) const
{
	return Items.Contains(Item);
}

void ANightSchoolDirector::RefreshKey()
{
	if (HasItem(ENightItem::OldKey))
	{
		return;
	}
	if (ANightSchoolProp* Key = FindProp(ENightPropId::OldKey))
	{
		Key->SetAvailable(HasAllStudents());
	}
}

void ANightSchoolDirector::TriggerEnding(ENightEnding Ending, bool bFromFear)
{
	if (IsEnding() || Ending == ENightEnding::None)
	{
		return;
	}
	CurrentEnding = Ending;
	bFearEnding = bFromFear;
	EndingTitle = NightSchoolStory::EndingTitle(Ending, bFromFear);
	EndingBody = NightSchoolStory::EndingBody(Ending, bFromFear);
	bNoteOpen = false;
	if (ANightSchoolCharacter* Current = Player())
	{
		Current->SetEnded(true);
	}
}

ANightSchoolCharacter* ANightSchoolDirector::Player() const
{
	return Cast<ANightSchoolCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
}

FString ANightSchoolDirector::GetObjective() const
{
	if (IsEnding())
	{
		return FString();
	}
	if (!bFrontDoorOpen)
	{
		return TEXT("推开旧教学楼的正门。");
	}
	if (!HasAllStudents() && !HasItem(ENightItem::OldKey))
	{
		return TEXT("西教室有三张课桌。把三篇作文读完。");
	}
	if (!HasItem(ENightItem::OldKey))
	{
		return TEXT("钥匙在西教室讲台上。");
	}
	if (!bOfficeOpen)
	{
		return TEXT("用旧钥匙打开北面的办公室。");
	}
	if (!bConfessionRead)
	{
		return TEXT("读办公室里那页压得很重的批注。");
	}
	if (!bTrueName)
	{
		return TEXT("到东边礼堂，在黑板上写下：陈望舒。");
	}
	if (!bBoilerOpen)
	{
		return TEXT("办公室北面是锅炉房。");
	}
	if (!bRegisterRead)
	{
		return TEXT("把没烧完的名册读完。");
	}
	if (!bGardenOpen)
	{
		return TEXT("名册读完了。推开北面的后园门。");
	}
	if (!HasAllStudents() || !bTrueName)
	{
		return TEXT("有名字掉了。用作文、名册或黑板写回来。");
	}
	return TEXT("后园有四种做法。也可以从正门走回工地离开。");
}

FString ANightSchoolDirector::GetNamesLine() const
{
	if (Names.Num() == 0)
	{
		return TEXT("还没有");
	}
	FString Line;
	for (int32 Index = 0; Index < Names.Num(); ++Index)
	{
		if (Index > 0)
		{
			Line += TEXT("、");
		}
		Line += NightSchoolStory::NameLabel(Names[Index]);
	}
	return Line;
}

FString ANightSchoolDirector::GetItemsLine() const
{
	if (Items.Num() == 0 && !bScarfReturned)
	{
		return TEXT("空的");
	}
	FString Line;
	auto Append = [&Line](const TCHAR* Word)
	{
		if (!Line.IsEmpty())
		{
			Line += TEXT("  ");
		}
		Line += Word;
	};
	if (HasItem(ENightItem::OldKey)) Append(TEXT("旧钥匙"));
	if (HasItem(ENightItem::Candle)) Append(TEXT("红烛"));
	if (HasItem(ENightItem::RedScarf)) Append(TEXT("红领巾"));
	if (bScarfReturned) Append(TEXT("领巾已放回"));
	if (HasItem(ENightItem::Register)) Append(TEXT("名册"));
	if (HasItem(ENightItem::LastPaper)) Append(TEXT("未完稿"));
	if (HasItem(ENightItem::Battery)) Append(TEXT("电池"));
	return Line.IsEmpty() ? TEXT("空的") : Line;
}

void ANightSchoolDirector::OnCaughtByApparition()
{
	if (IsEnding())
	{
		return;
	}
	Fear = FMath::Min(100.f, Fear + 26.f);
	FearLock = 2.6f;
	ForgetOneName();
	if (Fear >= 100.f)
	{
		TriggerEnding(ENightEnding::Nameless, true);
	}
}

void ANightSchoolDirector::HandleProp(ANightSchoolProp* Prop)
{
	if (!Prop || IsEnding())
	{
		return;
	}
	switch (Prop->GetPropId())
	{
	case ENightPropId::FrontDoor:
		bFrontDoorOpen = true;
		Prop->OpenDoor();
		ShowStatus(TEXT("门轴响了一声，像有人在里面把最后一课的椅子拉开。"), 4.f);
		break;
	case ENightPropId::ExitStreet:
		TriggerEnding(bReadAnyPerson ? ENightEnding::HalfPage : ENightEnding::Nameless, false);
		break;
	case ENightPropId::Notice:
		ShowNote(NightSchoolStory::NoticeTitle(), NightSchoolStory::NoticeBody());
		break;
	case ENightPropId::SignInBook:
		bReadAnyPerson = true;
		ShowNote(NightSchoolStory::SignInTitle(), NightSchoolStory::SignInBody());
		break;
	case ENightPropId::EssayLin:
		Remember(ENightName::LinQiuhe);
		ShowNote(NightSchoolStory::EssayLinTitle(), NightSchoolStory::EssayLinBody());
		break;
	case ENightPropId::EssayZhao:
		Remember(ENightName::ZhaoTiesheng);
		ShowNote(NightSchoolStory::EssayZhaoTitle(), NightSchoolStory::EssayZhaoBody());
		break;
	case ENightPropId::EssayZhou:
		Remember(ENightName::ZhouMian);
		ShowNote(NightSchoolStory::EssayZhouTitle(), NightSchoolStory::EssayZhouBody());
		break;
	case ENightPropId::OldKey:
		if (!HasAllStudents())
		{
			ShowNote(TEXT("钥匙"), NightSchoolStory::LockedOffice());
			break;
		}
		GiveItem(ENightItem::OldKey);
		Prop->Consume();
		ShowNote(NightSchoolStory::KeyTitle(), NightSchoolStory::KeyBody());
		break;
	case ENightPropId::OfficeDoor:
		if (!HasItem(ENightItem::OldKey))
		{
			ShowNote(TEXT("锁着"), NightSchoolStory::LockedOffice());
			break;
		}
		bOfficeOpen = true;
		Prop->OpenDoor();
		break;
	case ENightPropId::Confession:
		bConfessionRead = true;
		bReadAnyPerson = true;
		ShowNote(NightSchoolStory::ConfessionTitle(), NightSchoolStory::ConfessionBody());
		break;
	case ENightPropId::Candle:
		GiveItem(ENightItem::Candle);
		Prop->Consume();
		ShowNote(NightSchoolStory::CandleTitle(), NightSchoolStory::CandleBody());
		break;
	case ENightPropId::RedScarf:
		GiveItem(ENightItem::RedScarf);
		Prop->Consume();
		ShowNote(NightSchoolStory::ScarfTitle(), NightSchoolStory::ScarfBody());
		break;
	case ENightPropId::Battery:
		if (ANightSchoolCharacter* Current = Player())
		{
			Current->RefillBattery(58.f);
		}
		GiveItem(ENightItem::Battery);
		Prop->Consume();
		ShowNote(NightSchoolStory::BatteryTitle(), NightSchoolStory::BatteryBody());
		break;
	case ENightPropId::AuditoriumDoor:
		if (!bConfessionRead)
		{
			ShowNote(TEXT("顶着"), NightSchoolStory::LockedAuditorium());
			break;
		}
		bAuditoriumOpen = true;
		Prop->OpenDoor();
		if (!bAuditoriumGhost)
		{
			bAuditoriumGhost = true;
			const FBox Room(W(500.f, 50.f, -50.f), W(1620.f, 1280.f, 500.f));
			TArray<FVector> Points;
			Points.Add(W(760.f, 260.f, 128.f));
			Points.Add(W(1460.f, 360.f, 128.f));
			Points.Add(W(1400.f, 1100.f, 128.f));
			Points.Add(W(800.f, 980.f, 128.f));
			SpawnGhost(Room, Points, TEXT("忘"));
			ShowStatus(TEXT("礼堂里有个写错的人。手电能把它逼退，蹲下能让它看丢。"), 5.f);
		}
		break;
	case ENightPropId::NameTrue:
		bTrueName = true;
		Remember(ENightName::ChenWangshu);
		ShowNote(NightSchoolStory::NameTrueTitle(), NightSchoolStory::NameTrueBody());
		break;
	case ENightPropId::NameWrongA:
	case ENightPropId::NameWrongB:
		Fear = FMath::Min(100.f, Fear + 14.f);
		ShowNote(NightSchoolStory::NameWrongTitle(), NightSchoolStory::NameWrongBody());
		if (Fear >= 100.f)
		{
			TriggerEnding(ENightEnding::Nameless, true);
		}
		break;
	case ENightPropId::ScarfHook:
		if (bScarfReturned)
		{
			ShowNote(NightSchoolStory::ScarfHookTitle(), TEXT("红领巾已经在第一排。它不需要被再放一次。"));
			break;
		}
		if (!HasItem(ENightItem::RedScarf))
		{
			ShowNote(TEXT("第一排"), TEXT("桌角空着。红领巾还在办公室，如果没被你拿走，就还在原来的地方。"));
			break;
		}
		Items.Remove(ENightItem::RedScarf);
		bScarfReturned = true;
		ShowNote(NightSchoolStory::ScarfHookTitle(), NightSchoolStory::ScarfHookBody());
		break;
	case ENightPropId::BoilerDoor:
		if (!bTrueName)
		{
			ShowNote(TEXT("烫"), NightSchoolStory::LockedBoiler());
			break;
		}
		bBoilerOpen = true;
		Prop->OpenDoor();
		if (!bBoilerGhost)
		{
			bBoilerGhost = true;
			const FBox Room(W(-420.f, 1460.f, -50.f), W(420.f, 2000.f, 420.f));
			TArray<FVector> Points;
			Points.Add(W(-200.f, 1600.f, 128.f));
			Points.Add(W(220.f, 1680.f, 128.f));
			Points.Add(W(180.f, 1900.f, 128.f));
			Points.Add(W(-220.f, 1880.f, 128.f));
			SpawnGhost(Room, Points, TEXT("鬼"));
		}
		break;
	case ENightPropId::Register:
		Remember(ENightName::LinQiuhe);
		Remember(ENightName::ZhaoTiesheng);
		Remember(ENightName::ZhouMian);
		GiveItem(ENightItem::Register);
		bRegisterRead = true;
		ShowNote(NightSchoolStory::RegisterTitle(), NightSchoolStory::RegisterBody());
		break;
	case ENightPropId::LastPaper:
		Remember(ENightName::LinQiuhe);
		GiveItem(ENightItem::LastPaper);
		ShowNote(NightSchoolStory::LastPaperTitle(), NightSchoolStory::LastPaperBody());
		break;
	case ENightPropId::GardenDoor:
		if (!bRegisterRead)
		{
			ShowNote(TEXT("后园"), NightSchoolStory::LockedGarden());
			break;
		}
		bGardenOpen = true;
		Prop->OpenDoor();
		break;
	case ENightPropId::Thesis:
		if (!(bRegisterRead && bTrueName && HasAllStudents()))
		{
			ShowNote(TEXT("还不能交"), NightSchoolStory::NeedPublication());
			break;
		}
		TriggerEnding(ENightEnding::Publication, false);
		break;
	case ENightPropId::Brazier:
		if (!(HasItem(ENightItem::Register) && HasItem(ENightItem::Candle)))
		{
			ShowNote(TEXT("火盆"), NightSchoolStory::NeedBurn());
			break;
		}
		TriggerEnding(ENightEnding::BurnRegister, false);
		break;
	case ENightPropId::Chair:
		if (!(bRegisterRead && bTrueName && HasAllStudents() && bScarfReturned))
		{
			ShowNote(TEXT("讲台"), NightSchoolStory::NeedStay());
			break;
		}
		TriggerEnding(ENightEnding::Stay, false);
		break;
	case ENightPropId::Mailbox:
		if (!(HasItem(ENightItem::Register) && HasItem(ENightItem::LastPaper) && bConfessionRead))
		{
			ShowNote(TEXT("信箱"), NightSchoolStory::NeedReply());
			break;
		}
		TriggerEnding(ENightEnding::Reply, false);
		break;
	default:
		break;
	}
}
