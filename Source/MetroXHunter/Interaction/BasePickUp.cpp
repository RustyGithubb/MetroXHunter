/*
 * Implemented by Corentin Paya
 */

#include "Interaction/BasePickUp.h"
#include "Inventory/InventoryComponent.h"
#include "Interaction/InteractionComponent.h"
#include "Interaction/InteractableComponent.h"
#include "Interaction/InteractionType.h"
#include "HUD/InteractableWidget.h"

#include "Core/MetroGameInstance.h"

ABasePickUp::ABasePickUp()
{
	InteractableComponent->InteractionType = E_InteractionType::Pickup;
}

void ABasePickUp::BeginPlay()
{
	Super::BeginPlay();

	if ( !bIsAlreadySpawned )
	{
		UMetroGameInstance* GameInstance = CastChecked<UMetroGameInstance>( GetWorld()->GetGameInstance() );
		Amount *= GameInstance->AmmoMultiplier;
	}
}

void ABasePickUp::OnSpawn()
{
	bIsAlreadySpawned = true;

	UMetroGameInstance* GameInstance = CastChecked<UMetroGameInstance>( GetWorld()->GetGameInstance() );
	Amount *= GameInstance->AmmoMultiplier;
}

void ABasePickUp::Interact()
{
	int OverflowAmount = PlayerInventory->AddToInventory( PickupType, Amount );

	if ( OverflowAmount > 0 )
	{
		Amount = OverflowAmount;
		return;
	}

	PlayerInteractionComponent->RemoveNearInteractable( InteractableComponent );
	Destroy();
}

void ABasePickUp::OnInnerCircleOverlapBegin(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult
)
{
	Super::OnInnerCircleOverlapBegin(
		OverlappedComponent,
		OtherActor, OtherComp, OtherBodyIndex,
		bFromSweep, SweepResult
	);

	PlayerInventory = OtherActor->FindComponentByClass<UInventoryComponent>();
}

void ABasePickUp::OnInteractableTargeted()
{
	if ( IsValid( InteractableWidget ) )
	{
		InteractableWidget->OnPickupTargeted( Amount );
	}
}
