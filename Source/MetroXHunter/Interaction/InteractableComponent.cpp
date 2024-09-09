#include "Interaction/InteractableComponent.h"
#include "Interaction/InteractionComponent.h"
#include "Interaction/EInteractionType.h"

UInteractableComponent::UInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = GetOwner();
}

//void UInteractableComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
//{
//	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
//}

void UInteractableComponent::OnPlayerOverlap( UInteractionComponent* InteractionComponent )
{
	if ( !InteractionComponent ) return;

	InteractionComponent->AddNearInteractable( this );

}

void UInteractableComponent::OnPlayerOut( UInteractionComponent* InteractionComponent )
{
	if ( !InteractionComponent ) return;

	InteractionComponent->RemoveNearInteractable( this );
}
