#include "NightSchool/NightSchoolApparition.h"

#include "Animation/AnimationAsset.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NightSchool/NightSchoolArt.h"
#include "NightSchool/NightSchoolCharacter.h"
#include "NightSchool/NightSchoolDirector.h"

ANightSchoolApparition::ANightSchoolApparition()
{
	PrimaryActorTick.bCanEverTick = true;
	Tags.Add(TEXT("NightSchool"));
	AIControllerClass = nullptr;
	AutoPossessAI = EAutoPossessAI::Disabled;

	GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionObjectType(ECC_Pawn);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 280.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 90.f;
	GetCharacterMovement()->MaxAcceleration = 420.f;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ANightSchoolApparition::BeginPlay()
{
	Super::BeginPlay();

	USkeletalMesh* Body = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	if (!Body)
	{
		Body = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SK_Mannequin.SK_Mannequin"));
	}
	WalkAnim = LoadObject<UAnimationAsset>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd.MF_Unarmed_Walk_Fwd"));
	IdleAnim = LoadObject<UAnimationAsset>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle.MM_Idle"));

	if (Body)
	{
		GetMesh()->SetSkeletalMeshAsset(Body);
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
		GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		if (UMaterialInstanceDynamic* Dark = NightSchoolArt::MakeColor(this, FLinearColor(0.015f, 0.012f, 0.012f)))
		{
			const int32 Slots = FMath::Max(1, GetMesh()->GetNumMaterials());
			for (int32 Index = 0; Index < Slots; ++Index)
			{
				GetMesh()->SetMaterial(Index, Dark);
			}
		}
		if (IdleAnim)
		{
			GetMesh()->PlayAnimation(IdleAnim, true);
		}
	}
	else if (UStaticMeshComponent* Fallback = NewObject<UStaticMeshComponent>(this))
	{
		if (UStaticMesh* Cylinder = NightSchoolArt::Cylinder())
		{
			Fallback->SetStaticMesh(Cylinder);
			Fallback->SetupAttachment(GetCapsuleComponent());
			Fallback->SetRelativeScale3D(FVector(0.55f, 0.55f, 1.7f));
			Fallback->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			if (UMaterialInstanceDynamic* Dark = NightSchoolArt::MakeColor(this, FLinearColor(0.02f, 0.01f, 0.01f)))
			{
				Fallback->SetMaterial(0, Dark);
			}
			Fallback->RegisterComponent();
		}
	}
}

void ANightSchoolApparition::Configure(const FBox& InRoom, const TArray<FVector>& InWaypoints, const FString& InWrongGlyph)
{
	Room = InRoom;
	Waypoints = InWaypoints;
	WrongGlyph = InWrongGlyph;
	if (Waypoints.Num() > 0)
	{
		SetActorLocation(Waypoints[0]);
	}

	if (UFont* Font = NightSchoolArt::GetRuntimeFont())
	{
		UTextRenderComponent* Glyph = NewObject<UTextRenderComponent>(this);
		Glyph->SetupAttachment(GetCapsuleComponent());
		Glyph->SetRelativeLocation(FVector(0.f, 0.f, 110.f));
		Glyph->SetFont(Font);
		Glyph->SetText(FText::FromString(WrongGlyph));
		Glyph->SetWorldSize(28.f);
		Glyph->SetTextRenderColor(FColor(140, 20, 18));
		Glyph->SetHorizontalAlignment(EHTA_Center);
		Glyph->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Glyph->RegisterComponent();
	}
}

ANightSchoolCharacter* ANightSchoolApparition::FindPlayer() const
{
	return Cast<ANightSchoolCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
}

bool ANightSchoolApparition::CanSeePlayer(ANightSchoolCharacter* Player) const
{
	if (!Player || Player->IsEnded() || Player->IsReading())
	{
		return false;
	}
	const FVector MyEyes = GetActorLocation() + FVector(0.f, 0.f, 50.f);
	const FVector Target = Player->GetActorLocation();
	if (!Room.IsInsideXY(Target))
	{
		return false;
	}
	float Sight = 820.f;
	if (const ANightSchoolDirector* Director = ANightSchoolDirector::Find(GetWorld()))
	{
		if (Director->IsWearingEarplugs())
		{
			Sight = 380.f;
		}
	}
	const float Distance = FVector::Dist(MyEyes, Target);
	if (Distance > Sight)
	{
		return false;
	}

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ApparitionSight), false, this);
	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, MyEyes, Target, ECC_Visibility, Params);
	if (bBlocked && Hit.GetActor() != Player && Hit.Distance < Distance - 40.f)
	{
		return false;
	}
	if (Player->IsCrouched() && Distance > 240.f && !Player->IsFlashlightHitting(MyEyes))
	{
		return false;
	}
	return true;
}

void ANightSchoolApparition::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	ANightSchoolCharacter* Player = Cast<ANightSchoolCharacter>(OtherActor);
	if (!Player || Player->IsEnded() || Player->IsReading() || HitCooldown > 0.f || StunRemaining > 0.f)
	{
		return;
	}
	if (ANightSchoolDirector* Director = ANightSchoolDirector::Find(GetWorld()))
	{
		if (Director->IsEnding() || Director->IsNoteOpen())
		{
			return;
		}
		Director->OnCaughtByApparition();
	}
	HitCooldown = 2.8f;
	if (Waypoints.Num() > 0)
	{
		SetActorLocation(Waypoints[0] + FVector(0.f, 0.f, 88.f));
	}
}

void ANightSchoolApparition::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	HitCooldown = FMath::Max(0.f, HitCooldown - DeltaSeconds);
	StunRemaining = FMath::Max(0.f, StunRemaining - DeltaSeconds);

	ANightSchoolDirector* Director = ANightSchoolDirector::Find(GetWorld());
	if (Director && (Director->IsEnding() || Director->IsNoteOpen()))
	{
		return;
	}

	ANightSchoolCharacter* Player = FindPlayer();
	const bool bSees = CanSeePlayer(Player);
	if (bSees)
	{
		bChasing = true;
		LoseSightTime = 1.6f;
	}
	else
	{
		LoseSightTime = FMath::Max(0.f, LoseSightTime - DeltaSeconds);
		if (LoseSightTime <= 0.f)
		{
			bChasing = false;
		}
	}

	if (Player && Player->IsFlashlightHitting(GetActorLocation() + FVector(0.f, 0.f, 60.f)))
	{
		StunRemaining = 1.15f;
		bChasing = false;
	}

	FVector LurePoint = FVector::ZeroVector;
	const bool bLured = Director && Director->GetLurePoint(Room, LurePoint);

	FVector Goal = GetActorLocation();
	if (StunRemaining > 0.f && Player)
	{
		Goal = GetActorLocation() + (GetActorLocation() - Player->GetActorLocation()).GetSafeNormal() * 200.f;
		GetCharacterMovement()->MaxWalkSpeed = 150.f;
	}
	else if (bLured)
	{
		Goal = LurePoint;
		GetCharacterMovement()->MaxWalkSpeed = 130.f;
		bChasing = false;
	}
	else if (bChasing && Player)
	{
		Goal = Player->GetActorLocation();
		GetCharacterMovement()->MaxWalkSpeed = 165.f;
	}
	else if (Waypoints.Num() > 0)
	{
		Goal = Waypoints[WaypointIndex];
		if (FVector::Dist2D(GetActorLocation(), Goal) < 90.f)
		{
			WaypointIndex = (WaypointIndex + 1) % Waypoints.Num();
			Goal = Waypoints[WaypointIndex];
		}
		GetCharacterMovement()->MaxWalkSpeed = 85.f;
	}

	Goal.Z = GetActorLocation().Z;
	if (!Room.IsInsideXY(Goal))
	{
		Goal.X = FMath::Clamp(Goal.X, Room.Min.X + 80.f, Room.Max.X - 80.f);
		Goal.Y = FMath::Clamp(Goal.Y, Room.Min.Y + 80.f, Room.Max.Y - 80.f);
	}

	const FVector Direction = (Goal - GetActorLocation()).GetSafeNormal2D();
	if (!Direction.IsNearlyZero())
	{
		AddMovementInput(Direction, 1.f);
	}
	if (GetVelocity().SizeSquared() < 30.f)
	{
		AddMovementInput(GetActorRightVector(), 1.f);
	}

	const bool bMoving = GetVelocity().SizeSquared() > 80.f;
	if (bMoving != bWasMoving)
	{
		bWasMoving = bMoving;
		if (UAnimationAsset* Next = bMoving ? WalkAnim.Get() : IdleAnim.Get())
		{
			GetMesh()->PlayAnimation(Next, true);
		}
	}
}
