// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieSaveDataCommandBase.h"
#include "FaerieSaveCommand_Default.generated.h"

UCLASS()
class UFaerieSaveCommand_Default : public UFaerieSaveCommand
{
	GENERATED_BODY()

public:
	virtual void Run() override;
};