#include "Helpers/PatternGenerator.h"

#include "Async/Async.h"
#include "Kismet/KismetMathLibrary.h"

FPatternGenerator::FPatternGenerator(
	FPatternGeneratedEvent _OnPatternGenerated,
	FBranchingParams _BranchingParams,
	FVector _Start, FVector _End,
	FVector _StartDir, FVector _EndDir,
	float _NoiseMult,
	float _Step,
	uint32 _Segments,
	bool _bInEditor,
	int32 _Seed /*= -1*/) :

	OnPatternGenerated(_OnPatternGenerated),
	BranchingParams(_BranchingParams),
	Start(_Start), End(_End),
	StartDir(_StartDir), EndDir(_EndDir),
	NoiseMult(_NoiseMult),
	Step(_Step),
	Segments(_Segments),
	bInEditor(_bInEditor)
{
	if (_Seed == -1)
	{
		// Use a random seed
		Rnd.GenerateNewSeed();
	}
	else
	{
		// Use the specified seed
		Rnd.Initialize(_Seed);
	}
}

void FPatternGenerator::DoWork()
{
	FPatternParams RootParams;

	RootParams.Start = Start;
	RootParams.End = End;
	RootParams.DirStart = StartDir;
	RootParams.DirEnd = EndDir;
	RootParams.Order = 0;

	MakeLightningPattern(ResultPattern, RootParams);

	if ( ResultPattern.bIsValid )
	{
		FLightningPattern Out = ResultPattern;
		FPatternGeneratedEvent Event = OnPatternGenerated;
		bool bEditor = bInEditor;

		AsyncTask( ENamedThreads::GameThread, [Event, Out, bEditor]()
		{
		#if WITH_EDITOR
				if ( bEditor )
				{
					FEditorScriptExecutionGuard ScriptGuard;
					Event.ExecuteIfBound( Out );
				}
				else
		#endif // WITH_EDITOR
				{
					Event.ExecuteIfBound( Out );
				}
		} );
	}
}

void FPatternGenerator::MakeLightningPattern(FLightningPattern& OutPattern, const FPatternParams& Params)
{
	const FVector& StartToEndV = Params.End - Params.Start;
	OutPattern.Transform = FTransform(UKismetMathLibrary::MakeRotFromX(StartToEndV.GetSafeNormal()), Params.Start);

	// Add start point in local space (always zero)
	OutPattern.Points.Add(FVector(0, 0, 0));

	// Add end point in local space
	OutPattern.Points.Add(FVector(StartToEndV.Size(), 0, 0));

	CreateSegments(OutPattern, Params, 0, 0.f, 1.f, true);

	// Initialize last point of the pattern
	OutPattern.bIsValid =  InitializePoint(OutPattern, Params, OutPattern.Points.Num() - 1, 1.f);
	
	OutPattern.SparkData.SparkPosition = ULightningPatternLib::GetStartPoint(OutPattern);
	OutPattern.Seed = Rnd.GetCurrentSeed();
}

uint32 FPatternGenerator::CreateSegments(FLightningPattern& OutPattern, const FPatternParams& Params, int32 SegmentId, float AlphaStart, float AlphaEnd, bool bFirstSegment /*= false*/)
{
	FVector SegStart = OutPattern.Points[SegmentId];
	FVector SegEnd = OutPattern.Points[SegmentId + 1];

	const float SegmentLength = (SegStart - SegEnd).Size();

	if (Step >= SegmentLength)
	{
		/**
		* Passed segment is of length of spark step
		* (segment is indivisible),
		* calculating lightning length
		*/
		OutPattern.Length += SegmentLength;

		// Add point to rich curves and generate a branch if necessary
		InitializePoint(OutPattern, Params, SegmentId, AlphaStart);

		return 0;
	}

	/**
	* Computing path segments from A to B
	*/
	TArray<FVector> PathAB;

	// Noise is 0.06 per cm of length of path
	const float NoiseLevel = 0.06f * SegmentLength * NoiseMult;

	const float AlphaStep = 1.f / float(Segments);
	for (float Alpha = AlphaStep; Alpha < 1.f; Alpha += AlphaStep)
	{
		FVector Pt;
		if (bFirstSegment)
		{
			/**
			 * Cubic Bezier curve control points:
			 *  P0 = SegStart
			 *  P1 = P1
			 *  P2 = P2
			 *  P3 = SegEnd
			 *
			 *  https://en.wikipedia.org/wiki/Bezier_curve
			 */

			FVector P1 = SegStart + Params.DirStart;
			FVector P2 = SegEnd - Params.DirEnd;

			const float InvT = 1.f - Alpha;
			Pt = InvT * InvT * InvT * SegStart + 3 * Alpha * InvT * InvT * P1 + 3 * Alpha * Alpha * InvT * P2 + Alpha * Alpha * Alpha * SegEnd;
		}
		else
		{
			/** Linear interpolation */
			Pt = FMath::Lerp(SegStart, SegEnd, Alpha);
		}

		FVector Noise = FVector(Rnd.FRandRange(-1.f, 1.f), Rnd.FRandRange(-1.f, 1.f), Rnd.FRandRange(-1.f, 1.f)) * NoiseLevel;
		PathAB.Add(Pt + Noise);
	}

	/**
	* Inserting path segments between A and B points
	*/
	OutPattern.Points.Insert(PathAB, SegmentId + 1);

	/**
	* Creating path between created segments
	*/
	uint32 PtIndx = SegmentId;
	uint32 PointsCreated = PathAB.Num();
	float CurAlpha = AlphaStart;
	while (PtIndx <= SegmentId + PointsCreated)
	{
		const float NextAlpha = CurAlpha + AlphaStep * (AlphaEnd - AlphaStart);
		const uint32 NewPointsNum = CreateSegments(OutPattern, Params, PtIndx, CurAlpha, NextAlpha);
		PtIndx += NewPointsNum + 1;
		PointsCreated += NewPointsNum;
		CurAlpha = NextAlpha;
	}

	return PointsCreated;
}

bool FPatternGenerator::InitializePoint(FLightningPattern& Pattern, const FPatternParams& Params, uint32 PtIndx, float CurveAlpha)
{
	const FVector Point = Pattern.Points[PtIndx];

	// Convert pattern segments to Vector3 curve to parse it to the Niagara component later
	Pattern.PointsX.SetKeyInterpMode(Pattern.PointsX.AddKey(PtIndx, Point.X), ERichCurveInterpMode::RCIM_Linear);
	Pattern.PointsY.SetKeyInterpMode(Pattern.PointsY.AddKey(PtIndx, Point.Y), ERichCurveInterpMode::RCIM_Linear);
	Pattern.PointsZ.SetKeyInterpMode(Pattern.PointsZ.AddKey(PtIndx, Point.Z), ERichCurveInterpMode::RCIM_Linear);

	/*
	* Generate a branch
	*/
	if (PtIndx == 0 || PtIndx + 1 == Pattern.Points.Num())
	{
		// Cannot generage a branch on start and end of the lightning
		return true;
	}

	if (BranchingParams.BranchChance <= 0.f)
	{
		// No chance to spawn a branch
		return true;
	}

	if (BranchingParams.bUseBranchesLimit && NumBranches >= BranchingParams.BranchesLimit)
	{
		// Branches limit reached
		return true;
	}

	const float RandVal = Rnd.GetFraction();
	float BranchProb = BranchingParams.BranchChance * FMath::Pow(BranchingParams.BranchFadingChance, Params.Order);
	BranchProb = FMath::Clamp(BranchProb, 0.f, 1.f);
	if (RandVal <= BranchProb)
	{
		const FVector Prev = Pattern.Points[PtIndx - 1];
		const FVector Cur = Pattern.Points[PtIndx];
		const FVector Next = Pattern.Points[PtIndx + 1];

		/**
		* Calculating and rotating direction unit vector
		*/
		FVector Dir = (Cur - Prev).GetSafeNormal();
		FVector DirNext = (Next - Cur).GetSafeNormal();

		// Check Dir not to be equal to DirNext
		if (Dir.Equals(DirNext))
		{
			const float NoiseLevel = 0.01f;
			const float x = Rnd.FRandRange(-NoiseLevel, NoiseLevel);
			const float y = Rnd.FRandRange(-NoiseLevel, NoiseLevel);
			const float z = Rnd.FRandRange(-NoiseLevel, NoiseLevel);
			DirNext += FVector(x, y, z);
		}

		const FVector RotAxis = FVector::CrossProduct(Dir, DirNext).GetSafeNormal();

		const float Angle = Rnd.FRandRange(BranchingParams.RotationRange.GetLowerBoundValue(), BranchingParams.RotationRange.GetUpperBoundValue());
		Dir = Dir.RotateAngleAxis(Angle, RotAxis);

		/**
		* Calculating branch length depending on length of the root
		*/
		const float Length = Rnd.FRandRange(BranchingParams.LengthRange.GetLowerBoundValue(), BranchingParams.LengthRange.GetUpperBoundValue());
		const float RootLength = (ResultPattern.Points[0] - ResultPattern.Points[ResultPattern.Points.Num() - 1]).Size();
		const FVector BranchEnd = Cur + Dir * Length * RootLength;

		// Branch start and end points in world space
		const FVector BranchStartWs = Pattern.Transform.TransformPosition(Cur);
		const FVector BranchEndWs = Pattern.Transform.TransformPosition(BranchEnd);

		++NumBranches;

		/**
		 * Getting a spline that fits the parent curve well
		 */
		FVector P0 = Params.Start;
		FVector P1 = Params.Start + Params.DirStart;
		FVector P2 = Params.End - Params.DirEnd;
		FVector P3 = Params.End;

		FVector Q0 = FMath::Lerp(P0, P1, CurveAlpha);
		FVector Q1 = FMath::Lerp(P1, P2, CurveAlpha);
		FVector Q2 = FMath::Lerp(P2, P3, CurveAlpha);

		FVector R0 = FMath::Lerp(Q0, Q1, CurveAlpha);
		FVector R1 = FMath::Lerp(Q1, Q2, CurveAlpha);

		FVector B = FMath::Lerp(R0, R1, CurveAlpha);

		FPatternParams BranchRootParams;
		BranchRootParams.Start = BranchStartWs;
		BranchRootParams.End = FMath::Lerp(BranchEndWs, Params.End, BranchingParams.ConvergenceFactor);
		BranchRootParams.DirStart = (R1 - B) * BranchingParams.ConvergenceFactor;
		BranchRootParams.DirEnd = Params.DirEnd * BranchingParams.ConvergenceFactor * (1 - CurveAlpha);
		BranchRootParams.Order = Params.Order + 1;

		FLightningPattern BranchPattern;

		MakeLightningPattern( BranchPattern, BranchRootParams );
		if ( BranchPattern.bIsValid )
		{
			Pattern.Branches.Add( { (int32)PtIndx, BranchPattern } );
		}
	}
	return true;
}

