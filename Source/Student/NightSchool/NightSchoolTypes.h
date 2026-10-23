#pragma once

#include "CoreMinimal.h"
#include "NightSchoolTypes.generated.h"

/** 被记住或被涂掉的名字。名册只能交还学员，自己的名字只能自己写对。 */
UENUM(BlueprintType)
enum class ENightName : uint8
{
	LinQiuhe,
	ZhaoTiesheng,
	ZhouMian,
	ChenWangshu
};

/** 夜里能带走的东西。没有背包格子，只有有或没有。 */
UENUM(BlueprintType)
enum class ENightItem : uint8
{
	OldKey,
	Candle,
	RedScarf,
	Register,
	LastPaper,
	Battery
};

/** 结局。每一种都把记录者自己也写进代价里。 */
UENUM(BlueprintType)
enum class ENightEnding : uint8
{
	None,
	Publication,
	BurnRegister,
	Stay,
	Reply,
	HalfPage,
	Nameless
};

/** 可交互物的身份。门、文书、选择都走同一套交互接口。 */
UENUM(BlueprintType)
enum class ENightPropId : uint8
{
	FrontDoor,
	ExitStreet,
	Notice,
	SignInBook,
	EssayLin,
	EssayZhao,
	EssayZhou,
	OldKey,
	OfficeDoor,
	Confession,
	Candle,
	RedScarf,
	Battery,
	AuditoriumDoor,
	NameTrue,
	NameWrongA,
	NameWrongB,
	ScarfHook,
	BoilerDoor,
	Register,
	LastPaper,
	GardenDoor,
	Thesis,
	Brazier,
	Chair,
	Mailbox
};
