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

#include "Kismet/KismetMathLibrary.h"

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
	SetAmmoCount( MaxAmmoInMagazine );
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
	UInputComponent* PlayerInputComponent = PlayerController->InputComponent;

	// Set up action bindings
	auto EnhancedInputComponent = CastChecked<UEnhancedInputComponent>( PlayerInputComponent );

	// Interaction
	EnhancedInputComponent->BindAction(
		ReloadAction, ETriggerEvent::Started,
		this, &UReloadComponent::OnReloadInput
	);
}

void UReloadComponent::StartReloadSequence()
{
	if ( IsReloading() ) return;

	bIsReloadActive = true;
	CurrentGunState = EGunState::Reloading;
	ReloadElapsedTime = 0.0f;

	SetComponentTickEnabled( true );

	// Notify that the reload has started
	OnReloadStateChanged.Broadcast( EReloadState::Start );
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
		ComputeReloadAmmoCount( NewAmmoInMagazine, InventoryAmmoConsumed );

		FinalizeReload( NewAmmoInMagazine, ReloadAnimTime, InventoryAmmoConsumed );

		OnReloadStateChanged.Broadcast( ReloadState );
	}

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandleReloadPlayback, 
		[this]()
		{ bIsReloadActive = false; },
		FinalWaitingTime,
		false
	);
}

void UReloadComponent::RetrievePlayerInventory()
{
	AActor* Owner = GetOwner();
	if ( Owner )
	{
		PlayerInventory = CastChecked<UInventoryComponent>( Owner->GetComponentByClass( UInventoryComponent::StaticClass() ) );
	}
}

void UReloadComponent::RetrieveHUD()
{
	PlayerController = GetOwner()->GetInstigator()->GetLocalViewingPlayerController();
	verify( IsValid( PlayerController ) );

	HUD = PlayerController->GetHUD();
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
	if ( ReloadDataAsset == nullptr || ReloadDataAsset->NormalReloadDuration <= 0.0f )
	{
		OutPerfectReloadStartTime = 0.0f;
		OutActiveReloadStartTime = 0.0f;
		OutActiveReloadEndTime = 0.0f;
		return;
	}

	OutPerfectReloadStartTime = ReloadDataAsset->PerfectReloadStartTime / ReloadDataAsset->NormalReloadDuration;
	OutActiveReloadStartTime = ReloadDataAsset->ActiveReloadStartTime / ReloadDataAsset->NormalReloadDuration;
	OutActiveReloadEndTime = ReloadDataAsset->ActiveReloadEndTime / ReloadDataAsset->NormalReloadDuration;
}

float UReloadComponent::GetNormalizedReloadElapsedTime() const
{
	if ( ReloadDataAsset == nullptr || ReloadDataAsset->NormalReloadDuration <= 0.0f )
	{
		return 0.0f;
	}
	return ReloadElapsedTime / ReloadDataAsset->NormalReloadDuration;
}

void UReloadComponent::SetAmmoCount( int NewCount )
{
	CurrentAmmoInMagazine = NewCount;

	OnAmmoCountUpdated.Broadcast();
}

void UReloadComponent::DecrementAmmo()
{
	if ( CurrentAmmoInMagazine > 0 )
	{
		SetAmmoCount( CurrentAmmoInMagazine - 1 );
	}
}

void UReloadComponent::GetCurrentAmmo( int& CurrentAmmo ) const
{
	CurrentAmmo = CurrentAmmoInMagazine;
}

void UReloadComponent::GetMaxAmmo( int& MaxAmmo ) const
{
	MaxAmmo = MaxAmmoInMagazine;
}

// Computes the new ammo count after reloading based on available inventory and magazine space
void UReloadComponent::ComputeReloadAmmoCount( int& NewMagazineAmmoCount, int& InventoryAmmoConsumed )
{
	int AmmoToConsumeToMax = MaxAmmoInMagazine - CurrentAmmoInMagazine;

	// Get available ammo in the inventory
	int InventoryAmmo = PlayerInventory->GetCurrentAmmoAmount();

	if ( InventoryAmmo >= AmmoToConsumeToMax )
	{
		// Full reload possible
		NewMagazineAmmoCount = MaxAmmoInMagazine;
		InventoryAmmoConsumed = AmmoToConsumeToMax;
	}
	else
	{
		// Partial reload possible
		NewMagazineAmmoCount = CurrentAmmoInMagazine + InventoryAmmo;
		InventoryAmmoConsumed = InventoryAmmo;
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
	if ( IsAmmoFull() ) return;

	// Get ammo in inventory
	int CurrentAmmoAmount = PlayerInventory->GetCurrentAmmoAmount();

	// Check if we have infinite ammo or 0 in the inventory
	if ( !bUseInfiniteAmmo && CurrentAmmoAmount == 0 ) return;

	// Switch on the current Gun State
	if ( CurrentGunState == EGunState::Idle )
	{
		StartReloadSequence();
		return;
	}

	if ( CurrentGunState == EGunState::Firing ) return; 

	SetComponentTickEnabled( false);

	float CursorValue = ReloadDataAsset->ReloadCurve->GetFloatValue( 
		GetNormalizedReloadElapsedTime()) * 
		ReloadDataAsset->NormalReloadDuration;

	bool bIsInPerfectRange = UKismetMathLibrary::InRange_FloatFloat(
		CursorValue,
		ReloadDataAsset->PerfectReloadStartTime,
		ReloadDataAsset->ActiveReloadStartTime,
		true, // Min inclusif
		true // Max inclusif
	);

	if ( bIsInPerfectRange )
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

		if ( bIsBeforeActiveTime && CursorValue > ReloadDataAsset->PerfectReloadStartTime )
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

void UReloadComponent::FinalizeReload( int NewAmmoCount, float FinalWaitingTime, int InventoryAmmoCountUsed )
{
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandleReloadFinalize, 
		[this, NewAmmoCount, InventoryAmmoCountUsed]() 
		{
			bIsReloadActive = false;
			CurrentGunState = EGunState::Idle;

			// Update inventory with consumed ammo
			int AmmoToAdd = -InventoryAmmoCountUsed;

			if ( PlayerInventory )
			{
				PlayerInventory->AddToInventory( EPickupType::Ammo, AmmoToAdd );
			}

			SetAmmoCount( NewAmmoCount );
		},
		FinalWaitingTime, 
		false
	);
}

bool UReloadComponent::IsReloading() const
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
