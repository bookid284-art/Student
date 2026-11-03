#include "NightSchool/NightSchoolCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "Math/RotationMatrix.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Kismet/GameplayStatics.h"
#include "NightSchool/NightSchoolArt.h"
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
	Camera->bUsePawnControlRotation = false;
	Camera->SetFieldOfView(66.f);

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
		Flashlight->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	BuildBody();
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
	if (!bMappingAdded)
	{
		AddMappingContext();
	}
	if (Interaction)
	{
		Interaction->RefreshFocus();
	}
	if (bEnded)
	{
		GetCharacterMovement()->StopMovementImmediately();
		UpdateLean(DeltaSeconds);
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

	UpdateLean(DeltaSeconds);
	float Speed = WalkSpeed;
	if (bIsCrouched)
	{
		Speed = CrouchSpeed;
	}
	else if (bSprintHeld && !bExhausted && !bReading && FMath::Abs(LeanAlpha) < 0.2f)
	{
		Speed = SprintSpeed;
	}
	if (FMath::Abs(LeanAlpha) >= 0.2f)
	{
		Speed *= 0.62f;
	}
	GetCharacterMovement()->MaxWalkSpeed = Speed;

	if (bFlashlightOn)
	{
		Battery = FMath::Max(0.f, Battery - DeltaSeconds * 0.4f);
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

void ANightSchoolCharacter::SetPreviewing(bool bInPreview)
{
	bPreviewing = bInPreview;
	if (bPreviewing)
	{
		bSprintHeld = false;
		bLeanLeft = false;
		bLeanRight = false;
		GetCharacterMovement()->StopMovementImmediately();
	}
}

void ANightSchoolCharacter::ApplyResume(const FVector& Location, const FRotator& View, float InBattery, float InStamina, bool bWasCrouched, bool bLightOn)
{
	bViewReady = false;
	UCharacterMovementComponent* Move = GetCharacterMovement();
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	const float CrouchedHalf = Move ? Move->CrouchedHalfHeight : 60.f;
	float StandingHalf = 88.f;
	if (const ANightSchoolCharacter* Defaults = GetDefault<ANightSchoolCharacter>())
	{
		if (const UCapsuleComponent* DefaultCapsule = Defaults->GetCapsuleComponent())
		{
			StandingHalf = DefaultCapsule->GetUnscaledCapsuleHalfHeight();
		}
	}

	if (Move)
	{
		Move->StopMovementImmediately();
		Move->bWantsToCrouch = false;
	}
	if (Capsule && Capsule->GetUnscaledCapsuleHalfHeight() != StandingHalf)
	{
		Capsule->SetCapsuleSize(Capsule->GetUnscaledCapsuleRadius(), StandingHalf, false);
	}
	bIsCrouched = false;

	FVector Target = Location;
	if (bWasCrouched)
	{
		// 印台记下的是蹲着时的胶囊中心。先抬回站立高度，再让移动组件蹲下去，退回时才不会嵌进地板。
		Target.Z += FMath::Max(0.f, StandingHalf - CrouchedHalf);
	}
	SetActorLocation(Target, false, nullptr, ETeleportType::TeleportPhysics);
	if (Move)
	{
		Move->SetMovementMode(MOVE_Walking);
	}
	if (Controller)
	{
		Controller->SetControlRotation(View);
	}
	Battery = FMath::Clamp(InBattery, 0.f, 100.f);
	Stamina = FMath::Clamp(InStamina, 0.f, MaxStamina);
	bExhausted = Stamina < MaxStamina * 0.4f;
	bFlashlightOn = bLightOn && Battery > 1.f;
	bLeanLeft = false;
	bLeanRight = false;
	LeanAlpha = 0.f;
	if (Camera)
	{
		Camera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
		Camera->SetWorldRotation(View);
	}
	if (bWasCrouched)
	{
		Crouch();
	}
	UpdateLean(0.f);
}

void ANightSchoolCharacter::BuildBody()
{
	if (BodyRoot || !NightSchoolArt::Cube())
	{
		return;
	}
	BodyRoot = NewObject<USceneComponent>(this, TEXT("BodyRoot"));
	BodyRoot->SetupAttachment(GetCapsuleComponent());
	BodyRoot->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
	BodyRoot->RegisterComponent();

	const FLinearColor Skin(0.62f, 0.45f, 0.36f);
	const FLinearColor Hair(0.06f, 0.05f, 0.045f);
	const FLinearColor Jacket(0.12f, 0.18f, 0.32f);
	const FLinearColor Shirt(0.82f, 0.82f, 0.78f);
	const FLinearColor Trousers(0.10f, 0.11f, 0.13f);
	const FLinearColor Shoes(0.05f, 0.05f, 0.05f);
	const FLinearColor Glass(0.02f, 0.02f, 0.02f);
	const FLinearColor Pack(0.16f, 0.17f, 0.18f);
	const FLinearColor Book(0.55f, 0.16f, 0.12f);

	auto AddPart = [this](UStaticMesh* Mesh, const FVector& FromFeet, const FVector& Scale, const FLinearColor& Color, bool bHead)
	{
		if (!Mesh || !BodyRoot)
		{
			return;
		}
		UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this);
		Part->SetStaticMesh(Mesh);
		Part->SetupAttachment(BodyRoot);
		Part->SetRelativeLocation(FromFeet);
		Part->SetRelativeScale3D(Scale);
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetCastShadow(false);
		if (UMaterialInstanceDynamic* Mid = NightSchoolArt::MakeColor(this, Color))
		{
			Part->SetMaterial(0, Mid);
		}
		Part->RegisterComponent();
		if (bHead)
		{
			HeadParts.Add(Part);
		}
	};

	AddPart(NightSchoolArt::Cube(), FVector(2.f, 0.f, 3.f), FVector(0.30f, 0.22f, 0.06f), Shoes, false);
	AddPart(NightSchoolArt::Cube(), FVector(0.f, 0.f, 46.f), FVector(0.18f, 0.24f, 0.80f), Trousers, false);
	AddPart(NightSchoolArt::Cube(), FVector(0.f, 0.f, 108.f), FVector(0.20f, 0.38f, 0.52f), Jacket, false);
	AddPart(NightSchoolArt::Cube(), FVector(8.f, 0.f, 128.f), FVector(0.04f, 0.16f, 0.10f), Shirt, false);
	AddPart(NightSchoolArt::Cube(), FVector(0.f, -26.f, 108.f), FVector(0.10f, 0.10f, 0.46f), Jacket, false);
	AddPart(NightSchoolArt::Cube(), FVector(0.f, 26.f, 108.f), FVector(0.10f, 0.10f, 0.46f), Jacket, false);
	AddPart(NightSchoolArt::Cube(), FVector(2.f, -26.f, 82.f), FVector(0.08f, 0.08f, 0.08f), Skin, false);
	AddPart(NightSchoolArt::Cube(), FVector(8.f, 22.f, 84.f), FVector(0.08f, 0.08f, 0.08f), Skin, false);
	AddPart(NightSchoolArt::Cube(), FVector(14.f, 24.f, 86.f), FVector(0.14f, 0.10f, 0.02f), Book, false);
	AddPart(NightSchoolArt::Cube(), FVector(-16.f, 0.f, 112.f), FVector(0.12f, 0.28f, 0.36f), Pack, false);
	AddPart(NightSchoolArt::Cube(), FVector(0.f, 0.f, 137.f), FVector(0.10f, 0.10f, 0.10f), Skin, true);
	AddPart(NightSchoolArt::Sphere(), FVector(0.f, 0.f, 152.f), FVector(0.24f), Skin, true);
	AddPart(NightSchoolArt::Sphere(), FVector(-2.f, 0.f, 158.f), FVector(0.25f), Hair, true);
	AddPart(NightSchoolArt::Sphere(), FVector(8.f, 0.f, 150.f), FVector(0.20f), Hair, true);
	AddPart(NightSchoolArt::Cube(), FVector(-7.f, 0.f, 154.f), FVector(0.02f, 0.16f, 0.02f), Glass, true);
}

void ANightSchoolCharacter::UpdateLean(float DeltaSeconds)
{
	if (!Camera)
	{
		return;
	}
	float DesiredLean = 0.f;
	if (!bPreviewing && !bReading && !bEnded)
	{
		if (bLeanLeft && !bLeanRight)
		{
			DesiredLean = -1.f;
		}
		else if (bLeanRight && !bLeanLeft)
		{
			DesiredLean = 1.f;
		}
	}
	LeanAlpha = FMath::FInterpTo(LeanAlpha, DesiredLean, DeltaSeconds, 8.f);

	float Fear01 = 0.f;
	if (const ANightSchoolDirector* Director = ANightSchoolDirector::Find(GetWorld()))
	{
		Fear01 = FMath::Clamp(Director->GetFear() / 100.f, 0.f, 1.f);
	}
	FRotator Control = Controller ? Controller->GetControlRotation() : GetActorRotation();
	if (Controller)
	{
		const float ClampedPitch = FMath::ClampAngle(Control.Pitch, -48.f, 28.f);
		if (!FMath::IsNearlyEqual(FRotator::NormalizeAxis(Control.Pitch), ClampedPitch, 0.05f))
		{
			Control.Pitch = ClampedPitch;
			Controller->SetControlRotation(Control);
		}
	}
	const FRotator YawOnly(0.f, Control.Yaw, 0.f);
	const FVector Forward = YawOnly.Vector();
	const FVector Right = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y);
	const float Time = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	const float Arm = FMath::Lerp(104.f, 76.f, Fear01);
	const float Height = bIsCrouched ? 40.f : 60.f;
	// 射线从胶囊内部发出。肩膀外侧贴墙时会穿进墙里，从那里再探就找不到墙。
	const FVector Safe = GetActorLocation() + FVector(0.f, 0.f, Height);
	const FVector Aim = Safe + Right * (30.f + LeanAlpha * 42.f) + Forward * (FMath::Abs(LeanAlpha) * 14.f);
	auto PullCamera = [this](const FVector& From, const FVector& To)
	{
		if (!GetWorld())
		{
			return To;
		}
		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(NightShoulder), false, this);
		if (GetWorld()->LineTraceSingleByChannel(Hit, From, To, ECC_Visibility, Params))
		{
			return Hit.ImpactPoint + Hit.ImpactNormal * 16.f;
		}
		return To;
	};
	FVector Desired = PullCamera(Safe, Aim - Forward * Arm + FVector(0.f, 0.f, 8.f + FMath::Sin(Time * 2.1f) * Fear01 * 3.2f));
	if (FVector::DistSquared(Desired, Safe) < FMath::Square(42.f))
	{
		Desired = PullCamera(Safe, Safe - Forward * 28.f + Right * 34.f);
	}
	if (!bViewReady)
	{
		SmoothedView = Desired;
		bViewReady = true;
	}
	else if (GetWorld())
	{
		const FVector Next = FMath::VInterpTo(SmoothedView, Desired, DeltaSeconds, 8.f);
		FHitResult Block;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(NightShoulderSmooth), false, this);
		if (GetWorld()->LineTraceSingleByChannel(Block, SmoothedView, Next, ECC_Visibility, Params))
		{
			SmoothedView = Desired;
		}
		else
		{
			SmoothedView = Next;
		}
	}
	Camera->SetWorldLocation(SmoothedView);
	FRotator View = Control;
	View.Roll = LeanAlpha * -8.f + FMath::Sin(Time * 1.4f) * Fear01 * 1.6f;
	Camera->SetWorldRotation(View);

	const bool bShowHead = FVector::Dist(SmoothedView, Safe) > 46.f;
	for (UStaticMeshComponent* Part : HeadParts)
	{
		if (Part)
		{
			Part->SetVisibility(bShowHead);
		}
	}
	if (BodyRoot)
	{
		float Bob = 0.f;
		if (GetVelocity().SizeSquared() > 80.f)
		{
			Bob = FMath::Sin(Time * (bIsCrouched ? 6.f : 9.5f)) * (bIsCrouched ? 0.7f : 1.7f);
		}
		const float Half = GetCapsuleComponent() ? GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() : 88.f;
		BodyRoot->SetRelativeLocation(FVector(0.f, 0.f, -Half + Bob));
	}
	if (Flashlight)
	{
		const FVector Chest = GetActorLocation() + Forward * 34.f + Right * 14.f + FVector(0.f, 0.f, bIsCrouched ? 28.f : 46.f);
		Flashlight->SetWorldLocation(Chest);
		Flashlight->SetWorldRotation(View);
	}
}

void ANightSchoolCharacter::LeanLeftStarted()
{
	bLeanLeft = !bPreviewing && !bReading && !bEnded;
}

void ANightSchoolCharacter::LeanLeftStopped()
{
	bLeanLeft = false;
}

void ANightSchoolCharacter::LeanRightStarted()
{
	bLeanRight = !bPreviewing && !bReading && !bEnded;
}

void ANightSchoolCharacter::LeanRightStopped()
{
	bLeanRight = false;
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
	LeanLeftAction = NewObject<UInputAction>(this);
	LeanRightAction = NewObject<UInputAction>(this);

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
	MappingContext->MapKey(InteractAction, EKeys::G);
	MappingContext->MapKey(InteractAction, EKeys::Gamepad_FaceButton_Bottom);
	MappingContext->MapKey(FlashlightAction, EKeys::F);
	MappingContext->MapKey(FlashlightAction, EKeys::Gamepad_FaceButton_Left);
	MappingContext->MapKey(CloseAction, EKeys::Tab);
	MappingContext->MapKey(CloseAction, EKeys::Escape);
	MappingContext->MapKey(CloseAction, EKeys::Gamepad_FaceButton_Right);
	MappingContext->MapKey(LeanLeftAction, EKeys::Q);
	MappingContext->MapKey(LeanLeftAction, EKeys::Gamepad_DPad_Left);
	MappingContext->MapKey(LeanRightAction, EKeys::E);
	MappingContext->MapKey(LeanRightAction, EKeys::Gamepad_DPad_Right);
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
		Enhanced->BindAction(JumpAction, ETriggerEvent::Started, this, &ANightSchoolCharacter::TryJump);
		Enhanced->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		Enhanced->BindAction(LeanLeftAction, ETriggerEvent::Started, this, &ANightSchoolCharacter::LeanLeftStarted);
		Enhanced->BindAction(LeanLeftAction, ETriggerEvent::Completed, this, &ANightSchoolCharacter::LeanLeftStopped);
		Enhanced->BindAction(LeanLeftAction, ETriggerEvent::Canceled, this, &ANightSchoolCharacter::LeanLeftStopped);
		Enhanced->BindAction(LeanRightAction, ETriggerEvent::Started, this, &ANightSchoolCharacter::LeanRightStarted);
		Enhanced->BindAction(LeanRightAction, ETriggerEvent::Completed, this, &ANightSchoolCharacter::LeanRightStopped);
		Enhanced->BindAction(LeanRightAction, ETriggerEvent::Canceled, this, &ANightSchoolCharacter::LeanRightStopped);
	}
	else
	{
		UE_LOG(LogStudent, Error, TEXT("里屋：没有 Enhanced Input，移动不会生效。"));
	}
	AttachHud();
}

void ANightSchoolCharacter::Move(const FInputActionValue& Value)
{
	if (bReading || bPreviewing || bEnded || !Controller)
	{
		return;
	}
	const FVector2D Axis = Value.Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), Axis.Y);
	AddMovementInput(GetActorRightVector(), Axis.X);
}

void ANightSchoolCharacter::Look(const FInputActionValue& Value)
{
	if (!Controller || bPreviewing || bReading || bEnded)
	{
		return;
	}
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
	FRotator Rot = Controller->GetControlRotation();
	Rot.Pitch = FMath::ClampAngle(Rot.Pitch, -48.f, 28.f);
	Controller->SetControlRotation(Rot);
}

void ANightSchoolCharacter::SprintStarted()
{
	bSprintHeld = !bReading && !bPreviewing && !bEnded;
}

void ANightSchoolCharacter::SprintStopped()
{
	bSprintHeld = false;
}

void ANightSchoolCharacter::CrouchStarted()
{
	if (!bReading && !bPreviewing && !bEnded)
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
	if (bReading || bPreviewing || bEnded)
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
		if (Director->IsPreviewing())
		{
			Director->SkipPreview();
			return;
		}
		Director->CloseNote();
	}
	bReading = false;
}

void ANightSchoolCharacter::TryJump()
{
	if (bPreviewing || bReading || bEnded)
	{
		return;
	}
	Jump();
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
