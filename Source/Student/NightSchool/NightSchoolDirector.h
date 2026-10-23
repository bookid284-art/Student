#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NightSchoolTypes.h"
#include "NightSchoolDirector.generated.h"

class ANightSchoolApparition;
class ANightSchoolCharacter;
class ANightSchoolProp;
class APointLight;
class UAudioComponent;
class USceneComponent;
class USoundWaveProcedural;
class UStaticMeshComponent;
class UTexture2D;

UCLASS()
class STUDENT_API ANightSchoolDirector : public AActor
{
	GENERATED_BODY()

public:
	ANightSchoolDirector();

	virtual void Tick(float DeltaSeconds) override;

	void BuildWorld();
	void HandleProp(ANightSchoolProp* Prop);
	void OnCaughtByApparition();
	void CloseNote();

	bool IsNoteOpen() const { return bNoteOpen; }
	bool IsEnding() const { return CurrentEnding != ENightEnding::None; }
	bool IsWearingEarplugs() const { return bWearingEarplugs; }
	bool GetLurePoint(const FBox& Room, FVector& OutPoint) const;
	const FString& GetNoteTitle() const { return NoteTitle; }
	const FString& GetNoteBody() const { return NoteBody; }
	const FString& GetStatusLine() const { return StatusLine; }
	const FString& GetEndingTitle() const { return EndingTitle; }
	const FString& GetEndingBody() const { return EndingBody; }
	float GetFear() const { return Fear; }
	ENightEnding GetEnding() const { return CurrentEnding; }
	FString GetObjective() const;
	FString GetFacingLine() const;
	FString GetItemsLine() const;

	static ANightSchoolDirector* Find(UWorld* World);

private:
	FVector W(float X, float Y, float Z) const;
	UStaticMeshComponent* AddBox(const FVector& Center, const FVector& Size, const FLinearColor& Color, bool bShadow, bool bCollision);
	void AddSphere(const FVector& Center, float Radius, const FLinearColor& Color, bool bCollision);
	void AddCylinder(const FVector& Center, float Radius, float Height, const FRotator& Rotation, const FLinearColor& Color, bool bCollision);
	void AddLabel(const FVector& Location, float Yaw, const FString& Text, float WorldSize, const FColor& Color);
	void AddLantern(float X, float Y, float Z, float Intensity, bool bCalm);
	void AddSprite(const FVector& Location, float Yaw, UTexture2D* Texture, float WidthCm, float HeightCm);
	ANightSchoolProp* SpawnProp(ENightPropId Id, const FVector& Location, const FVector& Extent, const FLinearColor& Color, const FText& Prompt, const FVector& Slide, float LabelYaw, const FString& Label);
	ANightSchoolProp* FindProp(ENightPropId Id) const;
	void SpawnGhost(const FBox& Room, const TArray<FVector>& Points, const FString& Glyph);
	FBox HallRoom() const;
	FBox ClassRoom() const;
	bool IsInDorm(const FVector& Location) const;
	void RefreshEarplugPrompt();
	void SpawnHallGhosts();

	void ShowNote(const TCHAR* InTitle, const TCHAR* InBody);
	void ShowStatus(const FString& Line, float Seconds);
	void TriggerEnding(ENightEnding Ending);
	ANightSchoolCharacter* Player() const;
	void QueueDrone(float Seconds);
	bool IsInWarmLight(const FVector& Location) const;

	void BuildShell();
	void BuildStructure();
	void BuildDressing();
	void BuildLights();
	void BuildAudio();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY()
	TArray<TObjectPtr<ANightSchoolProp>> Props;

	UPROPERTY()
	TArray<TObjectPtr<ANightSchoolApparition>> Apparitions;

	UPROPERTY()
	TArray<TObjectPtr<APointLight>> FlickerLights;

	UPROPERTY()
	TArray<TObjectPtr<UTexture2D>> OwnedTextures;

	UPROPERTY()
	TObjectPtr<USoundWaveProcedural> DroneWave;

	UPROPERTY()
	TObjectPtr<UAudioComponent> DroneAudio;

	TArray<float> FlickerBase;
	TArray<float> FlickerPhase;
	TArray<FVector> WarmLights;

	FString NoteTitle;
	FString NoteBody;
	FString StatusLine;
	FString EndingTitle;
	FString EndingBody;
	FVector LurePoint = FVector::ZeroVector;

	ENightEnding CurrentEnding = ENightEnding::None;
	float Fear = 0.f;
	float FearLock = 0.f;
	float StatusTime = 0.f;
	float AudioTime = 0.f;
	float DronePhase = 0.f;
	float HeartPhase = 0.f;
	float LureTime = 0.f;
	bool bWorldBuilt = false;
	bool bNoteOpen = false;
	bool bPhoneRead = false;
	bool bHasEarplugs = false;
	bool bWearingEarplugs = false;
	bool bHasCloth = false;
	bool bCameraCovered = false;
	bool bHallOpen = false;
	bool bHasSoda = false;
	bool bSatAlone = false;
	bool bHasExam = false;
	bool bExamGiven = false;
	bool bOfficeOpen = false;
	bool bTeacherRead = false;
	bool bInfirmaryOpen = false;
	bool bZhouHeard = false;
	bool bLightOpen = false;
	bool bClosetOpen = false;
	bool bGhostsSpawned = false;
	bool bPressureWarned = false;
};
