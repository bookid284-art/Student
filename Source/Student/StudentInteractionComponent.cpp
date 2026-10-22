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

bool UStudentInteractionComponent::TraceForward(FHitResult& OutHit) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !GetWorld())
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	Owner->GetActorEyesViewPoint(ViewLocation, ViewRotation);

	const FVector TraceStart = ViewLocation;
	const FVector TraceEnd = TraceStart + (ViewRotation.Vector() * InteractionDistance);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(StudentInteractionTrace), false, Owner);
	const bool bHasHit = GetWorld()->LineTraceSingleByChannel(
		OutHit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams);

	if (bDrawDebugTrace)
	{
		const FVector DebugEnd = bHasHit ? OutHit.ImpactPoint : TraceEnd;
		DrawDebugLine(GetWorld(), TraceStart, DebugEnd, bHasHit ? FColor::Green : FColor::Red, false, 0.05f, 0, 1.0f);
		if (bHasHit)
		{
			DrawDebugPoint(GetWorld(), OutHit.ImpactPoint, 8.0f, FColor::Yellow, false, 0.05f);
		}
	}

	return bHasHit;
}

void UStudentInteractionComponent::RefreshFocus()
{
	FocusedActor = nullptr;

	FHitResult HitResult;
	if (!TraceForward(HitResult))
	{
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	if (HitActor && HitActor->GetClass()->ImplementsInterface(UStudentInteractableInterface::StaticClass()))
	{
		FocusedActor = HitActor;
	}
}

AActor* UStudentInteractionComponent::GetFocusedActor() const
{
	return FocusedActor.Get();
}

void UStudentInteractionComponent::SetInteractionDistance(float InDistance)
{
	InteractionDistance = FMath::Max(0.f, InDistance);
}

void UStudentInteractionComponent::TryInteract()
{
	FHitResult HitResult;
	if (!TraceForward(HitResult))
	{
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	if (HitActor && HitActor->GetClass()->ImplementsInterface(UStudentInteractableInterface::StaticClass()))
	{
		IStudentInteractableInterface::Execute_Interact(HitActor, GetOwner());
	}
}
