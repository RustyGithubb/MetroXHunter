/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "BaseSavedData.generated.h"

USTRUCT( Blueprintable, BlueprintType )
struct FBaseSavedData
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ActorData" )
	FTransform LastLocation {};

	UPROPERTY( EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ActorData" )
	bool bShouldSpawn = true;
};

USTRUCT( Blueprintable, BlueprintType )
struct FInteractableSavedData : public FBaseSavedData
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, SaveGame, Category = "ActorData" )
	bool bIsConsummed = false;
};

USTRUCT( BlueprintType )
struct FHealthSavedData
{
	GENERATED_BODY()

	UPROPERTY( BlueprintReadWrite, SaveGame, Category = "Health" )
	int32 CurrentHealth = 0;
};

USTRUCT( BlueprintType )
struct FReloadSavedData
{
	GENERATED_BODY()

	UPROPERTY( BlueprintReadWrite, SaveGame, Category = "Health" )
	int32 CurrentAmmoInMagazine = 0;
};

USTRUCT( BlueprintType )
struct FInventorySavedData
{
	GENERATED_BODY()

	UPROPERTY( BlueprintReadWrite, SaveGame, Category = "Health" )
	int32 CurrentAmmoAmount = 0;
	UPROPERTY( BlueprintReadWrite, SaveGame, Category = "Health" )
	int32 CurrentSyringeAmount = 0;
};