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

	for ( ALock* Lock : LocksList )
	{
		verifyf(
			IsValid( Lock ),
			TEXT( "Lock isn't valid ! Please check locks on %s" ), *GetName()
		);

		Lock->OnLockDown.AddDynamic( this, &ABaseDoor::RemoveLock );
	}

	SetInteractionFreezed( true );
}

void ABaseDoor::ConsumeInteraction_Implementation()
{
	Super::ConsumeInteraction_Implementation();
	
	RemoveAllLocks();
	bIsLocked = false;

	if ( const FInteractableSavedData* InteractableData = SaveComponent->SavedData.GetPtr<FInteractableSavedData>() )
	{
		SetDoorOpened( InteractableData->bShouldStartOpen );
	}
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
	FActorSpawnParameters SpawnInfo {};

	/* Get the location */
	FVector Origin {};
	FVector BoxExtend {};
	GetActorBounds( true, Origin, BoxExtend );
	Origin.Z += LocksList.Num() * 30 - 40;

	ALock* NewLock = GetWorld()->SpawnActor<ALock>( Origin, GetActorRotation(), SpawnInfo );
	if ( !NewLock ) return;

	FRotator NewRotation { 0 };
	NewRotation.Roll = FMath::FRandRange( -10.f, 10.f );

	NewLock->SetActorRelativeRotation( NewRotation );
	NewLock->AttachToActor( this, FAttachmentTransformRules::KeepWorldTransform );
	LocksList.Add( NewLock );

	CloseDoorEditor();
	bIsLocked = true;

	/* Set its mesh */
	if ( LockMesh )
	{
		NewLock->StaticMesh->SetStaticMesh( LockMesh );
	}
}

void ABaseDoor::RemoveAllLocks()
{
	for ( int Index = LocksList.Num() - 1; Index > -1; Index-- )
	{
		ALock* Lock = LocksList[Index];
		if ( !IsValid( Lock ) ) continue;

		Lock->Destroy();
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