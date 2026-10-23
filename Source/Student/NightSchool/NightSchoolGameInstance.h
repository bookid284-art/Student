#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NightSchoolGameInstance.generated.h"

/** 不改示例关卡的 GameMode 资产，进 Play 时自己把这一夜搭起来。 */
UCLASS()
class STUDENT_API UNightSchoolGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void OnStart() override;

private:
	void OnPostLoadMap(UWorld* LoadedWorld);
	void StartExperience();
	void SuppressTemplateWorld(UWorld* World);

	FTimerHandle StartTimer;
	TWeakObjectPtr<UWorld> ActiveWorld;
	TWeakObjectPtr<UWorld> BuiltWorld;
	bool bBuilding = false;
	int32 RetryCount = 0;
};
