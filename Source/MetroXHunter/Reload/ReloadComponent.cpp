/*
 * Implemented by BARRAU Benoit
 */

#include "Reload/ReloadComponent.h"
#include "Reload/ReloadData.h"
#include "HUD/MainHUD.h"
#include "Reload/IReloadSystemUpdate.h"
#include "Gun/GunCommunication.h"
#include "Inventory/InventoryComponent.h"
#include "Interaction/PickupType.h"

#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "GameplayTagContainer.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

UReloadComponent::UReloadComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UReloadComponent::BeginPlay()
{
	Super::BeginPlay();

	GetReferences();
	UpdateAmmoCount( MaxMagazineAmmoCount );
	SetupPlayerInputComponent();
}

void UReloadComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

void UReloadComponent::SetupPlayerInputComponent()
{
	if ( !PlayerController )
	{
		UE_LOG( LogTemp, Warning, TEXT( "PlayerController is not initialized!" ) );
		return;
	}

	UInputComponent* PlayerInputComponent = PlayerController->InputComponent;

	// Set up action bindings
	if ( UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( PlayerInputComponent ) )
	{
		// Interaction
		EnhancedInputComponent->BindAction( ReloadAction, ETriggerEvent::Started, this, &UReloadComponent::StartReloadSequence );
		UE_LOG( LogTemp, Warning, TEXT( "PlayerController  initialized !" ) );
	}
}

void UReloadComponent::StartReloadSequence()
{
	if ( !bIsReloadActive && !IsGunFireLocked() )
	{
		bIsReloadActive = true;
		CurrentGunState = EGunState::Reloading;

		// Notify that the reload has started
		OnReloadState.Broadcast(EReloadState::Start);
		// Update the current reload state
		UpdateCurrentReloadState(CurrentReloadState);
		// Start the timer to update the reload gauge
		GetWorld()->GetTimerManager().SetTimer( TimerHandleReloadPlayback, this, &UReloadComponent::UpdateReloadGauge, ReloadDataAsset->ReloadRefreshRate, true );
	}
}

// Updates the reload gauge over time
void UReloadComponent::UpdateReloadGauge()
{
	ReloadDataAsset->ReloadElapsedTime += ReloadDataAsset->ReloadRefreshRate;
	OnUpdateCursorPosition.Broadcast();

	// If the reload has reached its normal duration, trigger a normal reload
	if ( ReloadDataAsset->ReloadElapsedTime >= ReloadDataAsset->NormalReloadDuration )
	{
		GetWorld()->GetTimerManager().ClearTimer( TimerHandleReloadPlayback );
		TriggerNormalReload();
	}
}

void UReloadComponent::TriggerNormalReload()
{
	// Check if the player has infinite ammo
	if ( bUseInfiniteAmmo )
	{
		FinalizeReload( MaxMagazineAmmoCount, EGunReloadState::NormalFinished, ReloadDataAsset->NormalReloadAnimTime, 0 );
	}
	else
	{
		int NewMagazineAmmoCount;
		int InventoryAmmoConsumed;

		// Calculate the amount of ammo to reload
		ComputeReloadAmmoCount( NewMagazineAmmoCount, InventoryAmmoConsumed );

		FinalizeReload( NewMagazineAmmoCount, EGunReloadState::NormalFinished, ReloadDataAsset->NormalReloadAnimTime, 0 );

		OnReloadState.Broadcast( EReloadState::Normal);
	}

	// Update current state of reload
	UpdateCurrentReloadState(EGunReloadState::NormalFinished);

	// Desactivate the locking of the weapon after un brief delay
	UKismetSystemLibrary::Delay( this, ReloadDataAsset->NormalReloadAnimTime, FLatentActionInfo() );

	// Indicate that the reload is done
	bIsReloadActive = false;
}

void UReloadComponent::TriggerActiveReload()
{
	// Check if the player has infinite ammo
	if ( bUseInfiniteAmmo )
	{
		FinalizeReload( MaxMagazineAmmoCount, EGunReloadState::ActiveFinished, ReloadDataAsset->ActiveReloadAnimTime, 0 );
	}
	else
	{
		int NewMagazineAmmoCount;
		int InventoryAmmoConsumed;

		// Calculate the quantity of ammo to reload
		ComputeReloadAmmoCount( NewMagazineAmmoCount, InventoryAmmoConsumed );

		FinalizeReload( NewMagazineAmmoCount, EGunReloadState::ActiveFinished, ReloadDataAsset->ActiveReloadAnimTime, 0 );

		OnReloadState.Broadcast( EReloadState::Active );
	}

	// Calculate the final waiting time
	float FinalWaitingTime = ReloadDataAsset->ActiveReloadEndTime - ReloadDataAsset->ReloadElapsedTime + ReloadDataAsset->ActiveReloadAnimTime;

	// Update current state of reload
	UpdateCurrentReloadState(EGunReloadState::ActiveFinished);

	// Indicate that the reload is done
	bIsReloadActive = false;
}

void UReloadComponent::TriggerPerfectReload()
{
	// Check if the player has infinite ammo
	if ( bUseInfiniteAmmo )
	{
		FinalizeReload( MaxMagazineAmmoCount, EGunReloadState::PerfectFinished, ReloadDataAsset->PerfectReloadAnimTime, 0 );
	}
	else
	{
		int NewMagazineAmmoCount;
		int InventoryAmmoConsumed;

		// Calculate the quantity of ammo to reload
		ComputeReloadAmmoCount( NewMagazineAmmoCount, InventoryAmmoConsumed );

		FinalizeReload( NewMagazineAmmoCount, EGunReloadState::PerfectFinished, ReloadDataAsset->PerfectReloadAnimTime, 0 );

		OnReloadState.Broadcast( EReloadState::Perfect );
	}

	// Calculate the final waiting time
	float FinalWaitingTime = ReloadDataAsset->ActiveReloadStartTime - ReloadDataAsset->ReloadElapsedTime + ReloadDataAsset->PerfectReloadAnimTime;

	// Update current state of reload
	UpdateCurrentReloadState( EGunReloadState::PerfectFinished );

	// Indicate that the reload is done
	bIsReloadActive = false;
}

void UReloadComponent::TriggerFailedReload()
{
	// Check if the player has infinite ammo
	if ( bUseInfiniteAmmo )
	{
		FinalizeReload( MaxMagazineAmmoCount, EGunReloadState::FailedFinished, ReloadDataAsset->FailedReloadPenaltyTime, 0 );
	}
	else
	{
		int NewMagazineAmmoCount;
		int InventoryAmmoConsumed;

		// Calculate the quantity of ammo to reload
		ComputeReloadAmmoCount( NewMagazineAmmoCount, InventoryAmmoConsumed );

		FinalizeReload( NewMagazineAmmoCount, EGunReloadState::FailedFinished, ReloadDataAsset->FailedReloadPenaltyTime, 0 );

		OnReloadState.Broadcast( EReloadState::Failed );
	}

	// Calculate the final waiting time
	float FinalWaitingTime = ReloadDataAsset->NormalReloadDuration - ReloadDataAsset->ReloadElapsedTime + ReloadDataAsset->FailedReloadPenaltyTime;

	// Update current state of reload
	UpdateCurrentReloadState( EGunReloadState::FailedFinished );

	// Indicate that the reload is done
	bIsReloadActive = false;
}

// Updates the current reload state and informs the HUD (if available)
void UReloadComponent::UpdateCurrentReloadState( EGunReloadState NewState )
{
	CurrentReloadState = NewState;

	if ( HUD )
	{
		if ( IIReloadSystemUpdate* ReloadSystemInterface = Cast<IIReloadSystemUpdate>( HUD ) )
		{
			ReloadSystemInterface->UpdateReloadSystemState( CurrentReloadState );
		}
	}
}

void UReloadComponent::UpdateCurrentGunState( EGunState NewState )
{
	CurrentGunState = NewState;
}

void UReloadComponent::GetPlayerInventory()
{
	AActor* Owner = GetOwner();
	if ( Owner )
	{
		PlayerInventory = Cast<UInventoryComponent>( Owner->GetComponentByClass( UInventoryComponent::StaticClass() ) );
		UE_LOG( LogTemp, Warning, TEXT( "Inventory found !" ) );
	}
}

void UReloadComponent::GetHUDFromPlayerController()
{
	PlayerController = UGameplayStatics::GetPlayerController( this, 0 );
	if ( PlayerController )
	{
		HUD = PlayerController->GetHUD();
		UE_LOG( LogTemp, Warning, TEXT( "HUD found !" ) );
	}
}

// Retrieves a reference to the gun that implements the GunCommunication interface
void UReloadComponent::GetGunReference()
{
	AActor* Owner = GetOwner();
	if ( !Owner )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Owner not found!" ) );
		return;
	}

	UE_LOG( LogTemp, Warning, TEXT( "Owner found: %s" ), *Owner->GetName() );

	// Find the gun that implements the GunCommunication interface
	if ( Owner->GetClass()->ImplementsInterface( UGunCommunication::StaticClass() ) )
	{
		CharacterGun = Owner;
		GunInterface = Cast<IGunCommunication>( CharacterGun );
		UE_LOG( LogTemp, Warning, TEXT( "Owner implements GunCommunication interface" ) );
	}
	else
	{
		UChildActorComponent* ChildActorComp = Owner->FindComponentByClass<UChildActorComponent>();
		if ( ChildActorComp )
		{
			AActor* ChildActor = ChildActorComp->GetChildActor();
			if ( ChildActor && ChildActor->GetClass()->ImplementsInterface( UGunCommunication::StaticClass() ) )
			{
				CharacterGun = ChildActor;
				GunInterface = Cast<IGunCommunication>( CharacterGun );
				UE_LOG( LogTemp, Warning, TEXT( "ChildActor found and implements GunCommunication interface: %s" ), *ChildActor->GetName() );
			}
		}
	}

	if ( GunInterface )
	{
		MaxMagazineAmmoCount = GunInterface->Execute_GetMaxMagazineInAmmoCount( CharacterGun );
		UE_LOG( LogTemp, Warning, TEXT( "Gun interface initialized, MaxMagazineAmmoCount: %d" ), MaxMagazineAmmoCount );
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "CharacterGun not found or doesn't implement interface GunCommunication!" ) );
	}
}


EGunReloadState UReloadComponent::GetCurrentReloadState() const
{
	return CurrentReloadState;
}

void UReloadComponent::InitializeReloadData( UReloadData* NewReloadData )
{
	if ( NewReloadData )
	{
		ReloadDataAsset = NewReloadData;
		UE_LOG( LogTemp, Warning, TEXT( "Reload Data Asset initialized !" ));
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "Failed to initialize Reload Data Asset !" ));
	}
}

void UReloadComponent::GetNormalizedReloadTimings( float& OutPerfectReloadStartTime, float& OutActiveReloadStartTime, float& OutActiveReloadEndTime ) const
{
	if ( ReloadDataAsset && ReloadDataAsset->NormalReloadDuration > 0.0f )
	{
		OutPerfectReloadStartTime = ReloadDataAsset->PerfectReloadStartTime / ReloadDataAsset->NormalReloadDuration;
		OutActiveReloadStartTime = ReloadDataAsset->ActiveReloadStartTime / ReloadDataAsset->NormalReloadDuration;
		OutActiveReloadEndTime = ReloadDataAsset->ActiveReloadEndTime / ReloadDataAsset->NormalReloadDuration;
	}
	else
	{
		OutPerfectReloadStartTime = 0.0f;
		OutActiveReloadStartTime = 0.0f;
		OutActiveReloadEndTime = 0.0f;
	}
}

float UReloadComponent::GetNormalizedReloadElapsedTime() const
{
	if ( ReloadDataAsset && ReloadDataAsset->NormalReloadDuration > 0.0f )
	{
		return ReloadDataAsset->ReloadElapsedTime / ReloadDataAsset->NormalReloadDuration;
	}
	else
	{
		return 0.0f;
	}
}

void UReloadComponent::UpdateAmmoCount( int NewCount )
{
	if ( !GunInterface )
	{
		UE_LOG( LogTemp, Warning, TEXT( "GunInterface is not valid!" ) );
		return;
	}

	GunInterface->Execute_SetNewAmmoCount( CharacterGun, NewCount );
	UE_LOG( LogTemp, Warning, TEXT( "Updated Ammo Count : %d" ), NewCount );
}

void UReloadComponent::GetAmmoData( int& IndexMagazine, int& MaxMagazineAmmo ) const
{
	if ( !GunInterface )
	{
		IndexMagazine = 0;
		MaxMagazineAmmo = 0;
		UE_LOG( LogTemp, Warning, TEXT( "GunInterface is not valid! 0 AMMO" ) );
		return;
	}

	// Use the already available interface pointer
	IndexMagazine = GunInterface->Execute_GetCurrentMagazineAmmoCount( CharacterGun );
	MaxMagazineAmmo = GunInterface->Execute_GetMaxMagazineInAmmoCount( CharacterGun );
	UE_LOG( LogTemp, Warning, TEXT( "Ammo data retrieved from GunInterface" ) );
}

// Computes the new ammo count after reloading based on available inventory and magazine space
void UReloadComponent::ComputeReloadAmmoCount( int& NewMagazineAmmoCount, int& InventoryAmmoConsumed )
{
	if ( !GunInterface )
	{
		NewMagazineAmmoCount = 0;
		InventoryAmmoConsumed = 0;
		return;
	}

	// Get the current ammo count in the magazine using the GunInterface
	int CurrentAmmoCount = GunInterface->Execute_GetCurrentMagazineAmmoCount( CharacterGun );
	int AmmoToConsumeToMax = MaxMagazineAmmoCount - CurrentAmmoCount;

	// Get the player's available ammo in inventory
	int InventoryAmmo = PlayerInventory ? PlayerInventory->GetCurrentAmmoAmount() : 0;

	// Determine if we have enough ammo in the inventory
	if ( InventoryAmmo >= AmmoToConsumeToMax )
	{
		// Full reload possible
		NewMagazineAmmoCount = MaxMagazineAmmoCount;
		InventoryAmmoConsumed = AmmoToConsumeToMax;
	}
	else
	{
		// Partial Reload
		NewMagazineAmmoCount = CurrentAmmoCount + InventoryAmmo;
		InventoryAmmoConsumed = InventoryAmmo;
	}
}

void UReloadComponent::GetReferences()
{
	GetPlayerInventory();
	GetHUDFromPlayerController();
	GetGunReference();
}

void UReloadComponent::OnReloadInput()
{
	UE_LOG( LogTemp, Warning, TEXT( "ON RELOAD INPUT" ));
	// Check if the magazine is full
	if ( bIsAmmoFull() )
	{
		UKismetSystemLibrary::PrintText( this, FText::FromString( TEXT( " FULL AMMO CALL SCREEN FX OR POST PROCESS" ) ), true, true, FLinearColor( 0.0, 0.66f, 1.0f, 1.0f ), 2.0f );
		UE_LOG( LogTemp, Warning, TEXT( "FULL AMMO CALL SCREEN FX OR POST PROCESS" ) );
		return;
	}

	// Check if we have ammo in inventory or infinite ammo
	int CurrentAmmoAmount = PlayerInventory->GetCurrentAmmoAmount();
	bool bHasAmmo = ( bUseInfiniteAmmo || CurrentAmmoAmount > 0 );

	if ( !bHasAmmo )
	{
		UKismetSystemLibrary::PrintText( this, FText::FromString( TEXT( "Can't reload ! No ammo available in the inventory !" ) ), true, true, FLinearColor(1.0f, 0.0f, 0.46f,1.0f ), 5.0f);
		UE_LOG( LogTemp, Warning, TEXT( "Can't reload ! No ammo available in the inventory !" ) );
		return;
	}

	// Switch on the current Gun State
	switch ( CurrentGunState )
	{
	case EGunState::Idle:
		StartReloadSequence();
		UE_LOG( LogTemp, Warning, TEXT( "START RELOAD SEQUENCE" ) );
		break;

	case EGunState::Firing:
		UKismetSystemLibrary::PrintText( this, FText::FromString( TEXT( "Nothing for now !" ) ), true, true, FLinearColor( 1.0f, 0.0f, 0.46f, 1.0f ), 5.0f );
		UE_LOG( LogTemp, Warning, TEXT( "NOTHING FOR NOW" ) );
		break;

	case EGunState::Reloading:
		UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle( this, TimerHandleReloadPlayback );
		UE_LOG( LogTemp, Warning, TEXT( "DEFINE THE RELOAD BASED ON THE TIMING" ) );
		break;

	default:
		break;
	}

	bool bIsInPerfectReloadRange = UKismetMathLibrary::InRange_FloatFloat(
		ReloadDataAsset->ReloadElapsedTime,
		ReloadDataAsset->PerfectReloadStartTime,
		ReloadDataAsset->ActiveReloadEndTime,
		true, // Min inclusif
		true // Max inclusif
	);

	if ( bIsInPerfectReloadRange )
	{
		// If in the perfect reload range call TriggerPerfectReload
		TriggerPerfectReload();
		UE_LOG( LogTemp, Warning, TEXT( "PERFECT RELOAD" ) );
	}
	else
	{
		// Compare the ReloadElapsedTime with the ActiveReloadStartTime
		bool bIsBeforeActiveTime = ReloadDataAsset->ReloadElapsedTime < ReloadDataAsset->ActiveReloadStartTime;

		if ( bIsBeforeActiveTime )
		{
			TriggerActiveReload();
			UE_LOG( LogTemp, Warning, TEXT( "ACTIVE RELOAD" ) );
		}
		else
		{
			TriggerFailedReload();
			UE_LOG( LogTemp, Warning, TEXT( "FAILED RELOAD" ) );
		}
	}
}

void UReloadComponent::FinalizeReload( int NewAmmoCount, EGunReloadState ReloadType, float FinalWaitingTime, int InventoryAmmoCountUsed )
{
	UE_LOG( LogTemp, Warning, TEXT( "FINALIZE RELOAD" ) );
	// Update Reload State
	UpdateCurrentReloadState( ReloadType );

	// Delay for the final waiting time
	UKismetSystemLibrary::Delay( this, FinalWaitingTime, FLatentActionInfo() );

	// Update the ammo count after the reload
	UpdateAmmoCount( NewAmmoCount );

	// Update current Gun State
	CurrentGunState = EGunState::Reloading;

	// Multiply the number of ammo used by -1 and add them to the inventory
	int AmmoToAdd = InventoryAmmoCountUsed * -1;
	PlayerInventory->AddToInventory( EPickupType::Ammo, AmmoToAdd );
}

bool UReloadComponent::IsGunFireLocked() const
{
	return bIsReloadActive;
}

bool UReloadComponent::bIsAmmoFull() const
{
	if ( !GunInterface )
	{
		return false;
	}

	// Use GunInterface to get the ammo information
	int CurrentAmmo = GunInterface->Execute_GetCurrentMagazineAmmoCount( CharacterGun );
	int MaxAmmo = GunInterface->Execute_GetMaxMagazineInAmmoCount( CharacterGun );
	return CurrentAmmo == MaxAmmo;
}
