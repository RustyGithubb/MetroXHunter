/*
 * Implemented by Corentin Paya
 */

#include "Inventory/InventoryComponent.h"
#include "Inventory/InventoryData.h"
#include "Interaction/PickupType.h"

#include "Core/MetroGameInstance.h"

#include "HUD/MainHUD.h"

#include "GameFramework/PlayerController.h"
#include "Character/MetroPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "Library/GameplayLibrary.h"

#include "Engine.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = CastChecked<UMetroGameInstance>( GetOwner()->GetGameInstance() );

	InventoryDataAsset = GameInstance->InventoryDataAsset;

	CurrentAmmoAmount = InventoryDataAsset->StartingAmmoAmount;
	CurrentSyringeAmount = InventoryDataAsset->StartingSyringe;

	SetupPlayerInputComponent();
}

void UInventoryComponent::OnSaveData_Implementation( const FGuid& ActorID, UMetroSaveGame* SaveGame )
{
	FInventorySavedData InventoryData {};
	InventoryData.CurrentAmmoAmount = CurrentAmmoAmount;
	InventoryData.CurrentSyringeAmount = CurrentSyringeAmount;

	SaveGame->SavedInventoryComponent.Add( ActorID, InventoryData );
}

void UInventoryComponent::OnLoadData_Implementation( const FGuid& ActorID, UMetroSaveGame* SaveGame )
{
	if ( !SaveGame->SavedInventoryComponent.Contains( ActorID ) ) return;
	
	CurrentAmmoAmount = SaveGame->SavedInventoryComponent[ActorID].CurrentAmmoAmount;
	CurrentSyringeAmount = SaveGame->SavedInventoryComponent[ActorID].CurrentSyringeAmount;

	OnAmmoUpdate.Broadcast( CurrentAmmoAmount );
	OnSyringeUpdate.Broadcast( CurrentSyringeAmount );
}

void UInventoryComponent::SetupPlayerInputComponent()
{
	PlayerController = UGameplayLibrary::GetPlayerControllerChecked( GetOwner() );
	PlayerCharacter = PlayerController->GetPawn<AMetroPlayerCharacter>();
	verifyf( IsValid( PlayerCharacter ), TEXT( "InventoryComponent isn't own by a MetroPlayerCharacter" ) );

	// Get the InputComponent from the PlayerController
	UInputComponent* PlayerInputComponent = PlayerController->InputComponent;

	// Cast the InputComponent to EnhancedInputComponent
	auto EnhancedInputComponent = CastChecked<UEnhancedInputComponent>( PlayerInputComponent );

	// Bind the inventory action to the OnInventoryInput function
	EnhancedInputComponent->BindAction(
		InventoryAction, ETriggerEvent::Started,
		this, &UInventoryComponent::OnInventoryInput
	);
}

int UInventoryComponent::AddToInventory( EPickupType PickupType, int Amount )
{
	int OverflowAmount = 0;

	switch ( PickupType )
	{
		case EPickupType::Ammo:
		{
			UE_LOG( LogTemp, Warning, TEXT( "BEFORE AMOUNT: %d" ), Amount );
			OverflowAmount = CalculateOverflowAmount(
				Amount,
				CurrentAmmoAmount,
				InventoryDataAsset->MaxAmmoAmountCapacity
			);

			OnAmmoUpdate.Broadcast( CurrentAmmoAmount );
			UE_LOG( LogTemp, Warning, TEXT( "CURRENT AMMO: %d"), CurrentAmmoAmount );
			if ( bIsFirstLoot && Amount > 0 )
			{
				OnLootFirstTime.Broadcast();
				bIsFirstLoot = false;
			}
			break;
		}
		case EPickupType::Syringe:
		{
			OverflowAmount = CalculateOverflowAmount(
				Amount,
				CurrentSyringeAmount,
				InventoryDataAsset->MaxSyringeCapacity
			);

			OnSyringeUpdate.Broadcast( CurrentSyringeAmount );
			break;
		}
	}

	const int ActualAmountPickedUp = Amount - OverflowAmount;

	// == 0 => Full can't pick up loot
	// arkaht: Why do we need that second expression?
	if ( ActualAmountPickedUp > 0 || ( ActualAmountPickedUp == -1 && PickupType == EPickupType::Syringe ) )
	{
		OnLootGain.Broadcast( PickupType, ActualAmountPickedUp );
	}

	if ( OverflowAmount > 0 )
	{
		OnLootOverflow.Broadcast( PickupType, OverflowAmount );
	}

	return OverflowAmount;
}

void UInventoryComponent::ClearSyringeSegments()
{
	CurrentSyringeAmount = 0;
	OnSyringeUpdate.Broadcast( CurrentSyringeAmount );
}

void UInventoryComponent::ClearAmmoAmount()
{
	CurrentAmmoAmount = 0;
	OnAmmoUpdate.Broadcast( CurrentAmmoAmount );
}

void UInventoryComponent::FullyFillSyringe()
{
	CurrentSyringeAmount = InventoryDataAsset->MaxSyringeCapacity;
	OnSyringeUpdate.Broadcast( CurrentSyringeAmount );
}

void UInventoryComponent::FullyFillAmmo()
{
	CurrentAmmoAmount = InventoryDataAsset->MaxAmmoAmountCapacity;
	OnAmmoUpdate.Broadcast( CurrentAmmoAmount );
}

bool UInventoryComponent::IsInventoryOpened() const
{
	return bIsInventoryOpened;
}

bool UInventoryComponent::IsSyringeFull()
{
	return CurrentSyringeAmount == InventoryDataAsset->MaxSyringeCapacity;
}

int UInventoryComponent::GetCurrentAmmoAmount() const
{
	return CurrentAmmoAmount;
}

int UInventoryComponent::GetCurrentSyringeAmount() const
{
	return CurrentSyringeAmount;
}

int UInventoryComponent::GetMaxSyringeCapacity() const
{
	return InventoryDataAsset->MaxSyringeCapacity;
}

void UInventoryComponent::OnInventoryInput()
{
	if ( PlayerCharacter->bIsUnderAction ) return;

	// TODO: Fix this function not working?
	if ( IsInventoryOpened() ) return;

	OnInventoryInputReceived.Broadcast();
}

int UInventoryComponent::CalculateOverflowAmount( int AmountToAdd, int& CurrentAmount, int MaxAmount )
{
	int NewAmount = AmountToAdd + CurrentAmount;
	int OverflowAmount = NewAmount - MaxAmount;


	if ( OverflowAmount > 0 )
	{
		NewAmount = MaxAmount;
	}
	else
	{
		OverflowAmount = 0;
	}

	CurrentAmount = NewAmount;

	return OverflowAmount;
}
