#include "NightSchool/NightSchoolDirector.h"
#include "Sound/SoundGroups.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/LightComponent.h"

#include "Animation/AnimationAsset.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/AudioComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
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
#include "Engine/SkeletalMesh.h"
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
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundWaveProcedural.h"
#include "Components/SpotLightComponent.h"
#include "Student.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const FLinearColor Plaster(0.16f, 0.145f, 0.125f);
	const FLinearColor Wood(0.20f, 0.11f, 0.055f);
	const FLinearColor DarkWood(0.10f, 0.055f, 0.035f);
	const FLinearColor Concrete(0.055f, 0.055f, 0.058f);
	const FLinearColor Night(0.012f, 0.015f, 0.028f);
	const FLinearColor Paper(0.42f, 0.36f, 0.26f);
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

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlasterAsset(TEXT("/Game/NightSchool/Atmosphere/M_DampPlaster"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> FloorAsset(TEXT("/Game/NightSchool/Atmosphere/M_WornTerrazzo"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BodyAsset(TEXT("/Game/NightSchool/Characters/SM_XuZhixia_Night"));
	static ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> Wall(TEXT("/Game/NightSchool/Campus/M_SchoolWall"));
	static ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> SignIn(TEXT("/Game/NightSchool/Campus/Entry/M_DormSignIn"));
	static ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> Announcement(TEXT("/Game/NightSchool/Campus/Entry/M_DormAnnouncement"));
	static ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> EntranceSign(TEXT("/Game/NightSchool/Campus/Entry/M_DormEntranceSign"));
	static ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> CampusSky(TEXT("/Game/NightSchool/Campus/Entry/Outdoor/M_CampusBlackSky"));
	static ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> NightSky(TEXT("/Game/NightSchool/Campus/Entry/M_DormNightSky"));
	static ConstructorHelpers::FObjectFinderOptional<UStaticMesh> CourtyardTree(TEXT("/Game/NightSchool/Campus/Entry/Outdoor/SM_CampusBroadleaf/StaticMeshes/SM_CampusBroadleaf"));
	static ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> ExteriorTile(TEXT("/Game/NightSchool/Campus/Entry/M_DormExteriorTile"));
	static ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> Tube(TEXT("/Game/NightSchool/Campus/M_OldTube"));
	static ConstructorHelpers::FObjectFinderOptional<USoundWave> Air(TEXT("/Game/NightSchool/Campus/Audio/SW_CorridorAir"));
	static ConstructorHelpers::FObjectFinderOptional<USoundWave> Ballast(TEXT("/Game/NightSchool/Campus/Audio/SW_Ballast"));
	static ConstructorHelpers::FObjectFinderOptional<USoundWave> Knock(TEXT("/Game/NightSchool/Campus/Audio/SW_VentKnock"));
	static ConstructorHelpers::FObjectFinderOptional<USoundWave> Scrape(TEXT("/Game/NightSchool/Campus/Audio/SW_DistantDesk"));
	DampPlaster = Wall.Get() ? Wall.Get() : PlasterAsset.Object.Get();
	DormSignIn = SignIn.Get();
	DormAnnouncement = Announcement.Get();
	DormEntranceSign = EntranceSign.Get();
	DormNightSky = CampusSky.Get() ? CampusSky.Get() : NightSky.Get();
	DormCourtyardTree = CourtyardTree.Get();
	DormExteriorTile = ExteriorTile.Get() ? ExteriorTile.Get() : PlasterAsset.Object.Get();
	CampusTube = Tube.Get();
	CampusAir = Air.Get();
	CampusBallast = Ballast.Get();
	CampusKnock = Knock.Get();
	CampusScrape = Scrape.Get();
	WornTerrazzo = FloorAsset.Object;
	SelfModel = BodyAsset.Object;
	static ConstructorHelpers::FObjectFinderOptional<USkeletalMesh> Rig(TEXT("/Game/NightSchool/Characters/XuZhixia/SK_XuZhixia/SkeletalMeshes/SK_XuZhixia"));
	SelfRig = Rig.Get();
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

UStaticMeshComponent* ANightSchoolDirector::AddBox(const FVector& Center, const FVector& Size, const FLinearColor& Color, bool bShadow, bool bCollision, UMaterialInterface* Surface)
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
	if (Surface)
	{
		Mesh->SetMaterial(0, Surface);
	}
	else if (UMaterialInstanceDynamic* Mid = NightSchoolArt::MakeColor(this, Color))
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

void ANightSchoolDirector::AddLantern(float X, float Y, float Z, float Intensity, bool bCalm, bool bPracticalOnly, int32 Floor)
{
	const FVector At = NightSchool::AtFloor(X, Y, Z, Floor);
	auto Fixture = [Floor, X, Y](float OffsetX, float OffsetY, float Height)
	{
		return NightSchool::AtFloor(X + OffsetX, Y + OffsetY, Height, Floor);
	};
	const FLinearColor Housing(0.036f, 0.044f, 0.046f);
	const FLinearColor Tube(0.28f, 0.36f, 0.42f);
	AddBox(Fixture(0.f, 0.f, 302.f), FVector(106.f, 24.f, 8.f), Housing, true, false);
	for (float Offset : {-5.f, 5.f})
	{
		AddBox(Fixture(0.f, Offset, 296.5f), FVector(85.f, 3.6f, 3.6f), Tube, false, false, CampusTube);
	}
	AddBox(Fixture(-47.f, 0.f, 297.f), FVector(10.f, 20.f, 6.f), Rust, false, false);
	AddBox(Fixture(47.f, 0.f, 297.f), FVector(10.f, 20.f, 6.f), Rust, false, false);

	const float VisualIntensity = Intensity * (bCalm ? 0.17f : 0.29f);
	if (bCalm)
	{
		WarmLights.Add(At);
	}
	if (bPracticalOnly)
	{
		return;
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
		Component->SetIntensity(VisualIntensity);
		Component->SetAttenuationRadius(bCalm ? 265.f : 295.f);
		Component->SetLightColor(bCalm ? FLinearColor(0.65f, 0.76f, 0.81f) : FLinearColor(0.45f, 0.62f, 0.76f));
		Component->SetCastShadows(false);
	}
	FlickerLights.Add(Light);
	FlickerBase.Add(VisualIntensity);
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
	if (!Slide.IsNearlyZero() && FMath::Max(Extent.X, Extent.Y) == 68.f)
	{
		const bool bAlongX = Extent.X > Extent.Y;
		const FVector Along = bAlongX ? FVector::ForwardVector : FVector::RightVector;
		const FLinearColor Trim(.068f, .075f, .072f);
		for (float Side : {-1.f, 1.f})
		{
			AddBox(Location + Along * (Side * 72.f) + FVector(0.f, 0.f, 1.f),
				bAlongX ? FVector(9.f, 30.f, 208.f) : FVector(30.f, 9.f, 208.f), Trim, true, true);
		}
		AddBox(Location + FVector(0.f, 0.f, 104.f),
			bAlongX ? FVector(151.f, 30.f, 8.f) : FVector(30.f, 151.f, 8.f), Trim, true, true);
		AddBox(Location - FVector(0.f, 0.f, 102.5f),
			bAlongX ? FVector(151.f, 28.f, 5.f) : FVector(28.f, 151.f, 5.f), Trim, true, true);
		if (!Label.IsEmpty())
		{
			const FVector Face = FRotator(0.f, LabelYaw, 0.f).Vector() * 15.f;
			AddLabel(Location + Face + FVector(0.f, 0.f, 122.f), LabelYaw, Label, 13.f, FColor(210, 210, 193));
		}
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
	BuildDormFloors();
	BuildSideMap();
	BuildDressing();
	BuildLights();
	BuildDormExterior();
	BuildDormExteriorLighting();
	BuildCampusDressing();
	BuildCampusLighting();
	BuildRooms();
	BuildSelfFigure();
	BuildCheckpoints();
	BuildAudio();
	BuildCampusAudio();
	PreviewHold = 0.f;
	ShowStatus(TEXT("第一章。沿路灯走到宿舍楼门口。"), 18.f);
	bWorldBuilt = true;
	UE_LOG(LogStudent, Log, TEXT("里屋：幻觉已经搭好。"));
}

void ANightSchoolDirector::BuildShell()
{
	AddBox(W(600.f, 400.f, 20.f), FVector(6000.f, 6200.f, 40.f), Concrete, true, true);
	// Keep the interior shell above the rooms and leave the entrance court open to the sky.
	AddBox(W(-1050.f, 400.f, 1085.f), FVector(2700.f, 6200.f, 24.f), Night, false, true);
	AddBox(W(2940.f, 400.f, 860.f), FVector(1320.f, 6200.f, 24.f), Night, false, true);
	AddBox(W(1290.f, 1750.f, 860.f), FVector(1980.f, 3500.f, 24.f), Night, false, true);
	AddBox(W(1290.f, -2040.f, 860.f), FVector(1980.f, 1320.f, 24.f), Night, false, true);
	AddBox(W(600.f, 3500.f, 450.f), FVector(6000.f, 40.f, 820.f), Night, false, true);
	AddBox(W(600.f, -2700.f, 450.f), FVector(6000.f, 40.f, 820.f), Night, false, true);
	AddBox(W(3600.f, 400.f, 450.f), FVector(40.f, 6200.f, 820.f), Night, false, true);
	AddBox(W(-2400.f, 400.f, 450.f), FVector(40.f, 6200.f, 820.f), Night, false, true);
}

void ANightSchoolDirector::BuildStructure()
{
	auto WallX = [this](float X, float Y0, float Y1)
	{
		const float Length = FMath::Abs(Y1 - Y0);
		AddBox(W(X, (Y0 + Y1) * 0.5f, WallZ), FVector(Thick, Length, WallH), Plaster, true, true, DampPlaster);
	};
	auto WallY = [this](float Y, float X0, float X1)
	{
		const float Length = FMath::Abs(X1 - X0);
		AddBox(W((X0 + X1) * 0.5f, Y, WallZ), FVector(Length, Thick, WallH), Plaster, true, true, DampPlaster);
	};
	auto LintelX = [this](float X, float Y)
	{
		AddBox(W(X, Y, LintelZ), FVector(Thick, 150.f, LintelH), Plaster, true, true, DampPlaster);
	};
	auto LintelY = [this](float Y, float X)
	{
		AddBox(W(X, Y, LintelZ), FVector(150.f, Thick, LintelH), Plaster, true, true, DampPlaster);
	};

	WallY(-600.f, -1100.f, -775.f);
	WallY(-600.f, -625.f, 300.f);
	LintelY(-600.f, -700.f);
	WallX(-1100.f, -1400.f, -1075.f);
	WallX(-1100.f, -925.f, NightSchool::StairEntryY - 86.f);
	WallX(-1100.f, NightSchool::StairEntryY + 86.f, 800.f);
	LintelX(-1100.f, -1000.f);
	AddBox(W(-1100.f, NightSchool::StairEntryY, LintelZ),
		FVector(Thick, 172.f, LintelH), Plaster, true, true, DampPlaster);
	WallX(-2200.f, -1400.f, -600.f);
	WallY(-1400.f, -2200.f, -1100.f);
	WallY(-600.f, -2200.f, -1100.f);
	WallY(-1400.f, -1100.f, -350.f);
	WallY(-1400.f, -350.f, -275.f);
	WallY(-1400.f, -125.f, 300.f);
	LintelY(-1400.f, -200.f);
	WallX(300.f, -1400.f, -600.f);
	WallX(-350.f, -1400.f, -1075.f);
	WallX(-350.f, -925.f, -600.f);
	LintelX(-350.f, -1000.f);
	WallY(0.f, -1100.f, -775.f);
	WallY(0.f, -625.f, -75.f);
	WallY(0.f, 75.f, 300.f);
	LintelY(0.f, -700.f);
	LintelY(0.f, 0.f);
	WallX(NightSchool::DormFacadeX, -600.f, NightSchool::DormEntranceY - 100.f);
	WallX(NightSchool::DormFacadeX, NightSchool::DormEntranceY + 100.f, 0.f);
	AddBox(W(NightSchool::DormFacadeX, NightSchool::DormEntranceY, LintelZ),
		FVector(Thick, 200.f, LintelH), Plaster, true, true, DampPlaster);
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
	WallX(2320.f, 0.f, 325.f);
	WallX(2320.f, 475.f, 800.f);
	LintelX(2320.f, 400.f);
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
	WallY(1900.f, 900.f, 2200.f);
	WallX(2200.f, 1400.f, 1900.f);
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
	// Extend the soffits over the wall centerlines so door heads and corners remain covered.
	AddBox(W(-400.f, -300.f, 314.f), FVector(1400.f, 600.f, 16.f), Ceiling, true, true);
	AddBox(W(-700.f, 400.f, 314.f), FVector(800.f, 800.f, 16.f), Ceiling, true, true);
	AddBox(W(0.f, 500.f, 314.f), FVector(600.f, 1000.f, 16.f), Ceiling, true, true);
	AddBox(W(750.f, 400.f, 314.f), FVector(900.f, 800.f, 16.f), Ceiling, true, true);
	AddBox(W(600.f, 1100.f, 314.f), FVector(600.f, 600.f, 16.f), Ceiling, true, true);
	AddBox(W(0.f, 1300.f, 314.f), FVector(600.f, 600.f, 16.f), Ceiling, true, true);
	AddBox(W(600.f, 1650.f, 314.f), FVector(600.f, 500.f, 16.f), Ceiling, true, true);
	AddBox(W(-550.f, 1650.f, 314.f), FVector(500.f, 500.f, 16.f), Ceiling, true, true);
	AddBox(W(-400.f, -1000.f, 314.f), FVector(1400.f, 800.f, 16.f), Ceiling, true, true);
	AddBox(W(-700.f, 1100.f, 314.f), FVector(800.f, 600.f, 16.f), Ceiling, true, true);
	AddBox(W(1600.f, 400.f, 314.f), FVector(800.f, 800.f, 16.f), Ceiling, true, true);
	AddBox(W(1250.f, 1100.f, 314.f), FVector(700.f, 600.f, 16.f), Ceiling, true, true);
	AddBox(W(1900.f, 1100.f, 314.f), FVector(600.f, 600.f, 16.f), Ceiling, true, true);
	AddBox(W(1550.f, 1650.f, 314.f), FVector(1300.f, 500.f, 16.f), Ceiling, true, true);
	AddBox(W(-1650.f, -1000.f, 314.f), FVector(1100.f, 800.f, 16.f), Ceiling, true, true);
	AddBox(W(2160.f, 400.f, 314.f), FVector(320.f, 800.f, 16.f), Ceiling, true, true);

	const FLinearColor FloorWood(0.16f, 0.08f, 0.04f);
	AddBox(W(-400.f, -300.f, 43.f), FVector(1320.f, 520.f, 4.f), Concrete, false, false, WornTerrazzo);
	AddBox(W(-700.f, 400.f, 43.f), FVector(720.f, 720.f, 4.f), FloorWood, false, false, WornTerrazzo);
	AddBox(W(0.f, 500.f, 43.f), FVector(520.f, 920.f, 4.f), Concrete, false, false, WornTerrazzo);
	AddBox(W(750.f, 400.f, 43.f), FVector(820.f, 720.f, 4.f), FloorWood, false, false);
	AddBox(W(600.f, 1100.f, 43.f), FVector(520.f, 520.f, 4.f), DarkWood, false, false);
	AddBox(W(0.f, 1300.f, 43.f), FVector(520.f, 520.f, 4.f), FLinearColor(0.12f, 0.13f, 0.12f), false, false);
	AddBox(W(600.f, 1650.f, 43.f), FVector(520.f, 420.f, 4.f), FLinearColor(0.22f, 0.18f, 0.12f), false, false);
	AddBox(W(-550.f, 1650.f, 43.f), FVector(420.f, 420.f, 4.f), FLinearColor(0.04f, 0.035f, 0.035f), false, false);
	AddBox(W(-700.f, 0.f, 42.f), FVector(140.f, 36.f, 4.f), DarkWood, false, false);
	AddBox(W(300.f, 400.f, 42.f), FVector(36.f, 140.f, 4.f), DarkWood, false, false);
	AddBox(W(-700.f, -1000.f, 43.f), FVector(700.f, 720.f, 4.f), Concrete, false, false);
	AddBox(W(-40.f, -1100.f, 43.f), FVector(620.f, 520.f, 4.f), FLinearColor(0.1f, 0.11f, 0.12f), false, false);
	AddBox(W(-700.f, 1100.f, 43.f), FVector(700.f, 520.f, 4.f), DarkWood, false, false);
	AddBox(W(1600.f, 400.f, 43.f), FVector(720.f, 720.f, 4.f), FloorWood, false, false);
	AddBox(W(1250.f, 1100.f, 43.f), FVector(620.f, 520.f, 4.f), DarkWood, false, false);
	AddBox(W(1900.f, 1100.f, 43.f), FVector(520.f, 520.f, 4.f), FLinearColor(0.08f, 0.09f, 0.1f), false, false);
	AddBox(W(1550.f, 1650.f, 43.f), FVector(1200.f, 420.f, 4.f), Concrete, false, false);
	AddBox(W(-700.f, -600.f, 42.f), FVector(140.f, 36.f, 4.f), DarkWood, false, false);
	AddBox(W(-700.f, 800.f, 42.f), FVector(140.f, 36.f, 4.f), DarkWood, false, false);
	AddBox(W(1200.f, 400.f, 42.f), FVector(36.f, 140.f, 4.f), DarkWood, false, false);
	AddBox(W(-1650.f, -1000.f, 43.f), FVector(1000.f, 720.f, 4.f), Concrete, false, false);
	AddBox(W(2160.f, 400.f, 43.f), FVector(240.f, 720.f, 4.f), DarkWood, false, false);
	AddBox(W(-1100.f, -1000.f, 42.f), FVector(36.f, 140.f, 4.f), DarkWood, false, false);
	AddBox(W(2000.f, 400.f, 42.f), FVector(36.f, 140.f, 4.f), DarkWood, false, false);
}

void ANightSchoolDirector::BuildDressing()
{
	auto Desk = [this](float X, float Y)
	{
		AddWorktable(W(X, Y, 116.f), 120.f, 64.f, Wood);
	};

	using NightSchool::Dorm;
	AddBox(Dorm(-860.f, 600.f, 78.f), FVector(180.f, 90.f, 18.f), DarkWood, true, true);
	AddBox(Dorm(-860.f, 600.f, 90.f), FVector(172.f, 82.f, 6.f), FLinearColor(.17f, .18f, .16f), true, false);
	AddWorktable(Dorm(-980.f, 220.f, 116.f), 120.f, 64.f, Wood);
	AddBox(Dorm(-980.f, 220.f, 126.6f), FVector(18.f, 12.f, 1.2f), FLinearColor(.17f, .25f, .30f), false, false);
	AddBox(Dorm(-389.f, 200.f, 66.f), FVector(64.f, 69.f, 52.f), DarkWood, true, true);
	AddBox(Dorm(-700.f, 780.f, 176.f), FVector(26.f, 15.f, 16.f), FLinearColor(.025f, .029f, .034f), true, false);
	AddBox(Dorm(-700.f, 764.f, 174.f), FVector(18.f, 24.f, 9.f), FLinearColor(.03f, .034f, .037f), true, true);
	AddSphere(Dorm(-700.f, 739.f, 178.f), 4.f, FLinearColor(0.66f, 0.035f, 0.03f), false);
	AddWorktable(W(-155.f, -550.f, 88.f), 238.f, 82.f, DarkWood);
	BuildServiceRooms();
	AddBox(W(-1012.f, 365.f, 132.f), FVector(130.f, 240.f, 184.f), DarkWood, true, true);
	AddLabel(W(-700.f, 16.f, 264.f), 90.f, TEXT("储物间 / 北侧物柜"), 14.f, FColor(173, 187, 179));

	Desk(700.f, 180.f);
	Desk(700.f, 460.f);
	Desk(920.f, 200.f);
	Desk(1000.f, 600.f);
	AddBox(W(460.f, 180.f, 61.f), FVector(66.f, 50.f, 42.f), DarkWood, true, true);

	Desk(740.f, 1140.f);
	AddWorktable(W(-140.f, 1220.f, 70.f), 180.f, 70.f, DarkWood);
	AddBox(W(-140.f, 1220.f, 78.f), FVector(180.f, 70.f, 16.f), FLinearColor(.2f, .22f, .2f), true, false);
	AddBox(W(600.f, 1882.f, 180.f), FVector(70.f, 8.f, 86.f), FLinearColor(.04f, .045f, .05f), true, false);
	AddSphere(W(760.f, 1860.f, 160.f), 3.f, FLinearColor(.55f, .08f, .06f), false);
	AddBox(W(-680.f, 1867.f, 85.f), FVector(80.f, 38.f, 90.f), DarkWood, true, true);
	AddBox(W(1000.f, 782.f, 175.f), FVector(44.f, 8.f, 54.f), DarkWood, true, false);

	// Leave the middle of each room open; records sit on real desks or narrow wall counters.
	struct FTable { float X, Y, Top, Width, Depth; };
	const FTable Tables[] = {
		{-1900.f, -1200.f, 106.f, 110.f, 68.f}, {-1500.f, -900.f, 106.f, 112.f, 66.f},
		{-1800.f, -750.f, 106.f, 124.f, 66.f}, {-1600.f, -1100.f, 92.f, 96.f, 61.f},
		{1120.f, 700.f, 56.f, 58.f, 56.f}, {800.f, 691.f, 82.f, 97.f, 60.f},
		{1500.f, 230.f, 94.f, 112.f, 64.f}, {1850.f, 260.f, 97.f, 110.f, 66.f},
		{1700.f, 520.f, 107.f, 110.f, 68.f},
		{2220.f, 222.f, 97.f, 120.f, 62.f}, {2175.f, 593.f, 97.f, 176.f, 172.f},
		{450.f, 1000.f, 116.f, 92.f, 63.f}, {477.f, 1310.f, 116.f, 90.f, 55.f},
		{783.f, 1310.f, 116.f, 90.f, 55.f},
		{1120.f, 1120.f, 116.f, 143.f, 326.f}, {1440.f, 965.f, 116.f, 192.f, 78.f},
		{2000.f, 1200.f, 107.f, 98.f, 70.f},
		{-180.f, 1280.f, 96.f, 95.f, 65.f}, {-100.f, 1500.f, 92.f, 177.f, 90.f},
		{194.f, 1488.f, 96.f, 88.f, 75.f}, {190.f, 1260.f, 86.f, 80.f, 77.f},
		{720.f, 1600.f, 74.f, 113.f, 53.f}
	};
	for (const FTable& Table : Tables)
	{
		AddWorktable(W(Table.X, Table.Y, Table.Top), Table.Width, Table.Depth, DarkWood);
	}
	AddBox(W(-902.f, 1240.f, 71.f), FVector(147.f, 75.f, 62.f), DarkWood, true, true);
	AddBox(W(-860.f, 1030.f, 104.f), FVector(90.f, 44.f, 128.f), DarkWood, true, true);

	AddLabel(W(-1082.f, -280.f, 204.f), 0.f, TEXT("203 宿舍 · 二楼"), 15.f, FColor(180, 194, 185));
	AddLabel(W(284.f, 400.f, 269.f), 180.f, TEXT("教室"), 16.f, FColor(210, 210, 193));
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
			Component->SetFogDensity(0.006f);
			Component->SetFogHeightFalloff(0.2f);
			Component->SetFogInscatteringColor(FLinearColor(0.026f, 0.038f, 0.035f));
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
		// These low-intensity gameplay lights are not calibrated for physical camera EV100.
		Post->Settings.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
		Post->Settings.AutoExposureApplyPhysicalCameraExposure = false;
		Post->Settings.bOverride_AutoExposureBias = true;
		Post->Settings.AutoExposureBias = 0.31f;
		Post->Settings.bOverride_ColorSaturation = true;
		Post->Settings.ColorSaturation = FVector4(0.69f, 0.76f, 0.65f, 1.f);
		Post->Settings.bOverride_VignetteIntensity = true;
		Post->Settings.VignetteIntensity = 0.48f;
		Post->Settings.bOverride_BloomIntensity = true;
		Post->Settings.BloomIntensity = 0.45f;
		Post->Settings.bOverride_FilmGrainIntensity = true;
		Post->Settings.FilmGrainIntensity = 0.22f;
	}

	const FVector Eye = NightSchool::Dorm(-700.f, 731.f, 182.f);
	APointLight* CameraLed = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), FTransform(Eye), Params);
	if (CameraLed)
	{
		CameraLed->Tags.Add(TEXT("NightSchool"));
		CameraLed->SetMobility(EComponentMobility::Movable);
		if (UPointLightComponent* Component = CameraLed->PointLightComponent)
		{
			Component->SetMobility(EComponentMobility::Movable);
			Component->bUseInverseSquaredFalloff = false;
			Component->SetIntensity(3.f);
			Component->SetAttenuationRadius(155.f);
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
			Component->SetIntensity(17.f);
			Component->SetAttenuationRadius(380.f);
			Component->SetLightColor(FLinearColor(.84f, .83f, .75f));
			Component->SetCastShadows(false);
		}
		WarmLights.Add(GlowAt);
	}

	AddLantern(-700.f, -280.f, 230.f, 10.f, true);
	AddLantern(0.f, -280.f, 230.f, 8.f, true, true);

	AddLantern(-700.f, 420.f, 240.f, 8.f, true, true, 1);
	AddLantern(-700.f, 420.f, 240.f, 4.f, false);
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
	using NightSchool::Dorm;
	SpawnProp(ENightPropId::Phone, Dorm(-980.f, 220.f, 121.f), FVector(12.f, 8.f, 5.f), FLinearColor(0.04f, 0.045f, 0.05f), FText::FromString(TEXT("看未接来电")), FVector::ZeroVector, 0.f, FString());
	SpawnProp(ENightPropId::Earplugs, Dorm(-860.f, 600.f, 98.f), FVector(12.f, 10.f, 5.f), FLinearColor(0.65f, 0.67f, 0.59f), FText::FromString(TEXT("拿走耳塞")), FVector::ZeroVector, 0.f, FString());
	SpawnProp(ENightPropId::LensCloth, W(-886.f, -1205.f, 96.f), FVector(16.f, 3.f, 7.f), FLinearColor(.17f, .135f, .10f), FText::FromString(TEXT("打开抽屉，拿走镜头布")), FVector::ZeroVector, 90.f, FString());
	SpawnProp(ENightPropId::DormCamera, Dorm(-700.f, 751.f, 174.f), FVector(13.f, 9.f, 10.f), FLinearColor(.026f, .031f, .035f), FText::FromString(TEXT("盖上镜头")), FVector::ZeroVector, -90.f, FString());
	SpawnProp(ENightPropId::Soda, Dorm(-389.f, 200.f, 104.f), FVector(7.f, 7.f, 12.f), FLinearColor(0.15f, 0.28f, 0.18f), FText::FromString(TEXT("拿走汽水")), FVector::ZeroVector, 180.f, FString());
	SpawnProp(ENightPropId::HallDoor, W(0.f, 0.f, 145.f), FVector(68.f, 8.f, 100.f), DarkWood, FText::FromString(TEXT("推开走廊")), FVector(200.f, 0.f, 0.f), -90.f, TEXT("走廊"));
	SpawnProp(ENightPropId::LureHall, W(160.f, 640.f, 46.f), FVector(28.f, 28.f, 2.f), Concrete, FText::FromString(TEXT("放下汽水")), FVector::ZeroVector, -90.f, TEXT("地砖"));
	SpawnProp(ENightPropId::LureClass, W(640.f, 280.f, 46.f), FVector(28.f, 28.f, 2.f), Concrete, FText::FromString(TEXT("放下汽水")), FVector::ZeroVector, 180.f, TEXT("地砖"));
	SpawnProp(ENightPropId::ExamPaper, W(1000.f, 600.f, 121.f), FVector(24.f, 18.f, 5.f), Paper, FText::FromString(TEXT("拿走旧试卷")), FVector::ZeroVector, -90.f, TEXT("试卷"));
	SpawnProp(ENightPropId::UnwantedDesk, W(1120.f, 700.f, 84.f), FVector(24.f, 22.f, 28.f), Wood, FText::FromString(TEXT("在空位坐下")), FVector::ZeroVector, -90.f, TEXT("空位"));
	SpawnProp(ENightPropId::HideScore, W(460.f, 180.f, 100.f), FVector(22.f, 16.f, 18.f), DarkWood, FText::FromString(TEXT("把分数藏进抽屉")), FVector::ZeroVector, -90.f, TEXT("抽屉"));
	SpawnProp(ENightPropId::OfficeDoor, W(600.f, 800.f, 145.f), FVector(68.f, 8.f, 100.f), DarkWood, FText::FromString(TEXT("把试卷交到门上")), FVector(200.f, 0.f, 0.f), -90.f, TEXT("办公室"));
	SpawnProp(ENightPropId::TeacherNote, W(740.f, 1140.f, 120.f), FVector(22.f, 16.f, 4.f), Paper, FText::FromString(TEXT("读梁老师的字条")), FVector::ZeroVector, -90.f, TEXT("字条"));
	SpawnProp(ENightPropId::InfirmaryDoor, W(0.f, 1000.f, 145.f), FVector(68.f, 8.f, 100.f), FLinearColor(0.16f, 0.18f, 0.16f), FText::FromString(TEXT("进入校医室")), FVector(200.f, 0.f, 0.f), -90.f, TEXT("校医室"));
	SpawnProp(ENightPropId::Zhou, W(60.f, 1360.f, 76.f), FVector(28.f, 28.f, 36.f), FLinearColor(0.18f, 0.16f, 0.13f), FText::FromString(TEXT("听周先生说")), FVector::ZeroVector, -90.f, TEXT("周先生"));
	SpawnProp(ENightPropId::LightDoor, W(600.f, 1400.f, 145.f), FVector(68.f, 8.f, 100.f), FLinearColor(0.32f, 0.26f, 0.16f), FText::FromString(TEXT("进入光里")), FVector(200.f, 0.f, 0.f), -90.f, TEXT("光"));
	SpawnProp(ENightPropId::FinalCamera, W(760.f, 1874.f, 160.f), FVector(16.f, 12.f, 14.f), FLinearColor(0.03f, 0.03f, 0.035f), FText::FromString(TEXT("看这只镜头")), FVector::ZeroVector, 180.f, TEXT("镜头"));
	SpawnProp(ENightPropId::FaceSwitch, W(460.f, 1874.f, 130.f), FVector(12.f, 12.f, 20.f), FLinearColor(0.72f, 0.62f, 0.38f), FText::FromString(TEXT("用手拨开开关")), FVector::ZeroVector, -90.f, TEXT("开关"));
	SpawnProp(ENightPropId::ClosetDoor, W(-300.f, 1500.f, 145.f), FVector(8.f, 68.f, 100.f), DarkWood, FText::FromString(TEXT("打开柜子")), FVector(0.f, 200.f, 0.f), 0.f, TEXT("柜子"));
	SpawnProp(ENightPropId::ClosetLock, W(-560.f, 1872.f, 120.f), FVector(16.f, 14.f, 24.f), FLinearColor(0.12f, 0.1f, 0.09f), FText::FromString(TEXT("从里面把门带上")), FVector::ZeroVector, 0.f, TEXT("锁"));
	SpawnProp(ENightPropId::DutyLogA, W(-877.f, -1253.f, 119.f), FVector(21.f, 12.f, 3.f), Paper, FText::FromString(TEXT("读三月的记录")), FVector::ZeroVector, 90.f, TEXT("三月"));
	SpawnProp(ENightPropId::DutyLogB, W(-792.f, -1252.f, 119.f), FVector(17.f, 12.f, 3.f), Paper, FText::FromString(TEXT("读四月的记录")), FVector::ZeroVector, 90.f, TEXT("四月"));
	SpawnProp(ENightPropId::DutyLogC, W(-946.f, -708.f, 145.f), FVector(19.f, 3.f, 12.f), Paper, FText::FromString(TEXT("读昨夜的记录")), FVector::ZeroVector, -90.f, TEXT("昨夜"));
	SpawnProp(ENightPropId::Coin, Dorm(-520.f, 360.f, 100.f), FVector(12.f, 10.f, 6.f), FLinearColor(0.62f, 0.5f, 0.18f), FText::FromString(TEXT("拿走午饭钱")), FVector::ZeroVector, 0.f, FString());
	SpawnProp(ENightPropId::NoteSeat, W(980.f, 500.f, 46.f), FVector(16.f, 12.f, 1.5f), Paper, FText::FromString(TEXT("捡起桌腿下的条子")), FVector::ZeroVector, -90.f, TEXT("条子"));
	SpawnProp(ENightPropId::NoteHall, W(-278.f, 250.f, 120.f), FVector(8.f, 18.f, 22.f), Paper, FText::FromString(TEXT("读走廊上的条子")), FVector::ZeroVector, 0.f, TEXT("条子"));
	SpawnProp(ENightPropId::NoteShop, W(1700.f, 520.f, 110.f), FVector(16.f, 12.f, 3.f), Paper, FText::FromString(TEXT("读柜台里的条子")), FVector::ZeroVector, 180.f, TEXT("条子"));
	SpawnProp(ENightPropId::NoteLocker, W(-860.f, 1000.f, 120.f), FVector(14.f, 8.f, 18.f), Paper, FText::FromString(TEXT("抽出柜缝里的条子")), FVector::ZeroVector, 0.f, TEXT("条子"));
	SpawnProp(ENightPropId::ShopDoor, W(1200.f, 400.f, 145.f), FVector(8.f, 68.f, 100.f), FLinearColor(0.22f, 0.18f, 0.12f), FText::FromString(TEXT("把硬币送进卷帘")), FVector(0.f, 200.f, 0.f), 180.f, TEXT("小卖部"));
	SpawnProp(ENightPropId::Ledger, W(1500.f, 220.f, 100.f), FVector(22.f, 16.f, 6.f), DarkWood, FText::FromString(TEXT("拿走请客账")), FVector::ZeroVector, -90.f, TEXT("账本"));
	SpawnProp(ENightPropId::LedgerBoard, W(278.f, 720.f, 150.f), FVector(8.f, 28.f, 36.f), Wood, FText::FromString(TEXT("把账本钉上布告栏")), FVector::ZeroVector, 180.f, TEXT("布告"));
	SpawnProp(ENightPropId::GradeDoor, W(900.f, 1050.f, 145.f), FVector(8.f, 68.f, 100.f), DarkWood, FText::FromString(TEXT("进入年级组")), FVector(0.f, 200.f, 0.f), 180.f, TEXT("年级组"));
	SpawnProp(ENightPropId::MinutesSun, W(1100.f, 980.f, 120.f), FVector(20.f, 14.f, 4.f), Paper, FText::FromString(TEXT("读孙老师的记录")), FVector::ZeroVector, -90.f, TEXT("孙"));
	SpawnProp(ENightPropId::MinutesLiang, W(1390.f, 985.f, 120.f), FVector(20.f, 14.f, 4.f), Paper, FText::FromString(TEXT("读页边上的字")), FVector::ZeroVector, 180.f, TEXT("页边"));
	SpawnProp(ENightPropId::MinutesParent, W(1150.f, 1260.f, 120.f), FVector(20.f, 14.f, 4.f), Paper, FText::FromString(TEXT("读家委会那一页")), FVector::ZeroVector, -90.f, TEXT("家长"));
	SpawnProp(ENightPropId::NoticeA, W(1100.f, 1880.f, 150.f), FVector(18.f, 6.f, 28.f), Paper, FText::FromString(TEXT("读北廊第一张")), FVector::ZeroVector, -90.f, TEXT("一"));
	SpawnProp(ENightPropId::NoticeB, W(1450.f, 1880.f, 150.f), FVector(18.f, 6.f, 28.f), Paper, FText::FromString(TEXT("读北廊第二张")), FVector::ZeroVector, -90.f, TEXT("二"));
	SpawnProp(ENightPropId::NoticeC, W(1750.f, 1880.f, 150.f), FVector(18.f, 6.f, 28.f), Paper, FText::FromString(TEXT("读北廊第三张")), FVector::ZeroVector, -90.f, TEXT("三"));
	SpawnProp(ENightPropId::NoticeD, W(2050.f, 1880.f, 150.f), FVector(18.f, 6.f, 28.f), Paper, FText::FromString(TEXT("读北廊第四张")), FVector::ZeroVector, -90.f, TEXT("四"));
	SpawnProp(ENightPropId::PencilCase, W(-900.f, 1240.f, 110.f), FVector(16.f, 12.f, 8.f), FLinearColor(0.15f, 0.28f, 0.22f), FText::FromString(TEXT("打开写着分数的柜子")), FVector::ZeroVector, 0.f, TEXT("铅笔盒"));
	SpawnProp(ENightPropId::BusPass, W(2000.f, 1200.f, 110.f), FVector(16.f, 12.f, 3.f), Paper, FText::FromString(TEXT("拿走过期月票")), FVector::ZeroVector, 180.f, TEXT("月票"));
	SpawnProp(ENightPropId::Timetable, W(-75.f, -1352.f, 148.f), FVector(16.f, 2.f, 20.f), Paper, FText::FromString(TEXT("取下课表")), FVector::ZeroVector, 90.f, TEXT("课表"));
	SpawnProp(ENightPropId::ComputerDoor, W(1600.f, 1100.f, 145.f), FVector(8.f, 68.f, 100.f), FLinearColor(0.1f, 0.12f, 0.14f), FText::FromString(TEXT("进入微机室")), FVector(0.f, 200.f, 0.f), 180.f, TEXT("微机室"));
	SpawnProp(ENightPropId::ReturnTray, W(-80.f, 1480.f, 100.f), FVector(30.f, 22.f, 8.f), FLinearColor(0.2f, 0.22f, 0.2f), FText::FromString(TEXT("把三样东西放到托盘上")), FVector::ZeroVector, -90.f, TEXT("托盘"));
	SpawnProp(ENightPropId::EssayDorm, Dorm(-945.f, 550.f, 43.f), FVector(18.f, 14.f, 3.f), Paper, FText::FromString(TEXT("捡起床下的作文")), FVector::ZeroVector, 0.f, FString());
	SpawnProp(ENightPropId::EssayShop, W(1850.f, 260.f, 100.f), FVector(18.f, 14.f, 3.f), Paper, FText::FromString(TEXT("捡起半页作文")), FVector::ZeroVector, -90.f, TEXT("同桌"));
	SpawnProp(ENightPropId::EssayGrade, W(1480.f, 960.f, 119.f), FVector(18.f, 14.f, 3.f), Paper, FText::FromString(TEXT("捡起没交的作文")), FVector::ZeroVector, 180.f, TEXT("镜头"));
	SpawnProp(ENightPropId::EssayBench, W(720.f, 1600.f, 90.f), FVector(40.f, 16.f, 16.f), Wood, FText::FromString(TEXT("把三篇作文放上长凳")), FVector::ZeroVector, -90.f, TEXT("长凳"));
	SpawnProp(ENightPropId::SodaShop, W(1516.f, 253.f, 112.f), FVector(10.f, 10.f, 18.f), FLinearColor(0.15f, 0.38f, 0.18f), FText::FromString(TEXT("再拿一瓶汽水")), FVector::ZeroVector, 180.f, TEXT("汽水"));
	SpawnProp(ENightPropId::LureShop, W(1750.f, 620.f, 46.f), FVector(28.f, 28.f, 2.f), Concrete, FText::FromString(TEXT("放下汽水")), FVector::ZeroVector, 180.f, TEXT("地砖"));
	SpawnProp(ENightPropId::LureNotice, W(1600.f, 1580.f, 46.f), FVector(28.f, 28.f, 2.f), Concrete, FText::FromString(TEXT("放下汽水")), FVector::ZeroVector, -90.f, TEXT("地砖"));
	SpawnProp(ENightPropId::SpareCell, W(190.f, 1260.f, 100.f), FVector(10.f, 8.f, 14.f), FLinearColor(0.2f, 0.32f, 0.18f), FText::FromString(TEXT("拿走备用电池")), FVector::ZeroVector, -90.f, TEXT("电池"));
	SpawnProp(ENightPropId::CallDoor, W(-1100.f, -1000.f, 145.f), FVector(8.f, 68.f, 100.f), DarkWood, FText::FromString(TEXT("进入通话间")), FVector(0.f, -200.f, 0.f), 0.f, TEXT("通话间"));
	SpawnProp(ENightPropId::CallA, W(-1900.f, -1200.f, 110.f), FVector(20.f, 14.f, 4.f), Paper, FText::FromString(TEXT("读清晨那通")), FVector::ZeroVector, 90.f, TEXT("清晨"));
	SpawnProp(ENightPropId::CallB, W(-1500.f, -900.f, 110.f), FVector(20.f, 14.f, 4.f), Paper, FText::FromString(TEXT("读锁屏那通")), FVector::ZeroVector, 0.f, TEXT("锁屏"));
	SpawnProp(ENightPropId::CallC, W(-1800.f, -750.f, 110.f), FVector(20.f, 14.f, 4.f), Paper, FText::FromString(TEXT("读镜头那通")), FVector::ZeroVector, -90.f, TEXT("镜头"));
	SpawnProp(ENightPropId::StampCall, W(-1600.f, -1100.f, 100.f), FVector(22.f, 16.f, 8.f), Wood, FText::FromString(TEXT("盖上第三章的章")), FVector::ZeroVector, 90.f, TEXT("章"));
	SpawnProp(ENightPropId::StampDuty, W(-829.f, -1224.f, 124.f), FVector(18.f, 14.f, 8.f), Wood, FText::FromString(TEXT("盖上第二章的章")), FVector::ZeroVector, 90.f, TEXT("章"));
	SpawnProp(ENightPropId::StampAnte, W(-225.f, -548.f, 94.f), FVector(28.f, 20.f, 6.f), Wood, FText::FromString(TEXT("把第一章的章印按上")), FVector::ZeroVector, 90.f, FString());
	AddWorktable(Dorm(-480.f, 710.f, 92.f), 93.f, 54.f, DarkWood);
	SpawnProp(ENightPropId::StampDorm, Dorm(-480.f, 710.f, 100.f), FVector(18.f, 14.f, 8.f), Wood, FText::FromString(TEXT("放下第五章的章印")), FVector::ZeroVector, -90.f, FString());
	SpawnProp(ENightPropId::ClassLogA, W(700.f, 180.f, 120.f), FVector(18.f, 12.f, 4.f), Paper, FText::FromString(TEXT("读座位那页")), FVector::ZeroVector, 180.f, TEXT("座位"));
	SpawnProp(ENightPropId::ClassLogB, W(700.f, 460.f, 120.f), FVector(18.f, 12.f, 4.f), Paper, FText::FromString(TEXT("读奶茶那页")), FVector::ZeroVector, -90.f, TEXT("奶茶"));
	SpawnProp(ENightPropId::ClassLogC, W(920.f, 200.f, 120.f), FVector(18.f, 12.f, 4.f), Paper, FText::FromString(TEXT("读实验那页")), FVector::ZeroVector, 180.f, TEXT("实验"));
	SpawnProp(ENightPropId::StampClass, W(800.f, 680.f, 90.f), FVector(20.f, 14.f, 8.f), Wood, FText::FromString(TEXT("盖上第四章的章")), FVector::ZeroVector, -90.f, TEXT("章"));
	SpawnProp(ENightPropId::StoreDoor, W(2000.f, 400.f, 145.f), FVector(8.f, 68.f, 100.f), DarkWood, FText::FromString(TEXT("进入后仓")), FVector(0.f, 200.f, 0.f), 180.f, TEXT("后仓"));
	SpawnProp(ENightPropId::ReceiptA, W(2220.f, 222.f, 100.f), FVector(16.f, 12.f, 3.f), Paper, FText::FromString(TEXT("读已付的收据")), FVector::ZeroVector, 180.f, TEXT("已付"));
	SpawnProp(ENightPropId::ReceiptB, W(2100.f, 520.f, 100.f), FVector(16.f, 12.f, 3.f), Paper, FText::FromString(TEXT("读缺席的收据")), FVector::ZeroVector, 180.f, TEXT("缺席"));
	SpawnProp(ENightPropId::ReceiptC, W(2240.f, 650.f, 100.f), FVector(16.f, 12.f, 3.f), Paper, FText::FromString(TEXT("读收据背面")), FVector::ZeroVector, -90.f, TEXT("背面"));
	SpawnProp(ENightPropId::DraftA, W(450.f, 1000.f, 120.f), FVector(18.f, 12.f, 4.f), Paper, FText::FromString(TEXT("读第一页批注")), FVector::ZeroVector, -90.f, TEXT("批注"));
	SpawnProp(ENightPropId::DraftB, W(500.f, 1310.f, 120.f), FVector(18.f, 12.f, 4.f), Paper, FText::FromString(TEXT("读第二页批注")), FVector::ZeroVector, 180.f, TEXT("点名"));
	SpawnProp(ENightPropId::DraftC, W(780.f, 1310.f, 120.f), FVector(18.f, 12.f, 4.f), Paper, FText::FromString(TEXT("读第三页批注")), FVector::ZeroVector, -90.f, TEXT("后悔"));
	using NightSchool::Annex;
	SpawnProp(ENightPropId::AnnexDoor, NightSchool::AtFloor(NightSchool::AnnexRoomDoorX, 0.f, 142.f, 2), FVector(80.f, 8.f, 102.f),
		DarkWood, FText::FromString(TEXT("打开长榜展室")), FVector(188.f, 0.f, 0.f), -90.f, FString());
	SpawnProp(ENightPropId::NoticeE, Annex(1100.f, 2670.f, 150.f), FVector(18.f, 6.f, 28.f), Paper, FText::FromString(TEXT("读长榜第五张")), FVector::ZeroVector, -90.f, FString());
	SpawnProp(ENightPropId::NoticeF, Annex(1400.f, 2670.f, 150.f), FVector(18.f, 6.f, 28.f), Paper, FText::FromString(TEXT("读长榜第六张")), FVector::ZeroVector, -90.f, FString());
	SpawnProp(ENightPropId::NoticeG, Annex(1700.f, 2670.f, 150.f), FVector(18.f, 6.f, 28.f), Paper, FText::FromString(TEXT("读长榜第七张")), FVector::ZeroVector, -90.f, FString());
	SpawnProp(ENightPropId::NoticeH, Annex(2000.f, 2670.f, 150.f), FVector(18.f, 6.f, 28.f), Paper, FText::FromString(TEXT("读长榜第八张")), FVector::ZeroVector, -90.f, FString());
	SpawnProp(ENightPropId::LureAnnex, Annex(1600.f, 2250.f, 42.f), FVector(28.f, 28.f, 2.f),
		Concrete, FText::FromString(TEXT("把汽水放到地砖上")), FVector::ZeroVector, -90.f, FString());
	SpawnProp(ENightPropId::ChartA, W(-180.f, 1280.f, 100.f), FVector(18.f, 12.f, 4.f), Paper, FText::FromString(TEXT("读物品那页")), FVector::ZeroVector, 0.f, TEXT("物品"));
	SpawnProp(ENightPropId::ChartB, W(194.f, 1488.f, 100.f), FVector(18.f, 12.f, 4.f), Paper, FText::FromString(TEXT("读血压那页")), FVector::ZeroVector, 180.f, TEXT("血压"));
	SpawnProp(ENightPropId::ChartC, W(-146.f, 1520.f, 100.f), FVector(18.f, 12.f, 4.f), Paper, FText::FromString(TEXT("读门那页")), FVector::ZeroVector, 0.f, TEXT("门"));
	SpawnProp(ENightPropId::SelfFigure, W(620.f, 1788.f, 126.f), FVector(22.f, 16.f, 86.f), FLinearColor(0.12f, 0.18f, 0.32f), FText::FromString(TEXT("看清他")), FVector::ZeroVector, -90.f, TEXT("自己"));
	AddLabel(W(-700.f, -585.f, 267.f), 90.f, TEXT("值班室"), 16.f, FColor(210, 210, 193));
	AddLabel(W(-700.f, 785.f, 267.f), -90.f, TEXT("储物柜"), 16.f, FColor(210, 210, 193));
	AddLabel(W(1200.f, 1885.f, 211.f), -90.f, TEXT("长榜移至宿舍楼 3F"), 14.f, FColor(176, 188, 178));

	AddSprite(W(600.f, 1877.f, 180.f), -90.f, NightSchoolArt::MakeMirror(this), 56.f, 74.f);
	AddSprite(W(1000.f, 777.f, 175.f), -90.f, NightSchoolArt::MakeScoreSheet(this), 36.f, 46.f);
}

void ANightSchoolDirector::BuildAudio()
{
	DroneWave = NewObject<USoundWaveProcedural>(this);
	DroneWave->SetSampleRate(22050);
	DroneWave->NumChannels = 1;
	DroneWave->Duration = INDEFINITELY_LOOPING_DURATION;
	DroneWave->SoundGroup = SOUNDGROUP_Effects;
	DroneWave->bLooping = false;

	DroneAudio = NewObject<UAudioComponent>(this);
	DroneAudio->bAutoActivate = false;
	DroneAudio->SetupAttachment(GetRootComponent());
	DroneAudio->RegisterComponent();
	DroneAudio->SetSound(DroneWave);
	DroneAudio->SetVolumeMultiplier(0.25f);
	QueueDrone(0.4f);
	DroneAudio->Play();
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
	return NightSchool::FloorAt(Location) == 1
		&& Local.X > -1080.f && Local.X < -320.f && Local.Y > 30.f && Local.Y < 770.f;
}

bool ANightSchoolDirector::GetLurePoint(const FBox& Room, FVector& OutPoint) const
{
	if (LureTime <= 0.f || !Room.IsInside(LurePoint))
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
	using NightSchool::Annex;
	const FBox Room(Annex(980.f, 1950.f, -20.f), Annex(2120.f, 2660.f, 335.f));
	TArray<FVector> Points;
	Points.Add(Annex(1200.f, 2200.f, 128.f));
	Points.Add(Annex(1700.f, 2300.f, 128.f));
	Points.Add(Annex(2000.f, 2150.f, 128.f));
	Points.Add(Annex(1400.f, 2400.f, 128.f));
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
	ShowStatus(TEXT("北廊在年级组里面。看完北廊的告示，再回宿舍楼上三楼找长榜。"), 8.f);
}

void ANightSchoolDirector::BuildRooms()
{
	RoomViews.Reset();
	auto AddRoom = [this](const TCHAR* Id, float MinX, float MinY, float MaxX, float MaxY,
		const TCHAR* Name, int32 Floor = 0, bool bShowPreview = true)
	{
		FNightRoomView Room;
		Room.Id = FName(Id);
		Room.Name = Name;
		Room.Bounds = FBox(NightSchool::AtFloor(MinX, MinY, 40.f, Floor),
			NightSchool::AtFloor(MaxX, MaxY, 346.f, Floor));
		Room.bShowPreview = bShowPreview;
		RoomViews.Add(Room);
	};
	AddRoom(TEXT("DormExterior"), 327.f, -1290.f, 2200.f, -30.f, TEXT("宿舍楼外"));
	AddRoom(TEXT("Ante"), -1050.f, -550.f, 274.f, -50.f, TEXT("前厅"));
	AddRoom(TEXT("StoragePassage"), -1050.f, 50.f, -350.f, 750.f, TEXT("储物过道"), 0, false);
	AddRoom(TEXT("Stair1"), -1616.f, -580.f, -1090.f, -430.f, TEXT("宿舍楼梯"), 0, false);
	AddRoom(TEXT("Stair2"), -2220.f, -582.f, -320.f, -433.f, TEXT("二楼平台"), 1, false);
	AddRoom(TEXT("Stair3"), -2220.f, -582.f, -320.f, -433.f, TEXT("三楼平台"), 2, false);
	AddRoom(TEXT("Dorm"), -1050.f, 50.f, -350.f, 750.f, TEXT("203 宿舍"), 1);
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
	AddRoom(TEXT("Annex"), -2180.f, 62.f, -1200.f, 750.f, TEXT("长榜展室"), 2);
	AddRoom(TEXT("Infirmary"), -250.f, 1050.f, 250.f, 1550.f, TEXT("校医室"));
	AddRoom(TEXT("Light"), 350.f, 1480.f, 850.f, 1850.f, TEXT("光"));
	AddRoom(TEXT("Closet"), -750.f, 1480.f, -350.f, 1850.f, TEXT("柜子"));
	AddRoom(TEXT("South"), -430.f, -2500.f, 140.f, -1480.f, TEXT("南甬"));
	AddRoom(TEXT("Tool"), -1050.f, -2500.f, -530.f, -1480.f, TEXT("工具间"));
	AddRoom(TEXT("Porch"), 2380.f, 40.f, 2840.f, 760.f, TEXT("门廊"));
	AddRoom(TEXT("Shed"), 2960.f, 40.f, 3460.f, 760.f, TEXT("车棚"));
	AddRoom(TEXT("Read"), -980.f, 58.f, -445.f, 735.f, TEXT("旧阅览室"), 2);
}

void ANightSchoolDirector::BuildSelfFigure()
{
	const FVector Feet = W(620.f, 1788.f, 40.f);
	if (SelfRig)
	{
		USkeletalMeshComponent* Figure = NewObject<USkeletalMeshComponent>(this);
		Figure->SetupAttachment(GetRootComponent());
		Figure->SetSkeletalMeshAsset(SelfRig);
		Figure->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Figure->RegisterComponent();
		Figure->SetWorldLocationAndRotation(Feet, FRotator(0.f, -90.f, 0.f));
		return;
	}
	if (SelfModel)
	{
		UStaticMeshComponent* Figure = NewObject<UStaticMeshComponent>(this);
		Figure->SetupAttachment(GetRootComponent());
		Figure->SetMobility(EComponentMobility::Movable);
		Figure->SetStaticMesh(SelfModel);
		Figure->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Figure->RegisterComponent();
		Figure->SetWorldLocationAndRotation(Feet, FRotator(0.f, -90.f, 0.f));
		return;
	}
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
			const float LinearAlpha = FMath::Clamp((PreviewTime - Segment * Span) / Span, 0.f, 1.f);
			const bool bHasMovingSubject = PreviewLookPoints.Num() == PreviewPoints.Num();
			const float Alpha = bHasMovingSubject ? FMath::InterpEaseInOut(0.f, 1.f, LinearAlpha, 1.7f) : LinearAlpha;
			const FVector Next = FMath::Lerp(PreviewPoints[Segment], PreviewPoints[Segment + 1], Alpha);
			const FVector From = PreviewCamera->GetActorLocation();
			const FVector LookAt = bHasMovingSubject
				? FMath::Lerp(PreviewLookPoints[Segment], PreviewLookPoints[Segment + 1], Alpha)
				: PreviewLook;
			FVector Loc = Next;
			if (!From.Equals(Next, 0.1))
			{
				FHitResult Hit;
				FCollisionQueryParams Params(SCENE_QUERY_STAT(NightRoomPreview), false, Current);
				if (GetWorld()->SweepSingleByChannel(Hit, From, Next, FQuat::Identity, ECC_Camera,
					FCollisionShape::MakeSphere(16.f), Params))
				{
					Loc = Hit.bStartPenetrating ? From : Hit.Location;
				}
			}
			PreviewCamera->SetActorLocation(Loc);
			PreviewCamera->SetActorRotation((LookAt - Loc).Rotation());
		}
		if (PreviewTime >= PreviewDuration)
		{
			bPreviewReturning = true;
			PreviewReturnLeft = 0.55f;
			if (APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0))
			{
				Controller->SetViewTarget(Current);
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
		if (Room.bSeen || !Room.Bounds.IsInside(Loc))
		{
			continue;
		}
		if (!Room.bShowPreview)
		{
			Room.bSeen = true;
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
	PreviewLookPoints.Reset();
	const bool bExterior = Room.Id == TEXT("DormExterior");
	PreviewCamera->GetCameraComponent()->SetFieldOfView(76.f);
	if (bExterior)
	{
		PreviewPoints = {W(1720.f, -705.f, 245.f), W(1205.f, -542.f, 240.f),
			W(896.f, -418.f, 215.f), W(696.f, -344.f, 207.f)};
		PreviewLookPoints = {W(875.f, -569.f, 537.f), W(325.f, -258.f, 319.f),
			W(318.f, -305.f, 237.f), W(322.f, -305.f, 224.f)};
		PreviewLook = PreviewLookPoints[0];
		PreviewDuration = 8.8f;
		PreviewCaption = TEXT("夜深了。宿舍门口还亮着一盏灯。按 Tab 跳过。");
	}
	else if (Room.Id == TEXT("Ante"))
	{
		PreviewPoints = {W(205.f, -310.f, 174.f), W(-124.f, -333.f, 174.f), W(-548.f, -368.f, 170.f)};
		PreviewLookPoints = {W(-165.f, -530.f, 128.f), W(-694.f, -388.f, 130.f),
			W(-1100.f, NightSchool::StairEntryY, 153.f)};
		PreviewLook = PreviewLookPoints[0];
		PreviewDuration = 6.2f;
		PreviewCaption = TEXT("一楼前厅。接待柜靠南墙，去 203 宿舍要走西侧楼梯。按 Tab 跳过。");
	}
	else if (Room.Id == TEXT("Dorm"))
	{
		using NightSchool::Dorm;
		PreviewPoints = {Dorm(-695.f, 73.f, 174.f), Dorm(-626.f, 232.f, 170.f), Dorm(-619.f, 420.f, 172.f)};
		PreviewLookPoints = {Dorm(-976.f, 220.f, 116.f), Dorm(-886.f, 470.f, 123.f), Dorm(-751.f, 676.f, 153.f)};
		PreviewLook = PreviewLookPoints[0];
		PreviewDuration = 5.8f;
		PreviewCaption = TEXT("二楼 203。床头柜上的手机还亮着。按 Tab 跳过。");
	}
	else
	{
		const double EyeZ = Room.Bounds.Min.Z + 137.0;
		const float Width = Room.Bounds.Max.X - Room.Bounds.Min.X;
		const float Depth = Room.Bounds.Max.Y - Room.Bounds.Min.Y;
		const float InsetX = FMath::Clamp(Width * 0.18f, 36.f, 90.f);
		const float InsetY = FMath::Clamp(Depth * 0.18f, 36.f, 90.f);
		const float X0 = Room.Bounds.Min.X + InsetX;
		const float X1 = Room.Bounds.Max.X - InsetX;
		const float Y0 = Room.Bounds.Min.Y + InsetY;
		const float Y1 = Room.Bounds.Max.Y - InsetY;
		PreviewPoints = {FVector(X0, Y0, EyeZ), FVector(X1, Y1, EyeZ)};
		const FVector Center = Room.Bounds.GetCenter();
		PreviewLook = FVector(Center.X, Center.Y, EyeZ - 50.f);
		PreviewDuration = 5.8f;
		PreviewCaption = FString::Printf(TEXT("第一次走进%s。镜头把这间屋走一遍。按 Tab 跳过。"), *Room.Name);
	}
	PreviewTime = 0.f;
	bPreviewReturning = false;
	bPreviewing = true;
	FVector Start = PreviewPoints[0];
	FCollisionQueryParams Params(SCENE_QUERY_STAT(NightRoomPreviewStart), false, Current);
	if (World->OverlapBlockingTestByChannel(Start, FQuat::Identity, ECC_Camera, FCollisionShape::MakeSphere(16.f), Params))
	{
		FRotator EyeRotation;
		Current->GetActorEyesViewPoint(Start, EyeRotation);
		PreviewPoints[0] = Start;
	}
	PreviewCamera->SetActorLocation(Start);
	PreviewCamera->SetActorRotation((PreviewLook - Start).Rotation());
	Controller->SetViewTarget(PreviewCamera);
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
		if (Ghost && Ghost->Contains(Point))
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
	PreviewHold = FMath::Max(0.f, PreviewHold - DeltaSeconds);
	TickPreview(DeltaSeconds);
	TryBeginPreview();
	TickPuzzle(DeltaSeconds);
	TickCampus(Time, DeltaSeconds);
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
		if (NightSchool::FloorAt(Location) == NightSchool::FloorAt(Lamp)
			&& FVector::DistSquared(Location, Lamp) <= FMath::Square(480.f))
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
	if (!(NoticeCount() >= 8 && bPuzzle5 && bSeal5Set))
	{
		return 5;
	}
	return 6;
}

FString ANightSchoolDirector::GetPreviewLine() const
{
	return bPreviewing ? PreviewCaption : FString();
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
	if (bHasSideKey && !bSideOpen) Append(TEXT("门房钥匙"));
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
	case ENightPropId::DormEntrance:
		Prop->OpenDoor();
		if (!bPhoneRead)
		{
			ShowStatus(TEXT("穿过前厅走西侧楼梯。二楼 203 的手机还亮着。"), 6.f);
		}
		break;
	case ENightPropId::DormSignIn:
		ShowNote(TEXT("宿舍签到表"), TEXT("晚归请在此登记。\n\n日期　房号　姓名　返楼时间\n\n下面几行已经洇湿。最末一栏留了个空格。"));
		break;
	case ENightPropId::DormAnnouncement:
		ShowNote(TEXT("宿舍公告"), TEXT("本楼 22:30 熄灯。晚归请先在门口签到。\n\n熄灯后请勿在走廊逗留。听见敲门，请先确认门外是谁。\n\n宿舍管理室"));
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
		if (!bPuzzle8)
		{
			ShowNote(TEXT("办公室"), TEXT("走廊那组环还没对上。甲在教室，乙在通话间，丙在小卖部。八十秒。人还在走廊里走。"));
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
		if (!bPuzzle7)
		{
			ShowNote(TEXT("铁盒"), TEXT("三个环还没对上。甲写在前厅，乙在洗衣间，丙在南甬。拨的时候宿舍里会有人走。八十秒。"));
			break;
		}
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
	case ENightPropId::SideKey:
		bHasSideKey = true;
		Prop->Consume();
		ShowNote(TEXT("门房钥匙"), TEXT("后仓东墙有一扇不在章印上的门。钥匙只开那一扇。它不是新的一章，也不是第三扇门。"));
		break;
	case ENightPropId::SouthDoor:
		Prop->OpenDoor();
		ShowStatus(TEXT("南甬开了。这里没有章，走到头是工具间。"), 3.6f);
		break;
	case ENightPropId::ToolDoor:
		Prop->OpenDoor();
		break;
	case ENightPropId::SideDoor:
		if (!bHasSideKey && !bSideOpen)
		{
			ShowNote(TEXT("东门"), TEXT("锁是好的。钥匙在南甬尽头，如果还没捡。这扇门不认章印。"));
			break;
		}
		bSideOpen = true;
		Prop->OpenDoor();
		ShowStatus(TEXT("往东是车棚。三楼旧阅览室的钥匙线索在车棚的三只环里。"), 5.f);
		break;
	case ENightPropId::ShedDoor:
		bShedOpen = true;
		Prop->OpenDoor();
		break;
	case ENightPropId::ReadDoor:
		if (!bPuzzle9)
		{
			ShowNote(TEXT("旧阅览室"), TEXT("车棚的三只环还没对上。甲在二楼 203，乙在三楼长榜，丙在一楼校医室。"));
			break;
		}
		TryOpenReadingRoom();
		break;
	case ENightPropId::StudyA:
		bStudyA = true;
		ShowNote(TEXT("夜自习"), TEXT("名册上有许知夏。旁边的格子是空的。没有人把名字写进来，也没有人被写成必须坐在这里。"));
		break;
	case ENightPropId::StudyB:
		bStudyB = true;
		ShowNote(TEXT("车棚"), TEXT("去年的锁还挂着。林小满已经毕业。这把锁打不开一条把她接回来的路。"));
		break;
	case ENightPropId::StudyC:
		bStudyC = true;
		ShowNote(TEXT("值班表"), TEXT("周先生的钟点停在退休前。表上没有第三扇门。两扇还是那两扇。"));
		break;
	case ENightPropId::StudyD:
		bStudyD = true;
		ShowNote(TEXT("宿管"), TEXT("老吴看见宿舍那只红点还亮着。他没有去盖。布只有你手里那一块，如果已经盖上，也不是他盖的。"));
		break;
	case ENightPropId::StudyBench:
		if (StudyCount() < 4)
		{
			ShowNote(TEXT("阅览桌"), TEXT("四页夜自习还没齐。南甬、车棚、这间屋、储物柜。放齐了也不改变两扇门。"));
			break;
		}
		bStudyKept = true;
		ShowNote(TEXT("阅览桌"), TEXT("四页都在桌上。它们留在这里。开关不认这些纸，柜子也不认。"));
		break;
	case ENightPropId::ToolCabinet:
		if (!bPuzzle10)
		{
			ShowNote(TEXT("工具柜"), TEXT("三个环还没对上。数字写在南甬和这间屋的墙上。八十秒。"));
			break;
		}
		if (bToolCell)
		{
			ShowStatus(TEXT("柜子空了。那节电池已经在那点光里。"), 2.8f);
			break;
		}
		bToolCell = true;
		if (ANightSchoolCharacter* Current = Player())
		{
			Current->RefillBattery(100.f);
		}
		Prop->Consume();
		ShowStatus(TEXT("工具柜里有一节电池。它把那点用来后退的光照回来。不是离开这间屋的光。"), 4.2f);
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
	case ENightPropId::DialClue:
		ShowNote(TEXT("墙上的数字"), *Prop->GetMarker().ToString());
		break;
	default:
		break;
	}
}


