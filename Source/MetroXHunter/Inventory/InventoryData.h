/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InventoryData.generated.h"

/*
 * Data asset representing the initial amounts of the player's inventory.
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
