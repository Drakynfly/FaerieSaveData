// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieSaveSlotFragment.h"
#include "GameSessionSaveData.generated.h"

USTRUCT(BlueprintType)
struct FFaerieSessionArgs : public FFaerieSaveSlotInfoFragment
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Faerie|SessionArgs")
	FPrimaryAssetId ActiveExperience;

	UPROPERTY(BlueprintReadOnly, Category = "Faerie|SessionArgs")
	FName Level;

	UPROPERTY(BlueprintReadOnly, Category = "Faerie|SessionArgs")
	TMap<FString, FString> SessionArgs;
};