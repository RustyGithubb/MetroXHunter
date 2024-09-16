/*
 * Implemented by Corentin Paya
 */

#include "Interaction/BaseDoor.h"

#include <GameFramework/MovementComponent.h>

void ABaseDoor::Interact()
{
	// Open Door ?
}

void ABaseDoor::OnDoorHit( AActor* Player)
{
	UMovementComponent* PlayerMovement = Player->FindComponentByClass<UMovementComponent>();
	if ( !PlayerMovement ) return;

	float PlayerVelocity = PlayerMovement->Velocity.Length();
	if ( PlayerVelocity > MinimumVelocityRequired ) SetDoorOpened( true );
}
