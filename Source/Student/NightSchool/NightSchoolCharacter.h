#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Widgets/SWidget.h"
#include "NightSchoolCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class USpotLightComponent;
class UStudentInteractionComponent;
struct FInputActionValue;

UCLASS()
class STUDENT_API ANightSchoolCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ANightSchoolCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

	bool IsReading() const { return bReading; }
	bool IsPreviewing() const { return bPreviewing; }
	bool IsEnded() const { return bEnded; }
	bool IsFlashlightOn() const { return bFlashlightOn; }
	float GetBattery() const { return Battery; }
	float GetStamina() const { return Stamina; }
	float GetMaxStamina() const { return MaxStamina; }
	bool IsFlashlightHitting(const FVector& WorldPoint) const;
	AActor* GetFocusedActor() const;
	void SetReading(bool bInReading);
	void SetPreviewing(bool bInPreview);
	void SetEnded(bool bInEnded);
	void RefillBattery(float Amount);

private:
	void EnsureInputAssets();
	void AddMappingContext();
	void AttachHud();
	void DetachHud();

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void SprintStarted();
	void SprintStopped();
	void CrouchStarted();
	void CrouchStopped();
	void Interact();
	void ToggleFlashlight();
	void CloseReading();
	void RequestRestart();
	void TryJump();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpotLightComponent> Flashlight;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStudentInteractionComponent> Interaction;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY()
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY()
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY()
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY()
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY()
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY()
	TObjectPtr<UInputAction> FlashlightAction;

	UPROPERTY()
	TObjectPtr<UInputAction> CloseAction;

	UPROPERTY()
	TObjectPtr<UInputAction> RestartAction;

	UPROPERTY()
	TObjectPtr<UInputAction> JumpAction;

	TSharedPtr<SWidget> HudWidget;

	bool bInputReady = false;
	bool bMappingAdded = false;
	bool bHudAttached = false;
	bool bSprintHeld = false;
	bool bExhausted = false;
	bool bReading = false;
	bool bPreviewing = false;
	bool bEnded = false;
	bool bFlashlightOn = false;
	float Battery = 100.f;
	float Stamina = 3.6f;
	float MaxStamina = 3.6f;
	float WalkSpeed = 270.f;
	float SprintSpeed = 490.f;
	float CrouchSpeed = 150.f;
};
