// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieSaveDataCommandBase.h"
#include "FaerieLoadCommand_Default.generated.h"

UCLASS()
class UFaerieLoadCommand_Default : public UFaerieLoadCommand
{
	GENERATED_BODY()

public:
	virtual void Run() override;
};