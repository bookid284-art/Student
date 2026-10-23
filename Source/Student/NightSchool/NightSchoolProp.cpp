#include "NightSchool/NightSchoolProp.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
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
	SlideOffset = InSlide;
	bDoor = !InSlide.IsNearlyZero();
	ClosedLocation = GetActorLocation();
	OpenLocation = ClosedLocation + InSlide;
	if (CollisionBox)
	{
		CollisionBox->SetBoxExtent(InExtent);
	}

	if (UStaticMesh* Cube = NightSchoolArt::Cube())
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
}

const FText& ANightSchoolProp::GetPrompt() const
{
	static const FText Empty;
	if (!bAvailable || bConsumed || bDoorOpen)
	{
		return Empty;
	}
	return Prompt;
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
	SetActorEnableCollision(false);
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
	if (!bAvailable || bConsumed || bDoorOpen)
	{
		return;
	}
	if (ANightSchoolDirector* Director = ANightSchoolDirector::Find(GetWorld()))
	{
		Director->HandleProp(this);
	}
}

void ANightSchoolProp::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bDoorOpen || bSlideDone)
	{
		return;
	}
	const FVector Next = FMath::VInterpTo(GetActorLocation(), OpenLocation, DeltaSeconds, 3.f);
	SetActorLocation(Next);
	if (FVector::DistSquared(Next, OpenLocation) < 9.f)
	{
		SetActorLocation(OpenLocation);
		bSlideDone = true;
		SetActorTickEnabled(false);
	}
}
