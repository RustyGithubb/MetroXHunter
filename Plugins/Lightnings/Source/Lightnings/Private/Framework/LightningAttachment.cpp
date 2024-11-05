#include "Framework/LightningAttachment.h"
#include "Framework/LightningEmitter.h"

#include "DrawDebugHelpers.h"

FVector FLightningAttachment::GetAttachmentPoint(ALightningEmitter* Emitter, bool bRandomize)
{
	// Select a point by attachment type
	FVector Point = FVector(0.f, 0.f, 0.f);
	switch (Type)
	{
	case EAttachType::AT_Self:
	{
		Point = Emitter->GetActorLocation();
		break;
	}
	case EAttachType::AT_Location:
	{
		Point = Location;
		break;
	}
	case EAttachType::AT_Actor:
	{
		AActor* Actor = ActorAttachParams.Actor;
		if (Actor)
		{
			if (ActorAttachParams.ComponentName != NAME_None)
			{
				USceneComponent* AttachComponent = 
					Cast<USceneComponent>(Actor->GetDefaultSubobjectByName(ActorAttachParams.ComponentName));

				if (AttachComponent)
				{
					if (ActorAttachParams.SocketName != NAME_None)
					{
						Point = AttachComponent->GetSocketLocation(ActorAttachParams.SocketName);
					}
					else
					{
						Point = AttachComponent->GetComponentLocation();
					}
				}
			}
			else
			{
				Point = Actor->GetActorLocation();
			}
		}
		else
		{
			Point = Emitter->GetActorLocation();
		}
		break;
	}
	default:
		break;
	}

	// Add a random offset if necessary
	FVector Offset = FVector(0.f, 0.f, 0.f);
	if (bRandomize)
	{
		switch (AttachVolumeType)
		{
		case EAttachVolumeType::AV_Box:
		{
			const FVector HalfExtent = AttachVolumeBox.Size / 2.f;
			Offset.X = Emitter->EmitterRng.RandRange(-HalfExtent.X, HalfExtent.X);
			Offset.Y = Emitter->EmitterRng.RandRange(-HalfExtent.Y, HalfExtent.Y);
			Offset.Z = Emitter->EmitterRng.RandRange(-HalfExtent.Z, HalfExtent.Z);
			break;
		}
		case EAttachVolumeType::AV_Cylinder:
		{
			const float HalfHeight = AttachVolumeCylinder.Height / 2.f;

			const FVector Axis = AttachVolumeCylinder.Axis.GetSafeNormal();

			/**
			* Find radius vector to be normal to axis
			*/

			// Some vector other than axis vector
			FVector NonAxisVector = FVector(0.f, 1.f, 0.f);
			if (AttachVolumeCylinder.Axis == NonAxisVector)
			{
				NonAxisVector = FVector(0.f, 0.f, 1.f);
			}

			FVector RadiusVector = FVector::CrossProduct(Axis, NonAxisVector).GetSafeNormal();
			RadiusVector *= Emitter->EmitterRng.RandRange(0.f, AttachVolumeCylinder.Radius);
			RadiusVector = RadiusVector.RotateAngleAxis(Emitter->EmitterRng.RandRange(0.f, 360.f), Axis);

			const FVector AxisOffset = Axis * Emitter->EmitterRng.RandRange(-HalfHeight, HalfHeight);

			Offset = RadiusVector + AxisOffset;
			break;
		}
		case EAttachVolumeType::AV_Sphere:
		{
			FVector RadiusVector = FVector(Emitter->EmitterRng.RandRange(0.f, AttachVolumeSphere.Radius), 0.f, 0.f);

			// Y axis
			FVector RightVector = FVector(0.f, 1.f, 0.f);
			// Z axis
			const FVector UpVector = FVector(0.f, 1.f, 0.f);

			// Rotation angles (Z and Y axis)
			const float PolarAngle = Emitter->EmitterRng.RandRange(0.f, 360.f);
			const float AzimuthalAngle = Emitter->EmitterRng.RandRange(0.f, 360.f);

			RightVector = RightVector.RotateAngleAxis(PolarAngle, UpVector);
			RadiusVector = RadiusVector.RotateAngleAxis(PolarAngle, UpVector);
			RadiusVector = RadiusVector.RotateAngleAxis(AzimuthalAngle, RightVector);

			Offset = RadiusVector;
			break;
		}
		default:
			break;
		}
	}

	const FVector Result = Point + Offset;

	return Result;
}

FVector FLightningAttachment::GetLightningDirection() const
{
	auto Result = Direction;

	switch (Type)
	{
	case EAttachType::AT_Actor:
	{
		FRotator ActorRotation, ComponentRotation, SocketRotation;
		AActor* Actor = ActorAttachParams.Actor;
		if (Actor)
		{
			ActorRotation = Actor->GetActorRotation();
			if (ActorAttachParams.ComponentName != NAME_None)
			{
				USceneComponent* AttachComponent =
					Cast<USceneComponent>(Actor->GetDefaultSubobjectByName(ActorAttachParams.ComponentName));

				if (AttachComponent)
				{
					ComponentRotation = AttachComponent->GetComponentRotation();
					if (ActorAttachParams.SocketName != NAME_None)
					{
						SocketRotation = AttachComponent->GetSocketRotation(ActorAttachParams.SocketName);
					}
					else
					{
						SocketRotation = ComponentRotation;
					}
				}
				else
				{
					ComponentRotation = ActorRotation;
					SocketRotation = ActorRotation;
				}
			}
			else
			{
				ComponentRotation = ActorRotation;
				SocketRotation = ActorRotation;
			}
		}

		FRotator DirectionRotation = FRotator::ZeroRotator;
		switch (ActorAttachParams.DirectionSpace)
		{
		case EAttachActorDirectionSpace::DS_Actor:
		{
			DirectionRotation = ActorRotation;
			break;
		}
		case EAttachActorDirectionSpace::DS_Component:
		{
			DirectionRotation = ComponentRotation;
			break;
		}
		case EAttachActorDirectionSpace::DS_Socket:
		{
			DirectionRotation = SocketRotation;
			break;
		}
		default:
		{
			break;
		}
		}

		Result = DirectionRotation.RotateVector(Result);

		break;
	}
	default:
		break;
	}


	return Result;
}

#if WITH_EDITOR
void FLightningAttachment::DebugDrawVolume(ALightningEmitter* Emitter)
{
	if (AttachVolumeType == EAttachVolumeType::AV_Point)
	{
		return;
	}

	const FVector Center = GetAttachmentPoint(Emitter, false);
	const FColor Color = Emitter->AttachmentVolumesColor;

	switch (AttachVolumeType)
	{
	case EAttachVolumeType::AV_Box:
	{
		const FVector Extent = AttachVolumeBox.Size / 2.f;
		DrawDebugBox(Emitter->GetWorld(), Center, Extent, Color);
		break;
	}
	case EAttachVolumeType::AV_Cylinder:
	{
		const FVector FullLengthAxis = AttachVolumeCylinder.Axis.GetSafeNormal() * AttachVolumeCylinder.Height;
		const FVector Start = Center - FullLengthAxis;
		const FVector	End = Center + FullLengthAxis;
		DrawDebugCylinder(Emitter->GetWorld(), Start, End, AttachVolumeCylinder.Radius, 32, Color);
		break;
	}
	case EAttachVolumeType::AV_Sphere:
	{
		DrawDebugSphere(Emitter->GetWorld(), Center, AttachVolumeSphere.Radius, 32, Color);
		break;
	}
	default:
		break;
	}
}
#endif // WITH_EDITOR
