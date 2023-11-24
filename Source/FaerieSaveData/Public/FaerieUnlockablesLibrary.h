// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieSaveSlotFragment.h"
#include "StructView.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FaerieUnlockablesLibrary.generated.h"

class UUnlockableAssetBase;

USTRUCT(BlueprintType)
struct FAERIESAVEDATA_API FUnlocksArray
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Unlocks Array")
	TArray<TSoftObjectPtr<UUnlockableAssetBase>> Unlocks;
};

USTRUCT()
struct FAERIESAVEDATA_API FFaerieLocalUnlocks : public FFaerieSaveSlotDataFragment
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TSoftClassPtr<UUnlockableAssetBase>, FUnlocksArray> UnlockedFeatures;

	FConstStructView FindUnlocksArray(const TSoftClassPtr<UUnlockableAssetBase>& Class) const;
	FStructView FindUnlocksArray(const TSoftClassPtr<UUnlockableAssetBase>& Class);
	FUnlocksArray& FindOrAddUnlocksArray(const TSoftClassPtr<UUnlockableAssetBase>& Class);
};

/**
 *
 */
UCLASS()
class FAERIESAVEDATA_API UFaerieUnlockablesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Unlocked Features", meta = (WorldContext = Obj))
	static bool IsFeatureUnlocked(const UObject* Obj, const TSoftObjectPtr<UUnlockableAssetBase>& Feature);

	UFUNCTION(BlueprintCallable, Category = "Unlocked Features", meta = (WorldContext = Obj))
	static FUnlocksArray GetUnlockedFeaturesOfClass(const UObject* Obj, const TSoftClassPtr<UUnlockableAssetBase>& Feature);

	UFUNCTION(BlueprintCallable, Category = "Unlocked Features", meta = (WorldContext = Obj))
	static void AddUnlockedFeature(UObject* Obj, TSoftObjectPtr<UUnlockableAssetBase> Feature);

	UFUNCTION(BlueprintCallable, Category = "Unlocked Features", meta = (WorldContext = Obj))
	static void RemoveUnlockedFeature(UObject* Obj, TSoftObjectPtr<UUnlockableAssetBase> Feature);
};