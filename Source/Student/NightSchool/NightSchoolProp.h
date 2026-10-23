#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StudentInteractableInterface.h"
#include "NightSchoolTypes.h"
#include "NightSchoolProp.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class STUDENT_API ANightSchoolProp : public AActor, public IStudentInteractableInterface
{
	GENERATED_BODY()

public:
	ANightSchoolProp();

	virtual void Tick(float DeltaSeconds) override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	void Configure(ENightPropId InId, const FVector& InExtent, const FLinearColor& InColor, const FText& InPrompt, const FVector& InSlide);
	ENightPropId GetPropId() const { return PropId; }
	const FText& GetPrompt() const;
	void SetPrompt(const FText& InPrompt);
	void SetAvailable(bool bInAvailable);
	bool IsAvailable() const { return bAvailable; }
	void OpenDoor();
	bool IsDoorOpen() const { return bDoorOpen; }
	void Consume();

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Body;

	ENightPropId PropId = ENightPropId::Phone;
	FText Prompt;
	FVector ClosedLocation = FVector::ZeroVector;
	FVector OpenLocation = FVector::ZeroVector;
	FVector SlideOffset = FVector::ZeroVector;
	bool bAvailable = true;
	bool bDoor = false;
	bool bDoorOpen = false;
	bool bSlideDone = false;
	bool bConsumed = false;
};
