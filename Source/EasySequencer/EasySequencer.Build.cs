/*
 * Implemented by Corentin Paya
 */

using UnrealBuildTool;

public class EasySequencer : ModuleRules
{
	public EasySequencer( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivateIncludePaths.Add( "EasySequencer" );

        if (Target.Type == TargetType.Editor)
        {
            PublicDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
        }

        PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
                "Engine",
				"LevelSequence",
				"CinematicCamera"
			}
		);
	}
}