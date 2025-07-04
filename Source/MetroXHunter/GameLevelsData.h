/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "Engine/DataAsset.h"
#include "GameLevelsData.generated.h"

/*
 * Structure referencing a game level with an associated name.
 * Designed to be used for GameLevelsData.
 */
USTRUCT( BlueprintType )
struct FGameLevelReference
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	FString DisplayName = TEXT( "Level" );

	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	TSoftObjectPtr<UWorld> Level = nullptr;
};

/*
 * Data Asset containing the main level of the game as well as an arbitrary list
 * of levels to be populated in the level selection menu.
 */
UCLASS( BlueprintType )
class METROXHUNTER_API UGameLevelsData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Levels" )
	FGameLevelReference MainLevel {};

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Levels" )
	FGameLevelReference MenuLevel {};

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Levels" )
	TArray<FGameLevelReference> LevelChapters {};
};
