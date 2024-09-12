/*
 * Implemented by Corentin Paya
 */

#include "Interaction/PickUp.h"
#include "Inventory/InventoryComponent.h"
#include "HUD/InteractableWidget.h"

void APickUp::Interact()
{
	PlayerInventory->AddToInventory( PickupType, Amount );
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
