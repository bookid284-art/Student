#include "NightSchool/NightSchoolCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SpotLightComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Kismet/GameplayStatics.h"
#include "NightSchool/NightSchoolDirector.h"
#include "NightSchool/NightSchoolSlate.h"
#include "Student.h"
#include "StudentInteractionComponent.h"

namespace
{
	void MapAxisKey(UInputMappingContext* Context, UInputAction* Action, const FKey& Key, bool bNegate, bool bToY, UObject* Outer)
	{
		FEnhancedActionKeyMapping& Mapping = Context->MapKey(Action, Key);
		if (bToY)
		{
			UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(Outer);
			Swizzle->Order = EInputAxisSwizzle::YXZ;
			Mapping.Modifiers.Add(Swizzle);
		}
		if (bNegate)
		{
			UInputModifierNegate* Negate = NewObject<UInputModifierNegate>(Outer);
			Mapping.Modifiers.Add(Negate);
		}
	}
}

ANightSchoolCharacter::ANightSchoolCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	Tags.Add(TEXT("NightSchool"));

	GetCapsuleComponent()->InitCapsuleSize(36.f, 88.f);
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	GetCharacterMovement()->JumpZVelocity = 280.f;
	GetCharacterMovement()->AirControl = 0.25f;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->SetCrouchedHalfHeight(60.f);
	GetMesh()->SetHiddenInGame(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetCapsuleComponent());
	Camera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	Camera->bUsePawnControlRotation = true;
	Camera->SetFieldOfView(78.f);

	Flashlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Flashlight"));
	Flashlight->SetupAttachment(Camera);
	Flashlight->SetRelativeLocation(FVector(18.f, 0.f, -6.f));
	Flashlight->SetRelativeRotation(FRotator(-6.f, 0.f, 0.f));
	Flashlight->SetMobility(EComponentMobility::Movable);
	Flashlight->bUseInverseSquaredFalloff = false;
	Flashlight->SetIntensity(48.f);
	Flashlight->SetAttenuationRadius(1900.f);
	Flashlight->SetInnerConeAngle(14.f);
	Flashlight->SetOuterConeAngle(32.f);
	Flashlight->SetLightColor(FLinearColor(1.f, 0.95f, 0.86f));
	Flashlight->SetCastShadows(true);
	Flashlight->SetVisibility(false);

	Interaction = CreateDefaultSubobject<UStudentInteractionComponent>(TEXT("Interaction"));
}

void ANightSchoolCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (Interaction)
	{
		Interaction->SetInteractionDistance(280.f);
	}
	if (Flashlight)
	{
		Flashlight->SetVisibility(false);
	}
}

void ANightSchoolCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DetachHud();
	Super::EndPlay(EndPlayReason);
}

void ANightSchoolCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AttachHud();
}

void ANightSchoolCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (Interaction)
	{
		Interaction->RefreshFocus();
	}
	if (bEnded)
	{
		GetCharacterMovement()->StopMovementImmediately();
		return;
	}

	const bool bMoving = GetVelocity().SizeSquared() > 80.f;
	if (bSprintHeld && !bIsCrouched && !bReading && bMoving && !bExhausted)
	{
		Stamina = FMath::Max(0.f, Stamina - DeltaSeconds);
		if (Stamina <= 0.f)
		{
			bExhausted = true;
		}
	}
	else
	{
		Stamina = FMath::Min(MaxStamina, Stamina + DeltaSeconds * 0.62f);
		if (Stamina >= MaxStamina * 0.4f)
		{
			bExhausted = false;
		}
	}

	float Speed = WalkSpeed;
	if (bIsCrouched)
	{
		Speed = CrouchSpeed;
	}
	else if (bSprintHeld && !bExhausted && !bReading)
	{
		Speed = SprintSpeed;
	}
	GetCharacterMovement()->MaxWalkSpeed = Speed;

	if (bFlashlightOn)
	{
		Battery = FMath::Max(0.f, Battery - DeltaSeconds * 1.15f);
		if (Battery <= 0.f)
		{
			bFlashlightOn = false;
		}
	}
	if (Flashlight)
	{
		Flashlight->SetVisibility(bFlashlightOn && Battery > 0.f);
	}
}

void ANightSchoolCharacter::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	if (Camera)
	{
		OutLocation = Camera->GetComponentLocation();
		OutRotation = Camera->GetComponentRotation();
		return;
	}
	Super::GetActorEyesViewPoint(OutLocation, OutRotation);
}

bool ANightSchoolCharacter::IsFlashlightHitting(const FVector& WorldPoint) const
{
	if (!bFlashlightOn || Battery <= 1.f || !Camera)
	{
		return false;
	}
	const FVector ToPoint = WorldPoint - Camera->GetComponentLocation();
	if (ToPoint.SizeSquared() > FMath::Square(760.f))
	{
		return false;
	}
	return FVector::DotProduct(Camera->GetForwardVector(), ToPoint.GetSafeNormal()) > 0.84f;
}

AActor* ANightSchoolCharacter::GetFocusedActor() const
{
	return Interaction ? Interaction->GetFocusedActor() : nullptr;
}

void ANightSchoolCharacter::SetReading(bool bInReading)
{
	bReading = bInReading;
	if (bReading)
	{
		bSprintHeld = false;
	}
}

void ANightSchoolCharacter::SetEnded(bool bInEnded)
{
	bEnded = bInEnded;
	bReading = false;
	bSprintHeld = false;
	if (bEnded)
	{
		GetCharacterMovement()->DisableMovement();
	}
}

void ANightSchoolCharacter::RefillBattery(float Amount)
{
	Battery = FMath::Clamp(Battery + Amount, 0.f, 100.f);
}

void ANightSchoolCharacter::EnsureInputAssets()
{
	if (bInputReady)
	{
		return;
	}
	bInputReady = true;

	MoveAction = NewObject<UInputAction>(this);
	MoveAction->ValueType = EInputActionValueType::Axis2D;
	LookAction = NewObject<UInputAction>(this);
	LookAction->ValueType = EInputActionValueType::Axis2D;
	SprintAction = NewObject<UInputAction>(this);
	CrouchAction = NewObject<UInputAction>(this);
	InteractAction = NewObject<UInputAction>(this);
	FlashlightAction = NewObject<UInputAction>(this);
	CloseAction = NewObject<UInputAction>(this);
	RestartAction = NewObject<UInputAction>(this);
	JumpAction = NewObject<UInputAction>(this);

	MappingContext = NewObject<UInputMappingContext>(this);
	MapAxisKey(MappingContext, MoveAction, EKeys::W, false, true, this);
	MapAxisKey(MappingContext, MoveAction, EKeys::S, true, true, this);
	MapAxisKey(MappingContext, MoveAction, EKeys::D, false, false, this);
	MapAxisKey(MappingContext, MoveAction, EKeys::A, true, false, this);
	MapAxisKey(MappingContext, MoveAction, EKeys::Gamepad_LeftY, false, true, this);
	MapAxisKey(MappingContext, MoveAction, EKeys::Gamepad_LeftX, false, false, this);

	FEnhancedActionKeyMapping& Mouse = MappingContext->MapKey(LookAction, EKeys::Mouse2D);
	UInputModifierScalar* Scale = NewObject<UInputModifierScalar>(this);
	Scale->Scalar = FVector(0.065f, 0.065f, 0.f);
	UInputModifierNegate* NegateY = NewObject<UInputModifierNegate>(this);
	NegateY->bX = false;
	NegateY->bY = true;
	NegateY->bZ = false;
	Mouse.Modifiers.Add(Scale);
	Mouse.Modifiers.Add(NegateY);

	FEnhancedActionKeyMapping& Stick = MappingContext->MapKey(LookAction, EKeys::Gamepad_Right2D);
	UInputModifierScalar* StickScale = NewObject<UInputModifierScalar>(this);
	StickScale->Scalar = FVector(2.4f, 2.4f, 0.f);
	UInputModifierNegate* StickNegate = NewObject<UInputModifierNegate>(this);
	StickNegate->bX = false;
	StickNegate->bY = true;
	StickNegate->bZ = false;
	Stick.Modifiers.Add(StickScale);
	Stick.Modifiers.Add(StickNegate);

	MappingContext->MapKey(SprintAction, EKeys::LeftShift);
	MappingContext->MapKey(SprintAction, EKeys::RightShift);
	MappingContext->MapKey(SprintAction, EKeys::Gamepad_LeftThumbstick);
	MappingContext->MapKey(CrouchAction, EKeys::LeftControl);
	MappingContext->MapKey(CrouchAction, EKeys::RightControl);
	MappingContext->MapKey(CrouchAction, EKeys::Gamepad_RightShoulder);
	MappingContext->MapKey(InteractAction, EKeys::E);
	MappingContext->MapKey(InteractAction, EKeys::Gamepad_FaceButton_Bottom);
	MappingContext->MapKey(FlashlightAction, EKeys::F);
	MappingContext->MapKey(FlashlightAction, EKeys::Gamepad_FaceButton_Left);
	MappingContext->MapKey(CloseAction, EKeys::Q);
	MappingContext->MapKey(CloseAction, EKeys::Escape);
	MappingContext->MapKey(CloseAction, EKeys::Gamepad_FaceButton_Right);
	MappingContext->MapKey(RestartAction, EKeys::R);
	MappingContext->MapKey(JumpAction, EKeys::SpaceBar);
}

void ANightSchoolCharacter::AddMappingContext()
{
	if (bMappingAdded || !MappingContext)
	{
		return;
	}
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !PlayerController->GetLocalPlayer())
	{
		return;
	}
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(MappingContext, 0);
		bMappingAdded = true;
	}
}

void ANightSchoolCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	EnsureInputAssets();
	AddMappingContext();

	if (UEnhancedInputComponent* Enhanced = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Enhanced->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANightSchoolCharacter::Move);
		Enhanced->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANightSchoolCharacter::Look);
		Enhanced->BindAction(SprintAction, ETriggerEvent::Started, this, &ANightSchoolCharacter::SprintStarted);
		Enhanced->BindAction(SprintAction, ETriggerEvent::Completed, this, &ANightSchoolCharacter::SprintStopped);
		Enhanced->BindAction(SprintAction, ETriggerEvent::Canceled, this, &ANightSchoolCharacter::SprintStopped);
		Enhanced->BindAction(CrouchAction, ETriggerEvent::Started, this, &ANightSchoolCharacter::CrouchStarted);
		Enhanced->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ANightSchoolCharacter::CrouchStopped);
		Enhanced->BindAction(CrouchAction, ETriggerEvent::Canceled, this, &ANightSchoolCharacter::CrouchStopped);
		Enhanced->BindAction(InteractAction, ETriggerEvent::Started, this, &ANightSchoolCharacter::Interact);
		Enhanced->BindAction(FlashlightAction, ETriggerEvent::Started, this, &ANightSchoolCharacter::ToggleFlashlight);
		Enhanced->BindAction(CloseAction, ETriggerEvent::Started, this, &ANightSchoolCharacter::CloseReading);
		Enhanced->BindAction(RestartAction, ETriggerEvent::Started, this, &ANightSchoolCharacter::RequestRestart);
		Enhanced->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		Enhanced->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
	else
	{
		UE_LOG(LogStudent, Error, TEXT("里屋：没有 Enhanced Input，移动不会生效。"));
	}
	AttachHud();
}

void ANightSchoolCharacter::Move(const FInputActionValue& Value)
{
	if (bReading || bEnded || !Controller)
	{
		return;
	}
	const FVector2D Axis = Value.Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), Axis.Y);
	AddMovementInput(GetActorRightVector(), Axis.X);
}

void ANightSchoolCharacter::Look(const FInputActionValue& Value)
{
	if (!Controller)
	{
		return;
	}
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ANightSchoolCharacter::SprintStarted()
{
	bSprintHeld = !bReading && !bEnded;
}

void ANightSchoolCharacter::SprintStopped()
{
	bSprintHeld = false;
}

void ANightSchoolCharacter::CrouchStarted()
{
	if (!bReading && !bEnded)
	{
		Crouch();
	}
}

void ANightSchoolCharacter::CrouchStopped()
{
	UnCrouch();
}

void ANightSchoolCharacter::Interact()
{
	if (bReading || bEnded)
	{
		return;
	}
	if (Interaction)
	{
		Interaction->TryInteract();
	}
}

void ANightSchoolCharacter::ToggleFlashlight()
{
	if (bEnded || Battery <= 1.f)
	{
		bFlashlightOn = false;
		return;
	}
	bFlashlightOn = !bFlashlightOn;
}

void ANightSchoolCharacter::CloseReading()
{
	if (bEnded)
	{
		return;
	}
	if (ANightSchoolDirector* Director = ANightSchoolDirector::Find(GetWorld()))
	{
		Director->CloseNote();
	}
	bReading = false;
}

void ANightSchoolCharacter::RequestRestart()
{
	if (!bEnded || !GetWorld())
	{
		return;
	}
	DetachHud();
	const FString MapName = GetWorld()->GetOutermost()->GetName();
	UGameplayStatics::OpenLevel(this, FName(*MapName));
}

void ANightSchoolCharacter::AttachHud()
{
	if (bHudAttached || !IsLocallyControlled())
	{
		return;
	}
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !GEngine || !GEngine->GameViewport)
	{
		return;
	}
	HudWidget = MakeNightSchoolHud(this);
	GEngine->GameViewport->AddViewportWidgetContent(HudWidget.ToSharedRef(), 12);
	bHudAttached = true;
	FInputModeGameOnly Mode;
	PlayerController->SetInputMode(Mode);
	PlayerController->bShowMouseCursor = false;
}

void ANightSchoolCharacter::DetachHud()
{
	if (HudWidget.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(HudWidget.ToSharedRef());
	}
	HudWidget.Reset();
	bHudAttached = false;
}
