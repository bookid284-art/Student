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

void ANightSchoolDirector::AddLantern(float X, float Y, float Z, float Intensity, bool bCalm)
{
	const FVector At = W(X, Y, Z);
	AddCylinder(W(X, Y, Z + 55.f), 2.5f, 90.f, FRotator::ZeroRotator, Red, false);
	AddSphere(At, 18.f, FLinearColor(0.45f, 0.03f, 0.02f), false);
	AddSphere(At, 8.f, FLinearColor(1.f, 0.42f, 0.16f), false);
	if (bCalm)
	{
		WarmLights.Add(At);
	}
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
		UE_LOG(LogStudent, Error, TEXT("里屋：引擎基础方块没有加载，场景无法搭建。"));
		return;
	}
	BuildShell();
	BuildStructure();
	BuildDressing();
	BuildLights();
	BuildAudio();
	ShowStatus(TEXT("高三。住宿。手机在西边的宿舍里亮着。"), 12.f);
	bWorldBuilt = true;
	UE_LOG(LogStudent, Log, TEXT("里屋：幻觉已经搭好。"));
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

	FRandomStream Stars(20261023);
	for (int32 Index = 0; Index < 36; ++Index)
	{
		const float X = Stars.FRandRange(-1600.f, 1600.f);
		const float Y = Stars.FRandRange(-1200.f, 3000.f);
		if (X > -1300.f && X < 1400.f && Y > -800.f && Y < 2100.f)
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

	WallY(-600.f, -1100.f, 300.f);
	WallX(-1100.f, -600.f, 800.f);
	WallY(0.f, -1100.f, -775.f);
	WallY(0.f, -625.f, -75.f);
	WallY(0.f, 75.f, 300.f);
	LintelY(0.f, -700.f);
	LintelY(0.f, 0.f);
	WallX(300.f, -600.f, 0.f);
	WallX(300.f, 0.f, 325.f);
	WallX(300.f, 475.f, 1900.f);
	LintelX(300.f, 400.f);
	WallY(0.f, 300.f, 1200.f);
	WallX(1200.f, 0.f, 800.f);
	WallY(800.f, -1100.f, -300.f);
	WallY(800.f, 300.f, 525.f);
	WallY(800.f, 675.f, 1200.f);
	LintelY(800.f, 600.f);
	WallX(900.f, 800.f, 1900.f);
	WallY(1400.f, 300.f, 525.f);
	WallY(1400.f, 675.f, 900.f);
	LintelY(1400.f, 600.f);
	WallY(1900.f, 300.f, 900.f);
	WallY(1000.f, -300.f, -75.f);
	WallY(1000.f, 75.f, 300.f);
	LintelY(1000.f, 0.f);
	WallY(1600.f, -300.f, 300.f);
	WallX(-300.f, 0.f, 1425.f);
	WallX(-300.f, 1575.f, 1900.f);
	LintelX(-300.f, 1500.f);
	WallX(-800.f, 1400.f, 1900.f);
	WallY(1400.f, -800.f, -300.f);
	WallY(1900.f, -800.f, -300.f);
	WallY(1900.f, -300.f, 300.f);

	const FLinearColor Ceiling(0.07f, 0.065f, 0.06f);
	AddBox(W(-400.f, -300.f, 314.f), FVector(1360.f, 560.f, 16.f), Ceiling, true, true);
	AddBox(W(-700.f, 400.f, 314.f), FVector(760.f, 760.f, 16.f), Ceiling, true, true);
	AddBox(W(0.f, 500.f, 314.f), FVector(560.f, 960.f, 16.f), Ceiling, true, true);
	AddBox(W(750.f, 400.f, 314.f), FVector(860.f, 760.f, 16.f), Ceiling, true, true);
	AddBox(W(600.f, 1100.f, 314.f), FVector(560.f, 560.f, 16.f), Ceiling, true, true);
	AddBox(W(0.f, 1300.f, 314.f), FVector(560.f, 560.f, 16.f), Ceiling, true, true);
	AddBox(W(600.f, 1650.f, 314.f), FVector(560.f, 460.f, 16.f), Ceiling, true, true);
	AddBox(W(-550.f, 1650.f, 314.f), FVector(460.f, 460.f, 16.f), FLinearColor(0.03f, 0.028f, 0.03f), true, true);

	const FLinearColor FloorWood(0.16f, 0.08f, 0.04f);
	AddBox(W(-400.f, -300.f, 43.f), FVector(1320.f, 520.f, 4.f), Concrete, false, false);
	AddBox(W(-700.f, 400.f, 43.f), FVector(720.f, 720.f, 4.f), FloorWood, false, false);
	AddBox(W(0.f, 500.f, 43.f), FVector(520.f, 920.f, 4.f), Concrete, false, false);
	AddBox(W(750.f, 400.f, 43.f), FVector(820.f, 720.f, 4.f), FloorWood, false, false);
	AddBox(W(600.f, 1100.f, 43.f), FVector(520.f, 520.f, 4.f), DarkWood, false, false);
	AddBox(W(0.f, 1300.f, 43.f), FVector(520.f, 520.f, 4.f), FLinearColor(0.12f, 0.13f, 0.12f), false, false);
	AddBox(W(600.f, 1650.f, 43.f), FVector(520.f, 420.f, 4.f), FLinearColor(0.22f, 0.18f, 0.12f), false, false);
	AddBox(W(-550.f, 1650.f, 43.f), FVector(420.f, 420.f, 4.f), FLinearColor(0.04f, 0.035f, 0.035f), false, false);
	AddBox(W(-700.f, 0.f, 46.f), FVector(140.f, 36.f, 3.f), DarkWood, false, false);
	AddBox(W(300.f, 400.f, 46.f), FVector(36.f, 140.f, 3.f), DarkWood, false, false);
}

void ANightSchoolDirector::BuildDressing()
{
	auto Desk = [this](float X, float Y)
	{
		AddBox(W(X, Y, 112.f), FVector(120.f, 64.f, 8.f), Wood, true, false);
		AddBox(W(X - 46.f, Y - 22.f, 76.f), FVector(8.f, 8.f, 64.f), DarkWood, false, false);
		AddBox(W(X + 46.f, Y - 22.f, 76.f), FVector(8.f, 8.f, 64.f), DarkWood, false, false);
		AddBox(W(X - 46.f, Y + 22.f, 76.f), FVector(8.f, 8.f, 64.f), DarkWood, false, false);
		AddBox(W(X + 46.f, Y + 22.f, 76.f), FVector(8.f, 8.f, 64.f), DarkWood, false, false);
	};

	AddBox(W(-860.f, 600.f, 78.f), FVector(180.f, 90.f, 18.f), DarkWood, false, false);
	AddBox(W(-860.f, 660.f, 96.f), FVector(180.f, 10.f, 36.f), Wood, false, false);
	Desk(-980.f, 220.f);
	AddBox(W(-400.f, 180.f, 100.f), FVector(36.f, 28.f, 70.f), DarkWood, false, false);
	AddBox(W(-700.f, 772.f, 176.f), FVector(26.f, 12.f, 16.f), FLinearColor(0.02f, 0.02f, 0.025f), false, false);
	AddSphere(W(-700.f, 760.f, 188.f), 5.f, FLinearColor(0.85f, 0.05f, 0.04f), false);
	AddBox(W(-200.f, -320.f, 48.f), FVector(280.f, 36.f, 4.f), FLinearColor(0.28f, 0.08f, 0.05f), false, false);

	Desk(700.f, 180.f);
	Desk(700.f, 460.f);
	Desk(920.f, 200.f);
	Desk(1000.f, 600.f);
	AddBox(W(480.f, 150.f, 70.f), FVector(70.f, 36.f, 50.f), DarkWood, false, false);

	Desk(740.f, 1140.f);
	AddBox(W(-140.f, 1220.f, 78.f), FVector(180.f, 70.f, 16.f), FLinearColor(0.2f, 0.22f, 0.2f), false, false);
	AddBox(W(600.f, 1868.f, 150.f), FVector(70.f, 8.f, 110.f), FLinearColor(0.04f, 0.045f, 0.05f), false, false);
	AddSphere(W(760.f, 1720.f, 188.f), 4.f, FLinearColor(0.55f, 0.08f, 0.06f), false);
	AddBox(W(-680.f, 1820.f, 110.f), FVector(80.f, 28.f, 90.f), DarkWood, false, false);

	AddBox(W(-400.f, -300.f, 292.f), FVector(220.f, 16.f, 8.f), FLinearColor(0.55f, 0.58f, 0.55f), false, false);
	AddBox(W(0.f, 500.f, 292.f), FVector(180.f, 16.f, 8.f), FLinearColor(0.45f, 0.48f, 0.46f), false, false);
	AddBox(W(750.f, 400.f, 292.f), FVector(240.f, 16.f, 8.f), FLinearColor(0.5f, 0.52f, 0.48f), false, false);
	AddLabel(W(-520.f, -300.f, 170.f), 0.f, TEXT("往西 · 宿舍"), 16.f, FColor(220, 200, 160));
	AddLabel(W(250.f, 400.f, 210.f), 180.f, TEXT("教室"), 16.f, FColor(220, 200, 160));
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

	const FVector Eye = W(-700.f, 760.f, 188.f);
	APointLight* CameraLed = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), FTransform(Eye), Params);
	if (CameraLed)
	{
		CameraLed->Tags.Add(TEXT("NightSchool"));
		CameraLed->SetMobility(EComponentMobility::Movable);
		if (UPointLightComponent* Component = CameraLed->PointLightComponent)
		{
			Component->SetMobility(EComponentMobility::Movable);
			Component->bUseInverseSquaredFalloff = false;
			Component->SetIntensity(8.f);
			Component->SetAttenuationRadius(220.f);
			Component->SetLightColor(FLinearColor(1.f, 0.15f, 0.1f));
			Component->SetCastShadows(false);
		}
	}

	const FVector GlowAt = W(600.f, 1650.f, 250.f);
	APointLight* Glow = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), FTransform(GlowAt), Params);
	if (Glow)
	{
		Glow->Tags.Add(TEXT("NightSchool"));
		Glow->SetMobility(EComponentMobility::Movable);
		if (UPointLightComponent* Component = Glow->PointLightComponent)
		{
			Component->SetMobility(EComponentMobility::Movable);
			Component->bUseInverseSquaredFalloff = false;
			Component->SetIntensity(36.f);
			Component->SetAttenuationRadius(900.f);
			Component->SetLightColor(FLinearColor(1.f, 0.84f, 0.68f));
			Component->SetCastShadows(false);
		}
		WarmLights.Add(GlowAt);
	}

	AddLantern(-700.f, -280.f, 230.f, 16.f, true);
	AddLantern(0.f, -280.f, 230.f, 16.f, true);
	AddLantern(-700.f, 420.f, 240.f, 14.f, true);
	AddLantern(0.f, 500.f, 240.f, 11.f, false);
	AddLantern(750.f, 360.f, 240.f, 11.f, false);
	AddLantern(600.f, 1100.f, 230.f, 10.f, false);
	AddLantern(0.f, 1280.f, 230.f, 16.f, true);
	AddLantern(600.f, 1650.f, 240.f, 22.f, true);
	AddLantern(-550.f, 1650.f, 210.f, 7.f, true);

	SpawnProp(ENightPropId::Phone, W(-980.f, 220.f, 122.f), FVector(22.f, 16.f, 6.f), FLinearColor(0.04f, 0.045f, 0.05f), FText::FromString(TEXT("看未接来电")), FVector::ZeroVector, 0.f, TEXT("手机"));
	SpawnProp(ENightPropId::Earplugs, W(-860.f, 600.f, 96.f), FVector(12.f, 10.f, 5.f), FLinearColor(0.75f, 0.72f, 0.64f), FText::FromString(TEXT("拿走耳塞")), FVector::ZeroVector, 0.f, TEXT("耳塞"));
	SpawnProp(ENightPropId::LensCloth, W(-640.f, 700.f, 110.f), FVector(16.f, 12.f, 4.f), FLinearColor(0.55f, 0.56f, 0.58f), FText::FromString(TEXT("拿走镜头布")), FVector::ZeroVector, -90.f, TEXT("镜头布"));
	SpawnProp(ENightPropId::DormCamera, W(-700.f, 748.f, 170.f), FVector(18.f, 12.f, 16.f), FLinearColor(0.02f, 0.02f, 0.02f), FText::FromString(TEXT("盖上镜头")), FVector::ZeroVector, -90.f, TEXT("镜头"));
	SpawnProp(ENightPropId::Soda, W(-380.f, 200.f, 112.f), FVector(10.f, 10.f, 18.f), FLinearColor(0.15f, 0.38f, 0.18f), FText::FromString(TEXT("拿走汽水")), FVector::ZeroVector, 180.f, TEXT("汽水"));
	SpawnProp(ENightPropId::HallDoor, W(0.f, 0.f, 145.f), FVector(68.f, 8.f, 100.f), DarkWood, FText::FromString(TEXT("推开走廊")), FVector(200.f, 0.f, 0.f), -90.f, TEXT("走廊"));
	SpawnProp(ENightPropId::LureHall, W(160.f, 640.f, 88.f), FVector(28.f, 28.f, 12.f), Concrete, FText::FromString(TEXT("放下汽水")), FVector::ZeroVector, -90.f, TEXT("地砖"));
	SpawnProp(ENightPropId::LureClass, W(640.f, 280.f, 88.f), FVector(28.f, 28.f, 12.f), Concrete, FText::FromString(TEXT("放下汽水")), FVector::ZeroVector, 180.f, TEXT("地砖"));
	SpawnProp(ENightPropId::ExamPaper, W(1000.f, 600.f, 124.f), FVector(24.f, 18.f, 5.f), Paper, FText::FromString(TEXT("拿走旧试卷")), FVector::ZeroVector, -90.f, TEXT("试卷"));
	SpawnProp(ENightPropId::UnwantedDesk, W(1120.f, 700.f, 84.f), FVector(24.f, 22.f, 28.f), Wood, FText::FromString(TEXT("在空位坐下")), FVector::ZeroVector, -90.f, TEXT("空位"));
	SpawnProp(ENightPropId::HideScore, W(460.f, 180.f, 100.f), FVector(22.f, 16.f, 18.f), DarkWood, FText::FromString(TEXT("把分数藏进抽屉")), FVector::ZeroVector, -90.f, TEXT("抽屉"));
	SpawnProp(ENightPropId::OfficeDoor, W(600.f, 800.f, 145.f), FVector(68.f, 8.f, 100.f), DarkWood, FText::FromString(TEXT("把试卷交到门上")), FVector(200.f, 0.f, 0.f), -90.f, TEXT("办公室"));
	SpawnProp(ENightPropId::TeacherNote, W(740.f, 1140.f, 128.f), FVector(22.f, 16.f, 4.f), Paper, FText::FromString(TEXT("读梁老师的字条")), FVector::ZeroVector, -90.f, TEXT("字条"));
	SpawnProp(ENightPropId::InfirmaryDoor, W(0.f, 1000.f, 145.f), FVector(68.f, 8.f, 100.f), FLinearColor(0.16f, 0.18f, 0.16f), FText::FromString(TEXT("进入校医室")), FVector(200.f, 0.f, 0.f), -90.f, TEXT("校医室"));
	SpawnProp(ENightPropId::Zhou, W(60.f, 1360.f, 110.f), FVector(28.f, 28.f, 36.f), FLinearColor(0.18f, 0.16f, 0.13f), FText::FromString(TEXT("听周先生说")), FVector::ZeroVector, -90.f, TEXT("周先生"));
	SpawnProp(ENightPropId::LightDoor, W(600.f, 1400.f, 145.f), FVector(68.f, 8.f, 100.f), FLinearColor(0.32f, 0.26f, 0.16f), FText::FromString(TEXT("进入光里")), FVector(200.f, 0.f, 0.f), -90.f, TEXT("光"));
	SpawnProp(ENightPropId::FinalCamera, W(760.f, 1720.f, 160.f), FVector(16.f, 12.f, 14.f), FLinearColor(0.03f, 0.03f, 0.035f), FText::FromString(TEXT("看这只镜头")), FVector::ZeroVector, 180.f, TEXT("镜头"));
	SpawnProp(ENightPropId::FaceSwitch, W(460.f, 1680.f, 130.f), FVector(12.f, 12.f, 20.f), FLinearColor(0.72f, 0.62f, 0.38f), FText::FromString(TEXT("用手拨开开关")), FVector::ZeroVector, -90.f, TEXT("开关"));
	SpawnProp(ENightPropId::ClosetDoor, W(-300.f, 1500.f, 145.f), FVector(8.f, 68.f, 100.f), DarkWood, FText::FromString(TEXT("打开柜子")), FVector(0.f, 200.f, 0.f), 0.f, TEXT("柜子"));
	SpawnProp(ENightPropId::ClosetLock, W(-560.f, 1720.f, 120.f), FVector(16.f, 14.f, 24.f), FLinearColor(0.12f, 0.1f, 0.09f), FText::FromString(TEXT("从里面把门带上")), FVector::ZeroVector, 0.f, TEXT("锁"));

	AddSprite(W(600.f, 1868.f, 180.f), -90.f, NightSchoolArt::MakeMirror(this), 56.f, 74.f);
	AddSprite(W(1000.f, 560.f, 150.f), -90.f, NightSchoolArt::MakeScoreSheet(this), 36.f, 46.f);
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

FBox ANightSchoolDirector::HallRoom() const
{
	return FBox(W(-260.f, 80.f, -20.f), W(260.f, 960.f, 420.f));
}

FBox ANightSchoolDirector::ClassRoom() const
{
	return FBox(W(360.f, 40.f, -20.f), W(1160.f, 760.f, 420.f));
}

bool ANightSchoolDirector::IsInDorm(const FVector& Location) const
{
	const FVector Local = Location - NightSchool::Origin();
	return Local.X > -1080.f && Local.X < -320.f && Local.Y > 30.f && Local.Y < 770.f;
}

bool ANightSchoolDirector::GetLurePoint(const FBox& Room, FVector& OutPoint) const
{
	if (LureTime <= 0.f || !Room.IsInsideXY(LurePoint))
	{
		return false;
	}
	OutPoint = LurePoint;
	return true;
}

void ANightSchoolDirector::RefreshEarplugPrompt()
{
	ANightSchoolProp* Prop = FindProp(ENightPropId::Earplugs);
	if (!Prop)
	{
		return;
	}
	if (!bHasEarplugs)
	{
		Prop->SetPrompt(FText::FromString(TEXT("拿走耳塞")));
	}
	else if (bWearingEarplugs)
	{
		Prop->SetPrompt(FText::FromString(TEXT("拿下耳塞")));
	}
	else
	{
		Prop->SetPrompt(FText::FromString(TEXT("戴上耳塞")));
	}
}

void ANightSchoolDirector::SpawnHallGhosts()
{
	if (bGhostsSpawned)
	{
		return;
	}
	bGhostsSpawned = true;

	const FBox Hall = HallRoom();
	TArray<FVector> HallPoints;
	HallPoints.Add(W(-120.f, 220.f, 128.f));
	HallPoints.Add(W(140.f, 480.f, 128.f));
	HallPoints.Add(W(-80.f, 780.f, 128.f));
	HallPoints.Add(W(100.f, 300.f, 128.f));
	SpawnGhost(Hall, HallPoints, TEXT("笑"));

	const FBox Room = ClassRoom();
	TArray<FVector> ClassPoints;
	ClassPoints.Add(W(520.f, 200.f, 128.f));
	ClassPoints.Add(W(980.f, 220.f, 128.f));
	ClassPoints.Add(W(1000.f, 560.f, 128.f));
	ClassPoints.Add(W(560.f, 600.f, 128.f));
	SpawnGhost(Room, ClassPoints, TEXT("请"));
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
	if (!IsEnding() && !bNoteOpen)
	{
		LureTime = FMath::Max(0.f, LureTime - DeltaSeconds);
	}
	if (ANightSchoolCharacter* Current = Player())
	{
		if (!IsEnding() && !bNoteOpen)
		{
			if (!bCameraCovered && IsInDorm(Current->GetActorLocation()))
			{
				const float Rate = bWearingEarplugs ? 3.f : 6.f;
				Fear = FMath::Min(100.f, Fear + DeltaSeconds * Rate);
				if (Fear >= 70.f && !bPressureWarned)
				{
					bPressureWarned = true;
					ShowStatus(TEXT("你开始想找一个没有声音的柜子。"), 4.2f);
				}
			}
			else if (FearLock <= 0.f && IsInWarmLight(Current->GetActorLocation()))
			{
				Fear = FMath::Max(0.f, Fear - DeltaSeconds * 7.f);
			}
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
		if (FVector::DistSquared(Location, Lamp) <= FMath::Square(480.f))
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

void ANightSchoolDirector::TriggerEnding(ENightEnding Ending)
{
	if (IsEnding() || Ending == ENightEnding::None)
	{
		return;
	}
	CurrentEnding = Ending;
	EndingTitle = NightSchoolStory::EndingTitle(Ending);
	EndingBody = NightSchoolStory::EndingBody(Ending);
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
	if (!bPhoneRead)
	{
		return TEXT("西边是宿舍。先看那只每天都响的手机。");
	}
	if (!bHasCloth && !bCameraCovered)
	{
		return TEXT("床边有一块镜头布。北墙那只眼睛还亮着。");
	}
	if (!bCameraCovered || !bHallOpen)
	{
		return TEXT("把布盖上宿舍的镜头，走廊才会开。");
	}
	if (!bSatAlone)
	{
		return TEXT("教室在走廊东边。有一张没有人肯同桌的空位。");
	}
	if (!bHasExam && !bExamGiven)
	{
		return TEXT("旧试卷就在那张空位旁边。");
	}
	if (!bExamGiven || !bOfficeOpen)
	{
		return TEXT("把试卷交到办公室门口。藏进抽屉不算。");
	}
	if (!bTeacherRead)
	{
		return TEXT("读梁老师留在桌上的那张字条。");
	}
	if (!bInfirmaryOpen)
	{
		return TEXT("回走廊，往北进校医室。");
	}
	if (!bZhouHeard)
	{
		return TEXT("听周先生把两扇门说完。");
	}
	if (bWearingEarplugs)
	{
		return TEXT("光的开关要先回宿舍拿下耳塞。柜子在校医室西边，从里面带上门就会留下。");
	}
	return TEXT("办公室北面，用手拨开关才能离开。校医室西边的柜子，从里面带上门就会留下。");
}

FString ANightSchoolDirector::GetFacingLine() const
{
	TArray<FString> Parts;
	if (bPhoneRead) Parts.Add(TEXT("电话"));
	if (bCameraCovered) Parts.Add(TEXT("镜头"));
	if (bSatAlone) Parts.Add(TEXT("空位"));
	if (bExamGiven) Parts.Add(TEXT("交卷"));
	if (bTeacherRead) Parts.Add(TEXT("字条"));
	if (bZhouHeard) Parts.Add(TEXT("周先生"));
	if (bLightOpen) Parts.Add(TEXT("光门已开"));
	if (bClosetOpen) Parts.Add(TEXT("柜门已开"));
	if (bWearingEarplugs) Parts.Add(TEXT("耳塞还戴着"));
	if (Parts.Num() == 0)
	{
		return TEXT("还没开始");
	}
	FString Line;
	for (int32 Index = 0; Index < Parts.Num(); ++Index)
	{
		if (Index > 0)
		{
			Line += TEXT("、");
		}
		Line += Parts[Index];
	}
	return Line;
}

FString ANightSchoolDirector::GetItemsLine() const
{
	FString Line;
	auto Append = [&Line](const TCHAR* Word)
	{
		if (!Line.IsEmpty())
		{
			Line += TEXT("  ");
		}
		Line += Word;
	};
	if (bWearingEarplugs) Append(TEXT("耳塞戴着"));
	else if (bHasEarplugs) Append(TEXT("耳塞在手里"));
	if (bHasCloth) Append(TEXT("镜头布"));
	if (bHasSoda) Append(TEXT("汽水"));
	if (bHasExam) Append(TEXT("试卷"));
	return Line.IsEmpty() ? TEXT("空的") : Line;
}

void ANightSchoolDirector::OnCaughtByApparition()
{
	if (IsEnding())
	{
		return;
	}
	const float Amount = bWearingEarplugs ? 18.f : 28.f;
	Fear = FMath::Min(100.f, Fear + Amount);
	FearLock = 2.4f;
	if (Fear >= 70.f)
	{
		bPressureWarned = true;
		ShowStatus(TEXT("你开始想找一个没有声音的柜子。"), 3.6f);
	}
	else
	{
		ShowStatus(TEXT("笑声贴上来，又退开了。你没有可以还手的地方。"), 3.2f);
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
	case ENightPropId::Phone:
		bPhoneRead = true;
		ShowNote(NightSchoolStory::PhoneTitle(), NightSchoolStory::PhoneBody());
		break;
	case ENightPropId::Earplugs:
		if (!bHasEarplugs)
		{
			bHasEarplugs = true;
			bWearingEarplugs = true;
			RefreshEarplugPrompt();
			ShowNote(NightSchoolStory::EarplugTitle(), NightSchoolStory::EarplugBody());
			break;
		}
		bWearingEarplugs = !bWearingEarplugs;
		RefreshEarplugPrompt();
		ShowStatus(bWearingEarplugs ? TEXT("耳塞回到耳朵里。世界远了一点。") : TEXT("你把耳塞拿下来。声音重新贴着头皮。"), 3.4f);
		break;
	case ENightPropId::LensCloth:
		bHasCloth = true;
		Prop->Consume();
		ShowNote(NightSchoolStory::ClothTitle(), NightSchoolStory::ClothBody());
		break;
	case ENightPropId::DormCamera:
		if (!bHasCloth)
		{
			ShowNote(TEXT("红点"), TEXT("红点对着床。镜头布在床边，如果还没拿。布只能盖这一只。"));
			break;
		}
		bHasCloth = false;
		bCameraCovered = true;
		bHallOpen = true;
		if (ANightSchoolProp* Door = FindProp(ENightPropId::HallDoor))
		{
			Door->OpenDoor();
		}
		SpawnHallGhosts();
		Prop->Consume();
		ShowNote(TEXT("红点"), NightSchoolStory::CameraBody());
		break;
	case ENightPropId::HallDoor:
		if (!bCameraCovered)
		{
			ShowNote(TEXT("走廊"), NightSchoolStory::LockedHall());
			break;
		}
		bHallOpen = true;
		Prop->OpenDoor();
		break;
	case ENightPropId::Soda:
		bHasSoda = true;
		Prop->Consume();
		ShowNote(TEXT("汽水"), TEXT("一瓶没开封的汽水，瓶壁是凉的。放在走廊或教室的地砖上，气泡会响大约十八秒。谁听见谁就走过去。它打不碎谁，也伤不了你。"));
		break;
	case ENightPropId::LureHall:
	case ENightPropId::LureClass:
		if (!bHasSoda)
		{
			ShowNote(TEXT("地砖"), NightSchoolStory::SodaNeed());
			break;
		}
		if (!bGhostsSpawned)
		{
			ShowNote(TEXT("地砖"), NightSchoolStory::SodaEarly());
			break;
		}
		{
			const FVector Here = Prop->GetActorLocation();
			if (!HallRoom().IsInsideXY(Here) && !ClassRoom().IsInsideXY(Here))
			{
				ShowStatus(TEXT("这瓶汽水在这里引不来人。"), 3.f);
				break;
			}
			bHasSoda = false;
			LurePoint = Here;
			LureTime = 18.f;
			ShowStatus(TEXT("汽水在地砖上响了十八秒。听见的人会朝这边走。这不是武器。"), 4.5f);
		}
		break;
	case ENightPropId::UnwantedDesk:
		bSatAlone = true;
		ShowNote(NightSchoolStory::DeskTitle(), NightSchoolStory::DeskBody());
		break;
	case ENightPropId::ExamPaper:
		bHasExam = true;
		Prop->Consume();
		ShowNote(NightSchoolStory::ExamTitle(), NightSchoolStory::ExamBody());
		break;
	case ENightPropId::HideScore:
		if (bExamGiven)
		{
			ShowNote(NightSchoolStory::HideScoreTitle(), TEXT("卷子已经放在办公室门口。抽屉改变不了这件事。"));
			break;
		}
		if (!bHasExam)
		{
			ShowNote(NightSchoolStory::HideScoreTitle(), NightSchoolStory::HideScoreEmpty());
			break;
		}
		ShowNote(NightSchoolStory::HideScoreTitle(), NightSchoolStory::HideScoreBody());
		break;
	case ENightPropId::OfficeDoor:
		if (!bHasExam && !bExamGiven)
		{
			ShowNote(TEXT("办公室"), NightSchoolStory::LockedOffice());
			break;
		}
		bHasExam = false;
		bExamGiven = true;
		bOfficeOpen = true;
		Prop->OpenDoor();
		ShowStatus(TEXT("你把卷子放在门上。门开了。分数没有变。"), 4.5f);
		break;
	case ENightPropId::TeacherNote:
		bTeacherRead = true;
		ShowNote(NightSchoolStory::TeacherTitle(), NightSchoolStory::TeacherBody());
		if (!bInfirmaryOpen)
		{
			bInfirmaryOpen = true;
			if (ANightSchoolProp* Door = FindProp(ENightPropId::InfirmaryDoor))
			{
				Door->OpenDoor();
			}
			ShowStatus(TEXT("北面校医室的门开了。"), 14.f);
		}
		break;
	case ENightPropId::InfirmaryDoor:
		if (!bTeacherRead)
		{
			ShowNote(TEXT("校医室"), NightSchoolStory::LockedInfirmary());
			break;
		}
		bInfirmaryOpen = true;
		Prop->OpenDoor();
		break;
	case ENightPropId::Zhou:
		bZhouHeard = true;
		ShowNote(NightSchoolStory::ZhouTitle(), NightSchoolStory::ZhouBody());
		break;
	case ENightPropId::LightDoor:
		if (!bZhouHeard)
		{
			ShowNote(TEXT("光"), NightSchoolStory::LockedLight());
			break;
		}
		bLightOpen = true;
		Prop->OpenDoor();
		break;
	case ENightPropId::FinalCamera:
		ShowNote(NightSchoolStory::FinalCameraTitle(), NightSchoolStory::FinalCameraBody());
		break;
	case ENightPropId::FaceSwitch:
		if (!bZhouHeard)
		{
			ShowNote(TEXT("开关"), NightSchoolStory::NeedZhou());
			break;
		}
		if (bWearingEarplugs)
		{
			ShowNote(TEXT("开关"), NightSchoolStory::NeedEarplugsOff());
			break;
		}
		if (!bSatAlone)
		{
			ShowNote(TEXT("开关"), NightSchoolStory::NeedSit());
			break;
		}
		if (!bExamGiven)
		{
			ShowNote(TEXT("开关"), NightSchoolStory::NeedExam());
			break;
		}
		TriggerEnding(ENightEnding::InnerLight);
		break;
	case ENightPropId::ClosetDoor:
		if (!bZhouHeard)
		{
			ShowNote(TEXT("柜子"), NightSchoolStory::LockedCloset());
			break;
		}
		bClosetOpen = true;
		Prop->OpenDoor();
		break;
	case ENightPropId::ClosetLock:
		if (!bZhouHeard)
		{
			ShowNote(TEXT("柜子"), NightSchoolStory::LockedCloset());
			break;
		}
		bClosetOpen = true;
		TriggerEnding(ENightEnding::Sealed);
		break;
	default:
		break;
	}
}
