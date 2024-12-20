// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "EasyMultiSaveInterop.h"
#include "EMSObject.h"

FConstStructView UTimelinesEMSInfoSaveGame::GetFragmentData(const UScriptStruct* Type) const
{
	for (auto&& Fragment : Fragments)
	{
		if (Fragment.GetScriptStruct() == Type)
		{
			return FConstStructView(Type, Fragment.GetMemory());
		}
	}
	return FConstStructView();
}

FStructView UTimelinesEMSInfoSaveGame::GetMutableFragmentData(const UScriptStruct* Type)
{
	for (auto&& Fragment : Fragments)
	{
		if (Fragment.GetScriptStruct() == Type)
		{
			return FStructView(Fragment.GetScriptStruct(), Fragment.GetMutableMemory());
		}
	}

	auto&& NewFragment = Fragments.AddDefaulted_GetRef();
	NewFragment.InitializeAsScriptStruct(Type);
	return FStructView(NewFragment.GetScriptStruct(), NewFragment.GetMutableMemory());
}

UTimelinesEMSCustomSaveGame::UTimelinesEMSCustomSaveGame()
{
	bUseSaveSlot = true;
	SaveGameName = "SlotFragmentData";
}

FConstStructView UTimelinesEMSCustomSaveGame::GetFragmentData(const UScriptStruct* Type) const
{
	for (auto&& Fragment : Fragments)
	{
		if (Fragment.GetScriptStruct() == Type)
		{
			return FConstStructView(Type, Fragment.GetMemory());
		}
	}
	return FConstStructView();
}

FStructView UTimelinesEMSCustomSaveGame::GetMutableFragmentData(const UScriptStruct* Type)
{
	for (auto&& Fragment : Fragments)
	{
		if (Fragment.GetScriptStruct() == Type)
		{
			return FStructView(Fragment.GetScriptStruct(), Fragment.GetMutableMemory());
		}
	}

	auto&& NewFragment = Fragments.AddDefaulted_GetRef();
	NewFragment.InitializeAsScriptStruct(Type);
	return FStructView(NewFragment.GetScriptStruct(), NewFragment.GetMutableMemory());
}

UTimelinesEMSGlobalCustomSaveGame::UTimelinesEMSGlobalCustomSaveGame()
{
	bUseSaveSlot = false;
	SaveGameName = "GlobalFragmentData";
}

UEMSObject* UEasyMultiSaveInterop::GetEMSInternal() const
{
	if (EasyMultiSave.IsValid())
	{
		return EasyMultiSave.Get();
	}

	EasyMultiSave = UEMSObject::Get(GetOuterUFaerieLocalDataSubsystem());

	if (!EasyMultiSave.IsValid())
	{
		ServiceError(TEXTVIEW("Unable to find EMS subsystem"));
	}

	return EasyMultiSave.Get();
}

TOptional<FDateTime> UEasyMultiSaveInterop::GetTimestamp(const FStringView Slot) const
{
	UEMSObject* EMS = GetEMSInternal();
	if (!IsValid(EMS))
	{
		return TOptional<FDateTime>();
	}

	if (auto&& InfoObject = EMS->GetSlotInfoObject(Slot.GetData());
		IsValid(InfoObject))
	{
		return InfoObject->SlotInfo.TimeStamp;
	}

	return TOptional<FDateTime>();
}

FConstStructView UEasyMultiSaveInterop::GetFragmentData(const UScriptStruct* Type, const FStringView Slot) const
{
	UEMSObject* EMS = GetEMSInternal();
	if (!IsValid(EMS))
	{
		return FConstStructView();
	}

	if (Type->IsChildOf(FFaerieSaveSlotInfoFragment::StaticStruct()))
	{
		if (Slot.IsEmpty())
		{
			ServiceError(TEXTVIEW("Slot is empty while trying to fetch fragment for slot info!"));
			return FConstStructView();
		}

		// EMS must have the slot as current in order to fetch per-slot custom data.
		EMS->SetCurrentSaveGameName(Slot.GetData());

		auto&& LoadedSaveData = Cast<UTimelinesEMSInfoSaveGame>(EMS->GetSlotInfoObject(Slot.GetData()));
		if (!IsValid(LoadedSaveData))
		{
			ServiceError(TEXTVIEW("Slot info is invalid. Make sure TimelinesEMSInfoSaveGame is selected as Info Class in project Settings"));
			return FConstStructView();
		}

		return LoadedSaveData->GetFragmentData(Type);
	}

	if (Type->IsChildOf(FFaerieSaveSlotDataFragment::StaticStruct()))
	{
		if (Slot.IsEmpty())
		{
			// Load global data if slot is empty
			if (auto&& LoadedSaveData =
					Cast<UTimelinesEMSGlobalCustomSaveGame>(EMS->GetCustomSave(UTimelinesEMSGlobalCustomSaveGame::StaticClass(), FString(), FString())))
			{
				return LoadedSaveData->GetFragmentData(Type);
			}
		}
		else
		{
			// EMS must have the slot as current in order to fetch per-slot custom data. (no longer true, now parameter of GetCustomSave)
			//EMS->SetCurrentSaveGameName(Slot.GetData());

			// Load custom save for the slot
			if (auto&& LoadedSaveData =
					Cast<UTimelinesEMSCustomSaveGame>(EMS->GetCustomSave(UTimelinesEMSCustomSaveGame::StaticClass(), Slot.GetData(), FString())))
			{
				return LoadedSaveData->GetFragmentData(Type);
			}
		}
	}

	return FConstStructView();
}

void UEasyMultiSaveInterop::EditFragmentData(const UScriptStruct* Type, const FStringView Slot,
											 const TFunctionRef<void(FStructView)>& Edit)
{
	UEMSObject* EMS = GetEMSInternal();
	if (!IsValid(EMS))
	{
		return;
	}

	if (Type->IsChildOf(FFaerieSaveSlotInfoFragment::StaticStruct()))
	{
		if (Slot.IsEmpty())
		{
			ServiceError(TEXTVIEW("Slot is empty while trying to fetch fragment for slot info!"));
			return;
		}

		// EMS must have the slot as current in order to fetch per-slot custom data.
		EMS->SetCurrentSaveGameName(Slot.GetData());

		auto&& LoadedSaveData = Cast<UTimelinesEMSInfoSaveGame>(EMS->GetSlotInfoObject(Slot.GetData()));
		if (!IsValid(LoadedSaveData))
		{
			ServiceError(TEXTVIEW("Slot info is invalid. Make sure TimelinesEMSInfoSaveGame is selected as Info Class in project Settings"));
			return;
		}

		if (auto&& Fragment = LoadedSaveData->GetMutableFragmentData(Type);
			Fragment.IsValid())
		{
			Edit(Fragment);
		}

		EMS->SaveSlotInfoObject();

		return;
	}

	if (Type->IsChildOf(FFaerieSaveSlotDataFragment::StaticStruct()))
	{
		if (Slot.IsEmpty())
		{
			// Load global data if slot is empty
			if (auto&& LoadedSaveData = Cast<UTimelinesEMSGlobalCustomSaveGame>(EMS->GetCustomSave(UTimelinesEMSGlobalCustomSaveGame::StaticClass(), FString(), FString())))
			{
				if (auto&& Fragment = LoadedSaveData->GetMutableFragmentData(Type);
					Fragment.IsValid())
				{
					Edit(Fragment);
				}
			}
		}
		else
		{
			// EMS must have the slot as current in order to fetch per-slot custom data. (no longer true, now parameter of GetCustomSave)
			//EMS->SetCurrentSaveGameName(Slot.GetData());

			// Load custom save for the slot
			auto&& LoadedSaveData = Cast<UTimelinesEMSCustomSaveGame>(EMS->GetCustomSave(UTimelinesEMSCustomSaveGame::StaticClass(), Slot.GetData(), FString()));
			if (!IsValid(LoadedSaveData))
			{
				ServiceError(TEXTVIEW("Failed to retrieve custom slot save data!"));
				return;
			}
			if (auto&& Fragment = LoadedSaveData->GetMutableFragmentData(Type);
				Fragment.IsValid())
			{
				Edit(Fragment);
			}
		}
	}
}

TArray<FString> UEasyMultiSaveInterop::GetAllSlotsSorted() const
{
	const UEMSObject* EMS = GetEMSInternal();
	if (!IsValid(EMS)) return TArray<FString>();

	return EMS->GetSortedSaveSlots();
}

bool UEasyMultiSaveInterop::SaveSlot(const FStringView Slot, const FSaveSystemEventAsyncResult Result)
{
	UEMSObject* EMS = GetEMSInternal();
	if (!IsValid(EMS)) return false;

	CommandResult = Result;

	EMS->SetCurrentSaveGameName(Slot.GetData());

	if (auto&& CustomSave = EMS->GetCustomSave(UTimelinesEMSCustomSaveGame::StaticClass(), Slot.GetData(), FString()))
	{
		EMS->SaveCustom(CustomSave);
	}

	if (UEMSAsyncSaveGame* AsyncSaver = UEMSAsyncSaveGame::AsyncSaveActors(GetOuterUFaerieLocalDataSubsystem(),
			ENUM_TO_FLAG(ESaveTypeFlags::SF_Level)))
	{
		AsyncSaver->RegisterWithGameInstance(GetOuterUFaerieLocalDataSubsystem());
		AsyncSaver->OnCompleted.AddDynamic(this, &ThisClass::OnSaveComplete);
		AsyncSaver->Activate();
		return true;
	}

	return false;
}

bool UEasyMultiSaveInterop::LoadSlot(const FStringView Slot, const FSaveSystemEventAsyncResult Result)
{
	UEMSObject* EMS = GetEMSInternal();
	if (!IsValid(EMS)) return false;

	CommandResult = Result;

	EMS->SetCurrentSaveGameName(Slot.GetData());
	auto&& LoadedSaveData = Cast<UTimelinesEMSInfoSaveGame>(EMS->GetSlotInfoObject(FString())); // An empty string will return the current slot.
	check(Slot == EMS->CurrentSaveGameName);

	// Make sure we are in the level for this save game to load into.
	// "GetLevelName()" is the exact function that EMS uses when it sets SlotInfo.Level so this *will* match when the
	// level is the same.
	if (LoadedSaveData->SlotInfo.Level != EMS->GetLevelName())
	{
		// @todo logtemp replace
		UE_LOG(LogTemp, Warning,
			TEXT("Not in the correct level to load the current save.\nSaveInfo.Name : %s\nSaveInfo.Level : %s\nLevelName : %s"),
				*LoadedSaveData->SlotInfo.Name, *LoadedSaveData->SlotInfo.Level.ToString(), *EMS->GetLevelName().ToString())
		return false;
	}

	if (UEMSAsyncLoadGame* AsyncLoader = UEMSAsyncLoadGame::AsyncLoadActors(GetOuterUFaerieLocalDataSubsystem(),
				ENUM_TO_FLAG(ELoadTypeFlags::LF_Level), true))
	{
		AsyncLoader->RegisterWithGameInstance(GetOuterUFaerieLocalDataSubsystem());
		AsyncLoader->OnCompleted.AddDynamic(this, &ThisClass::OnLoadComplete);
		AsyncLoader->OnFailed.AddDynamic(this, &ThisClass::OnLoadFailed);
		AsyncLoader->Activate();
		return true;
	}

	return false;
}

bool UEasyMultiSaveInterop::DeleteSlot(const FStringView Slot, FSaveSystemEventAsyncResult)
{
	UEMSObject* EMS = GetEMSInternal();
	if (!IsValid(EMS)) return false;

	// EMS does not use async deletion. Result callback is not used.

	if (!EMS->DoesSaveGameExist(Slot.GetData()))
	{
		return false;
	}
	EMS->DeleteAllSaveDataForSlot(Slot.GetData());
	return true;
}

void UEasyMultiSaveInterop::OnSaveComplete()
{
	if (CommandResult.IsBound())
	{
		CommandResult.Execute(true);
	}
	CommandResult = nullptr;
}

void UEasyMultiSaveInterop::OnLoadComplete()
{
	if (CommandResult.IsBound())
	{
		CommandResult.Execute(true);
	}
	CommandResult = nullptr;
}

void UEasyMultiSaveInterop::OnLoadFailed()
{
	if (CommandResult.IsBound())
	{
		CommandResult.Execute(false);
	}
	CommandResult = nullptr;
}