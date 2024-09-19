/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InventoryData.generated.h"

/*
 * Inventory Data asset
 * The starting and max amount of the Syringe & ammo of the player.
 */
UCLASS()
class METROXHUNTER_API UInventoryData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Inventory Data|Ammo" )
	int StartingAmmoAmount = 16;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Inventory Data|Ammo" )
	int MaxAmmoAmountCapaicty = 999;
	
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Inventory Data|Healing Syringe" )
	int StartingSyringeSegment = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Inventory Data|Healing Syringe" )
	int MaxSyringeSegmentCapacity = 5;
};
