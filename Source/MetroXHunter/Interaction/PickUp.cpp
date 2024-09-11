/*
 * Implemented by Corentin Paya
 */

#include "Interaction/PickUp.h"

void APickUp::Interact()
{
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
}
