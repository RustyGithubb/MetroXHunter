/*
 * Implemented by Corentin Paya
 */

#include "Inventory/InventoryComponent.h"
#include "Inventory/InventoryData.h"
#include "Interaction/PickupType.h"
#include "HUD/MainHUD.h"
#include "Engine.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentAmmoAmount = InventoryDataAsset->StartingAmmoAmount;
	CurrentSyringeSegmentAmount = InventoryDataAsset->StartingSyringeSegment;

	// Late Begin Play
	GetWorld()->OnWorldBeginPlay.AddUObject( this, &UInventoryComponent::LateBeginPlay );
}

void UInventoryComponent::LateBeginPlay()
{
	APlayerController* PlayerController = GetOwner()->GetInstigator()->GetLocalViewingPlayerController();
	MainHUD = ( PlayerController->GetHUD() );
	IMainHUD::Execute_UpdateAmmoCount( MainHUD );
}

int UInventoryComponent::AddToInventory( EPickupType PickupType, int Amount )
{
	int OverflowAmount = 0;

	switch ( PickupType )
	{
		case EPickupType::Ammo:
		{
			OverflowAmount = CalculateOverflowAmount(
				Amount,
				CurrentAmmoAmount,
				InventoryDataAsset->MaxAmmoAmountCapaicty
			);

			IMainHUD::Execute_UpdateAmmoCount( MainHUD );
			break;
		}
		case EPickupType::Syringe:
		{
			OverflowAmount = CalculateOverflowAmount(
				Amount,
				CurrentSyringeSegmentAmount,
				InventoryDataAsset->MaxSyringeSegmentCapacity
			);
			break;
		}
	}

	return OverflowAmount;
}


void UInventoryComponent::ClearSyringeSegments()
{
	CurrentSyringeSegmentAmount = 0;
}

void UInventoryComponent::ClearAmmoAmount()
{
	CurrentAmmoAmount = 0;
	IMainHUD::Execute_UpdateAmmoCount( MainHUD );
}

void UInventoryComponent::FullyFillSyringe()
{
	CurrentSyringeSegmentAmount = InventoryDataAsset->MaxSyringeSegmentCapacity;
}

void UInventoryComponent::FullyFillAmmo()
{
	CurrentAmmoAmount = InventoryDataAsset->MaxAmmoAmountCapaicty;
	IMainHUD::Execute_UpdateAmmoCount( MainHUD );
}

bool UInventoryComponent::IsSyringeFull()
{
	return CurrentSyringeSegmentAmount == InventoryDataAsset->MaxSyringeSegmentCapacity;
}

int UInventoryComponent::GetCurrentAmmoAmount() const
{
	return CurrentAmmoAmount;
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
