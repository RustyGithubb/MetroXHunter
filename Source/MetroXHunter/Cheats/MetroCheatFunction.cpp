#include "MetroCheatFunction.h"
#include "MetroCheatManager.h"

/**
 * Cheat Function
 */

void UMetroCheatFunction::Init( UMetroCheatManager* _CheatManager )
{
	CheatManager = _CheatManager;

	OnInit();

	// If should start toggled, enable it right after initialization so it allows user to 
	// initialize first before activation
	if ( bIsTogglable && bShouldStartToggled )
	{
		FMetroCheatFunctionActionParams Params {};
		Params.bIsToggled = true;
		Cheat( Params );
	}
}

void UMetroCheatFunction::Cheat( FMetroCheatFunctionActionParams Params )
{
	if ( bIsTogglable )
	{
		bIsToggled = Params.bIsToggled;
	}

	OnCheat();
}

void UMetroCheatFunction::Cheat()
{
	FMetroCheatFunctionActionParams Params {};
	Params.bIsToggled = bIsToggled;
	Cheat( Params );
}

void UMetroCheatFunction::OnInit_Implementation() {}

void UMetroCheatFunction::OnCheat_Implementation() {}

UWorld* UMetroCheatFunction::GetWorld() const
{
	if ( !IsValid( CheatManager ) ) return nullptr;

	return CheatManager->GetWorld();
}

/**
 * Cheat Selection Function
 */

void UMetroCheatSelectionFunction::CycleToNext()
{
	//  Move index to the right and wrap it around the value names length
	Index = ( Index + 1 ) % ValueNames.Num();

	Cheat();
}

void UMetroCheatSelectionFunction::CycleToPrevious()
{
	//  Move index to the left and wrap it around the value names length
	Index = Index - 1 < 0
		? ValueNames.Num() - 1
		: Index - 1;

	Cheat();
}
