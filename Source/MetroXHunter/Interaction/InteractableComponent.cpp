#include "Interaction/InteractableComponent.h"
#include "Enums/E_InteractionType.h"

UInteractableComponent::UInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();
	
	Owner = GetOwner();
}



void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}