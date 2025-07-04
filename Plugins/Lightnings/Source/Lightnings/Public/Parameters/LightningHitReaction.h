/*
 * Implemented by BARRAU Benoit
 */

#pragma once

#include "CoreMinimal.h"
#include "LightningHitReaction.generated.h"

/*
 * How lightning should react on a hit event
 */
UENUM( BlueprintType )
enum class EHitReaction : uint8
{
	// No reaction & continue to growth
	HitReact_Ignore		UMETA( DisplayName = "Ignore" ),

	// Stop growth & induce discharge (flashing)
	HitReact_Discharge	UMETA( DisplayName = "Discharge" ),

	// Stop growth & start fading out
	HitReact_FadeOut	UMETA( DisplayName = "FadeOut" )
};
