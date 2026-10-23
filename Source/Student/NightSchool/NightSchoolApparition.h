#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NightSchoolApparition.generated.h"

class USkeletalMesh;
class UAnimationAsset;

/** 没有脸的同学。不是战斗单位，碰到人只把压力贴上来，再退开。 */
UCLASS()
class STUDENT_API ANightSchoolApparition : public ACharacter
{
	GENERATED_BODY()

public:
	ANightSchoolApparition();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	void Configure(const FBox& InRoom, const TArray<FVector>& InWaypoints, const FString& InWrongGlyph);

private:
	bool CanSeePlayer(class ANightSchoolCharacter* Player) const;
	class ANightSchoolCharacter* FindPlayer() const;

	FBox Room = FBox(EForceInit::ForceInit);
	TArray<FVector> Waypoints;
	int32 WaypointIndex = 0;
	float StunRemaining = 0.f;
	float HitCooldown = 0.f;
	float LoseSightTime = 0.f;
	bool bChasing = false;
	bool bWasMoving = false;
	FString WrongGlyph;

	UPROPERTY()
	TObjectPtr<UAnimationAsset> WalkAnim;

	UPROPERTY()
	TObjectPtr<UAnimationAsset> IdleAnim;
};
