#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "StudentPlayerCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UInputComponent;

/**
 * First-person player character for the Student demo.
 * Blueprint (BP_StudentPlayerCharacter) is expected to assign input assets.
 */
UCLASS(Blueprintable)
class STUDENT_API AStudentPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AStudentPlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** First person camera used for player view. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* FirstPersonCameraComponent;

	/** Enhanced Input mapping context assigned in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	/** Move input action (Vector2D). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	/** Look input action (Vector2D). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	/** Run input action (bool/triggered by Shift). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* RunAction;

	/** Crouch input action (bool/triggered by Ctrl). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* CrouchAction;

	/** Default walk speed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Speed", meta = (ClampMin = "0.0"))
	float WalkSpeed;

	/** Run speed while Shift is held. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Speed", meta = (ClampMin = "0.0"))
	float RunSpeed;

	/** Speed while crouching. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Speed", meta = (ClampMin = "0.0"))
	float CrouchSpeed;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartRun();
	void StopRun();
	void StartCrouch();
	void StopCrouch();
};
