#include "StudentPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "StudentInteractionComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"

AStudentPlayerCharacter::AStudentPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create and configure first-person camera.
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCameraComponent"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Rotate pawn with controller yaw for traditional FPS control.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Character movement baseline setup.
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	WalkSpeed = 300.0f;
	RunSpeed = 600.0f;
	CrouchSpeed = 180.0f;

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;

	InteractionComponent = CreateDefaultSubobject<UStudentInteractionComponent>(TEXT("InteractionComponent"));

	InteractAction = nullptr;
}

void AStudentPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Add mapping context for the local player at runtime.
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
			{
				if (DefaultMappingContext)
				{
					InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}
	}
}

void AStudentPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AStudentPlayerCharacter::Move);
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AStudentPlayerCharacter::Look);
		}

		if (RunAction)
		{
			EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &AStudentPlayerCharacter::StartRun);
			EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &AStudentPlayerCharacter::StopRun);
			EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Canceled, this, &AStudentPlayerCharacter::StopRun);
		}

		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AStudentPlayerCharacter::StartCrouch);
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AStudentPlayerCharacter::StopCrouch);
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Canceled, this, &AStudentPlayerCharacter::StopCrouch);
		}

		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AStudentPlayerCharacter::TryInteract);
		}
	}
}

void AStudentPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AStudentPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AStudentPlayerCharacter::StartRun()
{
	if (!bIsCrouched)
	{
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	}
}

void AStudentPlayerCharacter::StopRun()
{
	GetCharacterMovement()->MaxWalkSpeed = bIsCrouched ? CrouchSpeed : WalkSpeed;
}

void AStudentPlayerCharacter::StartCrouch()
{
	Crouch();
	GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
}

void AStudentPlayerCharacter::StopCrouch()
{
	UnCrouch();
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AStudentPlayerCharacter::TryInteract()
{
	if (InteractionComponent)
	{
		InteractionComponent->TryInteract();
	}
}
