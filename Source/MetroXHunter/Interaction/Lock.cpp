/*
 * Implemented by Corentin Paya
 */

#include "Interaction/Lock.h"
#include "Health/HealthComponent.h"

ALock::ALock()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "Object Mesh" ) );
	StaticMesh->SetupAttachment( RootComponent );

	HealthComponent = CreateDefaultSubobject<UHealthComponent>( TEXT( "Health Component" ) );
	HealthComponent->MaxHealth = 50.f;
	HealthComponent->OnDeath.AddDynamic( this, &ALock::OnDeath );

	// Needs to be fixed, can't destroy the lock with Monster profile at the moment
	//StaticMesh->SetCollisionProfileName( "Monster" );
}

void ALock::OnDeath( const FDamageContext& DamageContext )
{
	if ( OnLockDown.IsBound() )
	{
		OnLockDown.Broadcast( this );
	}

	Destroy();
}