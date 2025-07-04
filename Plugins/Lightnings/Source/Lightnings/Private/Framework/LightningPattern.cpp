#include "Framework/LightningPattern.h"

// Finds a branch in the lightning pattern based on the given JointId.
// If a branch with the specified JointId is found, it returns true and sets the output parameter OutBranch to the corresponding pattern.
// If no branch is found, it returns false.
bool FLightningPattern::FindBranch( int32 JointId, FLightningPattern& OutBranch ) const
{
	for ( FBranchData Branch : Branches ) // Iterate through all branches in the pattern
	{
		if ( Branch.JointId == JointId ) // Check if the branch's JointId matches the target JointId
		{
			OutBranch = Branch.Pattern; // Set the output branch pattern
			return true; // Return true if the branch is found
		}
	}
	return false; // Return false if no matching branch is found
}

// Retrieves the transformed position of a specific point in the pattern by index.
// If the pattern is invalid or the index is out of bounds, it returns FVector::ZeroVector.
FVector ULightningPatternLib::GetPoint( const FLightningPattern& Pattern, int32 PointIndex )
{
	// Validate the pattern and the point index
	if ( !Pattern.bIsValid || !Pattern.Points.IsValidIndex( PointIndex ) )
	{
		return FVector::ZeroVector; // Return a zero vector if invalid
	}

	// Transform the point position using the pattern's transformation matrix
	return Pattern.Transform.TransformPosition( Pattern.Points[PointIndex] );
}

// Retrieves the start point of the lightning pattern.
// If the pattern is invalid, it returns FVector::ZeroVector.
FVector ULightningPatternLib::GetStartPoint( const FLightningPattern& Pattern )
{
	if ( !Pattern.bIsValid ) // Check if the pattern is valid
	{
		return FVector::ZeroVector;
	}

	// Transform the position of the first point
	return Pattern.Transform.TransformPosition( Pattern.Points[0] );
}

// Retrieves the end point of the lightning pattern.
// If the pattern is invalid, it returns FVector::ZeroVector.
FVector ULightningPatternLib::GetEndPoint( const FLightningPattern& Pattern )
{
	if ( !Pattern.bIsValid ) // Check if the pattern is valid
	{
		return FVector::ZeroVector;
	}

	// Transform the position of the last point
	return Pattern.Transform.TransformPosition( Pattern.Points.Last( 0 ) );
}

// Retrieves the middle point of the lightning pattern.
// If the pattern is invalid, it returns FVector::ZeroVector.
FVector ULightningPatternLib::GetMiddlePoint( const FLightningPattern& Pattern )
{
	if ( !Pattern.bIsValid ) // Check if the pattern is valid
	{
		return FVector::ZeroVector;
	}

	// Transform the position of the middle point
	return Pattern.Transform.TransformPosition( Pattern.Points[Pattern.Points.Num() / 2] );
}
