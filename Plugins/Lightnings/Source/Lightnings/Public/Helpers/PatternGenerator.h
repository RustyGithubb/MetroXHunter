#pragma once

#include "CoreMinimal.h"
#include "Framework/LightningPattern.h"
#include "Parameters/BranchingParams.h"

#include "Engine/EngineTypes.h"
#include "Curves/RichCurve.h"
#include "Runtime/Core/Public/Async/AsyncWork.h"

DECLARE_DELEGATE_OneParam(FPatternGeneratedEvent, FLightningPattern);

/*
 * Pattern generator creates a lightning pattern using passed params.
 * Can do work in parallel thread.
 * 
 * If initialized with the same seed - generates the same pattern.
 */
class FPatternGenerator : public FNonAbandonableTask
{
	friend class FAsyncTask<FPatternGenerator>;

	struct FPatternParams
	{
		// Curve start point, world space
		FVector Start;

		// Curve end point, world space
		FVector End;

		// Curve direction at start point
		FVector DirStart;

		// Curve direction at end point
		FVector DirEnd;

		/*
		 * The curve order (root curve has order 0,
		 * root branches have order 1 and so on)
		 */
		uint32 Order = 0;
	};

public:
	FPatternGenerator(
		FPatternGeneratedEvent _OnPatternGenerated,
		FBranchingParams _BranchingParams,
		FVector _Start, FVector _End,
		FVector _StartDir, FVector _EndDir,
		float _NoiseMult,
		float _Step,
		uint32 _Segments,
		bool _bInEditor,
		int32 _Seed = -1);

protected:
	void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FPatternGenerator, STATGROUP_ThreadPoolAsyncTasks);
	}

private:
	/*
	 * Generates a lightning pattern from the lightning curve params.
	 * Params provide start and end points in world space,
	 * the pattern will contain the result lightning curve points and branches as nested patterns.
	 * 
	 * @param OutPattern - output pattern storage
	 * @param Params - parameters for the pattern
	 */
	void MakeLightningPattern(FLightningPattern& OutPattern, const FPatternParams& Params);
	uint32 CreateSegments(FLightningPattern& OutPattern, const FPatternParams& Params, int32 SegmentId, float AlphaStart, float AlphaEnd, bool bFirstSegment = false);
	void InitializePoint(FLightningPattern& Pattern, const FPatternParams& Params, uint32 PtIndx, float CurveAlpha);

	//~Begin input params
	// A callback function that is called when the pattern is generated
	FPatternGeneratedEvent OnPatternGenerated;

	FBranchingParams BranchingParams;

	FVector Start, End;
	FVector StartDir, EndDir;

	// Noise level multiplier
	float NoiseMult;

	/*
	 * A maximum length of a single segment of the lightning
	 * (it equals to step size of lightning's spark).
	 */
	float Step;

	// Number of divisions to generate segments
	uint32 Segments;

	// Is generated in editor world (not packaged game or PIE)
	bool bInEditor;
	//~End input params

	// A pattern to export
	FLightningPattern ResultPattern;

	// Number of generated branches
	int32 NumBranches = 0;

	// Random number generator
	FRandomStream Rnd;
};
