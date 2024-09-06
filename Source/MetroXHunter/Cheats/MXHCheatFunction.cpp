#include "MXHCheatFunction.h"

#include "MXHCheatManager.h"

/**
 * Cheat Function
 */

void UMXHCheatFunction::Init( UMXHCheatManager* _CheatManager )
{
	CheatManager = _CheatManager;

	OnInit();

	// If should start toggled, enable it right after initialization so it allows user to 
	// initialize first before activation
	if ( bIsTogglable && bShouldStartToggled )
	{
		FMXHCheatFunctionActionParams Params {};
		Params.bIsToggled = true;
		Cheat( Params );
	}
}

void UMXHCheatFunction::Cheat( FMXHCheatFunctionActionParams Params )
{
	if ( bIsTogglable )
	{
		bIsToggled = Params.bIsToggled;
	}

	OnCheat();
}

void UMXHCheatFunction::Cheat()
{
	FMXHCheatFunctionActionParams Params {};
	Params.bIsToggled = bIsToggled;
	Cheat( Params );
}

void UMXHCheatFunction::OnInit_Implementation() {}

void UMXHCheatFunction::OnCheat_Implementation() {}

UWorld* UMXHCheatFunction::GetWorld() const
{
	if ( !IsValid( CheatManager ) ) return nullptr;

	return CheatManager->GetWorld();
}

/**
 * Cheat Selection Function
 */

void UMXHCheatSelectionFunction::CycleToNext()
{
	//  Move index to the right and wrap it around the value names length
	Index = ( Index + 1 ) % ValueNames.Num();

	Cheat();
}

void UMXHCheatSelectionFunction::CycleToPrevious()
{
	//  Move index to the left and wrap it around the value names length
	Index = Index - 1 < 0
		? ValueNames.Num() - 1
		: Index - 1;

	Cheat();
}
