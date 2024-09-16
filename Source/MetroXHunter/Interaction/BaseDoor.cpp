/*
 * Implemented by Corentin Paya
 */

#include "Interaction/BaseDoor.h"
#include "Interaction/Lock.h"
#include "Health/HealthComponent.h"

#include <GameFramework/MovementComponent.h>

void ABaseDoor::BeginPlay()
{
	Super::BeginPlay();

	if ( !bIsLocked ) return;

	for ( auto Locks : LocksList )
	{
		Locks->OnLockDown.AddDynamic( this, &ABaseDoor::RemoveLock );
	}

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
	Origin.Z += LocksList.Num() * 30 - 40;

	ALock* newLock = GetWorld()->SpawnActor<ALock>( Origin, GetActorRotation(), SpawnInfo );
	if ( !newLock ) return;

	FRotator NewRotation
	{ 0 };
	NewRotation.Roll = FMath::FRandRange( -10.f, 10.f );

	newLock->SetActorRelativeRotation( NewRotation );
	newLock->AttachToActor( this, FAttachmentTransformRules::KeepWorldTransform );
	LocksList.Add( newLock );

	CloseDoorEditor();
	bIsLocked = true;

	/* Set its mesh */
	if ( LockMesh )
	{
		newLock->StaticMesh->SetStaticMesh( LockMesh );
	}
}

void ABaseDoor::RemoveAllLocks()
{
	for ( int i = LocksList.Num() - 1; i > -1; i--  )
	{
		LocksList[i]->Destroy();
	}

	LocksList.Empty();
	bIsLocked = false;
}

void ABaseDoor::RemoveLock( ALock* Lock )
{
	LocksList.Remove( Lock );

	if ( LocksList.IsEmpty() )
	{
		bIsLocked = false;

		SetDoorOpened( true );
	}
}
