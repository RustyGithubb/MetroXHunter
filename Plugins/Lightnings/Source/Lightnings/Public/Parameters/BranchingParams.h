#pragma once

#include "CoreMinimal.h"
#include "BranchingParams.generated.h"

/*
 * Structure for the branchs parameters
 */
USTRUCT( BlueprintType )
struct FBranchingParams
{
	GENERATED_USTRUCT_BODY()

	// Lightning branch probability
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, meta = ( UIMin = "0.0", UIMax = "1.0" ), Category = "LightningBranching" )
	float BranchChance = 0.03f;

	// Fading of BranchChance value
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "LightningBranching" )
	float BranchFadingChance = 0.025f;

	// Branch direction in degrees
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, meta = ( Units = "Degrees" ), Category = "LightningBranching" )
	FFloatRange RotationRange = FFloatRange( -30.f, 30.f );

	/*
	 * Distance between start point and end point of a branch in cm
	 * This value gets multiplied by length of root lightning
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, meta = ( Units = "Centimeters" ), Category = "LightningBranching")
	FFloatRange LengthRange = FFloatRange( 0.45f, 0.8f );

	/*
	 * Defines a multiplier of branches convergence to a lightning target point
	 * 0.0 means no convergence (branches are directed out of the lightning root)
	 * 1.0 means full convergence (branches are directed to a lightning target)
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, meta = ( UIMin = "0.0", UIMax = "1.0" ), Category = "LightningBranching" )
	float ConvergenceFactor = 0.00f;

	// If emitter use BranchesLimit parameter
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "LightningBranching" )
	bool bUseBranchesLimit = true;

	// Max branches limit
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, meta = ( EditCondition = "bUseBranchesLimit == true", UIMin = "0" ), Category = "LightningBranching" )
	int32 BranchesLimit = 20;
};
