/*
 * Implemented by Corentin Paya
 */

#include "Interaction/BaseDoor.h"
#include "Interaction/Lock.h"

#include <GameFramework/MovementComponent.h>

void ABaseDoor::Interact()
{
	if ( bIsOpened || bIsLocked ) return;

	SetDoorOpened( true );
}

void ABaseDoor::OnDoorHit( AActor* Player)
{
	if ( bIsOpened || bIsLocked ) return;

	UMovementComponent* PlayerMovement = Player->FindComponentByClass<UMovementComponent>();
	if ( !PlayerMovement ) return;

	float PlayerVelocity = PlayerMovement->Velocity.Length();
	if ( PlayerVelocity > MinimumVelocityRequired ) SetDoorOpened( true );
}

void ABaseDoor::AddLock()
{
	ALock* newLock = CreateDefaultSubobject<ALock>( TEXT( "Object Mesh" ) );

	LocksList.Add( newLock );
}
