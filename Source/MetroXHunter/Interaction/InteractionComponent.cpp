#include "Interaction/InteractionComponent.h"
#include "Interaction/InteractableComponent.h"
#include "Interaction/EInteractionType.h"
#include "HUD/MainHUD.h"
#include "Engine.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetComponentTickInterval( 0.2f );
	SetComponentTickEnabled( false );
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Late Begin Play
	GetWorld()->OnWorldBeginPlay.AddUObject( this, &UInteractionComponent::LateBeginPlay );
}

void UInteractionComponent::LateBeginPlay()
{
	GetReferences();
	SetupPlayerInputComponent();
}

void UInteractionComponent::GetReferences()
{
	PlayerController = GetOwner()->GetInstigator()->GetLocalViewingPlayerController();
}


void UInteractionComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	RetrieveClosestInteractable();
}

void UInteractionComponent::SetupPlayerInputComponent()
{
	UInputComponent* PlayerInputComponent = GetWorld()->GetFirstPlayerController()->InputComponent;

	// Set up action bindings
	if ( UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( PlayerInputComponent ) )
	{
		// Interaction
		EnhancedInputComponent->BindAction(
			InteractAction, ETriggerEvent::Started, this,
			&UInteractionComponent::Interact
		);
	}
}

void UInteractionComponent::RetrieveClosestInteractable()
{
	// Viewport Size
	const FVector2D ViewportSize = FVector2D( GEngine->GameViewport->Viewport->GetSizeXY() );

	// Viewport Center		
	const FVector2D ViewportCenter = FVector2D( ViewportSize.X / 2, ViewportSize.Y / 2 );

	// Get player's world location and direction
	FVector PlayerLocation {};
	FVector PlayerDirection {};
	PlayerController->DeprojectScreenPositionToWorld(
		ViewportCenter.X, ViewportCenter.Y,
		PlayerLocation, PlayerDirection
	);

	FVector TargetDirection {};
	float ClosestAlignement = 1.0f;
	UInteractableComponent* ClosestInteractable = nullptr;

	for ( auto Interactable : NearInteractables )
	{
		// Calculate Interactable to Player direction
		FVector InteractableLocation = Interactable->Owner->GetActorLocation();
		TargetDirection = PlayerLocation - InteractableLocation;
		TargetDirection.Normalize();

		// Calculate Dot product with player current View Direction
		float TargetAlignement = FVector::DotProduct( TargetDirection, PlayerDirection );
		if ( TargetAlignement < ClosestAlignement )
		{
			ClosestAlignement = TargetAlignement;
			ClosestInteractable = Interactable;
		}
	}

	// Check if new interactable
	if ( CurrentInteractable == ClosestInteractable ) return;

	// Untarget the last Targeted interactable if any
	if ( CurrentInteractable )
	{
		CurrentInteractable->OnUntargeted.Broadcast();
	}

	// Target the new interactable
	CurrentInteractable = ClosestInteractable;

	if ( CurrentInteractable )
	{
		CurrentInteractable->OnTargeted.Broadcast();
		UpdateViewport();
	}
}

void UInteractionComponent::AddNearInteractable( UInteractableComponent* InInteractable )
{
	SetComponentTickEnabled( true );

	NearInteractables.AddUnique( InInteractable );
	bIsNearInteractable = true;
}

void UInteractionComponent::RemoveNearInteractable( UInteractableComponent* InInteractable )
{
	NearInteractables.Remove( InInteractable );

	if ( NearInteractables.IsEmpty() )
	{
		bIsNearInteractable = false;
		CurrentInteractable = nullptr;

		SetComponentTickEnabled( false );
	}

	UpdateViewport();
}

void UInteractionComponent::UpdateViewport()
{
	IMainHUD* MainHUD = Cast<IMainHUD>( PlayerController->GetHUD() );

	if ( !MainHUD ) return;

	if ( NearInteractables.Num() > 0 )
	{
		MainHUD->UpdatePrompts( CurrentInteractable->InteractionType );
	}
	else
	{
		MainHUD->UpdatePrompts( E_InteractionType::Default );
	}
}

void UInteractionComponent::Interact()
{
	CurrentInteractable->OnInteract.Broadcast();
}
