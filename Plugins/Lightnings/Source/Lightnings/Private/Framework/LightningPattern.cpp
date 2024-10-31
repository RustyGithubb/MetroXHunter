#include "Framework/LightningPattern.h"

bool FLightningPattern::FindBranch(int32 JointId, FLightningPattern& OutBranch) const
{
	for (FBranchData Branch : Branches)
	{
		if (Branch.JointId == JointId)
		{
			OutBranch = Branch.Pattern;
			return true;
		}
	}
	return false;
}

FVector ULightningPatternLib::GetPoint(const FLightningPattern& Pattern, int32 PointIndex)
{
	if (!Pattern.bIsValid || !Pattern.Points.IsValidIndex(PointIndex))
	{
		return FVector::ZeroVector;
	}
	return Pattern.Transform.TransformPosition(Pattern.Points[PointIndex]);
}

FVector ULightningPatternLib::GetStartPoint(const FLightningPattern& Pattern)
{
	if (!Pattern.bIsValid)
	{
		return FVector::ZeroVector;
	}
	return Pattern.Transform.TransformPosition(Pattern.Points[0]);
}

FVector ULightningPatternLib::GetEndPoint(const FLightningPattern& Pattern)
{
	if (!Pattern.bIsValid)
	{
		return FVector::ZeroVector;
	}
	return Pattern.Transform.TransformPosition(Pattern.Points.Last(0));
}

FVector ULightningPatternLib::GetMiddlePoint(const FLightningPattern& Pattern)
{
	if (!Pattern.bIsValid)
	{
		return FVector::ZeroVector;
	}
	return Pattern.Transform.TransformPosition(Pattern.Points[Pattern.Points.Num() / 2]);
}
