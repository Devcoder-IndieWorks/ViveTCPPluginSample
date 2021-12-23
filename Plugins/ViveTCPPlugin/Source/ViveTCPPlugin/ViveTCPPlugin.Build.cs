// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ViveTCPPlugin : ModuleRules
{
	public ViveTCPPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnforceIWYU = false;
		
		PrivateIncludePaths.AddRange(
		    new string[] {
				"ViveTCPPlugin/Private"
			});

		PrivateDependencyModuleNames.AddRange(
		    new string[] {
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore", 
                "Sockets",
                "Networking", 
                "Slate", 
                "SlateCore"
			});
	}
}