// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieSaveCommand_Default.h"
#include "SaveSystemInteropBase.h"

void UFaerieSaveCommand_Default::Run()
{
	if (auto&& Service = GetOuterUSaveSystemInteropBase();
		!Service->SaveSlot(SlotName, FSaveSystemEventAsyncResult::CreateWeakLambda(this,
			[this](const bool Success)
			{
				if (!Success)
				{
					Fail(TEXTVIEW("'SaveSlot' [async] request to backend failed!"));
				}
				else
				{
					Succeed();
				}
			})))
	{
		Fail(TEXTVIEW("'SaveSlot' request to backend failed!"));
	}
}