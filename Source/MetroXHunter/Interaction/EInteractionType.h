/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "EInteractionType.generated.h"

UENUM( BlueprintType )
enum class E_InteractionType : uint8
{
	Default,

	Interact,
	Pickup					UMETA( DisplayName = "Pick Up" ),
	OpenClose				UMETA( DisplayName = "Open Close" ),
	TurnOn					UMETA( DisplayName = "Turn on / off" ),
};
