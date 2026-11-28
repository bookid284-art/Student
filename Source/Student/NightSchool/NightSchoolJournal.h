#pragma once

#include "CoreMinimal.h"

struct FNightSchoolJournalStep
{
	FName Id;
	FString Title;
	FString Hint;
	FString Place;
	TOptional<FVector> Destination;
	bool bDone = false;
};

struct FNightSchoolJournalEntry
{
	FName Id;
	FString Title;
	FString Summary;
	TArray<FNightSchoolJournalStep> Steps;
	int32 Chapter = 0;
	bool bLocked = false;
	bool bComplete = false;

	const FNightSchoolJournalStep* Next() const
	{
		return bLocked || bComplete ? nullptr : Steps.FindByPredicate(
			[](const FNightSchoolJournalStep& Step) { return !Step.bDone; });
	}
};

struct FNightSchoolJournalRoom
{
	FString Name;
	FBox Bounds;
};
