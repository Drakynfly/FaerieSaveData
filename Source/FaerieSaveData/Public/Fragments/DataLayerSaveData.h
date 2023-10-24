// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieSaveSlotFragment.h"
#include "WorldPartition/DataLayer/DataLayerAsset.h"

#include "DataLayerSaveData.generated.h"

USTRUCT(BlueprintType)
struct FDataLayerActivation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Later Activation")
	TSoftObjectPtr<const UDataLayerAsset> DataLayerAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Later Activation")
	bool Recursive = false;

	friend bool operator==(const FDataLayerActivation& Lhs, const FDataLayerActivation& RHS)
	{
		return Lhs.DataLayerAsset == RHS.DataLayerAsset
			   && Lhs.Recursive == RHS.Recursive;
	}

	friend bool operator!=(const FDataLayerActivation& Lhs, const FDataLayerActivation& RHS)
	{
		return !(Lhs == RHS);
	}
};
USTRUCT()
struct FFaerieSavedDataLayerActivation : public FFaerieSaveSlotDataFragment
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FDataLayerActivation> ActivatedDataLayers;
};