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
	StaticMesh->SetCollisionProfileName( TEXT( "Hitable" ) );
	HealthComponent = CreateDefaultSubobject<UHealthComponent>( TEXT( "Health Component" ) );
	HealthComponent->MaxHealth = 50.f;

	ElectrocutableComponent = CreateDefaultSubobject<UElectrocutableComponent>( TEXT( "Electrocutable Component" ) );
	ElectrocutableComponent->FXAttachmentComponents.Add( StaticMesh );
}

void ALock::BeginPlay()
{
	Super::BeginPlay();

	verify( IsValid( HealthComponent ) );

	HealthComponent->OnDeath.AddDynamic( this, &ALock::OnDeath );
}

bool ALock::TakeDamage_Implementation( FDamageContext& DamageContext )
{
	return DamageContext.DamageType == EDamageType::Bullet
		|| DamageContext.DamageType == EDamageType::Melee;
}

bool ALock::IsShownAsDamageableToPlayer_Implementation( EDamageType DamageType, const FHitResult& HitResult )
{
	return DamageType == EDamageType::Bullet;
}

void ALock::OnDeath( const FDamageContext& DamageContext )
{
	if ( OnLockDown.IsBound() )
	{
		OnLockDown.Broadcast( this );
	}

	Destroy();
}