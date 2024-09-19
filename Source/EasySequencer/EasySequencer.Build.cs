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