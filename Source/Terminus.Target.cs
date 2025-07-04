// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TerminusTarget : TargetRules
{
	public TerminusTarget( TargetInfo Target ) : base( Target )
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add( "MetroXHunter" );

		bOverrideBuildEnvironment = true;

        // Shipping definitions
        GlobalDefinitions.Add( "UE_WITH_CHEAT_MANAGER=1" );
		//GlobalDefinitions.Add( "ALLOW_CONSOLE_IN_SHIPPING=1" );
        //bUseLoggingInShipping = true;
    }
}
