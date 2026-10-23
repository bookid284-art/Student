#pragma once

#include "CoreMinimal.h"

/** 里屋抬到模板关卡上方，避免和第一人称示例几何叠在一起。 */
namespace NightSchool
{
	inline FVector Origin()
	{
		return FVector(0.f, 0.f, 20000.f);
	}

	/** 胶囊半高 88，地面顶面相对高度 40。出生在前厅，面朝宿舍和走廊。 */
	inline FVector PlayerSpawn()
	{
		return Origin() + FVector(0.f, -400.f, 132.f);
	}

	inline FRotator PlayerFacing()
	{
		return FRotator(0.f, 90.f, 0.f);
	}
}
