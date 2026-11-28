#include "NightSchool/NightSchoolApparition.h"

#include "Animation/AnimationAsset.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
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
	GetCharacterMovement()->bRunPhysicsWithNoController = true;
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
	ChaseAnim = LoadObject<UAnimationAsset>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd.MF_Unarmed_Jog_Fwd"));

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
		UpdateAnimation();
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
	const ANightSchoolDirector* Director = ANightSchoolDirector::Find(GetWorld());
	if (!Player || Player->IsEnded() || Player->IsPreviewing())
	{
		return false;
	}
	if (Player->IsReading() && !(Director && Director->IsPuzzleActive()))
	{
		return false;
	}
	const FVector MyEyes = GetActorLocation() + FVector(0.f, 0.f, 50.f);
	const FVector Target = Player->GetActorLocation();
	if (!Room.IsInside(Target))
	{
		return false;
	}
	float Sight = 820.f;
	if (Director && Director->IsWearingEarplugs())
	{
		Sight = 380.f;
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
	TryCatchPlayer(Cast<ANightSchoolCharacter>(OtherActor));
}

void ANightSchoolApparition::TryCatchPlayer(ANightSchoolCharacter* Player)
{
	ANightSchoolDirector* Director = ANightSchoolDirector::Find(GetWorld());
	const bool bPuzzle = Director && Director->IsPuzzleActive();
	if (!Player || Player->IsEnded() || Player->IsPreviewing() || HitCooldown > 0.f || StunRemaining > 0.f)
	{
		return;
	}
	if (Player->IsReading() && !bPuzzle)
	{
		return;
	}
	if (Director)
	{
		if (Director->IsEnding() || Director->IsPreviewing() || (Director->IsNoteOpen() && !bPuzzle))
		{
			return;
		}
		Director->OnCaughtByApparition();
	}
	HitCooldown = 2.8f;
	bChasing = false;
	LoseSightTime = 0.f;
	BlockedTime = 0.f;
	if (Waypoints.Num() > 0)
	{
		int32 Best = 0;
		float BestDist = -1.f;
		const FVector From = Player->GetActorLocation();
		for (int32 Index = 0; Index < Waypoints.Num(); ++Index)
		{
			const float Dist = FVector::DistSquared2D(Waypoints[Index], From);
			if (Dist > BestDist)
			{
				BestDist = Dist;
				Best = Index;
			}
		}
		WaypointIndex = (Best + 1) % Waypoints.Num();
		SetActorLocation(Waypoints[Best], false, nullptr, ETeleportType::TeleportPhysics);
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->StopMovementImmediately();
			Move->SetMovementMode(MOVE_Walking);
		}
	}
}

FVector ANightSchoolApparition::FindMoveDirection(const FVector& Goal, const AActor* Target) const
{
	const FVector Here = GetActorLocation();
	const FVector Toward = (Goal - Here).GetSafeNormal2D();
	const float Reach = FMath::Min(70.f, FVector::Dist2D(Goal, Here));
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ApparitionMove), false, this);
	if (Target)
	{
		Params.AddIgnoredActor(Target);
	}
	// Leave clearance above the floor so a grounded capsule can detect furniture and walls ahead.
	const UCapsuleComponent* Capsule = GetCapsuleComponent();
	const FCollisionShape Shape = FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius() - 1.f,
		Capsule->GetScaledCapsuleHalfHeight() - 3.f);
	FVector BestDirection = FVector::ZeroVector;
	float BestScore = 0.f;
	for (float Angle : {0.f, -40.f, 40.f, -72.f, 72.f, -105.f, 105.f})
	{
		const FVector Direction = Toward.RotateAngleAxis(Angle, FVector::UpVector);
		const FVector End = Here + Direction * Reach;
		if (!Room.IsInsideXY(End))
		{
			continue;
		}
		FHitResult Hit;
		const bool bBlocked = GetWorld()->SweepSingleByChannel(Hit, Here, End, FQuat::Identity, ECC_Pawn, Shape, Params);
		if (!bBlocked && Angle == 0.f)
		{
			return Direction;
		}
		const float Fraction = bBlocked ? Hit.Time : 1.f;
		const float Score = Fraction * (1.f - FMath::Abs(Angle) / 150.f);
		if (Score > BestScore && Reach * Fraction >= 8.f)
		{
			BestScore = Score;
			BestDirection = Direction;
		}
	}
	return BestDirection;
}

void ANightSchoolApparition::UpdateAnimation()
{
	const float Speed = GetVelocity().Size2D();
	UAnimationAsset* Next = IdleAnim;
	if (Speed > 8.f)
	{
		Next = bChasing && Speed > 121.f && ChaseAnim ? ChaseAnim.Get() : WalkAnim.Get();
	}
	if (Next && Next != PlayingAnim)
	{
		GetMesh()->PlayAnimation(Next, true);
		PlayingAnim = Next;
	}
}

void ANightSchoolApparition::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	HitCooldown = FMath::Max(0.f, HitCooldown - DeltaSeconds);
	StunRemaining = FMath::Max(0.f, StunRemaining - DeltaSeconds);

	ANightSchoolDirector* Director = ANightSchoolDirector::Find(GetWorld());
	const bool bPuzzle = Director && Director->IsPuzzleActive();
	if (Director && (Director->IsEnding() || Director->IsPreviewing() || (Director->IsNoteOpen() && !bPuzzle)))
	{
		ConsumeMovementInputVector();
		GetCharacterMovement()->StopMovementImmediately();
		UpdateAnimation();
		return;
	}

	ANightSchoolCharacter* Player = FindPlayer();
	const bool bSees = HitCooldown <= 0.f && CanSeePlayer(Player);
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

	const FVector Head = GetActorLocation() + FVector(0.f, 0.f, 60.f);
	if (Player && Player->IsFlashlightHitting(Head))
	{
		FVector Eyes;
		FRotator View;
		Player->GetActorEyesViewPoint(Eyes, View);
		FCollisionQueryParams Params(SCENE_QUERY_STAT(ApparitionFlashlight), false, Player);
		Params.AddIgnoredActor(this);
		if (!GetWorld()->LineTraceTestByChannel(Eyes, Head, ECC_Visibility, Params))
		{
			StunRemaining = 1.15f;
			bChasing = false;
		}
	}
	if (Player && IsOverlappingActor(Player))
	{
		TryCatchPlayer(Player);
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
		GetCharacterMovement()->MaxWalkSpeed = 195.f;
	}
	else if (Waypoints.Num() > 0)
	{
		Goal = Waypoints[WaypointIndex];
		if (FVector::Dist2D(GetActorLocation(), Goal) < 40.f || BlockedTime > 1.2f)
		{
			WaypointIndex = (WaypointIndex + 1) % Waypoints.Num();
			Goal = Waypoints[WaypointIndex];
			BlockedTime = 0.f;
		}
		GetCharacterMovement()->MaxWalkSpeed = 85.f;
	}

	Goal.Z = GetActorLocation().Z;
	if (!Room.IsInsideXY(Goal))
	{
		Goal.X = FMath::Clamp(Goal.X, Room.Min.X + 80.f, Room.Max.X - 80.f);
		Goal.Y = FMath::Clamp(Goal.Y, Room.Min.Y + 80.f, Room.Max.Y - 80.f);
	}

	if (FVector::DistSquared2D(Goal, GetActorLocation()) > FMath::Square(12.f))
	{
		AddMovementInput(FindMoveDirection(Goal, Player));
		BlockedTime = GetVelocity().SizeSquared2D() < FMath::Square(12.f) ? BlockedTime + DeltaSeconds : 0.f;
	}
	else
	{
		BlockedTime = 0.f;
	}
	UpdateAnimation();
}
