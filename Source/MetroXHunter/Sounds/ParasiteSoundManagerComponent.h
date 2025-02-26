/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ParasiteSoundManagerComponent.generated.h"

UCLASS( Abstract, Blueprintable, ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UParasiteSoundManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UParasiteSoundManagerComponent();

	/*
	 * Called whenever one animation notifies of a scream event.
	 * This currently happens before the parasite jumps on the player.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnScream();
	/*
	 * Called whenever one animation notifies of a footstep event.
	 * This currently happens during the walk animation.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnFootstep();
};
