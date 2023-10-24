// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieSaveDataSettings.h"

#include "FaerieLoadCommand_Default.h"
#include "FaerieSaveCommand_Default.h"

UFaerieSaveDataSettings::UFaerieSaveDataSettings()
{
	SaveExecClass = UFaerieSaveCommand_Default::StaticClass();
	LoadExecClass = UFaerieLoadCommand_Default::StaticClass();
}

FName UFaerieSaveDataSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}