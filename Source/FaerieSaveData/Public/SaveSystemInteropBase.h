// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieSaveSlotFragment.h"
#include "StructUtils/StructView.h"
#include "Algo/RemoveIf.h"
#include "Subsystems/WorldSubsystem.h"
#include "SaveSystemInteropBase.generated.h"

using FSaveSystemSlotEvent = TMulticastDelegate<void(FStringView)>;

class UFaerieLocalDataSubsystem;
class UFaerieSaveCommand;
class UFaerieLoadCommand;

namespace Faerie::SaveData
{
	static const FName DefaultService = FName(TEXTVIEW("DEFAULT"));
	static const FName PersistantService = FName(TEXTVIEW("PERSISTANT"));
}

DECLARE_DELEGATE_OneParam(FSaveSystemEventAsyncResult, bool);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDynamicSaveSystemSlotEvent, FString, SlotName);

UCLASS(Abstract, Within = FaerieLocalDataSubsystem)
class FAERIESAVEDATA_API USaveSystemInteropBase : public UObject
{
	GENERATED_BODY()

	friend class UFaerieSaveDataCommandBase;
	friend UFaerieLocalDataSubsystem;

public:
	USaveSystemInteropBase() {}

	virtual TOptional<FDateTime> GetTimestamp(FStringView Slot) const
		PURE_VIRTUAL(USaveSystemInteropBase::GetTimestamp, return FDateTime(); )

	/* Get fragment  */
	virtual FConstStructView GetFragmentData(const UScriptStruct* Type, FStringView Slot = FString()) const
		PURE_VIRTUAL(USaveSystemInteropBase::GetFragmentData, return FConstStructView(); )

	template <typename T>
	FConstStructView GetFragmentData(FStringView Slot = FString()) const
	{
		static_assert(TIsDerivedFrom<T, FFaerieSaveSlotFragmentBase>::Value, "T must derive from FFaerieSaveSlotFragmentBase");
		return GetFragmentData(TBaseStructure<T>::Get(), Slot);
	}

	virtual void EditFragmentData(const UScriptStruct* Type, FStringView Slot, const TFunctionRef<void(FStructView)>& Edit)
		PURE_VIRTUAL(USaveSystemInteropBase::EditFragmentData, )

	template <typename T>
	void EditFragmentData(FStringView Slot, const TFunctionRef<void(T&)>& Edit)
	{
		static_assert(TIsDerivedFrom<T, FFaerieSaveSlotFragmentBase>::Value, "T must derive from FFaerieSaveSlotFragmentBase");
		EditFragmentData(TBaseStructure<T>::Get(), Slot, [Edit](const FStructView Struct) { Edit(Struct.Get<T>()); });
	}

	virtual TArray<FString> GetAllSlotsSorted() const PURE_VIRTUAL(USaveSystemInteropBase::GetAllSlotsSorted, return TArray<FString>(); )

	// This function's implementation must call OnSaveComplete at some point or return false.
	virtual bool SaveSlot(FStringView Slot, FSaveSystemEventAsyncResult Result) PURE_VIRTUAL(USaveSystemInteropBase::SaveSlot, return false; )

	// This function's implementation must call OnLoadComplete at some point or return false.
	virtual bool LoadSlot(FStringView Slot, FSaveSystemEventAsyncResult Result) PURE_VIRTUAL(USaveSystemInteropBase::LoadSlot, return false; )

	virtual bool DeleteSlot(FStringView Slot, FSaveSystemEventAsyncResult Result) PURE_VIRTUAL(USaveSystemInteropBase::DeleteSlot, return false; )

	bool HasKey(const FName Key) const { return Keys.Contains(Key); }

	UFaerieLoadCommand* CreateLoadCommand(FStringView Slot, bool StallRunning);
	UFaerieSaveCommand* CreateSaveCommand(FStringView Slot, bool StallRunning);

	using FFragmentArray = TArray<FConstStructView>;
	using FConstFragmentArrayView = TConstArrayView<FConstStructView>;
	using FSlotPredicate = TFunctionRef<bool(FConstFragmentArrayView A)>;
	using FSlotComparator = TFunctionRef<bool(FConstFragmentArrayView A, FConstFragmentArrayView B)>;

	class FSlotQuery
	{
		friend USaveSystemInteropBase;

		FSlotQuery(const USaveSystemInteropBase& InteropBase, const TConstArrayView<const UScriptStruct*>& Types)
		  : RelevantTypes(Types)
		{
			for (const TArray<FString> Slots = InteropBase.GetAllSlotsSorted();
				 auto&& Slot : Slots)
			{
				auto&& SlotPair = SlotFragments.AddDefaulted_GetRef();
				SlotPair.Key = Slot;

				for (const UScriptStruct* Element : RelevantTypes)
				{
					if (auto&& Fragment = InteropBase.GetFragmentData(Element);
						Fragment.IsValid())
					{
						SlotPair.Value.Add(Fragment);
					}
				}
			}
		}

	public:
		FSlotQuery& Filter(FSlotPredicate Predicate)
		{
			(void)Algo::RemoveIf(SlotFragments,
				[Predicate](const TPair<FString, FFragmentArray>& Pair)
				{
					return !Predicate(Pair.Value);
				});
			return *this;
		}

		FSlotQuery& Sort(FSlotComparator Comparator)
		{
			Algo::Sort(SlotFragments,
				[Comparator](const TPair<FString, FFragmentArray>& A, const TPair<FString, FFragmentArray>& B)
				{
					return Comparator(A.Value, B.Value);
				});
			return *this;
		}

		// Get the last slot in the list. If Sort is called previous to this, the highest scoring slot will be returned.
		FSlotQuery& Best(FString& Slot)
		{
			if (!SlotFragments.IsEmpty())
			{
				Slot = SlotFragments.Last().Key;
			}

			return *this;
		}

		// Get all slots remaining after any filtering.
		FSlotQuery& All(TArray<FString>& Slots)
		{
			Algo::Transform(SlotFragments, Slots,
				[](const TPair<FString, FFragmentArray>& Pair){ return Pair.Key; } );
			return *this;
		}

	private:
		const TConstArrayView<const UScriptStruct*> RelevantTypes;
		TArray<TPair<FString, FFragmentArray>> SlotFragments;
	};

	FSlotQuery Query(const TConstArrayView<const UScriptStruct*>& Types) const
	{
		return FSlotQuery(*this, Types);
	}

	FSaveSystemSlotEvent& GetOnPreSlotSavedEvent() { return OnPreSlotSaveLaunched; }
	FSaveSystemSlotEvent& GetOnSaveCompletedEvent() { return OnSaveCompletedDelegate; }
	FSaveSystemSlotEvent& GetOnLoadCompletedEvent() { return OnLoadCompletedDelegate; }

	FSaveSystemSlotEvent& GetOnErrorEvent() { return OnErrorDelegate; }

protected:
	void ServiceError(FStringView ErrorMessage) const;

	void NotifyPreSaveEvent(FStringView Slot);
	void NotifySaveEventFinished(FStringView Slot, const TOptional<FString>& Failure);
	void NotifyLoadEventFinished(FStringView Slot, const TOptional<FString>& Failure);

	// Native callbacks
	FSaveSystemSlotEvent OnPreSlotSaveLaunched;
	FSaveSystemSlotEvent OnSaveCompletedDelegate;
	FSaveSystemSlotEvent OnLoadCompletedDelegate;
	FSaveSystemSlotEvent OnErrorDelegate;

private:
	UPROPERTY()
	TObjectPtr<UFaerieSaveDataCommandBase> CommandInProgress = nullptr;

	enum ESystemState
	{
		None,
		LoadingInProgress,
		SavingInProgress
	};

	ESystemState State = None;

	TSet<FName> Keys;
};

using FSaveSystemServiceEvent = TMulticastDelegate<void(USaveSystemInteropBase*, FStringView)>;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFaerieSaveSystemError, FName, Service, const FString&, Message);

UCLASS()
class FAERIESAVEDATA_API UFaerieLocalDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	friend USaveSystemInteropBase;

public:
	virtual UWorld* GetWorld() const override;

	//~ UGameInstanceSubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	//~ UGameInstanceSubsystem

	static USaveSystemInteropBase* GetService(const UObject* Obj, FName ServiceKey);

	USaveSystemInteropBase* InitSaveSystem(TSubclassOf<USaveSystemInteropBase> Class, FName Key);

	template <typename T>
	T* InitSaveSystem(FName Key)
	{
		static_assert(TIsDerivedFrom<T, USaveSystemInteropBase>::Value, "T must derive from USaveSystemInteropBase");
		return Cast<T>(InitSaveSystem(T::StaticClass(), Key));
	}

	USaveSystemInteropBase* GetService(FName ServiceKey) const;

	using FOnSubsystemInit = TDelegate<void(USaveSystemInteropBase*)>;
	void SetOnServiceInit(FName ServiceKey, const FOnSubsystemInit& Callback);

	FSaveSystemServiceEvent& GetPreSaveEvent() { return OnPreSaveLaunched; }
	FSaveSystemServiceEvent& GetSaveEvent() { return OnSaveCompleted; }
	FSaveSystemServiceEvent& GetLoadEvent() { return OnLoadCompleted; }

	UFUNCTION(BlueprintCallable, Category = "Faerie|LocalData")
	FDateTime GetSlotTimestamp(FName ServiceKey, const FString& Slot) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Faerie|LocalData", meta = (AutoCreateRefTerm = "Slot"))
	TInstancedStruct<FFaerieSaveSlotFragmentBase> GetSlotFragment(FName ServiceKey,
		UPARAM(meta = (MetaClass = "/Script/FaerieSaveData/FaerieSaveSlotFragmentBase")) const UScriptStruct* Type, const FString& Slot) const;

protected:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FFaerieSaveSystemError OnServiceError;

	// Broadcast after a successful save command.
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FDynamicSaveSystemSlotEvent OnSaveComplete;

	// Broadcast after a successful load command.
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FDynamicSaveSystemSlotEvent OnLoadComplete;

private:
	FSaveSystemServiceEvent OnPreSaveLaunched;
	FSaveSystemServiceEvent OnSaveCompleted;
	FSaveSystemServiceEvent OnLoadCompleted;

	UPROPERTY()
	TMap<FName, TObjectPtr<USaveSystemInteropBase>> Services;

	TSubclassOf<UFaerieSaveCommand> SaveExecClass;
	TSubclassOf<UFaerieLoadCommand> LoadExecClass;

	TMultiMap<FName, FOnSubsystemInit> AwaitingInitialization;
};

UCLASS()
class UFaerieLocalDataStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Faerie|LocalDataStatics", meta = (CompactNodeTitle = "DEFAULT"))
	static FName DefaultService() { return Faerie::SaveData::DefaultService; }

	UFUNCTION(BlueprintPure, Category = "Faerie|LocalDataStatics", meta = (CompactNodeTitle = "PERSISTANT"))
	static FName PersistantService() { return Faerie::SaveData::PersistantService; }
};