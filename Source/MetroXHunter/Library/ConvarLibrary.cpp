/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "Library/ConvarLibrary.h"

static TAutoConsoleVariable<bool> ConvarAIDebug(
	TEXT( "MXH.AI.Debug" ),
	false,
	TEXT( "Set debug mode for AI.\n" ),
	ECVF_Cheat | ECVF_SaveForNextBoot
);

bool UConvarLibrary::IsAIDebugConvarEnabled()
{
	return ConvarAIDebug.GetValueOnAnyThread();
}

static TAutoConsoleVariable<bool> ConvarAIIgnorePlayer(
	TEXT( "MXH.AI.IgnorePlayer" ),
	false,
	TEXT( "Set AI to ignore player.\n" ),
	ECVF_Cheat | ECVF_SaveForNextBoot
);

bool UConvarLibrary::IsAIIgnorePlayerConvarEnabled()
{
	return ConvarAIIgnorePlayer.GetValueOnAnyThread();
}

static TAutoConsoleVariable<int32> ConvarAIPlayerTokenOverride(
	TEXT( "MXH.AI.TokenOverride" ),
	-1,
	TEXT( "Override AI reservation tokens for player.\n" ),
	ECVF_Cheat | ECVF_SaveForNextBoot
);

int32 UConvarLibrary::GetAIPlayerTokenOverride()
{
	return ConvarAIPlayerTokenOverride.GetValueOnAnyThread();
}

static TAutoConsoleVariable<FString> ConvarTickDebuggerDefaultName(
	TEXT( "MXH.TickDebugger.DefaultName" ),
	TEXT( "" ),
	TEXT( 
		"Set the object name to tick debug on BeginPlay."
		"The value should represent a valid object name in the TickDebugger CheatFunction array.\n"
	),
	ECVF_Cheat | ECVF_SaveForNextBoot
);

FString UConvarLibrary::GetTickDebuggerDefaultNameConvarValue()
{
	return ConvarTickDebuggerDefaultName.GetValueOnAnyThread();
}

void UConvarLibrary::SetTickDebuggerDefaultNameConvarValue( const FString& Value )
{
	ConvarTickDebuggerDefaultName->Set( *Value );
}

static TAutoConsoleVariable<bool> ConvarGunDebug(
	TEXT( "MXH.Gun.Debug" ),
	false,
	TEXT( "Set debug mode for Gun\n" ),
	ECVF_Cheat | ECVF_SaveForNextBoot
);

bool UConvarLibrary::IsGunDebugEnabled()
{
	return ConvarGunDebug.GetValueOnAnyThread();
}

static TAutoConsoleVariable<bool> ConvarGunInfiniteAmmo(
	TEXT( "MXH.Gun.InfiniteAmmo" ),
	false,
	TEXT( "Set infinite ammo for Gun\n" ),
	ECVF_Cheat | ECVF_SaveForNextBoot
);

bool UConvarLibrary::IsGunInfiniteAmmoEnabled()
{
	return ConvarGunInfiniteAmmo.GetValueOnAnyThread();
}

static TAutoConsoleVariable<bool> ConvarPlayerInvincible(
	TEXT( "MXH.Player.Invincible" ),
	false,
	TEXT(
		"Set the player's invincibility. If activated, the player will not take any damage."
	),
	ECVF_Cheat | ECVF_SaveForNextBoot
);

bool UConvarLibrary::IsPlayerInvincibleEnabled()
{
	return ConvarPlayerInvincible.GetValueOnAnyThread();
}

static TAutoConsoleVariable<bool> ConvarPlayerSkipCinematic(
	TEXT( "MXH.Player.SkipCinematic" ),
	false,
	TEXT(
		"Set whether the player should skip cinematic."
	),
	ECVF_Cheat | ECVF_SaveForNextBoot
);

bool UConvarLibrary::IsPlayerSkipCinematicEnabled()
{
	return ConvarPlayerSkipCinematic.GetValueOnAnyThread();
}