#include "AI/ZeroEnemy.h"

#include "MXHUtilityLibrary.h"

#include "Components/ArrowComponent.h"

AZeroEnemy::AZeroEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	ProtoMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "ProtoMeshComponent" ) );
	ProtoMeshComponent->SetupAttachment( RootComponent );
}

void AZeroEnemy::BeginPlay()
{
	Super::BeginPlay();

	RetrieveReferences();
}
	
void AZeroEnemy::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

void AZeroEnemy::RetrieveReferences()
{
	verifyf( IsValid( Data ), TEXT( "%s doesn't reference a DataAsset" ), *GetName() );

	UMXHUtilityLibrary::PrintMessage( TEXT( "Hey!" ) );
	auto BulbSpots = UMXHUtilityLibrary::GetComponentsOfActorByTag<UArrowComponent>( this, Data->BulbSpotTag );
	auto BodyParts = UMXHUtilityLibrary::GetComponentsOfActorByTag<UStaticMeshComponent>( this, Data->BodyPartTag );
}
