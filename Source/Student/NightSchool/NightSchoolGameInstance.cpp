#include "NightSchool/NightSchoolGameInstance.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/LightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Engine.h"
#include "Engine/Light.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "NightSchool/NightSchoolCharacter.h"
#include "NightSchool/NightSchoolDirector.h"
#include "NightSchool/NightSchoolLayout.h"
#include "Student.h"
#include "TimerManager.h"

void UNightSchoolGameInstance::Init()
{
	Super::Init();
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UNightSchoolGameInstance::OnPostLoadMap);
}

void UNightSchoolGameInstance::Shutdown()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	Super::Shutdown();
}

void UNightSchoolGameInstance::OnStart()
{
	Super::OnStart();
	if (UWorld* World = GetWorld())
	{
		OnPostLoadMap(World);
	}
}

void UNightSchoolGameInstance::OnPostLoadMap(UWorld* LoadedWorld)
{
	if (!LoadedWorld || !LoadedWorld->IsGameWorld())
	{
		return;
	}
	if (LoadedWorld->WorldType != EWorldType::Game && LoadedWorld->WorldType != EWorldType::PIE && LoadedWorld->WorldType != EWorldType::GamePreview)
	{
		return;
	}
	ActiveWorld = LoadedWorld;
	RetryCount = 0;
	LoadedWorld->GetTimerManager().ClearTimer(StartTimer);
	LoadedWorld->GetTimerManager().SetTimer(StartTimer, this, &UNightSchoolGameInstance::StartExperience, 0.2f, false);
}

void UNightSchoolGameInstance::StartExperience()
{
	UWorld* World = ActiveWorld.Get();
	if (!World)
	{
		return;
	}
	if (BuiltWorld.Get() == World && ANightSchoolDirector::Find(World))
	{
		return;
	}
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	if (!PlayerController)
	{
		if (++RetryCount > 40)
		{
			UE_LOG(LogStudent, Error, TEXT("里屋：没有等到玩家控制器。"));
			return;
		}
		World->GetTimerManager().SetTimer(StartTimer, this, &UNightSchoolGameInstance::StartExperience, 0.1f, false);
		return;
	}
	if (bBuilding)
	{
		return;
	}
	bBuilding = true;

	SuppressTemplateWorld(World);
	if (AWorldSettings* Settings = World->GetWorldSettings())
	{
		Settings->KillZ = -100000.f;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ANightSchoolDirector* Director = World->SpawnActor<ANightSchoolDirector>(ANightSchoolDirector::StaticClass(), FTransform::Identity, Params);
	if (Director)
	{
		Director->BuildWorld();
	}

	const FTransform SpawnTransform(NightSchool::PlayerFacing(), NightSchool::PlayerSpawn());
	ANightSchoolCharacter* Player = World->SpawnActor<ANightSchoolCharacter>(ANightSchoolCharacter::StaticClass(), SpawnTransform, Params);
	if (Player && PlayerController)
	{
		APawn* Previous = PlayerController->GetPawn();
		PlayerController->Possess(Player);
		PlayerController->SetControlRotation(NightSchool::PlayerFacing());
		if (Previous && Previous != Player)
		{
			Previous->Destroy();
		}
	}

	BuiltWorld = World;
	bBuilding = false;
	UE_LOG(LogStudent, Log, TEXT("里屋：幻觉开始。"));
}

void UNightSchoolGameInstance::SuppressTemplateWorld(UWorld* World)
{
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor->Tags.Contains(TEXT("NightSchool")))
		{
			continue;
		}
		if (Actor->IsA<APlayerController>() || Actor->IsA<APlayerCameraManager>() || Actor->IsA<AGameModeBase>() || Actor->IsA<AGameStateBase>() || Actor->IsA<APlayerState>() || Actor->IsA<AHUD>() || Actor->IsA<AWorldSettings>())
		{
			continue;
		}
		if (Actor->IsA<ANightSchoolCharacter>() || Actor->IsA<ANightSchoolDirector>())
		{
			continue;
		}

		if (ALight* Light = Cast<ALight>(Actor))
		{
			if (ULightComponent* Component = Light->GetLightComponent())
			{
				Component->SetIntensity(0.f);
				Component->SetVisibility(false);
			}
		}
		if (ASkyLight* Sky = Cast<ASkyLight>(Actor))
		{
			if (USkyLightComponent* SkyComponent = Sky->GetLightComponent())
			{
				SkyComponent->SetIntensity(0.f);
			}
		}
		if (APostProcessVolume* Post = Cast<APostProcessVolume>(Actor))
		{
			Post->bEnabled = false;
			Post->BlendWeight = 0.f;
		}
		Actor->SetActorHiddenInGame(true);
		Actor->SetActorEnableCollision(false);
	}
}
