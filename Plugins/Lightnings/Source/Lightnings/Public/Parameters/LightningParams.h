/*
 * Implemented by BARRAU Benoit
 */

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "Math/UnitConversion.h"
#include "LightningParams.generated.h"

 /*
  * Structure for the variables of the FX
  */
USTRUCT( BlueprintType )
struct FLightningParams
{
	GENERATED_USTRUCT_BODY()

	// Particles Color
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "Variables" )
	FLinearColor ColorParticle = FLinearColor( 0.5f, 0.35f, 1.0f, 1.0f );

	// Width shape of origin lightning : HAxis lightning ribbons order (0-1) - VAxis Width
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Variables" )
	UCurveFloat* OriginParticlesWidth = NULL;

	// Width shape of Branches : HAxis lightning ribbons order (0-1) - VAxis Width
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Variables" )
	UCurveFloat* BranchParticlesWidth = NULL;

	// Particles scales width
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Variables" )
	float ParticlesScalesWidth = 5.0f;

	// Is the lightnings spawned immediatly ?
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Variables" )
	bool bIsInstant = false;

	// Linear velocity of sparks
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, meta = ( Units = "CentimetersPerSecond", EditCondition = "bIsInstant == false" ), Category = "Variables" )
	FFloatRange SparksRangeVelocity = FFloatRange( 1000000.0f, 2500000.0f );

	 // Sparks fading velocity
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, meta = ( EditCondition = "bIsInstant == false" ), Category = "Variables" )
	FFloatRange SparksFadingVelocity = FFloatRange( 0.65f, 0.80f );

	// Sparks delay duration for movement
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, meta = ( Units = "Seconds", EditCondition = "bIsInstant == false", UIMin = "0.0", UIMax = "2.0" ), Category = "Variables")
	FFloatRange SparksRangeDelay = FFloatRange(0.005f, 0.02f);

	// Noise multiplier
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Variables" )
	float NoiseMultiplier = 1.0f;

	// Lenght steps of spark
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, meta = ( Units = "Centimeters" ), Category = "Variables" )
	float SparksStep = 40.0f;

	// Flashing Origin Curve : HAxis Time - VAxis Intensity
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Variables" )
	UCurveFloat* FlashingCurve = NULL;

	// Flashing Origin Rate
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "Variables" )
	FFloatRange FlashingRate = FFloatRange( 5.0f, 10.0f);

	// Flashing Origin Force
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "Variables" )
	FFloatRange FlashingForce = FFloatRange( 0.0f, 10.0f );

	// Sparks Light Intensity
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "Variables" )
	float SparksIntensity = 500.0f;

	// Sparks Trails Light Intensity
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "Variables" )
	float TrailIntensity = 50.0f;

	// Origin Light Intensity
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "Variables" )
	float LightningBoltIntensity = 800.0f;

	// Sparks Trail Fading Intensity
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "Variables" )
	FFloatRange TrailFadingIntensity = FFloatRange(0.0f, 1.0f);

	// Sparks Fade Out multiplier (it will fade with the origin flashing speed)
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "Variables" )
	float SparksFadeOutMult = 50.0f;

	// Trail Fade Out multiplier (it will fade with the origin flashing speed)
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "Variables" )
	float TrailFadeOutMult = 1.20f;

	// Lightning Division, higher value mean more noise
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, meta = ( UIMin = "2" ), Category = "Variables" )
	int32 SegmentDivision = 10;
};
