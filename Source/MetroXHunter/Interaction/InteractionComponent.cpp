#include "Interaction/InteractionComponent.h"
#include "Interaction/InteractableComponent.h"
#include "Interaction/EInteractionType.h"
#include "HUD/MainHUD.h"

#include "Engine.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	this->SetComponentTickInterval( 0.2f );
	this->SetComponentTickEnabled( false );
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Late Begin Play
	GetWorld()->OnWorldBeginPlay.AddUObject( this, &UInteractionComponent::GetReferences );
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

void UInteractionComponent::RetrieveClosestInteractable()
{
	// Viewport Size
	const FVector2D ViewportSize = FVector2D( GEngine->GameViewport->Viewport->GetSizeXY() );

	// Viewport Center		
	const FVector2D ViewportCenter = FVector2D( ViewportSize.X / 2, ViewportSize.Y / 2 );

	// Get player's world location and direction
	FVector PlayerLocation;
	FVector PlayerDirection;
	PlayerController->DeprojectScreenPositionToWorld( ViewportCenter.X, ViewportCenter.Y, PlayerLocation, PlayerDirection );

	FVector TargetDirection;
	float ClosestAlignement = 1;
	UInteractableComponent* ClosestInteractable = nullptr;

	for ( auto Interactable : NearInteractables )
	{
		// Calculate Player to Interactable direction
		FVector interactableLocation = Interactable->Owner->GetActorLocation();
		TargetDirection = interactableLocation - PlayerLocation;
		TargetDirection.Normalize();

		// Calculate Dot product with player current View Direction
		float targetAlignement = FVector::DotProduct( TargetDirection, PlayerDirection );
		if ( targetAlignement < ClosestAlignement )
		{
			ClosestAlignement = targetAlignement;
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
	this->SetComponentTickEnabled( true );

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

		this->SetComponentTickEnabled( false );
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
