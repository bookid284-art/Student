#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "StudentInteractionComponent.generated.h"

/**
 * Handles first-person forward interaction tracing and interface dispatch.
 */
UCLASS(ClassGroup = (Custom), Blueprintable, meta = (BlueprintSpawnableComponent))
class STUDENT_API UStudentInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStudentInteractionComponent();

	/** Performs a camera-forward line trace and calls Interact on hit actors implementing the interactable interface. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryInteract();

	/** Updates the actor currently under the crosshair. Safe to call every tick. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RefreshFocus();

	/** Actor implementing the interactable interface under the crosshair, if any. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	AActor* GetFocusedActor() const;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractionDistance(float InDistance);

protected:
	/** Maximum trace distance used for interaction checks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = "0.0"))
	float InteractionDistance;

	/** Draw trace debug line and impact point when true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Debug")
	bool bDrawDebugTrace;

	bool TraceForward(FHitResult& OutHit) const;

	TWeakObjectPtr<AActor> FocusedActor;
};
