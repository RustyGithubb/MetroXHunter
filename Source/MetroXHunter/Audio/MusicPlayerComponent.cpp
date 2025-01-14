/* 
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "Audio/MusicPlayerComponent.h"

UMusicPlayerComponent::UMusicPlayerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMusicPlayerComponent::BeginPlay()
{
	Super::BeginPlay();
}
