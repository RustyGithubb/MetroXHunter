/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "AI/PossessableCorpse.h"
#include "AI/Parasite.h"
#include "AI/ZeroEnemy.h"

APossessableCorpse::APossessableCorpse()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>( TEXT( "MeshComponent" ) );
	RootComponent = MeshComponent;
}

void APossessableCorpse::BeginPlay()
{
	Super::BeginPlay();

	verifyf( IsValid( EnemyClass ), TEXT( "%s doesn't have a valid EnemyClass reference!" ), *GetName() );
}

bool APossessableCorpse::ReserveCorpse( AParasite* Parasite )
{
	ReservingParasite = Parasite;
	return true;
}

bool APossessableCorpse::IsReserved() const
{
	return IsValid( ReservingParasite ) && ReservingParasite->HealthComponent->IsAlive();
}

AParasite* APossessableCorpse::GetReserver() const
{
	return ReservingParasite;
}

