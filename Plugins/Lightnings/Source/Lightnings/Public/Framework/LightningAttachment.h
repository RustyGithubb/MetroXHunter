#pragma once

#include "CoreMinimal.h"
#include "LightningAttachment.generated.h"

class ALightningEmitter;

/*
 * Describes a type of the lightning attachment
 */
UENUM( BlueprintType )
enum class EAttachType : uint8
{
	// Attach to emitter location
	AT_Self			UMETA( DisplayName = "Self" ),

	// Attach to specific location
	AT_Location		UMETA( DisplayName = "Location" ),

	// Attach to specific actor location (or it's component location)
	AT_Actor		UMETA( DisplayName = "Actor" )
};

/*
 * Describes a type of the attachment's randomization volume
 */
UENUM( BlueprintType )
enum class EAttachVolumeType : uint8
{
	// Don't use randomization (point-type)
	AV_Point		UMETA( DisplayName = "Point" ),

	// Box-type is specified by extent-vector
	AV_Box			UMETA( DisplayName = "Box" ),

	// Cylinder-type is specified by axis vector, height and radius
	AV_Cylinder		UMETA( DisplayName = "Cylinder" ),

	// Sphere-type is specified by radius
	AV_Sphere		UMETA( DisplayName = "Sphere" )
};

/*
 * In which space the lightning direction should be calculated when is attached to actor
 */
UENUM( BlueprintType )
enum class EAttachActorDirectionSpace : uint8
{
	DS_World		UMETA( DisplayName = "World" ),
	DS_Actor		UMETA( DisplayName = "Actor" ),
	DS_Component	UMETA( DisplayName = "Component" ),
	DS_Socket		UMETA( DisplayName = "Socket" ),
};

/*
 * Specific structs for attachment volumes
 */
USTRUCT( BlueprintType )
struct FAttachVolumeBox
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "BoxAttachVolume" )
	FVector Size = FVector( 1.0f, 1.0f, 1.0f );
};

USTRUCT( BlueprintType )
struct FAttachVolumeCylinder
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "CylinderAttachVolume" )
	FVector Axis = FVector( 0.0f, 0.0f, 1.0f );
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "CylinderAttachVolume" )
	float Height = 1.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "CylinderAttachVolume" )
	float Radius = 1.0f;
};

USTRUCT( BlueprintType )
struct FAttachVolumeSphere
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "SphereAttachVolume" )
	float Radius = 1.0f;
};

/*
 * Contains information about the actor to which the lightning is attached
 */
USTRUCT( BlueprintType )
struct FActorAttachParams
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ActorAttachParams" )
	AActor* Actor = NULL;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ActorAttachParams" )
	FName ComponentName = NAME_None;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ActorAttachParams" )
	FName SocketName = NAME_None;

	// In which space the lightning direction should be calculated
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ActorAttachParams" )
	EAttachActorDirectionSpace DirectionSpace = EAttachActorDirectionSpace::DS_World;
};

/*
 * Contains information about attachment of
 * the lightning to a specific point in the world.
 *
 * A lightning can be attached to emitter's location,
 * some specific location or some actor's component
 * (to it's root component if component name is not specified).
 *
 * An attachment point can be adjusted by random offset in
 * a specific volume described by AttachVolumeType and AttachVolume.
 */
USTRUCT( BlueprintType )
struct FLightningAttachment
{
	GENERATED_USTRUCT_BODY()

	/*
	 * Calculate a point in this attachment volume for a specified lightning emitter.
	 * Also randomize it with an offset if necessary.
	 */
	FVector GetAttachmentPoint( ALightningEmitter* Emitter, bool bRandomize = true );

	/*
	 * @return calculated actual direction, for actor attachment it
	 * is converted to actor or component space if necessary
	 */
	FVector GetLightningDirection() const;

#if WITH_EDITOR
	// Draw attachment volume shape for a specified lightning emitter
	void DebugDrawVolume( ALightningEmitter* Emitter );
#endif //WITH_EDITOR

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "LightningAttachment" )
	EAttachType Type = EAttachType::AT_Self;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( EditCondition = "Type == EAttachType::AT_Location" ), Category = "LightningAttachment" )
	FVector Location = FVector( 0.0f, 0.0f, 0.0f );
	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( EditCondition = "Type == EAttachType::AT_Actor" ), Category = "LightningAttachment" )
	FActorAttachParams ActorAttachParams;

	/*
	 * Velocity direction of the lightning in that point.
	 * Use GetLightningDirection() to get exact direction, converted from desired space.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "LightningAttachment" )
	FVector Direction = FVector( 0.0f, 0.0f, 1.0f );

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "LightningAttachment" )
	EAttachVolumeType AttachVolumeType = EAttachVolumeType::AV_Point;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( EditCondition = "AttachVolumeType == EAttachVolumeType::AV_Box" ), Category = "LightningAttachment" )
	FAttachVolumeBox AttachVolumeBox;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( EditCondition = "AttachVolumeType == EAttachVolumeType::AV_Cylinder" ), Category = "LightningAttachment" )
	FAttachVolumeCylinder AttachVolumeCylinder;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( EditCondition = "AttachVolumeType == EAttachVolumeType::AV_Sphere" ), Category = "LightningAttachment" )
	FAttachVolumeSphere AttachVolumeSphere;
};
