#include "Checkpoint/CheckpointComponent.h"
#include "Telemetry/TelemetryComponent.h"

#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

UCheckpointComponent::UCheckpointComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCheckpointComponent::BeginPlay()
{
	Super::BeginPlay();

	// Retrieve character capsule's half height
	const ACharacter* OwnerAsCharacter = CastChecked<ACharacter>( GetOwner() );
	const UCapsuleComponent* CapsuleComponent = OwnerAsCharacter->GetCapsuleComponent();
	CharacterCapsuleHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
	CharacterCapsuleRadius = CapsuleComponent->GetScaledCapsuleRadius();

	// Retrieve telemetry component (optional)
	TelemetryComponent = GetOwner()->GetComponentByClass<UTelemetryComponent>();
}

void UCheckpointComponent::SaveCheckpoint(
	const FVector& NewCheckpointLocation,
	const FRotator& NewCheckpointRotation,
	const FString& NewCheckpointName
)
{
	const double TraceOffsetStart = 10.0;
	const double TraceOffsetEnd = -1000.0;

	// Relocate with a trace down to the ground to fit the player's character
	FHitResult HitResult {};
	const TArray<AActor*> IgnoredActors { GetOwner() };
	bool bHit = UKismetSystemLibrary::CapsuleTraceSingle( 
		this,
		NewCheckpointLocation + FVector { 0.0, 0.0, TraceOffsetStart },
		NewCheckpointLocation + FVector { 0.0, 0.0, TraceOffsetEnd },
		CharacterCapsuleRadius, CharacterCapsuleHalfHeight,
		RelocationTraceQuery,
		/* bTraceComplex = */ false,
		IgnoredActors,
		EDrawDebugTrace::ForDuration,
		HitResult,
		/* bIgnoreSelf = */ true // TODO: Check if IgnoredActors is required with that option enabled.
	);
	LastCheckpointLocation = bHit ? HitResult.Location : NewCheckpointLocation;

	// Rebuild the rotation by using forward to prevent weird rotation
	// arkaht: This code was ported from Blueprint, I don't know if it's actually useful to do it 
	//         this way, need testing...
	const FRotator FixedRotation = UKismetMathLibrary::MakeRotFromXZ( 
		NewCheckpointRotation.Vector(),
		FVector::UpVector
	);
	LastCheckpointRotation = FixedRotation;

	LastCheckpointName = NewCheckpointName;
	
	// Save telemetry
	if ( IsValid( TelemetryComponent ) )
	{
		TelemetryComponent->WriteDataToFile();
	}
}
