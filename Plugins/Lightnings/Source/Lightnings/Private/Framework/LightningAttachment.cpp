#include "Framework/LightningAttachment.h"
#include "Framework/LightningEmitter.h"

#include "DrawDebugHelpers.h"

FVector FLightningAttachment::GetAttachmentPoint( ALightningEmitter* Emitter, bool bRandomize )
{
	// Early exit if the Emitter is invalid
	if ( !Emitter )
	{
		return FVector::ZeroVector;
	}

	// Initialize the attachment point to a default value
	FVector Point = FVector::ZeroVector;

	// Determine the attachment point based on the type of attachment
	switch ( Type )
	{
	case EAttachType::AT_Self:
		// Attach to the Emitter's location
		Point = Emitter->GetActorLocation();
		break;

	case EAttachType::AT_Location:
		// Use the explicitly defined location
		Point = Location;
		break;

	case EAttachType::AT_Actor:
	{
		AActor* Actor = ActorAttachParams.Actor;
		if ( !Actor )
		{
			// Fallback to the Emitter's location if the actor is invalid
			Point = Emitter->GetActorLocation();
			break;
		}

		if ( ActorAttachParams.ComponentName == NAME_None )
		{
			// Attach directly to the actor's location if no specific component is specified
			Point = Actor->GetActorLocation();
			break;
		}

		// Attempt to find the specified component on the actor
		USceneComponent* AttachComponent = Cast<USceneComponent>( Actor->GetDefaultSubobjectByName( ActorAttachParams.ComponentName ) );
		if ( !AttachComponent )
		{
			// Fallback to the actor's location if the component is not found
			Point = Actor->GetActorLocation();
			break;
		}

		// Use the socket location if specified; otherwise, use the component's location
		Point = ( ActorAttachParams.SocketName != NAME_None )
			? AttachComponent->GetSocketLocation( ActorAttachParams.SocketName )
			: AttachComponent->GetComponentLocation();
		break;
	}

	default:
		// No specific behavior for other types
		break;
	}

	// If randomization is not requested, return the determined point
	if ( !bRandomize )
	{
		return Point;
	}

	// Initialize the offset vector to zero
	FVector Offset = FVector::ZeroVector;

	// Apply randomization based on the volume type
	switch ( AttachVolumeType )
	{
	case EAttachVolumeType::AV_Box:
	{
		// Randomize the offset within the bounds of the box
		const FVector HalfExtent = AttachVolumeBox.Size / 2.f;
		Offset = FVector(
			Emitter->EmitterRng.RandRange( -HalfExtent.X, HalfExtent.X ),
			Emitter->EmitterRng.RandRange( -HalfExtent.Y, HalfExtent.Y ),
			Emitter->EmitterRng.RandRange( -HalfExtent.Z, HalfExtent.Z )
		);
		break;
	}

	case EAttachVolumeType::AV_Cylinder:
	{
		// Randomize the offset within the bounds of the cylinder
		const float HalfHeight = AttachVolumeCylinder.Height / 2.f;
		const FVector Axis = AttachVolumeCylinder.Axis.GetSafeNormal();

		// Find a random point on the cylinder's surface
		FVector NonAxisVector = ( AttachVolumeCylinder.Axis.Equals( FVector( 0.f, 1.f, 0.f ) ) ) ? FVector( 0.f, 0.f, 1.f ) : FVector( 0.f, 1.f, 0.f );
		FVector RadiusVector = FVector::CrossProduct( Axis, NonAxisVector ).GetSafeNormal() * Emitter->EmitterRng.RandRange( 0.f, AttachVolumeCylinder.Radius );

		// Rotate the radius vector randomly around the axis
		RadiusVector = RadiusVector.RotateAngleAxis( Emitter->EmitterRng.RandRange( 0.f, 360.f ), Axis );
		Offset = RadiusVector + ( Axis * Emitter->EmitterRng.RandRange( -HalfHeight, HalfHeight ) );
		break;
	}

	case EAttachVolumeType::AV_Sphere:
	{
		// Randomize the offset within the sphere's bounds
		const float PolarAngle = Emitter->EmitterRng.RandRange( 0.f, 360.f );
		const float AzimuthalAngle = Emitter->EmitterRng.RandRange( 0.f, 360.f );

		FVector RadiusVector = FVector( Emitter->EmitterRng.RandRange( 0.f, AttachVolumeSphere.Radius ), 0.f, 0.f );
		FVector RightVector = FVector( 0.f, 1.f, 0.f );
		const FVector UpVector = FVector( 0.f, 0.f, 1.f );

		// Rotate the radius vector around the sphere using random angles
		RightVector = RightVector.RotateAngleAxis( PolarAngle, UpVector );
		RadiusVector = RadiusVector.RotateAngleAxis( PolarAngle, UpVector );
		RadiusVector = RadiusVector.RotateAngleAxis( AzimuthalAngle, RightVector );

		Offset = RadiusVector;
		break;
	}

	default:
		// No randomization for other types
		break;
	}

	// Return the final attachment point with the random offset applied
	return Point + Offset;
}

FVector FLightningAttachment::GetLightningDirection() const
{
	// Default to the defined direction if not attached to an actor
	if ( Type != EAttachType::AT_Actor || !ActorAttachParams.Actor )
	{
		return Direction;
	}

	// Retrieve the actor and initialize rotation variables
	AActor* Actor = ActorAttachParams.Actor;
	FRotator ActorRotation = Actor->GetActorRotation();
	FRotator ComponentRotation = ActorRotation;
	FRotator SocketRotation = ActorRotation;

	// Check for a valid component name
	if ( ActorAttachParams.ComponentName != NAME_None )
	{
		USceneComponent* AttachComponent = Cast<USceneComponent>( Actor->GetDefaultSubobjectByName( ActorAttachParams.ComponentName ) );
		if ( AttachComponent )
		{
			ComponentRotation = AttachComponent->GetComponentRotation();
			SocketRotation = ( ActorAttachParams.SocketName != NAME_None )
				? AttachComponent->GetSocketRotation( ActorAttachParams.SocketName )
				: ComponentRotation;
		}
	}

	// Determine the direction space to use for rotation
	FRotator DirectionRotation;
	switch ( ActorAttachParams.DirectionSpace )
	{
	case EAttachActorDirectionSpace::DS_Actor:
		DirectionRotation = ActorRotation;
		break;

	case EAttachActorDirectionSpace::DS_Component:
		DirectionRotation = ComponentRotation;
		break;

	case EAttachActorDirectionSpace::DS_Socket:
		DirectionRotation = SocketRotation;
		break;

	default:
		// No rotation applied if the space is undefined
		break;
	}

	// Return the rotated direction vector
	return DirectionRotation.RotateVector( Direction );
}

#if WITH_EDITOR
void FLightningAttachment::DebugDrawVolume( ALightningEmitter* Emitter )
{
	// Early exit if the emitter is invalid or the volume is a point
	if ( !Emitter || AttachVolumeType == EAttachVolumeType::AV_Point )
	{
		return;
	}

	// Get the center of the volume
	const FVector Center = GetAttachmentPoint( Emitter, false );
	const FColor Color = Emitter->AttachmentVolumesColor;

	// Draw the debug shape based on the volume type
	switch ( AttachVolumeType )
	{
	case EAttachVolumeType::AV_Box:
		DrawDebugBox( Emitter->GetWorld(), Center, AttachVolumeBox.Size / 2.f, Color );
		break;

	case EAttachVolumeType::AV_Cylinder:
	{
		const FVector Axis = AttachVolumeCylinder.Axis.GetSafeNormal();
		const FVector Start = Center - ( Axis * AttachVolumeCylinder.Height / 2.f );
		const FVector End = Center + ( Axis * AttachVolumeCylinder.Height / 2.f );
		DrawDebugCylinder( Emitter->GetWorld(), Start, End, AttachVolumeCylinder.Radius, 32, Color );
		break;
	}

	case EAttachVolumeType::AV_Sphere:
		DrawDebugSphere( Emitter->GetWorld(), Center, AttachVolumeSphere.Radius, 32, Color );
		break;

	default:
		// No debug drawing for other types
		break;
	}
}
#endif // WITH_EDITOR
