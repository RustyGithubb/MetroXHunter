/*
 * Implemented by BARRAU Benoit
 */

#include "Reload/ReloadComponent.h"
#include "Reload/ReloadData.h"
#include "HUD/MainHUD.h"
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
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UReloadComponent::BeginPlay()
{
	Super::BeginPlay();

	RetrieveReferences();
	UpdateAmmoCount( MaxAmmoInMagazine );
	SetupPlayerInputComponent();
}

void UReloadComponent::TickComponent( 
	float DeltaTime, 
	ELevelTick TickType, 
	FActorComponentTickFunction* ThisTickFunction 
)
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	ReloadElapsedTime += DeltaTime;
	OnUpdateCursorPosition.Broadcast();

	// If the reload has reached its normal duration, trigger a normal reload
	if ( ReloadElapsedTime >= ReloadDataAsset->NormalReloadDuration )
	{
		SetComponentTickEnabled( false );

		TriggerReload( EReloadState::Normal,
			ReloadDataAsset->NormalReloadAnimTime,
			ReloadDataAsset->NormalReloadAnimTime
		);
	}
}

void UReloadComponent::SetupPlayerInputComponent()
{
	if ( !PlayerController )
	{
		return;
	}

	UInputComponent* PlayerInputComponent = PlayerController->InputComponent;

	// Set up action bindings
	if ( UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( PlayerInputComponent ) )
	{
		// Interaction
		EnhancedInputComponent->BindAction( 
			ReloadAction, ETriggerEvent::Started, 
			this, &UReloadComponent::OnReloadInput 
		);
	}
}

void UReloadComponent::StartReloadSequence()
{
	if ( !bIsReloadActive && !IsGunFireLocked() )
	{
		bIsReloadActive = true;
		CurrentGunState = EGunState::Reloading;
		ReloadElapsedTime = 0.0f;

		SetComponentTickEnabled( true );

		// Notify that the reload has started
		OnReloadStateChanged.Broadcast(EReloadState::Start);
	}
}

void UReloadComponent::TriggerReload( EReloadState ReloadState, float ReloadAnimTime, float FinalWaitingTime )
{
	if ( bUseInfiniteAmmo )
	{
		FinalizeReload( MaxAmmoInMagazine, ReloadAnimTime, 0 );
	}
	else
	{
		int NewAmmoInMagazine;
		int InventoryAmmoConsumed;

		// Calculate the quantity of ammo to reload
		ComputeReloadAmmoCount(NewAmmoInMagazine, InventoryAmmoConsumed);

		FinalizeReload( NewAmmoInMagazine, ReloadAnimTime, InventoryAmmoConsumed );

		OnReloadStateChanged.Broadcast(ReloadState);
	}

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandleReloadPlayback, [this]()
		{ bIsReloadActive = false; },
		FinalWaitingTime,
		false
	);
}

void UReloadComponent::UpdateCurrentGunState( EGunState NewState )
{
	CurrentGunState = NewState;
}

void UReloadComponent::RetrievePlayerInventory()
{
	AActor* Owner = GetOwner();
	if ( Owner )
	{
		PlayerInventory = Cast<UInventoryComponent>( Owner->GetComponentByClass( UInventoryComponent::StaticClass() ) );
	}
}


void UReloadComponent::RetrieveHUD()
{
	PlayerController = UGameplayStatics::GetPlayerController( this, 0 );
	if ( PlayerController )
	{
		HUD = PlayerController->GetHUD();
	}
}

void UReloadComponent::InitializeReloadData( UReloadData* NewReloadData )
{
	if ( NewReloadData )
	{
		ReloadDataAsset = NewReloadData;
	}
}

void UReloadComponent::GetNormalizedReloadTimings( 
	float& OutPerfectReloadStartTime, 
	float& OutActiveReloadStartTime, 
	float& OutActiveReloadEndTime 
) const
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
		return ReloadElapsedTime / ReloadDataAsset->NormalReloadDuration;
	}
	else
	{
		return 0.0f;
	}
}

void UReloadComponent::UpdateAmmoCount( int NewCount )
{
	CurrentAmmoInMagazine = NewCount;

	OnAmmoCountUpdated.Broadcast();
}

void UReloadComponent::GetAmmoData( int& CurrentAmmo, int& MaxAmmo ) const
{
	CurrentAmmo = CurrentAmmoInMagazine;
	MaxAmmo = MaxAmmoInMagazine;
}

// Computes the new ammo count after reloading based on available inventory and magazine space
void UReloadComponent::ComputeReloadAmmoCount( int& NewMagazineAmmoCount, int& InventoryAmmoConsumed )
{
	int AmmoToConsumeToMax = MaxAmmoInMagazine - CurrentAmmoInMagazine;

	// Obtenir les munitions disponibles dans l'inventaire
	int InventoryAmmo = PlayerInventory ? PlayerInventory->GetCurrentAmmoAmount() : 0;

	if ( InventoryAmmo >= AmmoToConsumeToMax )
	{
		// Recharge complète possible
		NewMagazineAmmoCount = MaxAmmoInMagazine;
		InventoryAmmoConsumed = AmmoToConsumeToMax;
	}
	else
	{
		// Recharge partielle
		NewMagazineAmmoCount = CurrentAmmoInMagazine + InventoryAmmo;
		InventoryAmmoConsumed = InventoryAmmo;
	}
}


void UReloadComponent::DecrementAmmo()
{
	if ( CurrentAmmoInMagazine > 0 )
	{
		--CurrentAmmoInMagazine;
		OnAmmoCountUpdated.Broadcast();
	}
}

void UReloadComponent::RetrieveReferences()
{
	RetrievePlayerInventory();
	RetrieveHUD();
}

void UReloadComponent::OnReloadInput()
{
	// Check if the magazine is full
	if ( IsAmmoFull() )
	{
		return;
	}

	// Check if we have ammo in inventory or infinite ammo
	int CurrentAmmoAmount = PlayerInventory->GetCurrentAmmoAmount();
	if ( !bUseInfiniteAmmo && CurrentAmmoAmount == 0 )
	{
		return;
	}

	// Switch on the current Gun State
	if ( CurrentGunState == EGunState::Idle )
	{
		StartReloadSequence();
		return;
	}

	if ( CurrentGunState == EGunState::Firing )
	{
		return; 
	}

	SetComponentTickEnabled( false);

	float CursorValue = ReloadDataAsset->ReloadCurve->GetFloatValue( 
		GetNormalizedReloadElapsedTime()) * 
		ReloadDataAsset->NormalReloadDuration;

	bool bIsInPerfectReloadRange = UKismetMathLibrary::InRange_FloatFloat(
		CursorValue,
		ReloadDataAsset->PerfectReloadStartTime,
		ReloadDataAsset->ActiveReloadStartTime,
		true, // Min inclusif
		true // Max inclusif
	);

	if ( bIsInPerfectReloadRange )
	{
		// If in the perfect reload range call TriggerPerfectReload
		float FinalPerfectWaitingTime = 
			ReloadDataAsset->ActiveReloadStartTime - 
			ReloadElapsedTime + 
			ReloadDataAsset->PerfectReloadAnimTime;

		TriggerReload(EReloadState::Perfect, 
			ReloadDataAsset->PerfectReloadAnimTime, 
			FinalPerfectWaitingTime
		);
	}
	else
	{
		// Compare the ReloadElapsedTime with the ActiveReloadStartTime
		bool bIsBeforeActiveTime = CursorValue < ReloadDataAsset->ActiveReloadEndTime;

		if ( bIsBeforeActiveTime && CursorValue > ReloadDataAsset->PerfectReloadStartTime)
		{
			float FinalActiveWaitingTime = 
				ReloadDataAsset->ActiveReloadEndTime - 
				ReloadElapsedTime + 
				ReloadDataAsset->ActiveReloadAnimTime;

			TriggerReload( EReloadState::Active, 
				ReloadDataAsset->ActiveReloadAnimTime, 
				FinalActiveWaitingTime 
			);
		}
		else
		{
			float FinalFaileWaitingTime = 
				ReloadDataAsset->NormalReloadDuration - 
				ReloadElapsedTime + 
				ReloadDataAsset->FailedReloadPenaltyTime;

			TriggerReload(EReloadState::Failed, 
				ReloadDataAsset->FailedReloadPenaltyTime, 
				FinalFaileWaitingTime
			);
		}
	}
}

void UReloadComponent::FinalizeReload(int NewAmmoCount, float FinalWaitingTime, int InventoryAmmoCountUsed)
{
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandleReloadFinalize, [this, NewAmmoCount, InventoryAmmoCountUsed]() 
		{
			bIsReloadActive = false;
			CurrentGunState = EGunState::Idle;

			// Update inventory with consumed ammo
			int AmmoToAdd = InventoryAmmoCountUsed * -1;

			if ( PlayerInventory )
			{
				PlayerInventory->AddToInventory( EPickupType::Ammo, AmmoToAdd );
			}

			UpdateAmmoCount( NewAmmoCount );
		},
		FinalWaitingTime, false);
}

bool UReloadComponent::IsGunFireLocked() const
{
	return bIsReloadActive;
}

bool UReloadComponent::IsAmmoFull() const
{
	return CurrentAmmoInMagazine == MaxAmmoInMagazine;
}

bool UReloadComponent::IsGunEmpty() const
{
	return CurrentAmmoInMagazine == 0;
}