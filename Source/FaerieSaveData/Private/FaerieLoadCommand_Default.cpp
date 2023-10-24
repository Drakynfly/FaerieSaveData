// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieLoadCommand_Default.h"
#include "SaveSystemInteropBase.h"

void UFaerieLoadCommand_Default::Run()
{
	if (auto&& Service = GetOuterUSaveSystemInteropBase();
		!Service->LoadSlot(SlotName, FSaveSystemEventAsyncResult::CreateWeakLambda(this,
			[this](const bool Success)
			{
				if (!Success)
				{
					Fail(TEXTVIEW("'LoadSlot' [async] request to backend failed!"));
				}
				else
				{
					Succeed();
				}
			})))
	{
		Fail(TEXTVIEW("'LoadSlot' request to backend failed!"));
	}
}