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

	UFUNCTION( BlueprintCallable, meta = ( ReturnDisplayName = "OverflowAmount" ) )
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

	UFUNCTION( BlueprintCallable )
	int GetCurrentAmmoAmount() const;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnAmmoUpdate, int, NewAmmoCount );
	UPROPERTY( BlueprintAssignable, BlueprintCallable, Category = "Inventory|Event" )
	FOnAmmoUpdate OnAmmoUpdate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnSyringeUpdate, int, NewSyringeCount );
	UPROPERTY( BlueprintAssignable, BlueprintCallable, Category = "Inventory|Event" )
	FOnSyringeUpdate OnSyringeUpdate;

protected:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Inventory|DataAsset" )
	UInventoryData* InventoryDataAsset = nullptr;

	UPROPERTY( BlueprintReadOnly, Category = "Inventory|DataAsset" )
	int CurrentAmmoAmount = 0;

	UPROPERTY( BlueprintReadOnly, Category = "Inventory|DataAsset" )
	int CurrentSyringeSegmentAmount = 0;

private:
	AHUD* MainHUD = nullptr;

private:
	int CalculateOverflowAmount( int AmountToAdd, int& CurrentAmount, int MaxAmount );
};
