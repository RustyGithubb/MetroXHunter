using UnrealBuildTool;

public class LightningsEditor : ModuleRules
{
	public LightningsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PrecompileForTargets = PrecompileTargetsType.Any;

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"UnrealEd",
				"SlateCore",
				"Slate",
				"Lightnings"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine",
				"CoreUObject"
			}
			);
	}
}
