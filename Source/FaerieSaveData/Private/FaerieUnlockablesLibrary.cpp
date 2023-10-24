// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieUnlockablesLibrary.h"
#include "SaveSystemInteropBase.h"

#include "UnlockableAssetBase.h"

FConstStructView FFaerieLocalUnlocks::FindUnlocksArray(const TSoftClassPtr<UUnlockableAssetBase>& Class) const
{
	return UnlockedFeatures.Contains(Class) ? FConstStructView::Make(UnlockedFeatures[Class]) : FConstStructView();
}

FStructView FFaerieLocalUnlocks::FindUnlocksArray(const TSoftClassPtr<UUnlockableAssetBase>& Class)
{
	return UnlockedFeatures.Contains(Class) ? FStructView::Make(UnlockedFeatures[Class]) : FStructView();
}

FUnlocksArray& FFaerieLocalUnlocks::FindOrAddUnlocksArray(const TSoftClassPtr<UUnlockableAssetBase>& Class)
{
	return UnlockedFeatures.FindOrAdd(Class);
}

bool UFaerieUnlockablesLibrary::IsFeatureUnlocked(UObject* Obj, const TSoftObjectPtr<UUnlockableAssetBase>& Feature)
{
	return GetUnlockedFeaturesOfClass(Obj, Feature->GetClass()).Unlocks.Contains(Feature);
}

FUnlocksArray UFaerieUnlockablesLibrary::GetUnlockedFeaturesOfClass(UObject* Obj, const TSoftClassPtr<UUnlockableAssetBase>& Feature)
{
	auto&& Service = UFaerieLocalDataSubsystem::GetService(Obj, Faerie::SaveData::PersistantService);
	if (!IsValid(Service))
	{
		return FUnlocksArray();
	}

	auto&& Unlocks = Service->GetFragmentData<FFaerieLocalUnlocks>();

	if (auto&& List = Unlocks.Get<const FFaerieLocalUnlocks>().FindUnlocksArray(Feature->GetClass());
		List.IsValid())
	{
		return List.Get<const FUnlocksArray>();
	}
	return FUnlocksArray();
}

void UFaerieUnlockablesLibrary::AddUnlockedFeature(UObject* Obj, const TSoftObjectPtr<UUnlockableAssetBase> Feature)
{
	if (Feature.IsNull()) return;

	auto&& Service = UFaerieLocalDataSubsystem::GetService(Obj, Faerie::SaveData::PersistantService);
	if (!IsValid(Service))
	{
		return;
	}

	Service->EditFragmentData<FFaerieLocalUnlocks>(FString(),
		[Feature](FFaerieLocalUnlocks& Unlocks)
		{
			FUnlocksArray& List = Unlocks.FindOrAddUnlocksArray(Feature->GetClass());
			List.Unlocks.AddUnique(Feature);
		});
}

void UFaerieUnlockablesLibrary::RemoveUnlockedFeature(UObject* Obj, const TSoftObjectPtr<UUnlockableAssetBase> Feature)
{
	if (Feature.IsNull()) return;

	auto&& Service = UFaerieLocalDataSubsystem::GetService(Obj, Faerie::SaveData::PersistantService);
	if (!IsValid(Service))
	{
		return;
	}

	Service->EditFragmentData<FFaerieLocalUnlocks>(FString(),
		[Feature](FFaerieLocalUnlocks& Unlocks)
		{
			if (auto&& List = Unlocks.FindUnlocksArray(Feature->GetClass());
				List.IsValid())
			{
				List.Get<FUnlocksArray>().Unlocks.Remove(Feature);
			}
		});
}