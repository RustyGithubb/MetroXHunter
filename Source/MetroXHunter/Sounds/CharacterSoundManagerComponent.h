/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterSoundManagerComponent.generated.h"

UCLASS( Abstract, Blueprintable, ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UCharacterSoundManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCharacterSoundManagerComponent();

	/*
	 * Called each time a player animation notifies of a footstep event.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnFootstep( bool bIsRunning );
	/*
	 * Called when the flashlight change its state.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnFlashlightEnabled( bool bIsActivated, FVector FlashlightLocation );
};
