// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "EMSCustomSaveGame.h"
#include "EMSInfoSaveGame.h"
#include "FaerieSaveSlotFragment.h"
#include "SaveSystemInteropBase.h"
#include "EasyMultiSaveInterop.generated.h"

class UEMSObject;

/**
 *
 */
UCLASS()
class EMSBACKEND_API UTimelinesEMSInfoSaveGame : public UEMSInfoSaveGame
{
	GENERATED_BODY()

public:
	FConstStructView GetFragmentData(const UScriptStruct* Type) const;
	FStructView GetMutableFragmentData(const UScriptStruct* Type);

protected:
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GameSessionSaveData")
	TArray<TInstancedStruct<FFaerieSaveSlotFragmentBase>> Fragments;
};

/**
 *
 */
UCLASS()
class EMSBACKEND_API UTimelinesEMSCustomSaveGame : public UEMSCustomSaveGame
{
	GENERATED_BODY()

public:
	UTimelinesEMSCustomSaveGame();

	FConstStructView GetFragmentData(const UScriptStruct* Type) const;
	FStructView GetMutableFragmentData(const UScriptStruct* Type);

protected:
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GameSessionSaveData")
	TArray<TInstancedStruct<FFaerieSaveSlotFragmentBase>> Fragments;
};

/**
 *
 */
UCLASS()
class EMSBACKEND_API UTimelinesEMSGlobalCustomSaveGame : public UTimelinesEMSCustomSaveGame
{
	GENERATED_BODY()

public:
	UTimelinesEMSGlobalCustomSaveGame();
};

UCLASS()
class UEasyMultiSaveInterop : public USaveSystemInteropBase
{
	GENERATED_BODY()

	UEMSObject* GetEMSInternal() const;

public:
	virtual FConstStructView GetFragmentData(const UScriptStruct* Type, FStringView Slot) const override;
	virtual void EditFragmentData(const UScriptStruct* Type, FStringView Slot, const TFunctionRef<void(FStructView)>& Edit) override;
	virtual TArray<FString> GetAllSlotsSorted() const override;
	virtual bool SaveSlot(FStringView Slot, FSaveSystemEventAsyncResult Result) override;
	virtual bool LoadSlot(FStringView Slot, FSaveSystemEventAsyncResult Result) override;
	virtual bool DeleteSlot(FStringView Slot, FSaveSystemEventAsyncResult Result) override;

private:
	UFUNCTION()
	void OnSaveComplete();

	UFUNCTION()
	void OnLoadComplete();

	UFUNCTION()
	void OnLoadFailed();

	mutable TWeakObjectPtr<UEMSObject> EasyMultiSave;

	// @todo can both of these be running at once?
	FSaveSystemEventAsyncResult SaveCommandResult;
	FSaveSystemEventAsyncResult LoadCommandResult;
};