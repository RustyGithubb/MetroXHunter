/*
 * Implemented by Corentin Paya
 */

#include "Interaction/BaseDoor.h"
#include "Interaction/Lock.h"

#include <GameFramework/MovementComponent.h>

void ABaseDoor::BeginPlay()
{
	Super::BeginPlay();

	if ( !bIsLocked ) return;

	SetInteractionFreezed( true );
}

void ABaseDoor::Interact()
{
	if ( bIsOpened || bIsLocked ) return;

	SetDoorOpened( true );
}

void ABaseDoor::OnDoorHit( AActor* Player )
{
	if ( bIsOpened || bIsLocked ) return;

	UMovementComponent* PlayerMovement = Player->FindComponentByClass<UMovementComponent>();
	if ( !PlayerMovement ) return;

	/* Check if the player is running */
	float PlayerVelocity = PlayerMovement->Velocity.Length();
	if ( PlayerVelocity > MinimumVelocityRequired )
	{
		SetDoorOpened( true );
	}
}

void ABaseDoor::AddLock()
{
	/* Create a Lock */
	FActorSpawnParameters SpawnInfo;

	/* Get the location */
	FVector Origin;
	FVector BoxExtend;
	GetActorBounds( true, Origin, BoxExtend );

	ALock* newLock = GetWorld()->SpawnActor<ALock>( Origin, GetActorRotation(), SpawnInfo );
	if ( !newLock ) return;

	FRotator NewRotation
	{ 0 };
	NewRotation.Roll = FMath::FRandRange( -10.f, 10.f );

	newLock->SetActorRelativeRotation( NewRotation );

	newLock->AttachToActor( this, FAttachmentTransformRules::KeepWorldTransform );
	LocksList.Add( newLock );
	bIsLocked = true;

	/* Set its mesh */
	if ( LockMesh )
	{
		newLock->StaticMesh->SetStaticMesh( LockMesh );
	}
}
