// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "UnlockableAssetBase.h"
#include "UnlockableExperience.generated.h"

/**
 * An additional gameplay experience unlockable through gameplay.
 */
UCLASS()
class FAERIESAVEDATA_API UUnlockablePublicExperience : public UUnlockableAssetBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Unlockable Experience",
		meta = (AllowedTypes = "PublicExperienceDefinition"))
	FPrimaryAssetId ExperienceID;
};

/**
 * An additional main menu experience unlockable through gameplay.
 */
UCLASS()
class FAERIESAVEDATA_API UUnlockableMainMenuExperience : public UUnlockableAssetBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Unlockable Experience",
		meta = (AllowedTypes = "MainMenuExperience"))
	FPrimaryAssetId ExperienceID;
};