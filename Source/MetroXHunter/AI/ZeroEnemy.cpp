#include "AI/ZeroEnemy.h"

AZeroEnemy::AZeroEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	ProtoMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "ProtoMeshComponent" ) );
	ProtoMeshComponent->SetupAttachment( RootComponent );
}

void AZeroEnemy::BeginPlay()
{
	Super::BeginPlay();
}
	
void AZeroEnemy::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}
