/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Checkpoint/Saveable.h"

#include "InventoryComponent.generated.h"

class AMetroPlayerCharacter;
class UInventoryData;
class AHUD;
class UWidgetComponent;
class UInputAction;
enum class EPickupType : uint8;

/*
 * The inventory of the Player. Mostly use to store the ammo & syringe amount
 */
UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UInventoryComponent : public UActorComponent, public ISaveable
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	virtual void BeginPlay() override;

	// Begin ISaveable Interface
	virtual void OnSaveData_Implementation( const FGuid& ActorID, UMetroSaveGame* SaveGame );
	virtual void OnLoadData_Implementation( const FGuid& ActorID, UMetroSaveGame* SaveGame );
	// End ISaveable Interface

	// Set up player input component to open the inventory
	void SetupPlayerInputComponent();

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


	UFUNCTION( BlueprintCallable, Category = "Inventory" )
	bool IsInventoryOpened() const;
	UFUNCTION( BlueprintPure, BlueprintCallable )
	bool IsSyringeFull();
	UFUNCTION( BlueprintCallable )
	int GetCurrentAmmoAmount() const;
	UFUNCTION( BlueprintCallable )
	int GetCurrentSyringeAmount() const;
	UFUNCTION( BlueprintCallable )
	int GetMaxSyringeCapacity() const;

private:
	// Handle inventory input action
	void OnInventoryInput();

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnAmmoUpdate, int, NewAmmoCount );
	UPROPERTY( BlueprintAssignable, BlueprintCallable, Category = "Inventory|Event" )
	FOnAmmoUpdate OnAmmoUpdate {};
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnSyringeUpdate, int, NewSyringeCount );
	UPROPERTY( BlueprintAssignable, BlueprintCallable, Category = "Inventory|Event" )
	FOnSyringeUpdate OnSyringeUpdate {};
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnLootGain, EPickupType, LootType, int, AmmoDiff );
	UPROPERTY( BlueprintAssignable, BlueprintCallable, Category = "Inventory|Event" )
	FOnLootGain OnLootGain {};
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnLootOverflow, EPickupType, LootType, int, OverflowAmount );
	UPROPERTY( BlueprintAssignable, BlueprintCallable, Category = "Inventory|Event" )
	FOnLootOverflow OnLootOverflow {};

	// Event fired when inventory input is received
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnInventoryInputReceived );
	UPROPERTY( BlueprintAssignable, BlueprintCallable, Category = "Inventory|Events" )
	FOnInventoryInputReceived OnInventoryInputReceived {};

	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnLootFirstTime );
	UPROPERTY( BlueprintAssignable, BlueprintCallable, Category = "Inventory|Events" )
	FOnLootFirstTime OnLootFirstTime {};

	// Input action to open inventory
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Inventory|Inputs" )
	UInputAction* InventoryAction = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Inventory|Data" )
	UInventoryData* InventoryDataAsset = nullptr;

private:
	// Player controller reference
	APlayerController* PlayerController = nullptr;
	AMetroPlayerCharacter* PlayerCharacter = nullptr;

	// Game instance reference
	UMetroGameInstance* GameInstance = nullptr;

	// Flag indicating whether inventory is active
	UPROPERTY(meta = ( AllowPrivateAccess = "true" ))
	bool bIsInventoryOpened = false;

	UPROPERTY( meta = ( AllowPrivateAccess = "true" ) )
	bool bIsFirstLoot = true;

public:
	UPROPERTY( BlueprintReadOnly, Category = "Inventory|Data" )
	int CurrentAmmoAmount = 0;
	UPROPERTY( BlueprintReadOnly, Category = "Inventory|Data" )
	int CurrentSyringeAmount = 0;

private:
	int CalculateOverflowAmount( int AmountToAdd, int& CurrentAmount, int MaxAmount );
};
