/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "AI/Parasite.h"
#include "AI/PossessableCorpse.h"
#include "AI/ZeroEnemy.h"

#include "Health/HealthComponent.h"
#include "Perception/PawnSensingComponent.h"

AParasite::AParasite()
{
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>( TEXT( "HealthComponent" ) );
	HealthComponent->MaxHealth = 1;

	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>( TEXT( "PawnSensingComponent" ) );
}

void AParasite::BeginPlay()
{
	Super::BeginPlay();
}

void AParasite::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

void AParasite::Possess( APossessableCorpse* Corpse )
{
	verify( IsValid( Corpse ) );

	// Destroy corpse and parasite
	Corpse->Destroy();
	Destroy();

	// Set to always spawn to avoid annoying crashes because of collisions at spawn
	FActorSpawnParameters SpawnParams {};
	SpawnParams.SpawnCollisionHandlingOverride = 
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn enemy
	auto Enemy = GetWorld()->SpawnActor<AZeroEnemy>( 
		Corpse->EnemyClass,
		// TODO: Export location offset
		Corpse->GetActorLocation() + FVector { 0.0f, 0.0f, 180.0f * 0.5f },
		Corpse->GetActorRotation(),
		SpawnParams
	);
	verifyf( IsValid( Enemy ), TEXT( "Failed to spawn enemy during parasite possession!" ) );
	Enemy->Data = Corpse->DataAsset;
	Enemy->Stun( 2.0f, false );

	OnPossessCorpse.Broadcast( this, Corpse, Enemy );
}
