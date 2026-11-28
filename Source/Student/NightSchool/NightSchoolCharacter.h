#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Widgets/SWidget.h"
#include "NightSchoolCharacter.generated.h"

class UAnimationAsset;
class UCameraComponent;
class UInputAction;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;
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
	UFUNCTION(BlueprintPure, Category = "Night School|Journal")
	bool IsJournalOpen() const { return bJournalOpen; }
	void SetJournalOpen(bool bOpen);
	bool UsesGamepadHints() const { return bGamepadHints; }
	bool IsFlashlightOn() const { return bFlashlightOn; }
	float GetBattery() const { return Battery; }
	float GetStamina() const { return Stamina; }
	float GetMaxStamina() const { return MaxStamina; }
	float GetLean() const { return LeanAlpha; }
	void ApplyResume(const FVector& Location, const FRotator& View, float InBattery, float InStamina, bool bWasCrouched, bool bLightOn);
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
	void SetDocumentInput(bool bOpen);

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void SprintStarted();
	void SprintStopped();
	void CrouchStarted();
	void CrouchStopped();
	void Interact();
	void ToggleFlashlight();
	void CloseReading();
	void OpenJournal(const FInputActionValue& Value);
	void RequestRestart();
	void TryJump();
	void LeanLeftStarted();
	void LeanLeftStopped();
	void LeanRightStarted();
	void LeanRightStopped();
	void UpdateLean(float DeltaSeconds);
	void BuildBody();
	void UpdateBodyPose(bool bCameraClear);

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
	TObjectPtr<UInputAction> JournalAction;

	UPROPERTY()
	TObjectPtr<UInputAction> RestartAction;

	UPROPERTY()
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY()
	TObjectPtr<UInputAction> LeanLeftAction;

	UPROPERTY()
	TObjectPtr<UInputAction> LeanRightAction;

	TSharedPtr<SWidget> HudWidget;

	bool bInputReady = false;
	bool bMappingAdded = false;
	bool bHudAttached = false;
	bool bSprintHeld = false;
	bool bExhausted = false;
	bool bReading = false;
	bool bPreviewing = false;
	bool bEnded = false;
	bool bJournalOpen = false;
	bool bGamepadHints = false;
	bool bFlashlightOn = false;
	float Battery = 100.f;
	float Stamina = 3.6f;
	float MaxStamina = 3.6f;
	float WalkSpeed = 270.f;
	float SprintSpeed = 490.f;
	float CrouchSpeed = 150.f;
	float LeanAlpha = 0.f;
	bool bLeanLeft = false;
	bool bLeanRight = false;
	bool bViewReady = false;
	FVector SmoothedView = FVector::ZeroVector;

	UPROPERTY()
	TObjectPtr<USceneComponent> BodyRoot;

	UPROPERTY()
	TObjectPtr<UStaticMesh> StudentBodyAsset;

	UPROPERTY()
	TObjectPtr<UAnimationAsset> BodyIdle;

	UPROPERTY()
	TObjectPtr<UAnimationAsset> BodyWalk;

	UPROPERTY()
	TObjectPtr<UAnimationAsset> BodyRun;

	UPROPERTY()
	TObjectPtr<UAnimationAsset> BodyCrouchIdle;

	UPROPERTY()
	TObjectPtr<UAnimationAsset> BodyCrouchWalk;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> CurrentBodyAnimation;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> StudentBody;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> HeadParts;
};
