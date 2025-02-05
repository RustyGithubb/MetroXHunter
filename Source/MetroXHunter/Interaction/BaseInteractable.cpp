/*
 * Implemented by Corentin Paya
 */

#include "Interaction/BaseInteractable.h"
#include "Interaction/InteractableComponent.h"
#include "Interaction/InteractionComponent.h"
#include "Checkpoint/SaveLoadComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "HUD/InteractableWidget.h"
#include "PlayerController/PlayerInputHandler.h"

#include "Library/UtilityLibrary.h"
#include "InputMappingContext.h"
#include <Engine/Console.h>

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
	InnerCollision->SetupAttachment( StaticMesh );
	InnerCollision->SetWorldScale3D( FVector( 1 ) );
	InnerCollision->SetSphereRadius( InnerSphereRadius );

	OutterCollision = CreateDefaultSubobject<USphereComponent>( TEXT( "Outter Collision" ) );
	OutterCollision->SetupAttachment( StaticMesh );
	OutterCollision->SetWorldScale3D( FVector( 1 ) );
	OutterCollision->SetSphereRadius( OutterSphereRadius );

	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>( TEXT( "Interactable Component" ) );
	SaveComponent = CreateDefaultSubobject<USaveLoadComponent>( TEXT( "Save Component" ) );

	Widget = CreateDefaultSubobject<UWidgetComponent>( TEXT( "WidgetComponent" ) );
	Widget->SetupAttachment( StaticMesh );
	Widget->SetWorldScale3D( FVector(1) );
	Widget->SetVisibility( true );

	InnerCollision->SetCollisionProfileName( INTERACTABLE_PROFILE_NAME );
	OutterCollision->SetCollisionProfileName( INTERACTABLE_PROFILE_NAME );

	bGenerateOverlapEventsDuringLevelStreaming = true;
}

void ABaseInteractable::BeginPlay()
{
	Super::BeginPlay();

	BindToDelegates();

	/* Check if the Widget class has been set properly */
	InteractableWidget = Cast<UInteractableWidget>( Widget->GetUserWidgetObject() );

#if WITH_EDITOR
	if ( !InteractableWidget )
	{
		UUtilityLibrary::PrintError( TEXT( "WARNING: InteractableWidget not set on %s !!" ), *GetName() );
	}
#endif

	if ( !SaveComponent->bIsDataLoaded )
	{
		SaveComponent->Load();
	}
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

	SaveComponent->OnLoadActor.AddDynamic( this, &ABaseInteractable::OnLoadedData );
}

void ABaseInteractable::SetInteractionFreezed( bool bShouldFreeze )
{
	if ( !IsValid( InnerCollision ) 
		|| !IsValid( OutterCollision ) 
		|| !IsValid( InteractableComponent ) ) return;

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
	if ( IsValid( InnerCollision ) )
	{
		InnerCollision->DestroyComponent();
	}

	if ( IsValid( OutterCollision ) )
	{
		OutterCollision->DestroyComponent();
	}

	if ( IsValid( InteractableComponent ) )
	{
		InteractableComponent->DestroyComponent();
	}

	Widget->SetVisibility( false );

	// Update the Saved data structure of this interactable.
	if ( auto InteractableData = SaveComponent->SavedData.GetMutablePtr<FInteractableSavedData>() )
	{
		InteractableData->bIsConsummed = true;
	}
}

void ABaseInteractable::ConsumeInteraction_Implementation()
{
	RemoveInteractionComponent();
}

void ABaseInteractable::Interact()
{
	InteractableComponent->bIsUnderInteraction = true;
}

void ABaseInteractable::OnCancelInteraction()
{
	if ( !InteractableComponent->bIsUnderInteraction ) return;

	InteractableComponent->bIsUnderInteraction = false;
}

void ABaseInteractable::OverridePlayerMappingContext()
{
	verifyf(
		IsValid( PlayerController ),
		TEXT( "%s: PlayerController isn't valid !" ), *GetName()
	);

	verify( !InteractableMappingContext.IsNull() );
	IPlayerInputHandler::Execute_SetInputMappingContext(
		PlayerController,
		InteractableMappingContext.LoadSynchronous()
	);
}

void ABaseInteractable::SwitchCameraTarget()
{
	verifyf(
		IsValid( PlayerController ),
		TEXT( "%s: PlayerController isn't valid !" ), *GetName()
	);

	// Smooth camera transition from the player to the Interactable's camera
	PlayerController->SetViewTargetWithBlend(
		this,
		BlendTime,
		EViewTargetBlendFunction::VTBlend_EaseInOut,
		BlendExp
	);

	PlayerController->GetPawn()->SetActorHiddenInGame( true );
	OverridePlayerMappingContext();
}

void ABaseInteractable::ResetCameraTarget()
{
	verifyf(
		IsValid( PlayerController ),
		TEXT( "%s: PlayerController isn't valid !" ), *GetName()
	);

	// Smooth camera transition from the Interactable's camera to the Player
	PlayerController->SetViewTargetWithBlend(
		PlayerController->GetPawn(),
		BlendTime,
		EViewTargetBlendFunction::VTBlend_EaseInOut,
		BlendExp
	);

	PlayerController->GetPawn()->SetActorHiddenInGame( false );

	verify( !InteractableMappingContext.IsNull() );
	IPlayerInputHandler::Execute_RevertInputMappingContext( 
		PlayerController,
		InteractableMappingContext.LoadSynchronous()
	);
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
	if ( IsValid( InteractableWidget ) )
	{
		InteractableWidget->OnEnterVisibility();
	}
}

void ABaseInteractable::OnInnerCircleOverlapEnd(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex
)
{
	if ( PlayerInteractionComponent )
	{
		InteractableComponent->OnPlayerOut( PlayerInteractionComponent );

		if ( IsValid( InteractableWidget ) )
		{
			InteractableWidget->OnUntargeted();
		}

		PlayerInteractionComponent = nullptr;
	}
}

void ABaseInteractable::OnOutterCircleOverlapEnd(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex
)
{
	if ( IsValid( InteractableWidget ) )
	{
		InteractableWidget->OnExitVisibility();
	}
}

void ABaseInteractable::OnInteractableTargeted()
{
	if ( IsValid( InteractableWidget ) )
	{
		InteractableWidget->OnObjectTargeted();
	}
}

void ABaseInteractable::OnInteractableUntargeted()
{
	if ( IsValid( InteractableWidget ) )
	{
		InteractableWidget->OnUntargeted();
	}
}

void ABaseInteractable::OnLoadedData()
{
	// Update the Saved data structure of this interactable.
	if ( const FInteractableSavedData* InteractableData = SaveComponent->SavedData.GetPtr<FInteractableSavedData>() )
	{
		if ( InteractableData->bIsConsummed )
		{
			ConsumeInteraction();
		}
	}
}

