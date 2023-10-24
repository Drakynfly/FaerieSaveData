// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "FaerieSaveDataSettings.generated.h"

class USaveSystemInteropBase;
class UFaerieSaveCommand;
class UFaerieLoadCommand;

/**
 *
 */
UCLASS(config = Project, DefaultConfig, meta = (DisplayName = "Timelines"))
class FAERIESAVEDATA_API UFaerieSaveDataSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UFaerieSaveDataSettings();

	// UDeveloperSettings implementation
	virtual FName GetCategoryName() const override;
	// End UDeveloperSettings implementation

	// This is the default save servive used for saving slot data.
	UPROPERTY(config, EditAnywhere, NoClear)
	TSoftClassPtr<USaveSystemInteropBase> BackendSystemClass;

	// This is the save service used for saving persistance global data not tied to any slot.
	UPROPERTY(config, EditAnywhere, NoClear)
	TSoftClassPtr<USaveSystemInteropBase> PersistanceServiceClass;

	UPROPERTY(config, EditAnywhere, NoClear)
	TSoftClassPtr<UFaerieSaveCommand> SaveExecClass;

	UPROPERTY(config, EditAnywhere, NoClear)
	TSoftClassPtr<UFaerieLoadCommand> LoadExecClass;
};