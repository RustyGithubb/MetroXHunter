/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelSequencerHandler.generated.h"

class ALevelSequenceActor;
class ACineCameraActor;
class USceneComponent;

/*
 * The level sequencer handler needs to reference a level sequence and a cine camera.
 */
UCLASS()
class METROXHUNTER_API ALevelSequencerHandler : public AActor
{
	GENERATED_BODY()

public:
	ALevelSequencerHandler();
	
public:
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Interactable" )
	USceneComponent* SceneRoot = nullptr;

	UPROPERTY( VisibleAnywhere, BlueprintReadWrite )
	ALevelSequenceActor* LevelSequenceActor = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	ACineCameraActor* CineCamera = nullptr;
};
