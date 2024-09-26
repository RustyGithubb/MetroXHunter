/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "AI/PossessableCorpse.h"
#include "AI/ZeroEnemy.h"

APossessableCorpse::APossessableCorpse()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>( TEXT( "SceneComponent" ) );
	RootComponent = SceneComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "MeshComponent" ) );
	MeshComponent->SetupAttachment( RootComponent );
}

void APossessableCorpse::BeginPlay()
{
	Super::BeginPlay();

	verifyf( IsValid( EnemyClass ), TEXT( "%s doesn't have a valid EnemyClass reference!" ), *GetName() );
}

// TODO: Remove
void APossessableCorpse::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

