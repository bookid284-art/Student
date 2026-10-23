#pragma once

#include "CoreMinimal.h"
#include "NightSchoolTypes.generated.h"

/** 只有两种结局。压力会涨，但不会另外把人判进第三种结尾。 */
UENUM(BlueprintType)
enum class ENightEnding : uint8
{
	None,
	InnerLight,
	Sealed
};

/** 可交互物。门、纸、道具和两个结局都走同一套交互。 */
UENUM(BlueprintType)
enum class ENightPropId : uint8
{
	Phone,
	Earplugs,
	LensCloth,
	DormCamera,
	HallDoor,
	Soda,
	LureHall,
	LureClass,
	UnwantedDesk,
	ExamPaper,
	HideScore,
	OfficeDoor,
	TeacherNote,
	InfirmaryDoor,
	Zhou,
	LightDoor,
	FinalCamera,
	FaceSwitch,
	ClosetDoor,
	ClosetLock
};
