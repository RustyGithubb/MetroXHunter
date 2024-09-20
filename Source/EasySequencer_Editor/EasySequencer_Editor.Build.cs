/*
 * Implemented by Corentin Paya
 */

using UnrealBuildTool;

public class EasySequencer_Editor : ModuleRules
{
	public EasySequencer_Editor( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivateIncludePaths.Add( "EasySequencer_Editor" );

		PrivateDependencyModuleNames.AddRange( 
			new string[] {
				"Core",
				"Slate",
				"SlateCore",
				"ToolMenus",

                "CoreUObject",
                "Engine",

                "UnrealEd",
                "UMGEditor",
                "Blutility",
            }
		);
	}
}