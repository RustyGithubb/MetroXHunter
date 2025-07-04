/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MetroGameMode.generated.h"

DECLARE_LOG_CATEGORY_EXTERN( LogMetroWorldLoading, Log, All );

class UMetroGameInstance;

 /*
  * Custom Game Mode Base with LateBeginPlay to load the level.
  */
UCLASS()
class METROXHUNTER_API AMetroGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	void LateBeginPlay();

	UFUNCTION( BlueprintCallable, Category = "MetroGameMode" )
	void RestartFromLastCheckpoint();

	UFUNCTION( BlueprintPure, Category = "MetroGameMode" )
	bool IsWorldFullyLoaded() const;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnWorldFullyLoaded );
	UPROPERTY( BlueprintAssignable, Category = "MetroGameMode" )
	FOnWorldFullyLoaded OnWorldFullyLoaded {};

protected:
	UFUNCTION( BlueprintImplementableEvent, meta = ( DisplayName = "On World Fully Loaded" ) )
	void BP_OnWorldFullyLoaded();

private:
	void OnLevelStreamingStateChanged(
		UWorld* World,
		const ULevelStreaming* StreamingLevel,
		ULevel* LoadedLevel,
		ELevelStreamingState PreviousState, ELevelStreamingState NewState
	);

	void PrepareWorldLoading();
	void TriggerWorldFullyLoaded();

private:
	UMetroGameInstance* GameInstance = nullptr;

	bool bIsWorldFullyLoaded = false;
	TSet<const ULevelStreaming*> LoadingLevels {};
};
