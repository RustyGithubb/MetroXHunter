// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using UnrealBuildTool.Rules;

public class MetroXHunter : ModuleRules  
{
	public MetroXHunter( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"UMG",
				"EnhancedInput",
				"GameplayTags",
				"Niagara",
				"StructUtils",
				"CinematicCamera",
				"MoviePlayer",
				"DualSenseV",
				"TemplateSequence",
				"PhysicsCore",
                "CommonInput",
                "AudioMixer",
            }
		);

		PublicIncludePaths.AddRange(new string[]
		{
			"MetroXHunter",
			"MetroXHunter/Dialogue"
		});

		PrivateDependencyModuleNames.AddRange(
			new string[] { 
				"Slate",
				"SlateCore",
				"EngineSettings",
                "AsyncLoadingScreen",
            }
        );

		// Uncomment if you are using online features
		PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}