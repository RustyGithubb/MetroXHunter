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
	UInputComponent* PlayerInputComponent = PlayerController->InputComponent;

	// Set up action bindings
	if ( UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( PlayerInputComponent ) )
	{
		// Interaction
		EnhancedInputComponent->BindAction( ReloadAction, ETriggerEvent::Started, this, &UReloadComponent::StartReloadSequence );
	}
}

void UReloadComponent::StartReloadSequence()
{
	if ( !bIsReloadActive && !IsGunFireLocked() )
	{
		bIsReloadActive = true;
		CurrentGunState = EGunState::Reloading;

		UpdateCurrentReloadState(CurrentReloadState);
		GetWorld()->GetTimerManager().SetTimer( TimerHandleReloadPlayback, this, &UReloadComponent::UpdateReloadGauge, ReloadDataAsset->ReloadRefreshRate, true );
	}
}

void UReloadComponent::UpdateReloadGauge()
{
	ReloadDataAsset->ReloadElapsedTime += ReloadDataAsset->ReloadRefreshRate;
	OnUpdateCursorPosition.Broadcast();

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

		// Calculate the quantity of ammo to reload
		ComputeReloadAmmoCount( NewMagazineAmmoCount, InventoryAmmoConsumed );

		FinalizeReload( NewMagazineAmmoCount, EGunReloadState::NormalFinished, ReloadDataAsset->NormalReloadAnimTime, 0 );
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
	}

	// Calculate the final waiting time
	float FinalWaitingTime = ReloadDataAsset->NormalReloadDuration - ReloadDataAsset->ReloadElapsedTime + ReloadDataAsset->FailedReloadPenaltyTime;

	// Update current state of reload
	UpdateCurrentReloadState( EGunReloadState::FailedFinished );

	// Indicate that the reload is done
	bIsReloadActive = false;
}

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
	}
}

void UReloadComponent::GetHUDFromPlayerController()
{
	PlayerController = UGameplayStatics::GetPlayerController( this, 0 );
	if ( PlayerController )
	{
		HUD = PlayerController->GetHUD();
	}
}

void UReloadComponent::GetGunReference()
{
	AActor* Owner = GetOwner();
	if ( !Owner )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Owner not found !" ) );
		return;
	}

	// Check if the owner implement the interface GunCommunication
	if ( Owner->GetClass()->ImplementsInterface( UGunCommunication::StaticClass() ) )
	{
		CharacterGun = Owner;
	}
	else
	{
		// Search among the child actors if not implemented by the owner
		UChildActorComponent* ChildActorComp = Owner->FindComponentByClass<UChildActorComponent>();
		if ( ChildActorComp )
		{
			AActor* ChildActor = ChildActorComp->GetChildActor();
			if ( ChildActor && ChildActor->GetClass()->ImplementsInterface( UGunCommunication::StaticClass() ) )
			{
				CharacterGun = ChildActor;
			}
		}
	}

	// If CharacterGun is valid, get the Max capacity ammo
	if ( CharacterGun )
	{
		MaxMagazineAmmoCount = IGunCommunication::Execute_GetMaxMagazineInAmmoCount( CharacterGun);
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "CharacterGun not found or doesn't implement interface GunCommunication !" ) );
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
	if ( !CharacterGun )
	{
		UE_LOG( LogTemp, Warning, TEXT( "CharacterGun is not valid !" ) );
		return;
	}

	if ( CharacterGun->GetClass()->ImplementsInterface( UGunCommunication::StaticClass() ) )
	{
		IGunCommunication::Execute_SetNewAmmoCount( CharacterGun, NewCount );
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "CharacterGun does not implement GunCommunication interface !" ) );
	}
	UE_LOG( LogTemp, Warning, TEXT( "Updated Ammo Count : %d"), NewCount );
}

void UReloadComponent::GetAmmoData( int& IndexMagazine, int& MaxMagazineAmmo ) const
{
	if ( !CharacterGun )
	{
		IndexMagazine = 0;
		MaxMagazineAmmo = 0;
		return;
	}

	// Cast the gun to the GunCommunication interface
	IGunCommunication* GunInterface = Cast<IGunCommunication>( CharacterGun );
	if ( GunInterface )
	{
		IndexMagazine = GunInterface->GetCurrentMagazineAmmoCount();
		MaxMagazineAmmo = GunInterface->GetMaxMagazineInAmmoCount();
	}
	else
	{
		IndexMagazine = 0;
		MaxMagazineAmmo = 0;
	}
}

void UReloadComponent::ComputeReloadAmmoCount( int& NewMagazineAmmoCount, int& InventoryAmmoConsumed )
{
	if ( !CharacterGun )
	{
		NewMagazineAmmoCount = 0;
		InventoryAmmoConsumed = 0;
		return;
	}

	// Cast the gun to the GunCommunication interface
	IGunCommunication* GunInterface = Cast<IGunCommunication>( CharacterGun );
	if ( !GunInterface )
	{
		NewMagazineAmmoCount = 0;
		InventoryAmmoConsumed = 0;
		return;
	}

	// Get the current ammo count in the magazine
	int CurrentAmmoCount = GunInterface->GetCurrentMagazineAmmoCount();
	int LMaxMagazineAmmoCount = this->MaxMagazineAmmoCount;

	// Calculate how much ammo is needed to fully reload
	int AmmoToConsumeToMax = this->MaxMagazineAmmoCount - CurrentAmmoCount;
 
	// Get the player's available ammo in inventory
	int InventoryAmmo = PlayerInventory ? PlayerInventory->GetCurrentAmoAmount() : 0;

	// Determine if we have enough ammo in the inventory
	if ( InventoryAmmo )
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
	// Check if the magazine is full
	if ( bIsAmmoFull() )
	{
		UKismetSystemLibrary::PrintText( this, FText::FromString( TEXT( " FULL AMMO CALL SCREEN FX OR POST PROCESS" ) ), true, true, FLinearColor( 0.0, 0.66f, 1.0f ), 2.0f );
		return;
	}

	// Check if we have ammo in inventory or infinite ammo
	int CurrentAmmoAmount = PlayerInventory->GetCurrentAmoAmount();
}

void UReloadComponent::FinalizeReload( int NewAmmoCount, EGunReloadState ReloadType, float FinalWaitingTime, int InventoryAmmoCountUsed )
{
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
	if ( !CharacterGun )
	{
		return false;
	}

	// Cast the gun to the GunCommunication interface
	IGunCommunication* GunInterface = Cast<IGunCommunication>( CharacterGun );
	if ( GunInterface )
	{
		int CurrentAmmo = GunInterface->GetCurrentMagazineAmmoCount();
		int MaxAmmo = GunInterface->GetMaxMagazineInAmmoCount();

		// Check if current ammo is equal to the max ammo
		return CurrentAmmo == MaxAmmo;
	}
	return false;
}
