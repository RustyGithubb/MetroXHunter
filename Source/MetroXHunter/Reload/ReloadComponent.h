#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Checkpoint/Saveable.h"
#include "ReloadComponent.generated.h"

class UReloadData;
class UInventoryComponent;
class UHealthComponent;
class UInputAction;
class ABaseMetroPlayerCharacter;

// Enumeration for the gun's state
UENUM( BlueprintType )
enum class EGunState : uint8
{
    Idle,
    Firing,
    Reloading,
};

// Enumeration for the reload state
UENUM( BlueprintType )
enum class EReloadState : uint8
{
    Start,
    Normal,
    Perfect,
    Failed,
    Cancel,
};

UCLASS( BlueprintType, meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UReloadComponent : public UActorComponent, public ISaveable
{
    GENERATED_BODY()

public:
    UReloadComponent();

    virtual void BeginPlay() override;

    // Begin ISaveable Interface
    virtual void OnSaveData_Implementation( const FGuid& ActorID, UMetroSaveGame* SaveGame );
    virtual void OnLoadData_Implementation( const FGuid& ActorID, UMetroSaveGame* SaveGame );
    // End ISaveable Interface

    // Set up player input component for reloading
    void SetupPlayerInputComponent();

    // Functions related to reloading

    // Set the current ammo count in the magazine
    UFUNCTION( BlueprintCallable, Category = "Reload" )
    void SetAmmoCount( int NewCount ); 

    // Decrease the ammo count by one
    UFUNCTION( BlueprintCallable, Category = "Reload" )
    void DecrementAmmo(); 

    // Compute the new ammo counts after reloading
    UFUNCTION( BlueprintCallable, Category = "Reload" )
    void ComputeReloadAmmoCount( int& NewMagazineAmmoCount, int& InventoryAmmoConsumed ); 

    // Cancel the current reload process
    UFUNCTION( BlueprintCallable, Category = "Reload" )
    void CancelReload(); 

    // Finalize the reload process with given parameters
    UFUNCTION( BlueprintCallable, Category = "Reload" )
    void FinalizeReload( int NewAmmoCount, int InventoryAmmoCountUsed, float ReloadDuration ); 

    // Check if the gun is currently reloading
    UFUNCTION( BlueprintCallable, Category = "Reload" )
    bool IsReloading() const; 

    // Check if the magazine is full
    UFUNCTION( BlueprintCallable, Category = "Reload" )
    bool IsAmmoFull() const; 

    // Check if the magazine is empty
    UFUNCTION( BlueprintCallable, Category = "Reload" )
    bool IsGunEmpty() const; 

    // Get the current ammo count
    UFUNCTION( BlueprintCallable, Category = "Reload" )
    void GetCurrentAmmo( int& CurrentAmmo ) const; 

    // Get the maximum ammo capacity
    UFUNCTION( BlueprintCallable, Category = "Reload" )
    void GetMaxAmmo( int& MaxAmmo ) const; 

    // Function to process the reload based on the cursor value
    UFUNCTION( BlueprintCallable, Category = "Reload" )
    void ProcessReload( float CursorValue );

    // Start the reload timer with specified duration and state
    void StartReloadTimer( float Duration, EReloadState ReloadState );

    // EVENTS

    // Event fired when the ammo count is updated
    DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnAmmoCountUpdated );
    UPROPERTY( BlueprintAssignable, Category = "Reload|Events" )
    FOnAmmoCountUpdated OnAmmoCountUpdated; 

    // Event fired when the compute reload is triggered
    DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnComputeReload );
    UPROPERTY( BlueprintAssignable, Category = "Reload|Events" )
    FOnComputeReload OnComputeReload;


    // Event fired when reload input is received
    DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnReloadInputReceived );
    UPROPERTY( BlueprintAssignable, Category = "Reload|Events" )
    FOnReloadInputReceived OnReloadInputReceived; 

    // Event fired when the reload state changes
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnReloadStateChanged, EReloadState, ReloadType );
    UPROPERTY( BlueprintAssignable, Category = "Reload|Events" )
    FOnReloadStateChanged OnReloadStateChanged; 

    // Event fired when the reload couldn't trigger caused by a lack of ammo.
    DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnReloadLackAmmo );
    UPROPERTY( BlueprintAssignable, Category = "Reload|Events" )
    FOnReloadLackAmmo OnReloadLackAmmo;

    // Event fired when the reload is complete
    DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnReloadComplete );
    UPROPERTY( BlueprintAssignable, Category = "Reload|Events" )
    FOnReloadComplete OnReloadComplete; 

private:
    // Retrieve necessary references
    void RetrieveReferences();

    // Start the reload sequence
    void StartReloadSequence();

    // Handle reload input action
    void OnReloadInput();

    // Trigger the reload process with a given state and duration
    void TriggerReload( EReloadState ReloadState, float ReloadDuration );

    // Retrieve the player's inventory component
    void RetrievePlayerInventory();

    // Retrieve the player's health component
    void RetrievePlayerHealth();

private:
    // Current state of the gun
    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|Enum", meta = ( AllowPrivateAccess = "true" ) )
    EGunState CurrentGunState = EGunState::Idle;

    // Data asset containing reload information
    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|DataAsset", meta = ( AllowPrivateAccess = "true" ) )
    UReloadData* ReloadDataAsset = nullptr;

    // Reference to the player's inventory component
    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = "true" ) )
    UInventoryComponent* PlayerInventory = nullptr;

    // Reference to the player's health component
    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = "true" ) )
    UHealthComponent* PlayerHealth = nullptr;

    // Flag indicating whether reloading is active
    UPROPERTY( meta = ( AllowPrivateAccess = "true" ) )
    bool bIsReloadActive = false;

    // Player controller reference
    APlayerController* PlayerController = nullptr;

    // Player character reference
    ABaseMetroPlayerCharacter* PlayerCharacter = nullptr;

    // HUD reference
    AHUD* HUD = nullptr;

public:
    // Input action for reloading
    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|Inputs" )
    UInputAction* ReloadAction = nullptr;

    // Current ammo count in the magazine
    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|Ammo" )
    int CurrentAmmoInMagazine = 6;

    // Maximum ammo capacity of the magazine
    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|Ammo" )
    int MaxAmmoInMagazine = 6;

    // Timer handle for reload finalization
    FTimerHandle TimerHandleReloadFinalize;
};
