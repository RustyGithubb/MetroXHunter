#include "Interaction/GeneralInteractable.h"
#include "Interaction/InteractableComponent.h"
#include "Components/SphereComponent.h"

AGeneralInteractable::AGeneralInteractable()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>( TEXT( "Scene Root" ) );
	RootComponent = SceneRoot;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "Object Mesh" ) );
	StaticMesh->SetupAttachment( RootComponent );

	InnerCollision = CreateDefaultSubobject<USphereComponent>( TEXT( "Inner Collision" ) );
	InnerCollision->SetupAttachment( RootComponent );

	OutterCollision = CreateDefaultSubobject<USphereComponent>( TEXT( "Outter Collision" ) );
	OutterCollision->SetupAttachment( RootComponent );

	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>( TEXT( "Interactable Component" ) );
}

void AGeneralInteractable::BeginPlay()
{
	Super::BeginPlay();

}

//void AGeneralInteractable::Tick( float DeltaTime )
//{
//	Super::Tick( DeltaTime );
//
//}

