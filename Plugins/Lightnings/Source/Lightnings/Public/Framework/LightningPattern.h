#pragma once

#include "CoreMinimal.h"
#include "Curves/RichCurve.h"
#include "Engine/EngineTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "LightningPattern.generated.h"

/*
 * This structure describes a lightning spark position and movement
 */
USTRUCT( BlueprintType )
struct FLightningSparkData
{
	GENERATED_USTRUCT_BODY()

	// Spark world-space coordinates
	UPROPERTY( BlueprintReadOnly, Category = "LightningSpark" )
	FVector SparkPosition = FVector( 0.0f, 0.0f, 0.0f );

	// Spark world-space direction
	UPROPERTY( BlueprintReadOnly, Category = "LightningSpark" )
	FVector SparkDirection = FVector( 0.0f, 0.0f, 0.0f );

	// Spark scalar velocity value
	UPROPERTY( BlueprintReadOnly, Category = "LightningSpark" )
	float SparkVelocity = 0.0f;

	// Spark target point index in Points array
	UPROPERTY( BlueprintReadOnly, Category = "LightningSpark" )
	int32 SparkTargetPosIdx = 0;

	// Used for spark movement delays timer
	FTimerHandle SparkDelayTimer;
};

/*
 * This structure describes a lightning, including its child branches.
 * Essentialy geometry params and runtime params of the lightning.
 */
USTRUCT( BlueprintType )
struct FLightningPattern
{
	GENERATED_USTRUCT_BODY()

	bool FindBranch( int32 JointId, FLightningPattern& OutBranch ) const;

	// A transformation from FX pattern space to world space
	UPROPERTY( BlueprintReadOnly, Category = "LightningPattern" )
	FTransform Transform;

	// Lightning curve points locations, in lightning effect local space
	// These points are connected with ribbons on FX renderer
	UPROPERTY( BlueprintReadOnly, Category = "LightningPattern" )
	TArray<FVector> Points;

	// Branches map (joint index -- branch pattern ref)
	TArray<struct FBranchData> Branches;

	// Lightning full length
	UPROPERTY( BlueprintReadOnly, Category = "LightningPattern" )
	float Length = 0.f;

	// Lightning spark data
	UPROPERTY( BlueprintReadOnly, Category = "LightningPattern" )
	FLightningSparkData SparkData;

	// Params to export to NiagaraComponent
	FRichCurve PointsX;
	FRichCurve PointsY;
	FRichCurve PointsZ;

	// A seed value for lightning's RNG
	// Will be randomly generated if equals -1
	int32 Seed = -1;

	// Does this pattern have valid points
	bool bIsValid = false;
};

// Branches map entry
struct FBranchData
{
	int32 JointId;
	FLightningPattern Pattern;
};

/*
 * Helper BP functions for lightning patterns.
 * It is needed because the USTRUCTs don't support UFUNCTIONs.
 */
UCLASS()
class LIGHTNINGS_API ULightningPatternLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/*
	 * @param PointIndex - an index of the point, which location we want to get.
	 * @return the point of the lightning curve at index, in world space.
	 * Returns zeros if the pattern is not valid or index is out of range.
	 */
	UFUNCTION( BlueprintPure, Category = "LightningPattern" )
	static FVector GetPoint( const FLightningPattern& Pattern, int32 PointIndex );

	/*
	 * @return the first point of the lightning curve, in world space.
	 * Returns zeros if the pattern is not valid.
	 */
	UFUNCTION( BlueprintPure, Category = "LightningPattern" )
	static FVector GetStartPoint( const FLightningPattern& Pattern );

	/*
	 * @return the end point of the lightning curve, in world space.
	 * Returns zeros if the pattern is not valid.
	 */
	UFUNCTION( BlueprintPure, Category = "LightningPattern" )
	static FVector GetEndPoint( const FLightningPattern& Pattern );

	/*
	 * @return the point nearly in the middle of the lightning curve, in world space.
	 * Returns zeros if the pattern is not valid.
	 */
	UFUNCTION( BlueprintPure, Category = "LightningPattern" )
	static FVector GetMiddlePoint( const FLightningPattern& Pattern );
};
