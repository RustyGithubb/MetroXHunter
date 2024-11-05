/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "HUD/SubtitleComponent.h"

USubtitleComponent::USubtitleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USubtitleComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USubtitleComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

