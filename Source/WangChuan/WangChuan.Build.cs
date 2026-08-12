// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WangChuan : ModuleRules
{
	public WangChuan(ReadOnlyTargetRules Target) 
		: base(Target)
	{
		PCHUsage = 
			PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] { 
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore", 
				"EnhancedInput", 
				"UMG",
				"Niagara",
				"AIModule",
				"NavigationSystem"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore"
			}
		);
	}
}
