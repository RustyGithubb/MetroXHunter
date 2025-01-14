using UnrealBuildTool;

public class DualSenseV : ModuleRules
{
	public DualSenseV( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange( new string[] {} );
		
		PrivateIncludePaths.AddRange( new string[] {} );
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"DeveloperSettings",
			}
		);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"InputDevice",
				"ApplicationCore",
			}
		);
		
		DynamicallyLoadedModuleNames.AddRange( new string[] {} );

		// TODO: Replace with how RawInput does it for Windows
		if ( Target.Platform == UnrealTargetPlatform.Win64 )
		{
			PublicAdditionalLibraries.Add( "Hid.lib" );
			PublicSystemLibraries.Add( "Setupapi.lib" );
		}
	}
}
