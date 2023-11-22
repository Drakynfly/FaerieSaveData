// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "SaveSystemInteropBase.h"
#include "FaerieSaveDataCommandBase.h"
#include "FaerieSaveDataSettings.h"
#include "Logging/StructuredLog.h"

UFaerieLoadCommand* USaveSystemInteropBase::CreateLoadCommand(FStringView Slot, const bool StallRunning)
{
	if (State == LoadingInProgress)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cancelling LoadSlot_Impl: A load is already in progress!"))
		return nullptr;
	}

	if (State == SavingInProgress)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cancelling LoadSlot_Impl: Cannot load while a save is in progress!"))
		return nullptr;
	}

	UFaerieLoadCommand* LoadExec = NewObject<UFaerieLoadCommand>(this, GetOuterUFaerieLocalDataSubsystem()->LoadExecClass);
	if (!ensureMsgf(LoadExec, TEXT("Cancelling LoadSlot_Impl: Unable to create SaveExec object!")))
	{
		return nullptr;
	}

	State = LoadingInProgress;
	CommandInProgress = LoadExec;

	LoadExec->SlotName = Slot;

	if (!StallRunning)
	{
		LoadExec->RunCommand();
	}

	return LoadExec;
}

UFaerieSaveCommand* USaveSystemInteropBase::CreateSaveCommand(FStringView Slot, const bool StallRunning)
{
	if (State == LoadingInProgress)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cancelling SaveSlot_Impl: Cannot save while a load is in progress!"))
		return nullptr;
	}

	if (State == SavingInProgress)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cancelling SaveSlot_Impl: A save is already in progress!"))
		return nullptr;
	}

	UFaerieSaveCommand* SaveExec = NewObject<UFaerieSaveCommand>(this, GetOuterUFaerieLocalDataSubsystem()->SaveExecClass);
	if (!ensureMsgf(SaveExec, TEXT("Cancelling SaveSlot_Impl: Unable to create SaveExec object!")))
	{
		return nullptr;
	}

	State = SavingInProgress;
	CommandInProgress = SaveExec;

	SaveExec->SlotName = Slot;

	if (!StallRunning)
	{
		SaveExec->RunCommand();
	}

	return SaveExec;
}

void USaveSystemInteropBase::ServiceError(const FStringView ErrorMessage) const
{
	UE_LOGFMT(LogTemp, Error, "{message}", ErrorMessage);
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 8.0, FColor::Red, FString(ErrorMessage));
	OnErrorDelegate.Broadcast(ErrorMessage);
}

void USaveSystemInteropBase::NotifyPreSaveEvent(const FStringView Slot)
{
	OnPreSlotSaveLaunched.Broadcast(Slot);
}

void USaveSystemInteropBase::NotifySaveEventFinished(const FStringView Slot, const TOptional<FString>& Failure)
{
	if (!Failure.IsSet())
	{
		OnSaveCompletedDelegate.Broadcast(Slot);
	}
	else
	{
		TArray<FStringFormatArg> Args;
		Args.Add(FString(Slot));
		Args.Add(Failure.GetValue());
		const FString ErrorMessage = FString::Format(TEXT("Save to slot '{0}' failed: {1}"), Args);
		ServiceError(ErrorMessage);
	}
	State = None;
	CommandInProgress = nullptr;
}

void USaveSystemInteropBase::NotifyLoadEventFinished(const FStringView Slot, const TOptional<FString>& Failure)
{
	if (!Failure.IsSet())
	{
		OnLoadCompletedDelegate.Broadcast(Slot);
	}
	else
	{
		TArray<FStringFormatArg> Args;
		Args.Add(FString(Slot));
		Args.Add(Failure.GetValue());
		const FString ErrorMessage = FString::Format(TEXT("Load from slot '{0}' failed: {1}"), Args);
		ServiceError(ErrorMessage);
	}
	State = None;
	CommandInProgress = nullptr;
}

UWorld* UFaerieLocalDataSubsystem::GetWorld() const
{
	if (!IsTemplate())
	{
		return GetGameInstance()->GetWorld();
	}
	return nullptr;
}

void UFaerieLocalDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SaveExecClass = GetDefault<UFaerieSaveDataSettings>()->SaveExecClass.LoadSynchronous();
	LoadExecClass = GetDefault<UFaerieSaveDataSettings>()->LoadExecClass.LoadSynchronous();

	if (!ensureMsgf(SaveExecClass && SaveExecClass->IsChildOf<UFaerieSaveCommand>(),
		TEXT("A valid save exec class must be provided for USaveSystemInteropBase to function. Please check project settings!")))
	{
		return;
	}

	if (!ensureMsgf(LoadExecClass && LoadExecClass->IsChildOf<UFaerieLoadCommand>(),
		TEXT("A valid load exec class must be provided for USaveSystemInteropBase to function. Please check project settings!")))
	{
		return;
	}

	// Wait a tick before starting up backends, in case they need other subsystems to be ready.
	GetWorld()->GetTimerManager().SetTimerForNextTick(
		[this]
		{
			auto&& DefaultServiceClass = GetDefault<UFaerieSaveDataSettings>()->BackendSystemClass.LoadSynchronous();
			auto&& PersistanceServiceClass = GetDefault<UFaerieSaveDataSettings>()->PersistanceServiceClass.LoadSynchronous();

			if (!ensureMsgf(DefaultServiceClass && DefaultServiceClass->IsChildOf<USaveSystemInteropBase>(),
				TEXT("A valid interop class must be provided for FaerieLocalDataSubsystem to function. Please check project settings!")))
			{
				return;
			}

			InitSaveSystem(DefaultServiceClass, Faerie::SaveData::DefaultService);
			InitSaveSystem(PersistanceServiceClass, Faerie::SaveData::PersistantService);
		});
}

USaveSystemInteropBase* UFaerieLocalDataSubsystem::GetService(const UObject* Obj, const FName ServiceKey)
{
	if (!IsValid(Obj))
	{
		return nullptr;
	}

	if (auto&& Subsystem = Obj->GetWorld()->GetGameInstance()->GetSubsystem<UFaerieLocalDataSubsystem>())
	{
		return Subsystem->GetService(ServiceKey);
	}

	return nullptr;
}

USaveSystemInteropBase* UFaerieLocalDataSubsystem::InitSaveSystem(const TSubclassOf<USaveSystemInteropBase> Class, const FName Key)
{
	check(IsValid(Class));

	if (!IsValid(Class) ||
		Class->HasAnyClassFlags(CLASS_Abstract))
	{
		return nullptr;
	}

	// Re-use existing services by class.
	for (auto&& Element : Services)
	{
		if (Element.Value.IsA(Class))
		{
			return Services.Add(Key, Element.Value);
		}
	}

	auto&& NewService = NewObject<USaveSystemInteropBase>(this, Class);
	check(NewService);

	NewService->GetOnPreSlotSavedEvent().AddWeakLambda(this,
		[this](const FStringView Slot)
		{
			OnPreSaveLaunched.Broadcast(Slot);
		});

	NewService->GetOnLoadCompletedEvent().AddWeakLambda(this,
		[this](const FStringView Slot)
		{
			OnLoadCompleted.Broadcast(Slot);
		});

	NewService->GetOnSaveCompletedEvent().AddWeakLambda(this,
		[this](const FStringView Slot)
		{
			OnSaveCompleted.Broadcast(Slot);
		});

	NewService->GetOnErrorEvent().AddWeakLambda(this,
		[this, Service = TWeakObjectPtr<USaveSystemInteropBase>(NewService)](const FStringView Slot)
		{
			OnServiceError.Broadcast(FString(Slot));
		});

	Services.Add(Key, NewService);

	TArray<FOnSubsystemInit> Callbacks;
	AwaitingInitialization.MultiFind(Key, Callbacks);
	AwaitingInitialization.Remove(Key);
	for (auto&& Callback : Callbacks)
	{
		Callback.ExecuteIfBound(NewService);
	}

	return NewService;
}

USaveSystemInteropBase* UFaerieLocalDataSubsystem::GetService(const FName ServiceKey) const
{
	if (auto&& Service = Services.Find(ServiceKey))
	{
		return *Service;
	}
	return nullptr;
}

void UFaerieLocalDataSubsystem::SetOnServiceInit(const FName ServiceKey, const FOnSubsystemInit& Callback)
{
	if (!Callback.IsBound()) return;

	if (auto&& Service = Services.Find(ServiceKey))
	{
		Callback.Execute(Service->Get());
	}
	else
	{
		AwaitingInitialization.Add(ServiceKey, Callback);
	}
}

TInstancedStruct<FFaerieSaveSlotFragmentBase> UFaerieLocalDataSubsystem::GetSlotFragment(const FName ServiceKey, const UScriptStruct* Type, const FString& Slot) const
{
	if (!IsValid(Type))
	{
		return {};
	}

	TInstancedStruct<FFaerieSaveSlotFragmentBase> Out;

	if (auto&& Service = GetService(ServiceKey);
		IsValid(Service))
	{
		if (const FConstStructView Fragment = Service->GetFragmentData(Type, Slot);
			Fragment.IsValid())
		{
			Out.InitializeAsScriptStruct(Fragment.GetScriptStruct(), Fragment.GetMemory());
		}
	}

	return Out;
}