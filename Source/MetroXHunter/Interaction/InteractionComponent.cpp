#include "Interaction/InteractionComponent.h"
#include "Interaction/InteractableComponent.h"
#include <HUD/MainHUD.h>
#include "Enums/E_InteractionType.h"

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


void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if ( bNearInteractable )
		RetrieveClosestInteractable();
}

void UInteractionComponent::RetrieveClosestInteractable()
{
	//Viewport Size
	const FVector2D viewportSize = FVector2D( GEngine->GameViewport->Viewport->GetSizeXY() );

	//Viewport Center		
	const FVector2D  viewportCenter = FVector2D( viewportSize.X / 2, viewportSize.Y / 2 );

	// Get player's world location and direction
	FVector playerLocation;
	FVector playerDirection;
	PlayerController->DeprojectScreenPositionToWorld( viewportCenter.X, viewportCenter.Y, playerLocation, playerDirection);

	FVector targetDirecion;
	float closestAlignement = 1;
	UInteractableComponent* ClosestInteractable = nullptr;

	for ( auto interactable : NearInteractables )
	{
		// Calculate Interactable to Player's direction
		FVector interactableLocation = interactable->Owner->GetActorLocation();
		targetDirecion = interactableLocation - playerLocation;
		targetDirecion.Normalize();

		// Calculate Dot product with player current View Direction
		float targetAlignement = FVector::DotProduct( targetDirecion, playerDirection );
		if ( targetAlignement < closestAlignement )
		{
			closestAlignement = targetAlignement;
			ClosestInteractable = interactable;
		}
	}

	// Check if new interactable
	if ( CurrentInteractable == ClosestInteractable )
		return;

	// Untarget the last Targeted interactable if any
	if ( CurrentInteractable )
		CurrentInteractable->OnUntargeted.Broadcast();

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
	bNearInteractable = true;
}

void UInteractionComponent::RemoveNearInteractable( UInteractableComponent* InInteractable )
{
	NearInteractables.Remove( InInteractable );

	if ( NearInteractables.IsEmpty() )
	{
		bNearInteractable = false;
		CurrentInteractable = nullptr;

		this->SetComponentTickEnabled( false );
	}

	UpdateViewport();
}

void UInteractionComponent::UpdateViewport()
{
	IMainHUD* MainHUD = Cast<IMainHUD>(PlayerController->GetHUD());

	if ( !MainHUD )
		return;

	if ( NearInteractables.Num() > 0 )
		MainHUD->Execute_UpdatePrompts(this, CurrentInteractable->InteractionType);
	else
		MainHUD->UpdatePrompts(E_InteractionType::Default);
}
