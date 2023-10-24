// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieSaveSlotFragment.generated.h"

USTRUCT()
struct FFaerieSaveSlotFragmentBase
{
	GENERATED_BODY()
};

/**
 * Fragment for slot Info saves. Should be kept small.
 */
USTRUCT()
struct FAERIESAVEDATA_API FFaerieSaveSlotInfoFragment : public FFaerieSaveSlotFragmentBase
{
	GENERATED_BODY()
};

/**
 * Fragment for slot Data saves. Intended for bulk data
 */
USTRUCT()
struct FAERIESAVEDATA_API FFaerieSaveSlotDataFragment : public FFaerieSaveSlotFragmentBase
{
	GENERATED_BODY()
};