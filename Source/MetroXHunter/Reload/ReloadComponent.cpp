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

	UE_LOG( LogTemp, Warning, TEXT( "Update Reload Gauge" ) );
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
		EnhancedInputComponent->BindAction( 
			ReloadAction, ETriggerEvent::Started, 
			this, &UReloadComponent::OnReloadInput 
		);
		UE_LOG( LogTemp, Warning, TEXT( "PlayerController  initialized !" ) );
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
		
		UE_LOG( LogTemp, Warning, TEXT( "START RELOAD SEQUENCE" ) );
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
		if ( !PlayerInventory )
		{
			UE_LOG( LogTemp, Warning, TEXT( "Player inventory is not valid!" ) );
		}
		else
		{
			UE_LOG( LogTemp, Warning, TEXT( "Inventory found with %d ammo!" ), PlayerInventory->GetCurrentAmmoAmount() );
		}
	}
}


void UReloadComponent::RetrieveHUD()
{
	PlayerController = UGameplayStatics::GetPlayerController( this, 0 );
	if ( PlayerController )
	{
		HUD = PlayerController->GetHUD();
		UE_LOG( LogTemp, Warning, TEXT( "HUD found !" ) );
	}
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
	UE_LOG( LogTemp, Warning, TEXT( "Updated Ammo Count : %d" ), NewCount );
}

void UReloadComponent::GetAmmoData( int& CurrentAmmo, int& MaxAmmo ) const
{
	CurrentAmmo = CurrentAmmoInMagazine;
	MaxAmmo = MaxAmmoInMagazine;
	UE_LOG( LogTemp, Warning, TEXT( "Ammo data retrieved from ReloadComponent variables" ) );
}

// Computes the new ammo count after reloading based on available inventory and magazine space
void UReloadComponent::ComputeReloadAmmoCount( int& NewMagazineAmmoCount, int& InventoryAmmoConsumed )
{
	int AmmoToConsumeToMax = MaxAmmoInMagazine - CurrentAmmoInMagazine;

	// Obtenir les munitions disponibles dans l'inventaire
	int InventoryAmmo = PlayerInventory ? PlayerInventory->GetCurrentAmmoAmount() : 0;
	UE_LOG( LogTemp, Warning, TEXT( "Inventory Ammo: %d" ), InventoryAmmo );

	if ( InventoryAmmo >= AmmoToConsumeToMax )
	{
		// Recharge complète possible
		NewMagazineAmmoCount = MaxAmmoInMagazine;
		InventoryAmmoConsumed = AmmoToConsumeToMax;
		UE_LOG( LogTemp, Warning, TEXT( "Full Reload: %d" ), InventoryAmmoConsumed );
	}
	else
	{
		// Recharge partielle
		NewMagazineAmmoCount = CurrentAmmoInMagazine + InventoryAmmo;
		InventoryAmmoConsumed = InventoryAmmo;
		UE_LOG( LogTemp, Warning, TEXT( "Partial Reload: %d" ), InventoryAmmoConsumed );
	}
}


void UReloadComponent::DecrementAmmo()
{
	if ( CurrentAmmoInMagazine > 0 )
	{
		--CurrentAmmoInMagazine;
		UE_LOG( LogTemp, Warning, TEXT( "Ammo decremented, remaining: %d" ), CurrentAmmoInMagazine );
		OnAmmoCountUpdated.Broadcast();
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "No more Ammo in the magazine!" ) );
	}
}

void UReloadComponent::RetrieveReferences()
{
	RetrievePlayerInventory();
	RetrieveHUD();
}

void UReloadComponent::OnReloadInput()
{
	UE_LOG( LogTemp, Warning, TEXT( "ON RELOAD INPUT" ));
	// Check if the magazine is full
	if ( IsAmmoFull() )
	{
		UE_LOG( LogTemp, Warning, TEXT( "FULL AMMO CALL SCREEN FX OR POST PROCESS" ) );
		return;
	}

	// Check if we have ammo in inventory or infinite ammo
	int CurrentAmmoAmount = PlayerInventory->GetCurrentAmmoAmount();
	if ( !bUseInfiniteAmmo && CurrentAmmoAmount == 0 )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Can't reload ! No ammo available in the inventory !" ) );
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
		UE_LOG( LogTemp, Warning, TEXT( "NOTHING FOR NOW" ) );
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
		UE_LOG( LogTemp, Warning, TEXT( "PERFECT RELOAD" ) );
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
			UE_LOG( LogTemp, Warning, TEXT( "ACTIVE RELOAD" ) );
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
			UE_LOG( LogTemp, Warning, TEXT( "FAILED RELOAD" ) );
		}
	}
	UE_LOG( LogTemp, Warning, TEXT( "DEFINE THE RELOAD BASED ON THE TIMING" ) );
}

void UReloadComponent::FinalizeReload(int NewAmmoCount, float FinalWaitingTime, int InventoryAmmoCountUsed)
{
	UE_LOG(LogTemp, Warning, TEXT("FINALIZE RELOAD"));

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandleReloadFinalize, [this, NewAmmoCount, InventoryAmmoCountUsed]() 
		{
			bIsReloadActive = false;
			CurrentGunState = EGunState::Idle;

			// Update inventory with consumed ammo
			int AmmoToAdd = InventoryAmmoCountUsed * -1;

			if ( PlayerInventory )
			{
				UE_LOG( LogTemp, Warning, TEXT( "Attempting to remove %d ammo from inventory" ), AmmoToAdd );
				PlayerInventory->AddToInventory( EPickupType::Ammo, AmmoToAdd );
				UE_LOG( LogTemp, Warning, TEXT( "Ammo after removal: %d" ), PlayerInventory->GetCurrentAmmoAmount() );
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