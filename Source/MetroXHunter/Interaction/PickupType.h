/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "PickupType.generated.h"

UENUM( BlueprintType )
enum class EPickupType : uint8
{
	Ammo			UMETA( DisplayName = "Default Ammo" ),
	Syringe			UMETA( DisplayName = "Syringe Liquid" ),
};

