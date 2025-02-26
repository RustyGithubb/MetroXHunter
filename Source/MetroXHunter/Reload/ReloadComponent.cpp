/*
 * Implemented by BARRAU Benoit
 */

#include "Reload/ReloadComponent.h"
#include "Reload/ReloadData.h"
#include "HUD/MainHUD.h"
#include "Inventory/InventoryComponent.h"
#include "Interaction/PickupType.h"
#include "Character/BaseMetroPlayerCharacter.h"

#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "Library/ConvarLibrary.h"
#include "Library/UtilityLibrary.h"
#include "Library/GameplayLibrary.h"

UReloadComponent::UReloadComponent()
{
	// Disable component ticking as it's not needed by default
	PrimaryComponentTick.bCanEverTick = false;
}

void UReloadComponent::BeginPlay()
{
	Super::BeginPlay();
	// Ensure the ReloadDataAsset is valid
	verify( IsValid( ReloadDataAsset ) );

	// Retrieve necessary references and initialize the ammo count
	RetrieveReferences();
	SetAmmoCount( MaxAmmoInMagazine );
	SetupPlayerInputComponent();
}

void UReloadComponent::OnSaveData_Implementation(const FGuid& ActorID, UMetroSaveGame* SaveGame)
{
	FReloadSavedData ReloadData {};
	ReloadData.CurrentAmmoInMagazine = CurrentAmmoInMagazine;

	SaveGame->SavedReloadComponent.Add(ActorID, ReloadData );
}

void UReloadComponent::OnLoadData_Implementation(const FGuid& ActorID, UMetroSaveGame* SaveGame)
{
	if ( !SaveGame->SavedReloadComponent.Contains( ActorID ) ) return;

	CurrentAmmoInMagazine = SaveGame->SavedReloadComponent[ActorID].CurrentAmmoInMagazine;
}

void UReloadComponent::SetupPlayerInputComponent()
{
	PlayerController = UGameplayLibrary::GetPlayerControllerChecked( GetOwner() );

	// Get the InputComponent from the PlayerController
	UInputComponent* PlayerInputComponent = PlayerController->InputComponent;

	// Cast the InputComponent to EnhancedInputComponent
	auto EnhancedInputComponent = CastChecked<UEnhancedInputComponent>( PlayerInputComponent );

	// Bind the reload action to the OnReloadInput function
	EnhancedInputComponent->BindAction(
		ReloadAction, ETriggerEvent::Started,
		this, &UReloadComponent::OnReloadInput
	);
}

void UReloadComponent::OnReloadInput()
{
	// If the magazine is already full, do nothing
	if ( IsAmmoFull()) return;
	if ( PlayerCharacter->bIsUnderAction && !bIsReloadActive ) return;

	// Get the current ammo amount from the player's inventory
	int CurrentAmmoAmount = PlayerInventory->GetCurrentAmmoAmount();

	// If no ammo in inventory and infinite ammo is not enabled, do nothing
	if ( !UConvarLibrary::IsGunInfiniteAmmoEnabled() && CurrentAmmoAmount == 0 ) return;

	// If the gun is currently firing, ignore the reload input
	if ( CurrentGunState == EGunState::Firing ) return;

	// Always broadcast the event to notify the widget (e.g., to show reload UI)
	OnReloadInputReceived.Broadcast();

	// If not already reloading, start the reload sequence
	if ( !IsReloading() )
	{
		StartReloadSequence();
	}
}

void UReloadComponent::StartReloadSequence()
{
	// If already reloading, do nothing
	if ( IsReloading() ) return;

	// Set the reload state to active
	bIsReloadActive = true;
	CurrentGunState = EGunState::Reloading;

	PlayerCharacter->bIsUnderAction = true;

	// Start the normal reload timer (player did not interact)
	StartReloadTimer( ReloadDataAsset->NormalReloadDuration, EReloadState::Normal );
}

void UReloadComponent::ProcessReload( float CursorValue )
{
	// Cancel any existing reload timers
	GetWorld()->GetTimerManager().ClearTimer( TimerHandleReloadFinalize );

	// Log the cursor value for debugging purposes
	UE_LOG( LogTemp, Log, TEXT( "ProcessReload called with CursorValue: %f" ), CursorValue );

	// Determine if the reload is successful based on the cursor value
	bool bIsReloadSuccessful = ( CursorValue >= 0.35f && CursorValue <= 0.65f );

	if ( bIsReloadSuccessful )
	{
		UE_LOG( LogTemp, Log, TEXT( "Reload Succeeded!" ) );
		// If reload is successful, trigger a faster reload
		TriggerReload( EReloadState::Perfect, ReloadDataAsset->PerfectReloadDuration );
	}
	else
	{
		UE_LOG( LogTemp, Log, TEXT( "Reload Failed!" ) );
		// If reload fails, trigger the failed reload behavior
		TriggerReload( EReloadState::Failed, ReloadDataAsset->FailedReloadDuration );
	}
}

void UReloadComponent::TriggerReload( EReloadState ReloadState, float ReloadDuration )
{
	if ( ReloadState == EReloadState::Failed )
	{
		// For failed reload, set a delay before allowing the player to attempt to reload again
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandleReloadFinalize,
			[this]()
			{
				// After the delay, reset the reload state
				bIsReloadActive = false;
				CurrentGunState = EGunState::Idle;

				// Broadcast that the reload is complete (failed)
				OnReloadComplete.Broadcast();

			},
			ReloadDuration,
			false
		);

		// Notify that the reload state has changed (e.g., update UI)
		OnReloadStateChanged.Broadcast( ReloadState );

		PlayerCharacter->bIsUnderAction = false;
		return; // Do not proceed further, do not reload the weapon
	}

	// For successful or normal reloads, proceed to reload the weapon
	int NewAmmoInMagazine = 0;
	int InventoryAmmoConsumed = 0;

	// Compute the amount of ammo to reload
	if ( UConvarLibrary::IsGunInfiniteAmmoEnabled() )
	{
		// If infinite ammo is enabled, fill the magazine without consuming inventory ammo
		NewAmmoInMagazine = MaxAmmoInMagazine;
		InventoryAmmoConsumed = 0;
	}
	else
	{
		ComputeReloadAmmoCount( NewAmmoInMagazine, InventoryAmmoConsumed );
	}

	// Finalize the reload with the calculated ammo amounts
	FinalizeReload( NewAmmoInMagazine, InventoryAmmoConsumed, ReloadDuration );

	// Notify that the reload state has changed (e.g., update UI)
	OnReloadStateChanged.Broadcast( ReloadState );
}

void UReloadComponent::FinalizeReload( int NewAmmoCount, int InventoryAmmoCountUsed, float ReloadDuration )
{
	// Configure a timer to differentiate the final reload time
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandleReloadFinalize,
		[this, NewAmmoCount, InventoryAmmoCountUsed]()
		{
			// After a brief delay, reset the reload states
			bIsReloadActive = false;
			CurrentGunState = EGunState::Idle;

			// Update inventory with the ammo used
			int AmmoToAdd = -InventoryAmmoCountUsed;

			if ( PlayerInventory )
			{
				PlayerInventory->AddToInventory( EPickupType::Ammo, AmmoToAdd );
			}

			// Set new ammo count
			SetAmmoCount( NewAmmoCount );

			// Event that say reload is finished
			OnReloadComplete.Broadcast();
		},
		ReloadDuration,
		false
	);
}

void UReloadComponent::StartReloadTimer( float Duration, EReloadState ReloadState )
{
	// Configure a timer to manage the reload
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandleReloadFinalize,
		[this, ReloadState]()
		{
			// Get the duration of the reload based of his state
			float ReloadDuration = 0.0f;

			if ( ReloadState == EReloadState::Normal )
			{
				ReloadDuration = ReloadDataAsset->NormalReloadDuration;
			}
			else if ( ReloadState == EReloadState::Perfect )
			{
				ReloadDuration = ReloadDataAsset->PerfectReloadDuration;
			}
			else if ( ReloadState == EReloadState::Failed )
			{
				ReloadDuration = ReloadDataAsset->FailedReloadDuration;
			}

			// Trigger reloading with equivalent time
			TriggerReload( ReloadState, 0.5f );
		},
		Duration,
		false
	);
}

void UReloadComponent::RetrieveReferences()
{
	// Retrieve references to other components or systems
	RetrievePlayerInventory();

	PlayerCharacter = Cast<ABaseMetroPlayerCharacter>(GetOwner());
}

void UReloadComponent::RetrievePlayerInventory()
{
	AActor* Owner = GetOwner();
	if ( Owner )
	{
		// Get the InventoryComponent from the owner
		PlayerInventory = Owner->FindComponentByClass<UInventoryComponent>();
		// Ensure the PlayerInventory is valid
		verify( IsValid( PlayerInventory ) );
	}
}

void UReloadComponent::SetAmmoCount( int NewCount )
{
	// Update the current ammo in the magazine
	CurrentAmmoInMagazine = NewCount;

	// Broadcast the event to update the ammo count in the UI
	OnAmmoCountUpdated.Broadcast();
}

void UReloadComponent::DecrementAmmo()
{
	// If there is no ammo left, do nothing
	if ( CurrentAmmoInMagazine <= 0 ) return;

	// Decrease the ammo count by one and update the UI
	SetAmmoCount( CurrentAmmoInMagazine - 1 );
}

void UReloadComponent::GetCurrentAmmo( int& CurrentAmmo ) const
{
	// Return the current ammo in the magazine
	CurrentAmmo = CurrentAmmoInMagazine;
}

void UReloadComponent::GetMaxAmmo( int& MaxAmmo ) const
{
	// Return the maximum ammo capacity of the magazine
	MaxAmmo = MaxAmmoInMagazine;
}

void UReloadComponent::ComputeReloadAmmoCount( int& NewMagazineAmmoCount, int& InventoryAmmoConsumed )
{
	OnComputeReload.Broadcast();

	// Calculate how much ammo is needed to fill the magazine
	int AmmoNeeded = MaxAmmoInMagazine - CurrentAmmoInMagazine;

	// Get the ammo available in the player's inventory
	int InventoryAmmo = PlayerInventory->GetCurrentAmmoAmount();

	if ( InventoryAmmo >= AmmoNeeded )
	{
		// Enough ammo in the inventory to fully reload
		NewMagazineAmmoCount = MaxAmmoInMagazine;
		InventoryAmmoConsumed = AmmoNeeded;
	}
	else
	{
		// Not enough ammo, reload as much as possible
		NewMagazineAmmoCount = CurrentAmmoInMagazine + InventoryAmmo;
		InventoryAmmoConsumed = InventoryAmmo;
	}
}

void UReloadComponent::CancelReload()
{
	// Reset any existing reload timers
	GetWorld()->GetTimerManager().ClearTimer( TimerHandleReloadFinalize );

	// Reset the reload state
	bIsReloadActive = false;
	CurrentGunState = EGunState::Idle;

	// Notify that the reload has been canceled
	OnReloadStateChanged.Broadcast( EReloadState::Cancel );

	// Optionally broadcast that the reload is complete
	OnReloadComplete.Broadcast();
}

bool UReloadComponent::IsReloading() const
{
	// Return whether the gun is currently reloading
	return bIsReloadActive;
}

bool UReloadComponent::IsAmmoFull() const
{
	// Check if the magazine is full
	return CurrentAmmoInMagazine == MaxAmmoInMagazine;
}

bool UReloadComponent::IsGunEmpty() const
{
	// Check if the magazine is empty
	return CurrentAmmoInMagazine == 0;
}