// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "FaerieSaveDataCommandBase.generated.h"

class USaveSystemInteropBase;

/**
 *
 */
UCLASS(Abstract, Within = SaveSystemInteropBase)
class FAERIESAVEDATA_API UFaerieSaveDataCommandBase : public UObject
{
	GENERATED_BODY()

	friend USaveSystemInteropBase;

public:
	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintCallable, Category = "Faerie|SaveDataCommand")
	void RunCommand();

protected:
	virtual void Run() PURE_VIRTUAL(UTimelinesSaveExec::Run, )

	void Succeed();
	void Fail(FStringView Reason);

protected:
	FString SlotName;

private:
	bool IsRunning = false;
};

/**
 *
 */
UCLASS(Abstract, BlueprintType)
class FAERIESAVEDATA_API UFaerieSaveCommand : public UFaerieSaveDataCommandBase
{
	GENERATED_BODY()
};

/**
 *
 */
UCLASS(Abstract, BlueprintType)
class FAERIESAVEDATA_API UFaerieLoadCommand : public UFaerieSaveDataCommandBase
{
	GENERATED_BODY()
};