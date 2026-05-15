#include "StudentInteractionComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "StudentInteractableInterface.h"

UStudentInteractionComponent::UStudentInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	InteractionDistance = 300.0f;
	bDrawDebugTrace = false;
}

void UStudentInteractionComponent::TryInteract()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	Owner->GetActorEyesViewPoint(ViewLocation, ViewRotation);

	const FVector TraceStart = ViewLocation;
	const FVector TraceEnd = TraceStart + (ViewRotation.Vector() * InteractionDistance);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(StudentInteractionTrace), false, Owner);

	FHitResult HitResult;
	const bool bHasHit = GetWorld() && GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams);

	if (bDrawDebugTrace && GetWorld())
	{
		const FVector DebugEnd = bHasHit ? HitResult.ImpactPoint : TraceEnd;
		DrawDebugLine(GetWorld(), TraceStart, DebugEnd, bHasHit ? FColor::Green : FColor::Red, false, 1.0f, 0, 1.0f);
		if (bHasHit)
		{
			DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 8.0f, FColor::Yellow, false, 1.0f);
		}
	}

	if (!bHasHit)
	{
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	if (HitActor && HitActor->GetClass()->ImplementsInterface(UStudentInteractableInterface::StaticClass()))
	{
		IStudentInteractableInterface::Execute_Interact(HitActor, Owner);
	}
}
