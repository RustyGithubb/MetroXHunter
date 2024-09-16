/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "InteractionType.generated.h"

UENUM( BlueprintType )
enum class E_InteractionType : uint8
{
	Default,

	Interact,
	Pickup					UMETA( DisplayName = "Pick Up" ),
	OpenClose				UMETA( DisplayName = "Open" ),
	TurnOn					UMETA( DisplayName = "Turn on / off" ),
};
