#pragma once

#include "E_InteractionType.generated.h"

UENUM(BlueprintType)
enum class E_InteractionType : uint8
{
	INTERACT				UMETA( DisplayName = "INTERACT"),
	PICKUP					UMETA( DisplayName = "PICK UP"),
	OPENCLOSE				UMETA( DisplayName = "OPEN / CLOSE"),
	TURNON					UMETA( DisplayName = "TURN ON / OFF" ),

	DEFAULT,
};
