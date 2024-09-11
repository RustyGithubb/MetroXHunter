#include "Interaction/GeneralInteractable.h"
#include "Interaction/InteractableComponent.h"
#include "Interaction/InteractionComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "HUD/InteractableWidget.h"

constexpr auto INTERACTABLE_PROFILE_NAME = TEXT( "Interactable" );

AGeneralInteractable::AGeneralInteractable()
{
	PrimaryActorTick.bCanEverTick = false;

	/* Create all the components */
	SceneRoot = CreateDefaultSubobject<USceneComponent>( TEXT( "Scene Root" ) );
	RootComponent = SceneRoot;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "Object Mesh" ) );
	StaticMesh->SetupAttachment( RootComponent );

	InnerCollision = CreateDefaultSubobject<USphereComponent>( TEXT( "Inner Collision" ) );
	InnerCollision->SetupAttachment( RootComponent );

	OutterCollision = CreateDefaultSubobject<USphereComponent>( TEXT( "Outter Collision" ) );
	OutterCollision->SetupAttachment( RootComponent );

	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>( TEXT( "Interactable Component" ) );

	Widget = CreateDefaultSubobject<UWidgetComponent>( TEXT( "WidgetComponent" ) );
	Widget->SetupAttachment( RootComponent );
	Widget->SetVisibility( false );

	InnerCollision->SetCollisionProfileName( INTERACTABLE_PROFILE_NAME );
	OutterCollision->SetCollisionProfileName( INTERACTABLE_PROFILE_NAME );
}

void AGeneralInteractable::BeginPlay()
{
	Super::BeginPlay();

	BindToDelegates();

	/* Check if the Widget class has been set properly */
	InteractableWidget = CastChecked<UInteractableWidget>( Widget->GetUserWidgetObject() );
}

void AGeneralInteractable::BindToDelegates()
{
	InnerCollision->OnComponentBeginOverlap.AddDynamic( this, &AGeneralInteractable::OnInnerCircleOverlapBegin );
	OutterCollision->OnComponentBeginOverlap.AddDynamic( this, &AGeneralInteractable::OnOutterCircleOverlapBegin );

	InnerCollision->OnComponentEndOverlap.AddDynamic( this, &AGeneralInteractable::OnInnerCircleOverlapEnd );
	OutterCollision->OnComponentEndOverlap.AddDynamic( this, &AGeneralInteractable::OnOutterCircleOverlapEnd );

	InteractableComponent->OnTargeted.AddDynamic( this, &AGeneralInteractable::OnInteractableTargeted );
	InteractableComponent->OnUntargeted.AddDynamic( this, &AGeneralInteractable::OnInteractableUntargeted );
	InteractableComponent->OnInteract.AddDynamic( this, &AGeneralInteractable::Interact );
}

void AGeneralInteractable::OnInnerCircleOverlapBegin(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult
)
{
	PlayerInteractionComponent = OtherActor->FindComponentByClass<UInteractionComponent>();

	if ( PlayerInteractionComponent )
	{
		InteractableComponent->OnPlayerOverlap( PlayerInteractionComponent );
	}
}

void AGeneralInteractable::OnOutterCircleOverlapBegin(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult
)
{
	Widget->SetVisibility( true );
}

void AGeneralInteractable::OnInnerCircleOverlapEnd(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex
)
{
	if ( PlayerInteractionComponent )
	{
		InteractableComponent->OnPlayerOut( PlayerInteractionComponent );
		InteractableWidget->OnUntargeted();
	}
}

void AGeneralInteractable::OnOutterCircleOverlapEnd(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex
)
{
	Widget->SetVisibility( false );
}

void AGeneralInteractable::OnInteractableTargeted()
{
	InteractableWidget->OnObjectTargeted();
}

void AGeneralInteractable::OnInteractableUntargeted()
{
	InteractableWidget->OnUntargeted();
}

