#include "NightSchool/NightSchoolDirector.h"

#include "Animation/AnimationAsset.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/AudioComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/WidgetComponent.h"
#include "Math/NumericLimits.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Engine/DirectionalLight.h"
#include "GameFramework/PlayerController.h"
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

ANightSchoolApparition* ANightSchoolDirector::SpawnGhost(const FBox& Room, const TArray<FVector>& Points, const FString& Glyph)
{
	if (!GetWorld() || Points.Num() == 0)
	{
		return nullptr;
	}
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ANightSchoolApparition* Ghost = GetWorld()->SpawnActor<ANightSchoolApparition>(ANightSchoolApparition::StaticClass(), FTransform(FRotator::ZeroRotator, Points[0]), Params);
	if (!Ghost)
	{
		return nullptr;
	}
	Ghost->Configure(Room, Points, Glyph);
	Apparitions.Add(Ghost);
	return Ghost;
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
	BuildRooms();
	BuildSelfFigure();
	BuildCheckpoints();
	BuildAudio();
	PreviewHold = 1.1f;
	ShowStatus(TEXT("第一章。手机在西边的宿舍里亮着。这间屋一共六章。"), 12.f);
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
		const float X = Stars.FRandRange(-2300.f, 2300.f);
		const float Y = Stars.FRandRange(-1600.f, 3400.f);
		if (X > -2250.f && X < 2360.f && Y > -1450.f && Y < 2750.f)
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

	WallY(-600.f, -1100.f, -775.f);
	WallY(-600.f, -625.f, 300.f);
	LintelY(-600.f, -700.f);
	WallX(-1100.f, -1400.f, -1075.f);
	WallX(-1100.f, -925.f, 800.f);
	LintelX(-1100.f, -1000.f);
	WallX(-2200.f, -1400.f, -600.f);
	WallY(-1400.f, -2200.f, -1100.f);
	WallY(-600.f, -2200.f, -1100.f);
	WallY(-1400.f, -1100.f, 300.f);
	WallX(300.f, -1400.f, -600.f);
	WallX(-350.f, -1400.f, -1075.f);
	WallX(-350.f, -925.f, -600.f);
	LintelX(-350.f, -1000.f);
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
	WallX(1200.f, 0.f, 325.f);
	WallX(1200.f, 475.f, 800.f);
	LintelX(1200.f, 400.f);
	WallX(2000.f, 0.f, 325.f);
	WallX(2000.f, 475.f, 800.f);
	LintelX(2000.f, 400.f);
	WallX(2320.f, 0.f, 800.f);
	WallY(0.f, 1200.f, 2320.f);
	WallY(800.f, 1200.f, 2320.f);
	WallY(800.f, -1100.f, -775.f);
	WallY(800.f, -625.f, -300.f);
	LintelY(800.f, -700.f);
	WallX(-1100.f, 800.f, 1400.f);
	WallY(1400.f, -1100.f, -800.f);
	WallY(800.f, 300.f, 525.f);
	WallY(800.f, 675.f, 1200.f);
	LintelY(800.f, 600.f);
	WallX(900.f, 800.f, 975.f);
	WallX(900.f, 1125.f, 1900.f);
	LintelX(900.f, 1050.f);
	WallY(800.f, 900.f, 1600.f);
	WallX(1600.f, 800.f, 1025.f);
	WallX(1600.f, 1175.f, 1400.f);
	LintelX(1600.f, 1100.f);
	WallX(2200.f, 800.f, 1400.f);
	WallY(1400.f, 900.f, 1125.f);
	WallY(1400.f, 1275.f, 2200.f);
	LintelY(1400.f, 1200.f);
	WallY(1900.f, 900.f, 1525.f);
	WallY(1900.f, 1675.f, 2200.f);
	LintelY(1900.f, 1600.f);
	WallX(2200.f, 1400.f, 2700.f);
	WallX(900.f, 1900.f, 2700.f);
	WallY(2700.f, 900.f, 2200.f);
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
	AddBox(W(-400.f, -1000.f, 314.f), FVector(1360.f, 760.f, 16.f), Ceiling, true, true);
	AddBox(W(-700.f, 1100.f, 314.f), FVector(760.f, 560.f, 16.f), Ceiling, true, true);
	AddBox(W(1600.f, 400.f, 314.f), FVector(760.f, 760.f, 16.f), Ceiling, true, true);
	AddBox(W(1250.f, 1100.f, 314.f), FVector(660.f, 560.f, 16.f), Ceiling, true, true);
	AddBox(W(1900.f, 1100.f, 314.f), FVector(560.f, 560.f, 16.f), Ceiling, true, true);
	AddBox(W(1550.f, 1650.f, 314.f), FVector(1260.f, 460.f, 16.f), Ceiling, true, true);
	AddBox(W(-1650.f, -1000.f, 314.f), FVector(1060.f, 760.f, 16.f), Ceiling, true, true);
	AddBox(W(2160.f, 400.f, 314.f), FVector(280.f, 760.f, 16.f), Ceiling, true, true);
	AddBox(W(1550.f, 2300.f, 314.f), FVector(1260.f, 760.f, 16.f), Ceiling, true, true);

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
	AddBox(W(-700.f, -1000.f, 43.f), FVector(700.f, 720.f, 4.f), Concrete, false, false);
	AddBox(W(-40.f, -1100.f, 43.f), FVector(620.f, 520.f, 4.f), FLinearColor(0.1f, 0.11f, 0.12f), false, false);
	AddBox(W(-700.f, 1100.f, 43.f), FVector(700.f, 520.f, 4.f), DarkWood, false, false);
	AddBox(W(1600.f, 400.f, 43.f), FVector(720.f, 720.f, 4.f), FloorWood, false, false);
	AddBox(W(1250.f, 1100.f, 43.f), FVector(620.f, 520.f, 4.f), DarkWood, false, false);
	AddBox(W(1900.f, 1100.f, 43.f), FVector(520.f, 520.f, 4.f), FLinearColor(0.08f, 0.09f, 0.1f), false, false);
	AddBox(W(1550.f, 1650.f, 43.f), FVector(1200.f, 420.f, 4.f), Concrete, false, false);
	AddBox(W(-700.f, -600.f, 46.f), FVector(140.f, 36.f, 3.f), DarkWood, false, false);
	AddBox(W(-700.f, 800.f, 46.f), FVector(140.f, 36.f, 3.f), DarkWood, false, false);
	AddBox(W(1200.f, 400.f, 46.f), FVector(36.f, 140.f, 3.f), DarkWood, false, false);
	AddBox(W(-1650.f, -1000.f, 43.f), FVector(1000.f, 720.f, 4.f), Concrete, false, false);
	AddBox(W(2160.f, 400.f, 43.f), FVector(240.f, 720.f, 4.f), DarkWood, false, false);
	AddBox(W(1550.f, 2300.f, 43.f), FVector(1200.f, 720.f, 4.f), Concrete, false, false);
	AddBox(W(-1100.f, -1000.f, 46.f), FVector(36.f, 140.f, 3.f), DarkWood, false, false);
	AddBox(W(2000.f, 400.f, 46.f), FVector(36.f, 140.f, 3.f), DarkWood, false, false);
	AddBox(W(1600.f, 1900.f, 46.f), FVector(140.f, 36.f, 3.f), DarkWood, false, false);
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
	AddLantern(-750.f, -1050.f, 230.f, 14.f, true);
	AddLantern(-40.f, -1100.f, 220.f, 8.f, false);
	AddLantern(-700.f, 1100.f, 230.f, 8.f, false);
	AddLantern(1600.f, 400.f, 230.f, 10.f, false);
	AddLantern(1250.f, 1100.f, 230.f, 14.f, true);
	AddLantern(1900.f, 1100.f, 220.f, 8.f, false);
	AddLantern(1550.f, 1650.f, 230.f, 9.f, false);
	AddLantern(-1650.f, -1000.f, 230.f, 14.f, true);
	AddLantern(2160.f, 400.f, 220.f, 8.f, false);
	AddLantern(1550.f, 2300.f, 230.f, 8.f, false);

	SpawnProp(ENightPropId::Phone, W(-980.f, 220.f, 122.f), FVector(22.f, 16.f, 6.f), FLinearColor(0.04f, 0.045f, 0.05f), FText::FromString(TEXT("看未接来电")), FVector::ZeroVector, 0.f, TEXT("手机"));
	SpawnProp(ENightPropId::Earplugs, W(-860.f, 600.f, 96.f), FVector(12.f, 10.f, 5.f), FLinearColor(0.75f, 0.72f, 0.64f), FText::FromString(TEXT("拿走耳塞")), FVector::ZeroVector, 0.f, TEXT("耳塞"));
	SpawnProp(ENightPropId::LensCloth, W(-780.f, -1100.f, 110.f), FVector(16.f, 12.f, 4.f), FLinearColor(0.55f, 0.56f, 0.58f), FText::FromString(TEXT("拿走镜头布")), FVector::ZeroVector, 90.f, TEXT("镜头布"));
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
	SpawnProp(ENightPropId::DutyLogA, W(-900.f, -1200.f, 110.f), FVector(22.f, 16.f, 4.f), Paper, FText::FromString(TEXT("读三月的记录")), FVector::ZeroVector, 90.f, TEXT("三月"));
	SpawnProp(ENightPropId::DutyLogB, W(-620.f, -900.f, 110.f), FVector(22.f, 16.f, 4.f), Paper, FText::FromString(TEXT("读四月的记录")), FVector::ZeroVector, 0.f, TEXT("四月"));
	SpawnProp(ENightPropId::DutyLogC, W(-860.f, -780.f, 110.f), FVector(22.f, 16.f, 4.f), Paper, FText::FromString(TEXT("读昨夜的记录")), FVector::ZeroVector, -90.f, TEXT("昨夜"));
	SpawnProp(ENightPropId::Coin, W(-520.f, 360.f, 100.f), FVector(12.f, 10.f, 6.f), FLinearColor(0.62f, 0.5f, 0.18f), FText::FromString(TEXT("拿走午饭钱")), FVector::ZeroVector, 0.f, TEXT("铁盒"));
	SpawnProp(ENightPropId::NoteSeat, W(980.f, 500.f, 90.f), FVector(16.f, 12.f, 3.f), Paper, FText::FromString(TEXT("捡起桌腿下的条子")), FVector::ZeroVector, -90.f, TEXT("条子"));
	SpawnProp(ENightPropId::NoteHall, W(-180.f, 250.f, 120.f), FVector(18.f, 8.f, 22.f), Paper, FText::FromString(TEXT("读走廊上的条子")), FVector::ZeroVector, 0.f, TEXT("条子"));
	SpawnProp(ENightPropId::NoteShop, W(1700.f, 520.f, 110.f), FVector(16.f, 12.f, 3.f), Paper, FText::FromString(TEXT("读柜台里的条子")), FVector::ZeroVector, 180.f, TEXT("条子"));
	SpawnProp(ENightPropId::NoteLocker, W(-860.f, 1000.f, 120.f), FVector(14.f, 8.f, 18.f), Paper, FText::FromString(TEXT("抽出柜缝里的条子")), FVector::ZeroVector, 0.f, TEXT("条子"));
	SpawnProp(ENightPropId::ShopDoor, W(1200.f, 400.f, 145.f), FVector(8.f, 68.f, 100.f), FLinearColor(0.22f, 0.18f, 0.12f), FText::FromString(TEXT("把硬币送进卷帘")), FVector(0.f, 200.f, 0.f), 180.f, TEXT("小卖部"));
	SpawnProp(ENightPropId::Ledger, W(1500.f, 220.f, 100.f), FVector(22.f, 16.f, 6.f), DarkWood, FText::FromString(TEXT("拿走请客账")), FVector::ZeroVector, -90.f, TEXT("账本"));
	SpawnProp(ENightPropId::LedgerBoard, W(200.f, 720.f, 150.f), FVector(28.f, 8.f, 36.f), Wood, FText::FromString(TEXT("把账本钉上布告栏")), FVector::ZeroVector, 180.f, TEXT("布告"));
	SpawnProp(ENightPropId::GradeDoor, W(900.f, 1050.f, 145.f), FVector(8.f, 68.f, 100.f), DarkWood, FText::FromString(TEXT("进入年级组")), FVector(0.f, 200.f, 0.f), 180.f, TEXT("年级组"));
	SpawnProp(ENightPropId::MinutesSun, W(1100.f, 980.f, 120.f), FVector(20.f, 14.f, 4.f), Paper, FText::FromString(TEXT("读孙老师的记录")), FVector::ZeroVector, -90.f, TEXT("孙"));
	SpawnProp(ENightPropId::MinutesLiang, W(1400.f, 1100.f, 120.f), FVector(20.f, 14.f, 4.f), Paper, FText::FromString(TEXT("读页边上的字")), FVector::ZeroVector, 180.f, TEXT("页边"));
	SpawnProp(ENightPropId::MinutesParent, W(1150.f, 1260.f, 120.f), FVector(20.f, 14.f, 4.f), Paper, FText::FromString(TEXT("读家委会那一页")), FVector::ZeroVector, -90.f, TEXT("家长"));
	SpawnProp(ENightPropId::NoticeA, W(1100.f, 1760.f, 150.f), FVector(18.f, 6.f, 28.f), Paper, FText::FromString(TEXT("读北廊第一张")), FVector::ZeroVector, -90.f, TEXT("一"));
	SpawnProp(ENightPropId::NoticeB, W(1450.f, 1760.f, 150.f), FVector(18.f, 6.f, 28.f), Paper, FText::FromString(TEXT("读北廊第二张")), FVector::ZeroVector, -90.f, TEXT("二"));
	SpawnProp(ENightPropId::NoticeC, W(1750.f, 1760.f, 150.f), FVector(18.f, 6.f, 28.f), Paper, FText::FromString(TEXT("读北廊第三张")), FVector::ZeroVector, -90.f, TEXT("三"));
	SpawnProp(ENightPropId::NoticeD, W(2050.f, 1760.f, 150.f), FVector(18.f, 6.f, 28.f), Paper, FText::FromString(TEXT("读北廊第四张")), FVector::ZeroVector, -90.f, TEXT("四"));
	SpawnProp(ENightPropId::PencilCase, W(-900.f, 1240.f, 110.f), FVector(16.f, 12.f, 8.f), FLinearColor(0.15f, 0.28f, 0.22f), FText::FromString(TEXT("打开写着分数的柜子")), FVector::ZeroVector, 0.f, TEXT("铅笔盒"));
	SpawnProp(ENightPropId::BusPass, W(2000.f, 1200.f, 110.f), FVector(16.f, 12.f, 3.f), Paper, FText::FromString(TEXT("拿走过期月票")), FVector::ZeroVector, 180.f, TEXT("月票"));
	SpawnProp(ENightPropId::Timetable, W(-80.f, -1200.f, 140.f), FVector(16.f, 6.f, 22.f), Paper, FText::FromString(TEXT("取下课表")), FVector::ZeroVector, 90.f, TEXT("课表"));
	SpawnProp(ENightPropId::ComputerDoor, W(1600.f, 1100.f, 145.f), FVector(8.f, 68.f, 100.f), FLinearColor(0.1f, 0.12f, 0.14f), FText::FromString(TEXT("进入微机室")), FVector(0.f, 200.f, 0.f), 180.f, TEXT("微机室"));
	SpawnProp(ENightPropId::ReturnTray, W(-80.f, 1480.f, 100.f), FVector(30.f, 22.f, 8.f), FLinearColor(0.2f, 0.22f, 0.2f), FText::FromString(TEXT("把三样东西放到托盘上")), FVector::ZeroVector, -90.f, TEXT("托盘"));
	SpawnProp(ENightPropId::EssayDorm, W(-960.f, 430.f, 70.f), FVector(18.f, 14.f, 3.f), Paper, FText::FromString(TEXT("捡起床下的作文")), FVector::ZeroVector, 0.f, TEXT("晚饭"));
	SpawnProp(ENightPropId::EssayShop, W(1850.f, 260.f, 100.f), FVector(18.f, 14.f, 3.f), Paper, FText::FromString(TEXT("捡起半页作文")), FVector::ZeroVector, -90.f, TEXT("同桌"));
	SpawnProp(ENightPropId::EssayGrade, W(1480.f, 960.f, 110.f), FVector(18.f, 14.f, 3.f), Paper, FText::FromString(TEXT("捡起没交的作文")), FVector::ZeroVector, 180.f, TEXT("镜头"));
	SpawnProp(ENightPropId::EssayBench, W(720.f, 1600.f, 90.f), FVector(40.f, 16.f, 16.f), Wood, FText::FromString(TEXT("把三篇作文放上长凳")), FVector::ZeroVector, -90.f, TEXT("长凳"));
	SpawnProp(ENightPropId::SodaShop, W(1550.f, 360.f, 100.f), FVector(10.f, 10.f, 18.f), FLinearColor(0.15f, 0.38f, 0.18f), FText::FromString(TEXT("再拿一瓶汽水")), FVector::ZeroVector, 180.f, TEXT("汽水"));
	SpawnProp(ENightPropId::LureShop, W(1750.f, 620.f, 88.f), FVector(28.f, 28.f, 12.f), Concrete, FText::FromString(TEXT("放下汽水")), FVector::ZeroVector, 180.f, TEXT("地砖"));
	SpawnProp(ENightPropId::LureNotice, W(1600.f, 1580.f, 88.f), FVector(28.f, 28.f, 12.f), Concrete, FText::FromString(TEXT("放下汽水")), FVector::ZeroVector, -90.f, TEXT("地砖"));
	SpawnProp(ENightPropId::SpareCell, W(180.f, 1280.f, 100.f), FVector(10.f, 8.f, 14.f), FLinearColor(0.2f, 0.32f, 0.18f), FText::FromString(TEXT("拿走备用电池")), FVector::ZeroVector, -90.f, TEXT("电池"));
	SpawnProp(ENightPropId::CallDoor, W(-1100.f, -1000.f, 145.f), FVector(8.f, 68.f, 100.f), DarkWood, FText::FromString(TEXT("进入通话间")), FVector(0.f, -200.f, 0.f), 0.f, TEXT("通话间"));
	SpawnProp(ENightPropId::CallA, W(-1900.f, -1200.f, 110.f), FVector(20.f, 14.f, 4.f), Paper, FText::FromString(TEXT("读清晨那通")), FVector::ZeroVector, 90.f, TEXT("清晨"));
	SpawnProp(ENightPropId::CallB, W(-1500.f, -900.f, 110.f), FVector(20.f, 14.f, 4.f), Paper, FText::FromString(TEXT("读锁屏那通")), FVector::ZeroVector, 0.f, TEXT("锁屏"));
	SpawnProp(ENightPropId::CallC, W(-1800.f, -750.f, 110.f), FVector(20.f, 14.f, 4.f), Paper, FText::FromString(TEXT("读镜头那通")), FVector::ZeroVector, -90.f, TEXT("镜头"));
	SpawnProp(ENightPropId::StampCall, W(-1600.f, -1100.f, 100.f), FVector(22.f, 16.f, 8.f), Wood, FText::FromString(TEXT("盖上第三章的章")), FVector::ZeroVector, 90.f, TEXT("章"));
	SpawnProp(ENightPropId::StampDuty, W(-980.f, -1000.f, 110.f), FVector(22.f, 16.f, 8.f), Wood, FText::FromString(TEXT("盖上第二章的章")), FVector::ZeroVector, 0.f, TEXT("章"));
	SpawnProp(ENightPropId::StampAnte, W(-240.f, -250.f, 110.f), FVector(28.f, 10.f, 24.f), Wood, FText::FromString(TEXT("把第一章的章印按上")), FVector::ZeroVector, 90.f, TEXT("木板"));
	SpawnProp(ENightPropId::StampDorm, W(-480.f, 560.f, 100.f), FVector(18.f, 14.f, 8.f), Wood, FText::FromString(TEXT("放下第五章的章印")), FVector::ZeroVector, -90.f, TEXT("章"));
	SpawnProp(ENightPropId::ClassLogA, W(500.f, 300.f, 120.f), FVector(18.f, 12.f, 4.f), Paper, FText::FromString(TEXT("读座位那页")), FVector::ZeroVector, 180.f, TEXT("座位"));
	SpawnProp(ENightPropId::ClassLogB, W(800.f, 500.f, 120.f), FVector(18.f, 12.f, 4.f), Paper, FText::FromString(TEXT("读奶茶那页")), FVector::ZeroVector, -90.f, TEXT("奶茶"));
	SpawnProp(ENightPropId::ClassLogC, W(1050.f, 350.f, 120.f), FVector(18.f, 12.f, 4.f), Paper, FText::FromString(TEXT("读实验那页")), FVector::ZeroVector, 180.f, TEXT("实验"));
	SpawnProp(ENightPropId::StampClass, W(800.f, 680.f, 90.f), FVector(20.f, 14.f, 8.f), Wood, FText::FromString(TEXT("盖上第四章的章")), FVector::ZeroVector, -90.f, TEXT("章"));
	SpawnProp(ENightPropId::StoreDoor, W(2000.f, 400.f, 145.f), FVector(8.f, 68.f, 100.f), DarkWood, FText::FromString(TEXT("进入后仓")), FVector(0.f, 200.f, 0.f), 180.f, TEXT("后仓"));
	SpawnProp(ENightPropId::ReceiptA, W(2200.f, 220.f, 100.f), FVector(16.f, 12.f, 3.f), Paper, FText::FromString(TEXT("读已付的收据")), FVector::ZeroVector, 180.f, TEXT("已付"));
	SpawnProp(ENightPropId::ReceiptB, W(2100.f, 520.f, 100.f), FVector(16.f, 12.f, 3.f), Paper, FText::FromString(TEXT("读缺席的收据")), FVector::ZeroVector, 180.f, TEXT("缺席"));
	SpawnProp(ENightPropId::ReceiptC, W(2240.f, 650.f, 100.f), FVector(16.f, 12.f, 3.f), Paper, FText::FromString(TEXT("读收据背面")), FVector::ZeroVector, -90.f, TEXT("背面"));
	SpawnProp(ENightPropId::DraftA, W(450.f, 1000.f, 120.f), FVector(18.f, 12.f, 4.f), Paper, FText::FromString(TEXT("读第一页批注")), FVector::ZeroVector, -90.f, TEXT("批注"));
	SpawnProp(ENightPropId::DraftB, W(500.f, 1280.f, 120.f), FVector(18.f, 12.f, 4.f), Paper, FText::FromString(TEXT("读第二页批注")), FVector::ZeroVector, 180.f, TEXT("点名"));
	SpawnProp(ENightPropId::DraftC, W(780.f, 1280.f, 120.f), FVector(18.f, 12.f, 4.f), Paper, FText::FromString(TEXT("读第三页批注")), FVector::ZeroVector, -90.f, TEXT("后悔"));
	SpawnProp(ENightPropId::AnnexDoor, W(1600.f, 1900.f, 145.f), FVector(68.f, 8.f, 100.f), DarkWood, FText::FromString(TEXT("进入长榜")), FVector(200.f, 0.f, 0.f), -90.f, TEXT("长榜"));
	SpawnProp(ENightPropId::NoticeE, W(1100.f, 2520.f, 150.f), FVector(18.f, 6.f, 28.f), Paper, FText::FromString(TEXT("读长榜第五张")), FVector::ZeroVector, -90.f, TEXT("五"));
	SpawnProp(ENightPropId::NoticeF, W(1400.f, 2520.f, 150.f), FVector(18.f, 6.f, 28.f), Paper, FText::FromString(TEXT("读长榜第六张")), FVector::ZeroVector, -90.f, TEXT("六"));
	SpawnProp(ENightPropId::NoticeG, W(1700.f, 2520.f, 150.f), FVector(18.f, 6.f, 28.f), Paper, FText::FromString(TEXT("读长榜第七张")), FVector::ZeroVector, -90.f, TEXT("七"));
	SpawnProp(ENightPropId::NoticeH, W(2000.f, 2520.f, 150.f), FVector(18.f, 6.f, 28.f), Paper, FText::FromString(TEXT("读长榜第八张")), FVector::ZeroVector, -90.f, TEXT("八"));
	SpawnProp(ENightPropId::LureAnnex, W(1600.f, 2250.f, 88.f), FVector(28.f, 28.f, 12.f), Concrete, FText::FromString(TEXT("放下汽水")), FVector::ZeroVector, -90.f, TEXT("地砖"));
	SpawnProp(ENightPropId::ChartA, W(-200.f, 1280.f, 100.f), FVector(18.f, 12.f, 4.f), Paper, FText::FromString(TEXT("读物品那页")), FVector::ZeroVector, 0.f, TEXT("物品"));
	SpawnProp(ENightPropId::ChartB, W(200.f, 1500.f, 100.f), FVector(18.f, 12.f, 4.f), Paper, FText::FromString(TEXT("读血压那页")), FVector::ZeroVector, 180.f, TEXT("血压"));
	SpawnProp(ENightPropId::ChartC, W(-180.f, 1520.f, 100.f), FVector(18.f, 12.f, 4.f), Paper, FText::FromString(TEXT("读门那页")), FVector::ZeroVector, 0.f, TEXT("门"));
	SpawnProp(ENightPropId::SelfFigure, W(620.f, 1720.f, 120.f), FVector(22.f, 16.f, 40.f), FLinearColor(0.12f, 0.18f, 0.32f), FText::FromString(TEXT("看清他")), FVector::ZeroVector, -90.f, TEXT("自己"));
	AddLabel(W(-700.f, -540.f, 210.f), 90.f, TEXT("值班室"), 16.f, FColor(220, 200, 160));
	AddLabel(W(-700.f, 760.f, 220.f), -90.f, TEXT("储物柜"), 16.f, FColor(220, 200, 160));
	AddLabel(W(-1500.f, -560.f, 210.f), 90.f, TEXT("通话间"), 16.f, FColor(220, 200, 160));
	AddLabel(W(2100.f, 140.f, 210.f), -90.f, TEXT("后仓"), 16.f, FColor(220, 200, 160));
	AddLabel(W(1200.f, 1960.f, 210.f), 90.f, TEXT("长榜"), 16.f, FColor(220, 200, 160));

	AddSprite(W(600.f, 1868.f, 180.f), -90.f, NightSchoolArt::MakeMirror(this), 56.f, 74.f);
	AddSprite(W(1000.f, 560.f, 150.f), -90.f, NightSchoolArt::MakeScoreSheet(this), 36.f, 46.f);
}

void ANightSchoolDirector::BuildAudio()
{
	DroneWave = NewObject<USoundWaveProcedural>(this);
	DroneWave->SetSampleRate(22050);
	DroneWave->NumChannels = 1;
	DroneWave->Duration = 100000.f;
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
	float Beat = HeartPhase;
	const float BeatStep = HeartRate / static_cast<float>(SampleRate);
	for (int32 Frame = 0; Frame < Frames; ++Frame)
	{
		const float Time = Cursor;
		Cursor += 1.f / static_cast<float>(SampleRate);
		if (Cursor > 64.f)
		{
			Cursor -= 64.f;
		}
		Beat += BeatStep;
		if (Beat >= 1.f)
		{
			Beat -= 1.f;
		}
		float Sample = 0.045f * FMath::Sin(2.f * PI * 55.f * Time);
		Sample += 0.03f * FMath::Sin(2.f * PI * 82.4f * Time);
		Sample += 0.012f * FMath::Sin(2.f * PI * 110.f * Time + 0.4f);
		if (Fear > 28.f)
		{
			const float Lub = FMath::Exp(-FMath::Square((Beat - 0.08f) * 28.f));
			const float Dub = 0.65f * FMath::Exp(-FMath::Square((Beat - 0.22f) * 34.f));
			Sample += (Lub + Dub) * FMath::Lerp(0.02f, 0.11f, Fear / 100.f);
		}
		Samples[Frame] = static_cast<int16>(FMath::Clamp(Sample, -1.f, 1.f) * 32000.f);
	}
	DronePhase = Cursor;
	HeartPhase = Beat;
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

	const FBox Laundry(W(-270.f, -1320.f, -20.f), W(250.f, -680.f, 420.f));
	TArray<FVector> LaundryPoints;
	LaundryPoints.Add(W(120.f, -1000.f, 128.f));
	LaundryPoints.Add(W(140.f, -860.f, 128.f));
	LaundryPoints.Add(W(-20.f, -780.f, 128.f));
	LaundryPoints.Add(W(160.f, -1240.f, 128.f));
	SpawnGhost(Laundry, LaundryPoints, TEXT("看"));

	const FBox Lockers(W(-1050.f, 860.f, -20.f), W(-360.f, 1360.f, 420.f));
	TArray<FVector> LockerPoints;
	LockerPoints.Add(W(-700.f, 1080.f, 128.f));
	LockerPoints.Add(W(-520.f, 1220.f, 128.f));
	LockerPoints.Add(W(-760.f, 1120.f, 128.f));
	LockerPoints.Add(W(-600.f, 980.f, 128.f));
	SpawnGhost(Lockers, LockerPoints, TEXT("空"));
}

void ANightSchoolDirector::SpawnShopGhost()
{
	if (bShopGhost)
	{
		return;
	}
	bShopGhost = true;
	const FBox Room(W(1280.f, 80.f, -20.f), W(1960.f, 740.f, 420.f));
	TArray<FVector> Points;
	Points.Add(W(1500.f, 480.f, 128.f));
	Points.Add(W(1760.f, 280.f, 128.f));
	Points.Add(W(1600.f, 600.f, 128.f));
	Points.Add(W(1400.f, 240.f, 128.f));
	SpawnGhost(Room, Points, TEXT("账"));
}

void ANightSchoolDirector::SpawnNoticeGhost()
{
	if (bNoticeGhost)
	{
		return;
	}
	bNoticeGhost = true;
	const FBox Room(W(1000.f, 1460.f, -20.f), W(2100.f, 1860.f, 420.f));
	TArray<FVector> Points;
	Points.Add(W(1200.f, 1600.f, 128.f));
	Points.Add(W(1500.f, 1610.f, 128.f));
	Points.Add(W(1850.f, 1600.f, 128.f));
	Points.Add(W(1700.f, 1640.f, 128.f));
	SpawnGhost(Room, Points, TEXT("榜"));
}

void ANightSchoolDirector::SpawnComputerGhost()
{
	if (bComputerGhost)
	{
		return;
	}
	bComputerGhost = true;
	const FBox Room(W(1720.f, 900.f, -20.f), W(2120.f, 1320.f, 420.f));
	TArray<FVector> Points;
	Points.Add(W(1900.f, 1100.f, 128.f));
	Points.Add(W(1980.f, 1000.f, 128.f));
	Points.Add(W(1860.f, 1200.f, 128.f));
	SpawnGhost(Room, Points, TEXT("分"));
}

void ANightSchoolDirector::SpawnAnnexGhost()
{
	if (bAnnexGhost)
	{
		return;
	}
	bAnnexGhost = true;
	const FBox Room(W(980.f, 1980.f, -20.f), W(2120.f, 2620.f, 420.f));
	TArray<FVector> Points;
	Points.Add(W(1200.f, 2200.f, 128.f));
	Points.Add(W(1700.f, 2300.f, 128.f));
	Points.Add(W(2000.f, 2150.f, 128.f));
	Points.Add(W(1400.f, 2400.f, 128.f));
	SpawnGhost(Room, Points, TEXT("名"));
}

int32 ANightSchoolDirector::CallCount() const
{
	return (bCallA ? 1 : 0) + (bCallB ? 1 : 0) + (bCallC ? 1 : 0);
}

int32 ANightSchoolDirector::ClassCount() const
{
	return (bClassA ? 1 : 0) + (bClassB ? 1 : 0) + (bClassC ? 1 : 0);
}

int32 ANightSchoolDirector::ReceiptCount() const
{
	return (bReceiptA ? 1 : 0) + (bReceiptB ? 1 : 0) + (bReceiptC ? 1 : 0);
}

int32 ANightSchoolDirector::DraftCount() const
{
	return (bDraftA ? 1 : 0) + (bDraftB ? 1 : 0) + (bDraftC ? 1 : 0);
}

int32 ANightSchoolDirector::ChartCount() const
{
	return (bChartA ? 1 : 0) + (bChartB ? 1 : 0) + (bChartC ? 1 : 0);
}

void ANightSchoolDirector::GrantSeals()
{
	if (bCameraCovered && !bSeal1Set)
	{
		bSeal1Held = true;
	}
	if (bSatAlone && !bSeal2Set)
	{
		bSeal2Held = true;
	}
	if (bLedgerPosted && !bSeal3Set)
	{
		bSeal3Held = true;
	}
	if (bTeacherRead && DraftCount() >= 3 && MinutesCount() >= 3 && !bSeal4Set)
	{
		bSeal4Held = true;
	}
	if (NoticeCount() >= 8 && !bSeal5Set)
	{
		bSeal5Held = true;
	}
}

void ANightSchoolDirector::TryOpenGrade()
{
	if (bGradeOpen || !bTeacherRead || DraftCount() < 3)
	{
		return;
	}
	bGradeOpen = true;
	if (ANightSchoolProp* Door = FindProp(ENightPropId::GradeDoor))
	{
		Door->OpenDoor();
	}
	SpawnNoticeGhost();
	ShowStatus(TEXT("年级组的门开了。北廊在它里面再往北。长榜还要再盖一枚章。"), 8.f);
}

void ANightSchoolDirector::BuildRooms()
{
	RoomViews.Reset();
	auto AddRoom = [this](const TCHAR* Id, float MinX, float MinY, float MaxX, float MaxY, const TCHAR* Name)
	{
		FNightRoomView Room;
		Room.Id = FName(Id);
		Room.Name = Name;
		Room.Bounds = FBox(W(MinX, MinY, -50.f), W(MaxX, MaxY, 400.f));
		RoomViews.Add(Room);
	};
	AddRoom(TEXT("Ante"), -1050.f, -550.f, 250.f, -50.f, TEXT("前厅"));
	AddRoom(TEXT("Dorm"), -1050.f, 50.f, -350.f, 750.f, TEXT("宿舍"));
	AddRoom(TEXT("Duty"), -1050.f, -1350.f, -400.f, -650.f, TEXT("值班室"));
	AddRoom(TEXT("Laundry"), -280.f, -1350.f, 250.f, -650.f, TEXT("洗衣间"));
	AddRoom(TEXT("Call"), -2150.f, -1350.f, -1150.f, -650.f, TEXT("通话间"));
	AddRoom(TEXT("Locker"), -1050.f, 850.f, -350.f, 1350.f, TEXT("储物柜"));
	AddRoom(TEXT("Hall"), -250.f, 50.f, 250.f, 950.f, TEXT("走廊"));
	AddRoom(TEXT("Class"), 350.f, 50.f, 1150.f, 750.f, TEXT("教室"));
	AddRoom(TEXT("Shop"), 1250.f, 50.f, 1950.f, 750.f, TEXT("小卖部"));
	AddRoom(TEXT("Store"), 2060.f, 50.f, 2280.f, 750.f, TEXT("后仓"));
	AddRoom(TEXT("Office"), 350.f, 850.f, 850.f, 1350.f, TEXT("办公室"));
	AddRoom(TEXT("Grade"), 950.f, 850.f, 1550.f, 1350.f, TEXT("年级组"));
	AddRoom(TEXT("Computer"), 1680.f, 850.f, 2150.f, 1350.f, TEXT("微机室"));
	AddRoom(TEXT("Notice"), 980.f, 1480.f, 2150.f, 1850.f, TEXT("北廊"));
	AddRoom(TEXT("Annex"), 980.f, 1980.f, 2150.f, 2620.f, TEXT("长榜"));
	AddRoom(TEXT("Infirmary"), -250.f, 1050.f, 250.f, 1550.f, TEXT("校医室"));
	AddRoom(TEXT("Light"), 350.f, 1480.f, 850.f, 1850.f, TEXT("光"));
	AddRoom(TEXT("Closet"), -750.f, 1480.f, -350.f, 1850.f, TEXT("柜子"));
}

void ANightSchoolDirector::BuildSelfFigure()
{
	const FVector Feet = W(620.f, 1788.f, 40.f);
	const FLinearColor Skin(0.62f, 0.45f, 0.36f);
	const FLinearColor Hair(0.06f, 0.05f, 0.045f);
	const FLinearColor Jacket(0.12f, 0.18f, 0.32f);
	const FLinearColor Shirt(0.82f, 0.82f, 0.78f);
	const FLinearColor Trousers(0.10f, 0.11f, 0.13f);
	const FLinearColor Shoes(0.05f, 0.05f, 0.05f);
	const FLinearColor Glass(0.02f, 0.02f, 0.02f);
	const FLinearColor Pack(0.16f, 0.17f, 0.18f);
	const FLinearColor Book(0.55f, 0.16f, 0.12f);
	AddBox(Feet + FVector(0.f, 2.f, 3.f), FVector(22.f, 30.f, 6.f), Shoes, false, false);
	AddBox(Feet + FVector(0.f, 0.f, 46.f), FVector(24.f, 18.f, 80.f), Trousers, false, false);
	AddBox(Feet + FVector(0.f, 0.f, 108.f), FVector(38.f, 20.f, 52.f), Jacket, false, false);
	AddBox(Feet + FVector(0.f, 0.f, 137.f), FVector(10.f, 10.f, 10.f), Skin, false, false);
	AddBox(Feet + FVector(0.f, -9.f, 128.f), FVector(16.f, 4.f, 10.f), Shirt, false, false);
	AddBox(Feet + FVector(-26.f, 0.f, 108.f), FVector(10.f, 10.f, 46.f), Jacket, false, false);
	AddBox(Feet + FVector(26.f, 0.f, 108.f), FVector(10.f, 10.f, 46.f), Jacket, false, false);
	AddBox(Feet + FVector(-26.f, -2.f, 82.f), FVector(8.f, 8.f, 8.f), Skin, false, false);
	AddBox(Feet + FVector(26.f, -6.f, 84.f), FVector(8.f, 10.f, 8.f), Skin, false, false);
	AddSphere(Feet + FVector(0.f, 0.f, 152.f), 12.f, Skin, false);
	AddSphere(Feet + FVector(0.f, 2.f, 158.f), 12.5f, Hair, false);
	AddSphere(Feet + FVector(-4.5f, -10.f, 154.f), 1.4f, FLinearColor(0.08f, 0.07f, 0.06f), false);
	AddSphere(Feet + FVector(4.5f, -10.f, 154.f), 1.4f, FLinearColor(0.08f, 0.07f, 0.06f), false);
	AddBox(Feet + FVector(0.f, -12.f, 154.f), FVector(13.f, 1.2f, 3.5f), Glass, false, false);
	AddBox(Feet + FVector(0.f, 16.f, 112.f), FVector(28.f, 12.f, 36.f), Pack, false, false);
	AddBox(Feet + FVector(26.f, -12.f, 86.f), FVector(10.f, 14.f, 2.f), Book, false, false);
}

void ANightSchoolDirector::TickPreview(float DeltaSeconds)
{
	if (!bPreviewing)
	{
		return;
	}
	ANightSchoolCharacter* Current = Player();
	if (Current)
	{
		Current->SetPreviewing(true);
	}
	if (!bPreviewReturning)
	{
		PreviewTime += DeltaSeconds;
		if (PreviewPoints.Num() >= 2 && PreviewCamera)
		{
			const float Span = PreviewDuration / static_cast<float>(PreviewPoints.Num() - 1);
			const int32 Last = PreviewPoints.Num() - 2;
			const int32 Segment = FMath::Clamp(FMath::FloorToInt(PreviewTime / Span), 0, Last);
			const float Alpha = FMath::Clamp((PreviewTime - Segment * Span) / Span, 0.f, 1.f);
			const FVector Loc = FMath::Lerp(PreviewPoints[Segment], PreviewPoints[Segment + 1], Alpha);
			PreviewCamera->SetActorLocation(Loc);
			PreviewCamera->SetActorRotation((PreviewLook - Loc).Rotation());
		}
		if (PreviewTime >= PreviewDuration)
		{
			bPreviewReturning = true;
			PreviewReturnLeft = 0.55f;
			if (APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0))
			{
				Controller->SetViewTargetWithBlend(Current, 0.45f, VTBlend_Cubic, 0.f, false);
			}
		}
		return;
	}
	PreviewReturnLeft -= DeltaSeconds;
	if (PreviewReturnLeft <= 0.f)
	{
		bPreviewing = false;
		bPreviewReturning = false;
		PreviewHold = 0.35f;
		if (Current)
		{
			Current->SetPreviewing(false);
		}
	}
}

void ANightSchoolDirector::TryBeginPreview()
{
	if (bPreviewing || bNoteOpen || IsEnding() || PreviewHold > 0.f || ActivePuzzle >= 0)
	{
		return;
	}
	ANightSchoolCharacter* Current = Player();
	if (!Current)
	{
		return;
	}
	const FVector Loc = Current->GetActorLocation();
	int32 Best = INDEX_NONE;
	float BestArea = TNumericLimits<float>::Max();
	for (int32 Index = 0; Index < RoomViews.Num(); ++Index)
	{
		FNightRoomView& Room = RoomViews[Index];
		if (Room.bSeen || !Room.Bounds.IsInsideXY(Loc))
		{
			continue;
		}
		const FVector Extent = Room.Bounds.GetExtent();
		const float Area = Extent.X * Extent.Y;
		if (Area < BestArea)
		{
			BestArea = Area;
			Best = Index;
		}
	}
	if (Best != INDEX_NONE)
	{
		BeginRoomPreview(RoomViews[Best]);
	}
}

void ANightSchoolDirector::BeginRoomPreview(FNightRoomView& Room)
{
	Room.bSeen = true;
	UWorld* World = GetWorld();
	ANightSchoolCharacter* Current = Player();
	APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0);
	if (!World || !Current || !Controller)
	{
		return;
	}
	if (!PreviewCamera)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PreviewCamera = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), FTransform(Current->GetActorLocation()), Params);
		if (PreviewCamera)
		{
			PreviewCamera->Tags.Add(TEXT("NightSchool"));
			if (UCameraComponent* Lens = PreviewCamera->GetCameraComponent())
			{
				Lens->SetFieldOfView(88.f);
			}
		}
	}
	if (!PreviewCamera)
	{
		return;
	}
	const float EyeZ = NightSchool::Origin().Z + 190.f;
	const float Width = Room.Bounds.Max.X - Room.Bounds.Min.X;
	const float Depth = Room.Bounds.Max.Y - Room.Bounds.Min.Y;
	const float InsetX = FMath::Clamp(Width * 0.18f, 36.f, 90.f);
	const float InsetY = FMath::Clamp(Depth * 0.18f, 36.f, 90.f);
	const float X0 = Room.Bounds.Min.X + InsetX;
	const float X1 = Room.Bounds.Max.X - InsetX;
	const float Y0 = Room.Bounds.Min.Y + InsetY;
	const float Y1 = Room.Bounds.Max.Y - InsetY;
	PreviewPoints.Reset();
	PreviewPoints.Add(FVector(X0, Y0, EyeZ));
	PreviewPoints.Add(FVector(X1, Y0, EyeZ));
	PreviewPoints.Add(FVector(X1, Y1, EyeZ));
	PreviewPoints.Add(FVector(X0, Y1, EyeZ));
	PreviewPoints.Add(FVector(X0, Y0, EyeZ));
	const FVector Center = Room.Bounds.GetCenter();
	PreviewLook = FVector(Center.X, Center.Y, EyeZ - 50.f);
	PreviewTime = 0.f;
	PreviewDuration = 10.f;
	bPreviewReturning = false;
	bPreviewing = true;
	PreviewCaption = FString::Printf(TEXT("第一次走进%s。镜头把这间屋走一遍。按 Tab 跳过。"), *Room.Name);
	PreviewCamera->SetActorLocation(PreviewPoints[0]);
	PreviewCamera->SetActorRotation((PreviewLook - PreviewPoints[0]).Rotation());
	Controller->SetViewTargetWithBlend(PreviewCamera, 0.35f, VTBlend_Cubic, 0.f, false);
	Current->SetPreviewing(true);
}

void ANightSchoolDirector::SkipPreview()
{
	if (!bPreviewing || bPreviewReturning)
	{
		return;
	}
	PreviewTime = PreviewDuration;
}

bool ANightSchoolDirector::AnyGhostContains(const FVector& Point) const
{
	for (const TObjectPtr<ANightSchoolApparition>& Ghost : Apparitions)
	{
		if (Ghost && Ghost->ContainsXY(Point))
		{
			return true;
		}
	}
	return false;
}

int32 ANightSchoolDirector::DutyCount() const
{
	return (bDutyA ? 1 : 0) + (bDutyB ? 1 : 0) + (bDutyC ? 1 : 0);
}

int32 ANightSchoolDirector::NoteCount() const
{
	return (bNoteSeat ? 1 : 0) + (bNoteHall ? 1 : 0) + (bNoteShop ? 1 : 0) + (bNoteLocker ? 1 : 0);
}

int32 ANightSchoolDirector::MinutesCount() const
{
	return (bMinSun ? 1 : 0) + (bMinLiang ? 1 : 0) + (bMinParent ? 1 : 0);
}

int32 ANightSchoolDirector::NoticeCount() const
{
	return (bNoticeA ? 1 : 0) + (bNoticeB ? 1 : 0) + (bNoticeC ? 1 : 0) + (bNoticeD ? 1 : 0)
		+ (bNoticeE ? 1 : 0) + (bNoticeF ? 1 : 0) + (bNoticeG ? 1 : 0) + (bNoticeH ? 1 : 0);
}

int32 ANightSchoolDirector::EssayCount() const
{
	return (bEssayDorm ? 1 : 0) + (bEssayShop ? 1 : 0) + (bEssayGrade ? 1 : 0);
}

bool ANightSchoolDirector::HasAllItems() const
{
	return bHasPencil && bHasBus && bHasTime;
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
	GrantSeals();
	EnsureOpeningCheckpoint();
	const int32 ChapterNow = GetChapterIndex();
	if (ChapterNow != ShownChapter)
	{
		ShownChapter = ChapterNow;
		ChapterBanner = NightSchoolStory::ChapterBanner(ChapterNow);
		ChapterBannerTime = 7.f;
	}
	ChapterBannerTime = FMath::Max(0.f, ChapterBannerTime - DeltaSeconds);
	PreviewHold = FMath::Max(0.f, PreviewHold - DeltaSeconds);
	TickPreview(DeltaSeconds);
	TryBeginPreview();
	TickPuzzle(DeltaSeconds);
	if (!IsEnding() && !bNoteOpen && !bPreviewing)
	{
		LureTime = FMath::Max(0.f, LureTime - DeltaSeconds);
	}
	if (ANightSchoolCharacter* Current = Player())
	{
		if (!IsEnding() && !bNoteOpen && !bPreviewing)
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

	if (DroneWave && DroneWave->GetAvailableAudioByteCount() < 22050 * static_cast<int32>(sizeof(int16)))
	{
		QueueDrone(0.5f);
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
	if (Ending == ENightEnding::InnerLight && bEssaysKept)
	{
		EndingBody += TEXT("\n\n");
		EndingBody += NightSchoolStory::EssayCoda();
	}
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
		return TEXT("西边是宿舍。先看那只每天都响的手机。每间屋有一座红印台，按 G 记下。解不开就从上一座再来。");
	}
	if (CallCount() < 3)
	{
		return TEXT("值班室西边是通话间。三页通话记录要读完。");
	}
	if (DutyCount() < 3)
	{
		return TEXT("南边值班室有老吴的三页记录。读完，再开那只抽屉。");
	}
	if (!bPuzzle1)
	{
		return TEXT("通话间墙上写着三个数字。把三环转到那三个数。按下去之后它们会走过来。八十秒，拨不完就退回印台。");
	}
	if (!bHasCloth && !bCameraCovered)
	{
		return TEXT("镜头布在值班室抽屉里。宿舍北墙那只眼睛还亮着。");
	}
	if (!bCameraCovered)
	{
		return TEXT("把布盖上宿舍的镜头。章印会回到你身上。");
	}
	if (!bSeal1Set)
	{
		return TEXT("把第一章的章印按到前厅木板上。走廊这时才开。");
	}
	if (NoteCount() < 4)
	{
		if (!bNoteShop && !bShopOpen)
		{
			return TEXT("江晚的条子在空位旁、走廊和储物柜。第四张在小卖部，宿舍铁盒里的硬币能把卷帘送上去。");
		}
		return TEXT("江晚的条子还没齐：空位旁、走廊、小卖部柜台、储物柜。");
	}
	if (ClassCount() < 3)
	{
		return TEXT("教室里还有三页班级日志。看完再坐那张空位。");
	}
	if (!bPuzzle2)
	{
		return TEXT("教室墙上有三个数字。空位上的人还在走。八十秒。");
	}
	if (!bSatAlone)
	{
		return TEXT("条子和日志都齐了。在那张没有人肯同桌的空位上坐下。");
	}
	if (!bSeal2Set)
	{
		return TEXT("把第二章的章印盖到值班室。然后才能钉账。");
	}
	if (!bShopOpen)
	{
		return TEXT("宿舍铁盒里的硬币能打开东边小卖部。那不是请客。");
	}
	if (ReceiptCount() < 3)
	{
		return TEXT("卷帘后面还有后仓。三张收据看完，账本才肯被拿走。");
	}
	if (!bPuzzle3)
	{
		return TEXT("小卖部墙上有三个数字。账还在货架之间。八十秒。");
	}
	if (!bLedgerPosted)
	{
		if (!bLedgerHeld)
		{
			return TEXT("柜台下的账本现在可以拿。");
		}
		return TEXT("把账本钉到走廊的布告栏上。");
	}
	if (!bSeal3Set)
	{
		return TEXT("把第三章的章印盖进西边通话间。办公室才收卷子。");
	}
	if (!bHasExam && !bExamGiven)
	{
		return TEXT("旧试卷在教室那张桌子上。");
	}
	if (!bExamGiven || !bOfficeOpen)
	{
		return TEXT("空位、账本、章印都齐了。把试卷交到办公室门口。藏进抽屉不算。");
	}
	if (DraftCount() < 3)
	{
		return TEXT("办公室里有三页改卷批注。看完，年级组才开。");
	}
	if (!bPuzzle4)
	{
		return TEXT("办公室墙上有三个数字。这间屋会进来人。八十秒。");
	}
	if (!bTeacherRead)
	{
		return TEXT("读梁老师留在桌上的那张字条。");
	}
	if (MinutesCount() < 3)
	{
		return TEXT("年级组里有三页会议记录。");
	}
	if (!bSeal4Set)
	{
		return TEXT("把第四章的章印盖回教室那张空位。长榜才会开。");
	}
	if (NoticeCount() < 8)
	{
		return TEXT("北廊四张，再往北的长榜还有四张。八张都要看。");
	}
	if (!bPuzzle5)
	{
		return TEXT("长榜墙上有三个数字。榜上的人还在走。八十秒。");
	}
	if (!bSeal5Set)
	{
		return TEXT("把第五章的章印送回宿舍那只已经盖住的镜头旁边。");
	}
	if (!bInfirmaryOpen)
	{
		return TEXT("回走廊，往北进校医室。");
	}
	if (ChartCount() < 3)
	{
		return TEXT("校医室有三页旧病历。先看完，再叫周先生。");
	}
	if (!bZhouIntro)
	{
		return TEXT("去听周先生说话。两扇门他还不会一次说完。");
	}
	if (!bItemsReturned)
	{
		return TEXT("铅笔盒在储物柜，月票在微机室，课表在南边洗衣间。三样放到托盘上。");
	}
	if (!bPuzzle6)
	{
		return TEXT("校医室墙上有三个数字。柜开着的时候有人进来。八十秒。");
	}
	if (!bZhouHeard)
	{
		return TEXT("再去听周先生把两扇门说完。");
	}
	if (bWearingEarplugs)
	{
		return TEXT("光的开关要先回宿舍拿下耳塞。");
	}
	if (!bSawSelf)
	{
		return TEXT("光里站着一个人。先看清他。然后拨开关，或者去西边的柜子。");
	}
	return TEXT("用手拨开关离开，或从柜子里面把门带上。三篇作文可以放在长凳上，不放也能走。");
}

int32 ANightSchoolDirector::GetChapterIndex() const
{
	if (!bSeal1Set)
	{
		return 1;
	}
	if (!bSatAlone || !bSeal2Set)
	{
		return 2;
	}
	if (!bLedgerPosted || !bSeal3Set)
	{
		return 3;
	}
	if (!(bTeacherRead && DraftCount() >= 3 && MinutesCount() >= 3 && bSeal4Set))
	{
		return 4;
	}
	if (!(NoticeCount() >= 8 && bSeal5Set))
	{
		return 5;
	}
	return 6;
}

FString ANightSchoolDirector::GetChapterLine() const
{
	FString Line;
	switch (GetChapterIndex())
	{
	case 1: Line = TEXT("第一章  未接"); break;
	case 2: Line = TEXT("第二章  空位"); break;
	case 3: Line = TEXT("第三章  请客"); break;
	case 4: Line = TEXT("第四章  恨铁"); break;
	case 5: Line = TEXT("第五章  榜"); break;
	default: Line = TEXT("第六章  两扇门"); break;
	}
	if (ChapterBannerTime > 0.f && !ChapterBanner.IsEmpty())
	{
		Line += TEXT("\n");
		Line += ChapterBanner;
	}
	return Line;
}

FString ANightSchoolDirector::GetPreviewLine() const
{
	return bPreviewing ? PreviewCaption : FString();
}

FString ANightSchoolDirector::GetPuzzleLine() const
{
	if (ActivePuzzle < 0)
	{
		return FString();
	}
	return FString::Printf(TEXT("还剩 %d 秒。人在往这边走。拨不完就退回印台。"), FMath::CeilToInt(PuzzleTime));
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
	if (bCoin) Append(TEXT("硬币"));
	if (bLedgerHeld) Append(TEXT("账本"));
	if (bHasPencil) Append(TEXT("笔袋"));
	if (bHasBus) Append(TEXT("月票"));
	if (bHasTime) Append(TEXT("课表"));
	if (!bEssaysKept && EssayCount() > 0) Append(TEXT("作文"));
	if (bSeal1Held || bSeal2Held || bSeal3Held || bSeal4Held || bSeal5Held) Append(TEXT("章印"));
	return Line.IsEmpty() ? TEXT("空的") : Line;
}

FString ANightSchoolDirector::GetThreadsLine() const
{
	const int32 Items = bItemsReturned ? 3 : ((bHasPencil ? 1 : 0) + (bHasBus ? 1 : 0) + (bHasTime ? 1 : 0));
	FString Line = FString::Printf(
		TEXT("通话 %d/3  值班 %d/3  条子 %d/4  日志 %d/3  收据 %d/3  批注 %d/3  会议 %d/3  告示 %d/8  病历 %d/3  旧物 %d/3  作文 %d/3"),
		CallCount(),
		DutyCount(),
		NoteCount(),
		ClassCount(),
		ReceiptCount(),
		DraftCount(),
		MinutesCount(),
		NoticeCount(),
		ChartCount(),
		Items,
		EssayCount());
	const int32 Rings = (bPuzzle1 ? 1 : 0) + (bPuzzle2 ? 1 : 0) + (bPuzzle3 ? 1 : 0) + (bPuzzle4 ? 1 : 0) + (bPuzzle5 ? 1 : 0) + (bPuzzle6 ? 1 : 0);
	Line += FString::Printf(TEXT("  环 %d/6"), Rings);
	if (bCoin && !bCoinSpent)
	{
		Line += TEXT("  ·硬币");
	}
	if (bShopOpen)
	{
		Line += TEXT("  ·小卖部");
	}
	if (bLedgerPosted)
	{
		Line += TEXT("  ·账已钉");
	}
	else if (bLedgerHeld)
	{
		Line += TEXT("  ·账本");
	}
	if (bGradeOpen)
	{
		Line += TEXT("  ·年级组");
	}
	if (bComputerOpen)
	{
		Line += TEXT("  ·微机室");
	}
	if (bZhouIntro && !bZhouHeard)
	{
		Line += TEXT("  ·还没说完");
	}
	if (bSpareTaken)
	{
		Line += TEXT("  ·电池");
	}
	if (bEssaysKept)
	{
		Line += TEXT("  ·长凳");
	}
	if (bShopSodaTaken)
	{
		Line += TEXT("  ·第二瓶");
	}
	if (bStoreOpen)
	{
		Line += TEXT("  ·后仓");
	}
	if (bAnnexOpen)
	{
		Line += TEXT("  ·长榜");
	}
	if (bSawSelf)
	{
		Line += TEXT("  ·看见了");
	}
	if (bSeal1Set && bSeal2Set && bSeal3Set && bSeal4Set && bSeal5Set)
	{
		Line += TEXT("  ·五章已盖");
	}
	const int32 Awake = (bShopGhost ? 1 : 0) + (bNoticeGhost ? 1 : 0) + (bComputerGhost ? 1 : 0) + (bAnnexGhost ? 1 : 0);
	if (Awake > 0 && Line.IsEmpty())
	{
		Line = TEXT("还有人在屋里走");
	}
	return Line;
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
		if (!bPuzzle1 && bPhoneRead && DutyCount() >= 3 && CallCount() >= 3)
		{
			ShowNote(TEXT("抽屉"), TEXT("三环还没对上。数字写在通话间的墙上。八十秒，它们会走过来。拨不完就退回印台。"));
			break;
		}
		if (!bPhoneRead || DutyCount() < 3 || CallCount() < 3)
		{
			ShowNote(TEXT("抽屉"), NightSchoolStory::ClothEarly());
			break;
		}
		bHasCloth = true;
		Prop->Consume();
		ShowNote(NightSchoolStory::ClothTitle(), NightSchoolStory::ClothBody());
		break;
	case ENightPropId::DormCamera:
		if (!bHasCloth)
		{
			ShowNote(TEXT("红点"), TEXT("红点对着床。镜头布在南边值班室的抽屉里，如果还没拿。布只能盖这一只。"));
			break;
		}
		bHasCloth = false;
		bCameraCovered = true;
		bSeal1Held = true;
		SpawnHallGhosts();
		Prop->Consume();
		ShowNote(TEXT("红点"), NightSchoolStory::CameraBody());
		ShowStatus(TEXT("章印在你身上。按到前厅的木板上，走廊才开。"), 6.f);
		break;
	case ENightPropId::HallDoor:
		if (!bSeal1Set)
		{
			ShowNote(TEXT("走廊"), TEXT("走廊从里面顶着。镜头盖上以后，章印要先按在前厅的木板上。"));
			break;
		}
		bHallOpen = true;
		Prop->OpenDoor();
		break;
	case ENightPropId::Soda:
		bHasSoda = true;
		Prop->Consume();
		ShowNote(TEXT("汽水"), TEXT("一瓶没开封的汽水，瓶壁是凉的。放在走廊、教室、小卖部、北廊或长榜的地砖上，气泡会响大约十八秒。谁听见谁就走过去。它打不碎谁，也伤不了你。"));
		break;
	case ENightPropId::LureHall:
	case ENightPropId::LureClass:
	case ENightPropId::LureShop:
	case ENightPropId::LureNotice:
	case ENightPropId::LureAnnex:
		if (!bHasSoda)
		{
			if (bShopOpen && !bShopSodaTaken)
			{
				ShowNote(TEXT("地砖"), TEXT("汽水不在你手里。宿舍东墙的架子上有一瓶。小卖部柜台边还有一瓶，如果还没拿。它只能响一会儿，打不了人。"));
			}
			else
			{
				ShowNote(TEXT("地砖"), NightSchoolStory::SodaNeed());
			}
			break;
		}
		if (!bGhostsSpawned)
		{
			ShowNote(TEXT("地砖"), NightSchoolStory::SodaEarly());
			break;
		}
		{
			const FVector Here = Prop->GetActorLocation();
			if (!AnyGhostContains(Here))
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
		if (NoteCount() >= 4 && ClassCount() >= 3 && !bPuzzle2)
		{
			ShowNote(TEXT("空位"), TEXT("教室墙上的三个数字还没拨完。人还在教室里走。八十秒。"));
			break;
		}
		if (NoteCount() < 4 || ClassCount() < 3)
		{
			ShowNote(TEXT("空位"), NightSchoolStory::DeskEarly());
			break;
		}
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
		if (!bSatAlone || !bLedgerPosted || !bSeal3Set || (!bHasExam && !bExamGiven))
		{
			ShowNote(TEXT("办公室"), NightSchoolStory::LockedOffice());
			break;
		}
		if (!bExamGiven)
		{
			bHasExam = false;
			bExamGiven = true;
			ShowStatus(TEXT("你把卷子放在门上。门开了。分数没有变。"), 4.5f);
		}
		bOfficeOpen = true;
		Prop->OpenDoor();
		break;
	case ENightPropId::TeacherNote:
		if (DraftCount() >= 3 && !bPuzzle4)
		{
			ShowNote(TEXT("字条"), TEXT("墙上的三个数字还没拨完。这间屋会进来人。八十秒。"));
			break;
		}
		if (DraftCount() < 3)
		{
			ShowNote(TEXT("字条"), NightSchoolStory::NeedDrafts());
			break;
		}
		bTeacherRead = true;
		ShowNote(NightSchoolStory::TeacherTitle(), NightSchoolStory::TeacherBody());
		TryOpenGrade();
		break;
	case ENightPropId::InfirmaryDoor:
		if (bTeacherRead && MinutesCount() >= 3 && NoticeCount() >= 8 && bSeal5Set && !bPuzzle5)
		{
			ShowNote(TEXT("校医室"), TEXT("长榜墙上的三个数字还没拨完。榜上的人还在走。八十秒。"));
			break;
		}
		if (!bTeacherRead || MinutesCount() < 3 || NoticeCount() < 8 || !bSeal5Set)
		{
			ShowNote(TEXT("校医室"), NightSchoolStory::LockedInfirmary());
			break;
		}
		bInfirmaryOpen = true;
		Prop->OpenDoor();
		break;
	case ENightPropId::Zhou:
		if (ChartCount() < 3)
		{
			ShowNote(TEXT("周先生"), NightSchoolStory::ChartBodyC());
			break;
		}
		if (!bItemsReturned)
		{
			if (!bZhouIntro)
			{
				bZhouIntro = true;
				bComputerOpen = true;
				if (ANightSchoolProp* Door = FindProp(ENightPropId::ComputerDoor))
				{
					Door->OpenDoor();
				}
				SpawnComputerGhost();
				Prop->SetPrompt(FText::FromString(TEXT("三样东西还没放上托盘")));
			}
			ShowNote(NightSchoolStory::ZhouIntroTitle(), NightSchoolStory::ZhouIntroBody());
			break;
		}
		if (!bPuzzle6)
		{
			ShowNote(TEXT("药柜"), TEXT("墙上的三个数字还没拨完。八十秒。人会在这间屋里走。拨不完就退回印台。"));
			break;
		}
		bZhouHeard = true;
		Prop->SetPrompt(FText::FromString(TEXT("周先生已经说完")));
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
		if (!bSawSelf)
		{
			ShowNote(TEXT("开关"), NightSchoolStory::NeedSelf());
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
	case ENightPropId::DutyLogA:
		bDutyA = true;
		ShowNote(NightSchoolStory::DutyTitleA(), NightSchoolStory::DutyBodyA());
		break;
	case ENightPropId::DutyLogB:
		bDutyB = true;
		ShowNote(NightSchoolStory::DutyTitleB(), NightSchoolStory::DutyBodyB());
		break;
	case ENightPropId::DutyLogC:
		bDutyC = true;
		ShowNote(NightSchoolStory::DutyTitleC(), NightSchoolStory::DutyBodyC());
		break;
	case ENightPropId::Coin:
		bCoin = true;
		Prop->Consume();
		ShowNote(NightSchoolStory::CoinTitle(), NightSchoolStory::CoinBody());
		break;
	case ENightPropId::NoteSeat:
		bNoteSeat = true;
		ShowNote(NightSchoolStory::NoteSeatTitle(), NightSchoolStory::NoteSeatBody());
		break;
	case ENightPropId::NoteHall:
		bNoteHall = true;
		ShowNote(NightSchoolStory::NoteHallTitle(), NightSchoolStory::NoteHallBody());
		break;
	case ENightPropId::NoteShop:
		bNoteShop = true;
		ShowNote(NightSchoolStory::NoteShopTitle(), NightSchoolStory::NoteShopBody());
		break;
	case ENightPropId::NoteLocker:
		bNoteLocker = true;
		ShowNote(NightSchoolStory::NoteLockerTitle(), NightSchoolStory::NoteLockerBody());
		break;
	case ENightPropId::ShopDoor:
		if (!bCoin && !bShopOpen)
		{
			ShowNote(TEXT("小卖部"), NightSchoolStory::ShopLocked());
			break;
		}
		bCoin = false;
		bCoinSpent = true;
		bShopOpen = true;
		Prop->OpenDoor();
		SpawnShopGhost();
		ShowStatus(TEXT("卷帘上去了。这不是请客。账在柜台底下。"), 4.5f);
		break;
	case ENightPropId::Ledger:
		if (ReceiptCount() >= 3 && !bPuzzle3)
		{
			ShowNote(TEXT("账本"), TEXT("小卖部墙上的三个数字还没拨完。账还在货架之间。八十秒。"));
			break;
		}
		if (ReceiptCount() < 3)
		{
			ShowNote(TEXT("账本"), NightSchoolStory::NeedReceipts());
			break;
		}
		bLedgerHeld = true;
		Prop->Consume();
		ShowNote(NightSchoolStory::LedgerTitle(), NightSchoolStory::LedgerBody());
		break;
	case ENightPropId::LedgerBoard:
		if (!bSeal2Set)
		{
			ShowStatus(TEXT("先把第二章的章印盖到值班室，再来钉这本账。"), 3.6f);
			break;
		}
		if (!bLedgerHeld && !bLedgerPosted)
		{
			ShowStatus(TEXT("布告栏是空的。账本还在小卖部柜台底下。"), 3.6f);
			break;
		}
		if (!bLedgerPosted)
		{
			bLedgerPosted = true;
			bLedgerHeld = false;
		}
		ShowNote(TEXT("布告"), NightSchoolStory::LedgerPosted());
		break;
	case ENightPropId::GradeDoor:
		if (!bTeacherRead)
		{
			ShowNote(TEXT("年级组"), NightSchoolStory::GradeLocked());
			break;
		}
		bGradeOpen = true;
		Prop->OpenDoor();
		SpawnNoticeGhost();
		break;
	case ENightPropId::MinutesSun:
		bMinSun = true;
		ShowNote(NightSchoolStory::MinutesSunTitle(), NightSchoolStory::MinutesSunBody());
		break;
	case ENightPropId::MinutesLiang:
		bMinLiang = true;
		ShowNote(NightSchoolStory::MinutesLiangTitle(), NightSchoolStory::MinutesLiangBody());
		break;
	case ENightPropId::MinutesParent:
		bMinParent = true;
		ShowNote(NightSchoolStory::MinutesParentTitle(), NightSchoolStory::MinutesParentBody());
		break;
	case ENightPropId::NoticeA:
		bNoticeA = true;
		ShowNote(NightSchoolStory::NoticeTitleA(), NightSchoolStory::NoticeBodyA());
		break;
	case ENightPropId::NoticeB:
		bNoticeB = true;
		ShowNote(NightSchoolStory::NoticeTitleB(), NightSchoolStory::NoticeBodyB());
		break;
	case ENightPropId::NoticeC:
		bNoticeC = true;
		ShowNote(NightSchoolStory::NoticeTitleC(), NightSchoolStory::NoticeBodyC());
		break;
	case ENightPropId::NoticeD:
		bNoticeD = true;
		ShowNote(NightSchoolStory::NoticeTitleD(), NightSchoolStory::NoticeBodyD());
		break;
	case ENightPropId::PencilCase:
		if (!bHasExam && !bExamGiven)
		{
			ShowNote(TEXT("储物柜"), NightSchoolStory::LockerHint());
			break;
		}
		bHasPencil = true;
		Prop->Consume();
		ShowNote(NightSchoolStory::PencilTitle(), NightSchoolStory::PencilBody());
		break;
	case ENightPropId::BusPass:
		bHasBus = true;
		Prop->Consume();
		ShowNote(NightSchoolStory::BusTitle(), NightSchoolStory::BusBody());
		break;
	case ENightPropId::Timetable:
		bHasTime = true;
		Prop->Consume();
		ShowNote(NightSchoolStory::TimeTitle(), NightSchoolStory::TimeBody());
		break;
	case ENightPropId::ComputerDoor:
		if (!bZhouIntro)
		{
			ShowNote(TEXT("微机室"), NightSchoolStory::ComputerLocked());
			break;
		}
		bComputerOpen = true;
		Prop->OpenDoor();
		SpawnComputerGhost();
		break;
	case ENightPropId::ReturnTray:
		if (bItemsReturned)
		{
			ShowNote(TEXT("托盘"), NightSchoolStory::TrayDone());
			break;
		}
		if (!HasAllItems())
		{
			ShowNote(TEXT("托盘"), NightSchoolStory::TrayMissing());
			break;
		}
		bItemsReturned = true;
		bHasPencil = false;
		bHasBus = false;
		bHasTime = false;
		if (ANightSchoolProp* ZhouProp = FindProp(ENightPropId::Zhou))
		{
			ZhouProp->SetPrompt(FText::FromString(TEXT("听周先生把两扇门说完")));
		}
		ShowNote(TEXT("托盘"), NightSchoolStory::TrayDone());
		break;
	case ENightPropId::EssayDorm:
		bEssayDorm = true;
		Prop->Consume();
		ShowNote(NightSchoolStory::EssayTitleA(), NightSchoolStory::EssayBodyA());
		break;
	case ENightPropId::EssayShop:
		bEssayShop = true;
		Prop->Consume();
		ShowNote(NightSchoolStory::EssayTitleB(), NightSchoolStory::EssayBodyB());
		break;
	case ENightPropId::EssayGrade:
		bEssayGrade = true;
		Prop->Consume();
		ShowNote(NightSchoolStory::EssayTitleC(), NightSchoolStory::EssayBodyC());
		break;
	case ENightPropId::EssayBench:
		if (EssayCount() < 3)
		{
			ShowNote(TEXT("长凳"), NightSchoolStory::EssayBenchBody());
			break;
		}
		bEssaysKept = true;
		ShowNote(TEXT("长凳"), TEXT("三篇都在长凳上了。它们会跟你走到门口，如果那扇门是你选的。不放也行。开关不认作文。"));
		break;
	case ENightPropId::SodaShop:
		if (bHasSoda)
		{
			ShowStatus(TEXT("手里已经有一瓶。先放下，再来拿这一瓶。"), 3.2f);
			break;
		}
		bShopSodaTaken = true;
		bHasSoda = true;
		Prop->Consume();
		ShowNote(TEXT("汽水"), TEXT("柜台边还有一瓶，瓶壁一样凉。地砖在走廊、教室、小卖部、北廊和长榜。十八秒，谁听见谁走过去。打不了人。"));
		break;
	case ENightPropId::SpareCell:
		bSpareTaken = true;
		if (ANightSchoolCharacter* Current = Player())
		{
			Current->RefillBattery(100.f);
		}
		Prop->Consume();
		ShowStatus(TEXT("备用电池装上了。那点光又能亮一阵。它不是离开这间屋的光。"), 4.2f);
		break;
	case ENightPropId::CallA:
		bCallA = true;
		ShowNote(NightSchoolStory::CallTitleA(), NightSchoolStory::CallBodyA());
		break;
	case ENightPropId::CallB:
		bCallB = true;
		ShowNote(NightSchoolStory::CallTitleB(), NightSchoolStory::CallBodyB());
		break;
	case ENightPropId::CallC:
		bCallC = true;
		ShowNote(NightSchoolStory::CallTitleC(), NightSchoolStory::CallBodyC());
		break;
	case ENightPropId::CallDoor:
		Prop->OpenDoor();
		break;
	case ENightPropId::ClassLogA:
		bClassA = true;
		ShowNote(NightSchoolStory::ClassLogTitleA(), NightSchoolStory::ClassLogBodyA());
		break;
	case ENightPropId::ClassLogB:
		bClassB = true;
		ShowNote(NightSchoolStory::ClassLogTitleB(), NightSchoolStory::ClassLogBodyB());
		break;
	case ENightPropId::ClassLogC:
		bClassC = true;
		ShowNote(NightSchoolStory::ClassLogTitleC(), NightSchoolStory::ClassLogBodyC());
		break;
	case ENightPropId::ReceiptA:
		bReceiptA = true;
		ShowNote(NightSchoolStory::ReceiptTitleA(), NightSchoolStory::ReceiptBodyA());
		break;
	case ENightPropId::ReceiptB:
		bReceiptB = true;
		ShowNote(NightSchoolStory::ReceiptTitleB(), NightSchoolStory::ReceiptBodyB());
		break;
	case ENightPropId::ReceiptC:
		bReceiptC = true;
		ShowNote(NightSchoolStory::ReceiptTitleC(), NightSchoolStory::ReceiptBodyC());
		break;
	case ENightPropId::StoreDoor:
		if (!bShopOpen)
		{
			ShowNote(TEXT("后仓"), NightSchoolStory::StoreLocked());
			break;
		}
		bStoreOpen = true;
		Prop->OpenDoor();
		break;
	case ENightPropId::DraftA:
		bDraftA = true;
		ShowNote(NightSchoolStory::DraftTitleA(), NightSchoolStory::DraftBodyA());
		TryOpenGrade();
		break;
	case ENightPropId::DraftB:
		bDraftB = true;
		ShowNote(NightSchoolStory::DraftTitleB(), NightSchoolStory::DraftBodyB());
		TryOpenGrade();
		break;
	case ENightPropId::DraftC:
		bDraftC = true;
		ShowNote(NightSchoolStory::DraftTitleC(), NightSchoolStory::DraftBodyC());
		TryOpenGrade();
		break;
	case ENightPropId::NoticeE:
		bNoticeE = true;
		ShowNote(NightSchoolStory::NoticeTitleE(), NightSchoolStory::NoticeBodyE());
		break;
	case ENightPropId::NoticeF:
		bNoticeF = true;
		ShowNote(NightSchoolStory::NoticeTitleF(), NightSchoolStory::NoticeBodyF());
		break;
	case ENightPropId::NoticeG:
		bNoticeG = true;
		ShowNote(NightSchoolStory::NoticeTitleG(), NightSchoolStory::NoticeBodyG());
		break;
	case ENightPropId::NoticeH:
		bNoticeH = true;
		ShowNote(NightSchoolStory::NoticeTitleH(), NightSchoolStory::NoticeBodyH());
		break;
	case ENightPropId::AnnexDoor:
		if (!bGradeOpen || !bSeal4Set)
		{
			ShowNote(TEXT("长榜"), NightSchoolStory::AnnexLocked());
			break;
		}
		bAnnexOpen = true;
		Prop->OpenDoor();
		SpawnAnnexGhost();
		break;
	case ENightPropId::ChartA:
		bChartA = true;
		ShowNote(NightSchoolStory::ChartTitleA(), NightSchoolStory::ChartBodyA());
		break;
	case ENightPropId::ChartB:
		bChartB = true;
		ShowNote(NightSchoolStory::ChartTitleB(), NightSchoolStory::ChartBodyB());
		break;
	case ENightPropId::ChartC:
		bChartC = true;
		ShowNote(NightSchoolStory::ChartTitleC(), NightSchoolStory::ChartBodyC());
		break;
	case ENightPropId::StampAnte:
		if (!bSeal1Held || bSeal1Set)
		{
			ShowNote(TEXT("木板"), bSeal1Set ? NightSchoolStory::StampDone1() : NightSchoolStory::StampIdle());
			break;
		}
		bSeal1Held = false;
		bSeal1Set = true;
		bHallOpen = true;
		if (ANightSchoolProp* Door = FindProp(ENightPropId::HallDoor))
		{
			Door->OpenDoor();
		}
		ShowNote(TEXT("第一章"), NightSchoolStory::StampDone1());
		break;
	case ENightPropId::StampDuty:
		if (!bSeal2Held || bSeal2Set)
		{
			ShowNote(TEXT("章"), bSeal2Set ? NightSchoolStory::StampDone2() : NightSchoolStory::StampIdle());
			break;
		}
		bSeal2Held = false;
		bSeal2Set = true;
		ShowNote(TEXT("第二章"), NightSchoolStory::StampDone2());
		break;
	case ENightPropId::StampCall:
		if (!bSeal3Held || bSeal3Set)
		{
			ShowNote(TEXT("章"), bSeal3Set ? NightSchoolStory::StampDone3() : NightSchoolStory::StampIdle());
			break;
		}
		bSeal3Held = false;
		bSeal3Set = true;
		ShowNote(TEXT("第三章"), NightSchoolStory::StampDone3());
		break;
	case ENightPropId::StampClass:
		if (!bSeal4Held || bSeal4Set)
		{
			ShowNote(TEXT("章"), bSeal4Set ? NightSchoolStory::StampDone4() : NightSchoolStory::StampIdle());
			break;
		}
		bSeal4Held = false;
		bSeal4Set = true;
		ShowNote(TEXT("第四章"), NightSchoolStory::StampDone4());
		break;
	case ENightPropId::StampDorm:
		if (!bSeal5Held || bSeal5Set)
		{
			ShowNote(TEXT("章"), bSeal5Set ? NightSchoolStory::StampDone5() : NightSchoolStory::StampIdle());
			break;
		}
		bSeal5Held = false;
		bSeal5Set = true;
		ShowNote(TEXT("第五章"), NightSchoolStory::StampDone5());
		break;
	case ENightPropId::SelfFigure:
		bSawSelf = true;
		ShowNote(NightSchoolStory::SelfTitle(), NightSchoolStory::SelfBody());
		break;
	case ENightPropId::InkPad:
		UseInkPad(Prop);
		break;
	case ENightPropId::Dial:
		UseDial(Prop);
		break;
	case ENightPropId::DialConfirm:
		UseDialConfirm(Prop);
		break;
	default:
		break;
	}
}
