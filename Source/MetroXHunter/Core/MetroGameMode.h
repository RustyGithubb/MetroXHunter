/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MetroGameMode.generated.h"

 /*
  * Custom Game Mode Base with LateBeginPlay to load the level.
  * TODO: Rename as AMetroGameMode
  */
UCLASS()
class METROXHUNTER_API AMetroGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	void LateBeginPlay();

	UFUNCTION( BlueprintCallable )
	void RestartFromLastCheckpoint();
};
