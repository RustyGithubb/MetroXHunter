#pragma once

#include "E_InteractionType.generated.h"

UENUM(BlueprintType)
enum class E_InteractionType : uint8
{
	Interact,
	Pickup					UMETA( DisplayName = "Pick Up"),
	OpenClose				UMETA( DisplayName = "Open Close"),
	TurnOn					UMETA( DisplayName = "Turn on / off" ),

	Default,
};
