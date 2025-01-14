/*
 * Implemented by Corentin Paya
 */

#include "MetroGameMode.h"

#include "Core/MetroGameInstance.h"
#include "SaveGame/MetroSaveGame.h"

#include "Kismet/GameplayStatics.h"

void AMetroGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Late Begin Play
	GetWorld()->OnWorldBeginPlay.AddUObject( this, &AMetroGameMode::LateBeginPlay );
}

void AMetroGameMode::LateBeginPlay()
{
	// Check if a save needs to be loaded after the level initialization.
	UMetroGameInstance* GameInstance = CastChecked<UMetroGameInstance>( GetGameInstance() );

	if ( GameInstance->HasSaveGame() )
	{
		GameInstance->LoadLevel();
	}
	else
	{
		GameInstance->CreateNewSave();
	}
}

void AMetroGameMode::RestartFromLastCheckpoint()
{
	// Check if a save needs to be loaded before the level initialization.
	UMetroGameInstance* GameInstance = CastChecked<UMetroGameInstance>( GetGameInstance() );

	if ( GameInstance->HasSaveGame() )
	{
		GameInstance->LoadLastSave();
	}

	UGameplayStatics::OpenLevel( this, FName( *GetWorld()->GetName() ), false );
}
