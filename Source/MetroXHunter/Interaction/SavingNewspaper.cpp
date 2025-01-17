/*
 * Implemented by Corentin Paya
 */

#include "SavingNewspaper.h"
#include "Core/MetroGameInstance.h"

#include "Library/UtilityLibrary.h"

void ASavingNewspaper::Save()
{
	auto GameInstance = CastChecked<UMetroGameInstance>( GetGameInstance() );
	GameInstance->SaveLevel();

	UUtilityLibrary::PrintMessage( TEXT( "SavingNewspaper: Saved player's progress" ) );
}

void ASavingNewspaper::Interact()
{
	// Everything is coded inside the blueprint
}
