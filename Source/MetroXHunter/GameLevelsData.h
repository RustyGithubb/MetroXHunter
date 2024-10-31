/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "Engine/DataAsset.h"
#include "GameLevelsData.generated.h"

USTRUCT( BlueprintType )
struct FGameLevelReference
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	FString DisplayName = TEXT( "Level" );

	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	TSoftObjectPtr<UWorld> Level = nullptr;
};

/**
 * 
 */
UCLASS( BlueprintType )
class METROXHUNTER_API UGameLevelsData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Levels" )
	FGameLevelReference MainLevel {};

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Levels" )
	TArray<FGameLevelReference> LevelChapters {};
};
