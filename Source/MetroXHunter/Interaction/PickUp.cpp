/*
 * Implemented by Corentin Paya
 */

#include "Interaction/PickUp.h"
#include "Inventory/InventoryComponent.h"
#include "Interaction/InteractionComponent.h"
#include "Interaction/InteractableComponent.h"
#include "Interaction/InteractionType.h"
#include "HUD/InteractableWidget.h"

APickUp::APickUp()
{
	InteractableComponent->InteractionType = E_InteractionType::Pickup;

}

void APickUp::BeginPlay()
{
	Super::BeginPlay();

	InteractableWidget->EditSprite( Sprite );
}

void APickUp::Interact()
{
	int OverflowAmount = PlayerInventory->AddToInventory( PickupType, Amount );

	if ( OverflowAmount > 0 )
	{
		Amount = OverflowAmount;
		return;
	}

	PlayerInteractionComponent->RemoveNearInteractable(InteractableComponent);
	Destroy();
}

void APickUp::OnInnerCircleOverlapBegin(
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

void APickUp::OnInteractableTargeted()
{
	InteractableWidget->OnPickupTargeted( Amount );
}
