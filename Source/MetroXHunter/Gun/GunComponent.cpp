/* 
 * Implemented by BARRAU Benoit 
 */

#include "Gun/GunComponent.h"

UGunComponent::UGunComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGunComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGunComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

