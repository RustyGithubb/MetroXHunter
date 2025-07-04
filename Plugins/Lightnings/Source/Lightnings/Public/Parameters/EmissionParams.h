/*
 * Implemented by BARRAU Benoit
 */

#pragma once

#include "CoreMinimal.h"
#include "EmissionParams.generated.h"

 /*
  * Structure for the lighting data
  */
USTRUCT( BlueprintType )
struct FEmissionParams
{
	GENERATED_USTRUCT_BODY()

	// If we use the niagara light renderer or not
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "LightningEmission" )
	bool bUseLightRenderer = false;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "LightningEmission" )
	float LightMultiplier = 1.0f;

	// The light Intensity Fading with distance (exponent)
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "LightningEmission" )
	float LightExponent = 50.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "LightningEmission" )
	float LightRadius = 100.0f;

	// Light volumetric scattering
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "LightningEmission" )
	float LightVolumetricScattering = 0.0f;
};
