/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UInventoryData;
class AHUD;
enum class EPickupType : uint8;

/*
 * The inventory of the Player. Mostly use to store the ammo & syringe amount
 */
UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();
	virtual void BeginPlay() override;
	void LateBeginPlay();

	UFUNCTION( BlueprintCallable , meta = ( ReturnDisplayName = "Overflow Amount") )
	int AddToInventory( EPickupType PickupType, int Amount );

	UFUNCTION( BlueprintCallable )
	void ClearSyringeSegments();

	UFUNCTION( BlueprintCallable )
	void ClearAmmoAmount();

	UFUNCTION( BlueprintCallable )
	void FullyFillSyringe();

	UFUNCTION( BlueprintCallable )
	void FullyFillAmmo();

	UFUNCTION( BlueprintPure, BlueprintCallable )
	bool IsSyringeFull();

protected:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Inventory|Data Asset" )
	UInventoryData* InventoryDataAsset = nullptr;

	UPROPERTY( BlueprintReadOnly, Category = "Inventory|Data Asset" )
	int CurrentAmmoAmount = 0;

	UPROPERTY( BlueprintReadOnly, Category = "Inventory|Data Asset" )
	int CurrentSyringeSegmentAmount = 0;

private:
	AHUD* MainHUD = nullptr;

private:
	int CalculateOverflowAmount( int AmountToAdd, int& CurrentAmount, int MaxAmount);
};
