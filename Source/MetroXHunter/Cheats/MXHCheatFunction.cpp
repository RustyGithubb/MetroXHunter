#include "MXHCheatFunction.h"

#include "MXHCheatManager.h"

void UMXHCheatFunction::Init( UMXHCheatManager* _CheatManager )
{
	CheatManager = _CheatManager;

	OnInit();

	//  If should start toggled, enable it right after initialization so it allows user to initialize first before activation
	if ( IsTogglable && ShouldStartToggled )
	{
		FMXHCheatFunctionActionParams Params {};
		Params.IsToggled = true;
		Cheat( Params );
	}
}

void UMXHCheatFunction::Cheat( FMXHCheatFunctionActionParams Params )
{
	if ( IsTogglable )
	{
		IsToggled = Params.IsToggled;
	}

	OnCheat();
}

void UMXHCheatFunction::OnInit_Implementation() {}

void UMXHCheatFunction::OnCheat_Implementation() {}

UWorld* UMXHCheatFunction::GetWorld() const
{
	if ( !IsValid( CheatManager ) ) return nullptr;

	return CheatManager->GetWorld();
}
