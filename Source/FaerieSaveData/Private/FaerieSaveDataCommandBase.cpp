// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieSaveDataCommandBase.h"
#include "SaveSystemInteropBase.h"

void UFaerieSaveDataCommandBase::RunCommand()
{
	if (!IsRunning)
	{
		if (IsA<UFaerieSaveCommand>())
		{
			GetOuterUSaveSystemInteropBase()->NotifyPreSaveEvent(SlotName);
		}

		IsRunning = true;
		Run();
	}
}

void UFaerieSaveDataCommandBase::Succeed()
{
	IsRunning = false;
	if (IsA<UFaerieSaveCommand>())
	{
		GetOuterUSaveSystemInteropBase()->NotifySaveEventFinished(SlotName, {});
	}
	else if (IsA<UFaerieLoadCommand>())
	{
		GetOuterUSaveSystemInteropBase()->NotifyLoadEventFinished(SlotName, {});
	}
}

void UFaerieSaveDataCommandBase::Fail(FStringView Reason)
{
	IsRunning = false;
	if (IsA<UFaerieSaveCommand>())
	{
		GetOuterUSaveSystemInteropBase()->NotifySaveEventFinished(SlotName, FString(Reason));
	}
	else if (IsA<UFaerieLoadCommand>())
	{
		GetOuterUSaveSystemInteropBase()->NotifyLoadEventFinished(SlotName, FString(Reason));
	}
}

UWorld* UFaerieSaveDataCommandBase::GetWorld() const
{
	if (!IsTemplate())
	{
		return GetOuterUSaveSystemInteropBase()->GetOuterUFaerieLocalDataSubsystem()->GetWorld();
	}
	return nullptr;
}