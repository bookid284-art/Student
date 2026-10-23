#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NightSchoolTypes.h"
#include "NightSchoolDirector.generated.h"

class ACameraActor;
class ANightSchoolApparition;
class ANightSchoolCharacter;
class ANightSchoolProp;
class APointLight;

/** 第一次走进房间时，镜头要扫过的范围。 */
struct FNightRoomView
{
	FName Id;
	FBox Bounds;
	FString Name;
	bool bSeen = false;
};
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
	bool IsPreviewing() const { return bPreviewing; }
	bool IsEnding() const { return CurrentEnding != ENightEnding::None; }
	void SkipPreview();
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
	FString GetChapterLine() const;
	FString GetPreviewLine() const;
	FString GetFacingLine() const;
	FString GetThreadsLine() const;
	FString GetItemsLine() const;
	int32 GetChapterIndex() const;

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
	bool AnyGhostContains(const FVector& Point) const;
	void RefreshEarplugPrompt();
	void SpawnHallGhosts();
	void SpawnShopGhost();
	void SpawnNoticeGhost();
	void SpawnComputerGhost();
	void SpawnAnnexGhost();
	int32 DutyCount() const;
	int32 NoteCount() const;
	int32 MinutesCount() const;
	int32 NoticeCount() const;
	int32 EssayCount() const;
	int32 CallCount() const;
	int32 ClassCount() const;
	int32 ReceiptCount() const;
	int32 DraftCount() const;
	int32 ChartCount() const;
	bool HasAllItems() const;
	void GrantSeals();
	void TryOpenGrade();
	void BuildRooms();
	void BuildSelfFigure();
	void TickPreview(float DeltaSeconds);
	void TryBeginPreview();
	void BeginRoomPreview(FNightRoomView& Room);

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
	bool bDutyA = false;
	bool bDutyB = false;
	bool bDutyC = false;
	bool bCoin = false;
	bool bCoinSpent = false;
	bool bNoteSeat = false;
	bool bNoteHall = false;
	bool bNoteShop = false;
	bool bNoteLocker = false;
	bool bShopOpen = false;
	bool bLedgerHeld = false;
	bool bLedgerPosted = false;
	bool bGradeOpen = false;
	bool bMinSun = false;
	bool bMinLiang = false;
	bool bMinParent = false;
	bool bNoticeA = false;
	bool bNoticeB = false;
	bool bNoticeC = false;
	bool bNoticeD = false;
	bool bHasPencil = false;
	bool bHasBus = false;
	bool bHasTime = false;
	bool bItemsReturned = false;
	bool bZhouIntro = false;
	bool bComputerOpen = false;
	bool bEssayDorm = false;
	bool bEssayShop = false;
	bool bEssayGrade = false;
	bool bEssaysKept = false;
	bool bShopGhost = false;
	bool bNoticeGhost = false;
	bool bComputerGhost = false;
	bool bShopSodaTaken = false;
	bool bSpareTaken = false;
	bool bCallA = false;
	bool bCallB = false;
	bool bCallC = false;
	bool bClassA = false;
	bool bClassB = false;
	bool bClassC = false;
	bool bReceiptA = false;
	bool bReceiptB = false;
	bool bReceiptC = false;
	bool bDraftA = false;
	bool bDraftB = false;
	bool bDraftC = false;
	bool bChartA = false;
	bool bChartB = false;
	bool bChartC = false;
	bool bNoticeE = false;
	bool bNoticeF = false;
	bool bNoticeG = false;
	bool bNoticeH = false;
	bool bSeal1Held = false;
	bool bSeal1Set = false;
	bool bSeal2Held = false;
	bool bSeal2Set = false;
	bool bSeal3Held = false;
	bool bSeal3Set = false;
	bool bSeal4Held = false;
	bool bSeal4Set = false;
	bool bSeal5Held = false;
	bool bSeal5Set = false;
	bool bStoreOpen = false;
	bool bAnnexOpen = false;
	bool bAnnexGhost = false;
	bool bSawSelf = false;
	bool bPreviewing = false;
	bool bPreviewReturning = false;

	TArray<FNightRoomView> RoomViews;
	TArray<FVector> PreviewPoints;
	FVector PreviewLook = FVector::ZeroVector;
	FString PreviewCaption;
	FString ChapterBanner;
	int32 ShownChapter = 0;
	float ChapterBannerTime = 0.f;
	float PreviewTime = 0.f;
	float PreviewDuration = 1.f;
	float PreviewReturnLeft = 0.f;
	float PreviewHold = 1.f;

	UPROPERTY()
	TObjectPtr<ACameraActor> PreviewCamera;
};
