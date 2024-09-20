/*
 * Implemented by Corentin Paya
 */

#include "Interaction/BaseInteractable.h"
#include "Interaction/InteractableComponent.h"
#include "Interaction/InteractionComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "HUD/InteractableWidget.h"

constexpr auto INTERACTABLE_PROFILE_NAME = TEXT( "Interactable" );

ABaseInteractable::ABaseInteractable()
{
	PrimaryActorTick.bCanEverTick = false;

	/* Create all the components */
	SceneRoot = CreateDefaultSubobject<USceneComponent>( TEXT( "Scene Root" ) );
	RootComponent = SceneRoot;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "Object Mesh" ) );
	StaticMesh->SetupAttachment( RootComponent );

	InnerCollision = CreateDefaultSubobject<USphereComponent>( TEXT( "Inner Collision" ) );
	InnerCollision->SetupAttachment( RootComponent );
	InnerCollision->SetSphereRadius( InnerSphereRadius );

	OutterCollision = CreateDefaultSubobject<USphereComponent>( TEXT( "Outter Collision" ) );
	OutterCollision->SetupAttachment( RootComponent );
	OutterCollision->SetSphereRadius( OutterSphereRadius );

	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>( TEXT( "Interactable Component" ) );

	Widget = CreateDefaultSubobject<UWidgetComponent>( TEXT( "WidgetComponent" ) );
	Widget->SetupAttachment( RootComponent );
	Widget->SetVisibility( false );

	InnerCollision->SetCollisionProfileName( INTERACTABLE_PROFILE_NAME );
	OutterCollision->SetCollisionProfileName( INTERACTABLE_PROFILE_NAME );
}

void ABaseInteractable::BeginPlay()
{
	Super::BeginPlay();

	BindToDelegates();

	/* Check if the Widget class has been set properly */
	InteractableWidget = CastChecked<UInteractableWidget>( Widget->GetUserWidgetObject() );
}

void ABaseInteractable::BindToDelegates()
{
	InnerCollision->OnComponentBeginOverlap.AddDynamic( this, &ABaseInteractable::OnInnerCircleOverlapBegin );
	OutterCollision->OnComponentBeginOverlap.AddDynamic( this, &ABaseInteractable::OnOutterCircleOverlapBegin );

	InnerCollision->OnComponentEndOverlap.AddDynamic( this, &ABaseInteractable::OnInnerCircleOverlapEnd );
	OutterCollision->OnComponentEndOverlap.AddDynamic( this, &ABaseInteractable::OnOutterCircleOverlapEnd );

	InteractableComponent->OnTargeted.AddDynamic( this, &ABaseInteractable::OnInteractableTargeted );
	InteractableComponent->OnUntargeted.AddDynamic( this, &ABaseInteractable::OnInteractableUntargeted );
	InteractableComponent->OnInteract.AddDynamic( this, &ABaseInteractable::Interact );
	InteractableComponent->OnCancelInteract.AddDynamic( this, &ABaseInteractable::OnCancelInteraction );
}

void ABaseInteractable::SetInteractionFreezed( bool bShouldFreeze )
{
	if ( !bShouldFreeze )
	{
		InnerCollision->SetCollisionEnabled( ECollisionEnabled::QueryOnly );
		OutterCollision->SetCollisionEnabled( ECollisionEnabled::QueryOnly );

		bIsInteractableSleeping = false;
		return;
	}

	InnerCollision->SetCollisionEnabled( ECollisionEnabled::NoCollision );
	OutterCollision->SetCollisionEnabled( ECollisionEnabled::NoCollision );

	bIsInteractableSleeping = true;

	if ( PlayerInteractionComponent )
	{
		PlayerInteractionComponent->RemoveNearInteractable( InteractableComponent );
	}
}

void ABaseInteractable::RemoveInteractionComponent()
{
	InnerCollision->DestroyComponent();
	OutterCollision->DestroyComponent();
	InteractableComponent->DestroyComponent();
	Widget->DestroyComponent();

	InteractableWidget = nullptr;
}

void ABaseInteractable::SwitchCameraTarget()
{
	// Smooth camera transition from the player to the Interactable's camera
	PlayerController->SetViewTargetWithBlend(
		this,
		BlendTime,
		EViewTargetBlendFunction::VTBlend_EaseInOut,
		BlendExp
	);

	// Should refactor the inputs so we still see the Visualizer
	PlayerController->GetPawn()->DisableInput( PlayerController );
	PlayerController->DisableInput( PlayerController );
}

void ABaseInteractable::ResetCameraTarget()
{
	// Smooth camera transition from the Interactable's camera to the Player
	PlayerController->SetViewTargetWithBlend(
		PlayerController->GetPawn(),
		BlendTime,
		EViewTargetBlendFunction::VTBlend_EaseInOut,
		BlendExp
	);

	PlayerController->GetPawn()->EnableInput( PlayerController );
	PlayerController->EnableInput( PlayerController );
}

void ABaseInteractable::OnInnerCircleOverlapBegin(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult
)
{
	PlayerInteractionComponent = OtherActor->FindComponentByClass<UInteractionComponent>();

	if ( PlayerInteractionComponent )
	{
		PlayerController = GetWorld()->GetFirstPlayerController();
		InteractableComponent->OnPlayerOverlap( PlayerInteractionComponent );
	}
}

void ABaseInteractable::OnOutterCircleOverlapBegin(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult
)
{
	Widget->SetVisibility( true );
}

void ABaseInteractable::OnInnerCircleOverlapEnd(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex
)
{
	if ( PlayerInteractionComponent )
	{
		PlayerController = nullptr;
		InteractableComponent->OnPlayerOut( PlayerInteractionComponent );
		InteractableWidget->OnUntargeted();

		PlayerInteractionComponent = nullptr;
	}
}

void ABaseInteractable::OnOutterCircleOverlapEnd(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex
)
{
	Widget->SetVisibility( false );
}

void ABaseInteractable::OnInteractableTargeted()
{
	InteractableWidget->OnObjectTargeted();
}

void ABaseInteractable::OnInteractableUntargeted()
{
	InteractableWidget->OnUntargeted();
}

