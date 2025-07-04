/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "InteractionType.generated.h"

// TODO: If it's only for text purposes, don't make an enum. If we keep it, rename it EInteractionType
UENUM( BlueprintType )
enum class E_InteractionType : uint8
{
	Default,

	Interact,
	Pickup					UMETA( DisplayName = "Pick Up" ),
	OpenClose				UMETA( DisplayName = "Open" ),
	TurnOn					UMETA( DisplayName = "Turn on / off" ),
	Save					UMETA( DisplayName = "Save" ),
};
