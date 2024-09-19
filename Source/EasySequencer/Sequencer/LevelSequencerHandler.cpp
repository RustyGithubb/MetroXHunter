/*
 * Implemented by Corentin Paya
 */

#include "Sequencer/LevelSequencerHandler.h"
#include "LevelSequenceActor.h"
#include "CineCameraActor.h"

ALevelSequencerHandler::ALevelSequencerHandler()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>( TEXT( "Scene Root" ) );
	RootComponent = SceneRoot;

	// Define the spawn parameters
	FActorSpawnParameters SpawnParams {};
	SpawnParams.Owner = this;

	UWorld* World = GetWorld();
	if ( !World ) return;

	// Spawn the LevelSequenceActor at the current actor's location
	LevelSequenceActor = World->SpawnActor<ALevelSequenceActor>(
		ALevelSequenceActor::StaticClass(),
		GetActorTransform(),
		SpawnParams
	);
}