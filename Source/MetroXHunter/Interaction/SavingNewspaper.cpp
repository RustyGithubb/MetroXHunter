/*
 * Implemented by Corentin Paya
 */

#include "SavingNewspaper.h"
#include "Core/MetroGameInstance.h"

#include "Library/UtilityLibrary.h"

void ASavingNewspaper::Interact()
{
	auto GameInstance = CastChecked<UMetroGameInstance>( GetGameInstance() );
	GameInstance->SaveLevel();

	UUtilityLibrary::PrintMessage( TEXT( "You have saved your progress (kinda)!" ) );
}
