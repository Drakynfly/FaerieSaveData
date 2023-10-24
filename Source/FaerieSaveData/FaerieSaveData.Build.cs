// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

using UnrealBuildTool;

public class FaerieSaveData : ModuleRules
{
	public FaerieSaveData(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new []
			{
				"Core",
				"StructUtils",
				"DeveloperSettings"
			});

		PrivateDependencyModuleNames.AddRange(
			new []
			{
				"CoreUObject",
				"Engine"
			});
	}
}