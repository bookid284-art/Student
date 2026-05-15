#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StudentInteractableInterface.generated.h"

UINTERFACE(BlueprintType)
class STUDENT_API UStudentInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Generic interactable interface for actor-based interactions.
 */
class STUDENT_API IStudentInteractableInterface
{
	GENERATED_BODY()

public:
	/**
	 * Called when an interactor (typically the player) triggers interaction.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Interactor);
};
