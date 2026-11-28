#include "NightSchool/NightSchoolProp.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NightSchool/NightSchoolArt.h"
#include "NightSchool/NightSchoolDirector.h"

ANightSchoolProp::ANightSchoolProp()
{
	PrimaryActorTick.bCanEverTick = true;
	Tags.Add(TEXT("NightSchool"));

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetMobility(EComponentMobility::Movable);
	CollisionBox->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionBox->SetBoxExtent(FVector(30.f, 30.f, 30.f));
	CollisionBox->SetHiddenInGame(true);
}

void ANightSchoolProp::Configure(ENightPropId InId, const FVector& InExtent, const FLinearColor& InColor, const FText& InPrompt, const FVector& InSlide)
{
	PropId = InId;
	Prompt = InPrompt;
	bDoor = !InSlide.IsNearlyZero();
	ClosedLocation = GetActorLocation();
	OpenLocation = ClosedLocation + InSlide;
	if (CollisionBox)
	{
		CollisionBox->SetBoxExtent(InExtent);
		if (InId == ENightPropId::LureHall || InId == ENightPropId::LureClass || InId == ENightPropId::LureShop
			|| InId == ENightPropId::LureNotice || InId == ENightPropId::LureAnnex || InId == ENightPropId::SelfFigure)
		{
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
			CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		}
	}

	// The final figure already has a visible model. Its interaction volume needs no second body.
	if (UStaticMesh* Cube = InId == ENightPropId::SelfFigure ? nullptr : NightSchoolArt::Cube())
	{
		Body = NewObject<UStaticMeshComponent>(this, TEXT("Body"));
		Body->SetStaticMesh(Cube);
		Body->SetupAttachment(GetRootComponent());
		Body->SetRelativeScale3D((InExtent * 2.f) / 100.f);
		Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Body->SetCastShadow(true);
		if (UMaterialInstanceDynamic* Mid = NightSchoolArt::MakeColor(this, InColor))
		{
			Body->SetMaterial(0, Mid);
		}
		Body->RegisterComponent();
	}
	if (bDoor)
	{
		BuildDoorHardware(InExtent);
	}
}

void ANightSchoolProp::BuildDoorHardware(const FVector& Extent)
{
	UStaticMesh* Cylinder = NightSchoolArt::Cylinder();
	UMaterialInstanceDynamic* Plate = NightSchoolArt::MakeColor(this, FLinearColor(.09f, .09f, .077f));
	UMaterialInstanceDynamic* Grip = NightSchoolArt::MakeColor(this, FLinearColor(.24f, .19f, .115f));
	if (!Cylinder || !Plate || !Grip)
	{
		return;
	}
	const bool bEntrance = PropId == ENightPropId::DormEntrance;
	const bool bAlongX = Extent.X > Extent.Y;
	const FVector Along = bAlongX ? FVector::ForwardVector : FVector::RightVector;
	const FVector Normal = bAlongX ? FVector::RightVector : FVector::ForwardVector;
	const FVector FreeEdge = Along * ((OpenLocation - ClosedLocation).Dot(Along) >= 0.f ? -1.f : 1.f);
	const FVector Position = FreeEdge * (FMath::Max(Extent.X, Extent.Y) - (bEntrance ? 27.f : 18.f));
	const float FaceDepth = bEntrance ? Extent.X + 4.f : FMath::Min(Extent.X, Extent.Y);
	auto RoundPart = [this, Cylinder](const FVector& Location, const FVector& Axis, float Radius,
		float Length, UMaterialInterface* Material)
	{
		UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this);
		Part->SetupAttachment(GetRootComponent());
		Part->SetStaticMesh(Cylinder);
		Part->SetMaterial(0, Material);
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetRelativeLocation(Location);
		Part->SetRelativeRotation(FQuat::FindBetweenNormals(FVector::UpVector, Axis));
		Part->SetRelativeScale3D(FVector(Radius / 50.f, Radius / 50.f, Length / 100.f));
		Part->RegisterComponent();
		return Part;
	};
	for (float Side : {-1.f, 1.f})
	{
		const FVector Face = Normal * Side;
		const FVector Center = Position + Face * FaceDepth - FVector(0.f, 0.f, 4.f);
		if (bEntrance)
		{
			for (float Height : {-13.f, 13.f})
			{
				const FVector Mount = Center + FVector(0.f, 0.f, Height);
				RoundPart(Mount - Face * 1.8f, Face, 3.6f, 5.8f, Plate);
				RoundPart(Mount + Face * 3.5f, Face, 1.65f, 7.f, Grip);
			}
			RoundPart(Center + Face * 7.f, FVector::UpVector, 1.9f, 29.f, Grip)->ComponentTags.Add(TEXT("NightSchoolDoorGrip"));
		}
		else
		{
			RoundPart(Center, Face, 3.75f, 1.8f, Plate);
			RoundPart(Center + Face * 3.f, Face, 1.8f, 6.8f, Grip);
			RoundPart(Center + Face * 6.3f - FreeEdge * 5.3f, Along, 1.5f, 13.5f, Grip)->ComponentTags.Add(TEXT("NightSchoolDoorGrip"));
			RoundPart(Center - FVector(0.f, 0.f, 8.f), Face, 1.8f, 1.1f, Plate);
		}
	}
	if (bEntrance)
	{
		for (float Height : {-77.f, 0.f, 77.f})
		{
			RoundPart(FVector(-Extent.X, Extent.Y - 3.f, Height), FVector::UpVector, 2.15f, 14.f, Plate);
		}
	}
}

const FText& ANightSchoolProp::GetPrompt() const
{
	static const FText Empty;
	static const FText CloseEntrance = NSLOCTEXT("NightSchool", "CloseDormEntrance", "关上宿舍大门");
	if (!bAvailable || bConsumed)
	{
		return Empty;
	}
	if (bDoorOpen)
	{
		return PropId == ENightPropId::DormEntrance ? CloseEntrance : Empty;
	}
	return Prompt;
}

void ANightSchoolProp::SetPrompt(const FText& InPrompt)
{
	Prompt = InPrompt;
}

void ANightSchoolProp::SetAvailable(bool bInAvailable)
{
	bAvailable = bInAvailable;
	SetActorHiddenInGame(!bInAvailable);
	SetActorEnableCollision(bInAvailable);
}

void ANightSchoolProp::OpenDoor()
{
	if (!bDoor || bDoorOpen)
	{
		return;
	}
	bDoorOpen = true;
	bSlideDone = false;
	SetActorTickEnabled(true);
	if (PropId != ENightPropId::DormEntrance)
	{
		SetActorEnableCollision(false);
	}
}

void ANightSchoolProp::CloseDoor()
{
	if (!bDoor || (!bDoorOpen && DoorProgress <= 0.f))
	{
		return;
	}
	bDoorOpen = false;
	bSlideDone = true;
	DoorProgress = 0.f;
	SetActorLocationAndRotation(ClosedLocation, FRotator::ZeroRotator);
	SetActorEnableCollision(bAvailable);
	SetActorTickEnabled(false);
}

void ANightSchoolProp::Revive()
{
	if (!bConsumed)
	{
		return;
	}
	bConsumed = false;
	bAvailable = true;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}

void ANightSchoolProp::SetMarker(FName InMarker)
{
	Marker = InMarker;
}

void ANightSchoolProp::Consume()
{
	bConsumed = true;
	bAvailable = false;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void ANightSchoolProp::Interact_Implementation(AActor* Interactor)
{
	if (!bAvailable || bConsumed)
	{
		return;
	}
	ANightSchoolDirector* Director = ANightSchoolDirector::Find(GetWorld());
	if (bDoorOpen)
	{
		if (PropId == ENightPropId::DormEntrance && Director && !Director->IsEnding())
		{
			bDoorOpen = false;
			bSlideDone = DoorProgress <= 0.f;
			SetActorTickEnabled(!bSlideDone);
		}
		return;
	}
	if (Director)
	{
		Director->HandleProp(this);
	}
}

void ANightSchoolProp::SetEntrancePose(float Progress)
{
	const FVector Extent = CollisionBox->GetUnscaledBoxExtent();
	const FVector HingeOffset(-Extent.X, Extent.Y - 3.f, 0.f);
	const float Smooth = FMath::SmoothStep(0.f, 1.f, Progress);
	const FQuat Rotation(FVector::UpVector, FMath::DegreesToRadians(-91.f * Smooth));
	const FVector Center = ClosedLocation + HingeOffset - Rotation.RotateVector(HingeOffset);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(DormEntranceSwing), false, this);
	const FCollisionObjectQueryParams Pawns(ECC_Pawn);
	if (GetWorld()->OverlapAnyTestByObjectType(Center, Rotation, Pawns, CollisionBox->GetCollisionShape(), Params))
	{
		return;
	}
	SetActorLocationAndRotation(Center, Rotation);
	DoorProgress = Progress;
}

void ANightSchoolProp::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bDoor || bSlideDone || (!bDoorOpen && DoorProgress <= 0.f))
	{
		return;
	}
	const float Target = bDoorOpen ? 1.f : 0.f;
	const float Next = FMath::FInterpConstantTo(DoorProgress, Target, DeltaSeconds, 1.15f);
	if (PropId == ENightPropId::DormEntrance)
	{
		// Limit each rotation step so a long frame cannot swing across a player in the doorway.
		SetEntrancePose(DoorProgress + FMath::Clamp(Next - DoorProgress, -.04f, .04f));
	}
	else
	{
		DoorProgress = Next;
		SetActorLocation(FMath::Lerp(ClosedLocation, OpenLocation, FMath::SmoothStep(0.f, 1.f, Next)));
	}
	if (DoorProgress == Target)
	{
		bSlideDone = true;
		SetActorTickEnabled(false);
	}
}
