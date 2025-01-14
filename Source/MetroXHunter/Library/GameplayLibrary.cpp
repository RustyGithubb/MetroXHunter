/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "Library/GameplayLibrary.h"
#include "Library/UtilityLibrary.h"

#include "Kismet/KismetSystemLibrary.h"

bool UGameplayLibrary::LineTraceSingleByChannelFromBone(
	USkeletalMeshComponent* MeshComponent,
	FName BoneName,
	FHitResult& Hit,
	TArray<AActor*> IgnoredActors,
	float Distance,
	ECollisionChannel TraceChannel,
	EDrawDebugTrace::Type DrawDebug
)
{
	const FTransform SpawnBoneTransform = MeshComponent->GetBoneTransform( BoneName );

	return UKismetSystemLibrary::LineTraceSingle(
		MeshComponent,
		SpawnBoneTransform.GetLocation(),
		SpawnBoneTransform.GetLocation() + FVector::UpVector * -Distance,
		UEngineTypes::ConvertToTraceType( TraceChannel ),
		/* bTraceComplex */ false,
		IgnoredActors,
		DrawDebug,
		Hit,
		/* bIgnoreSelf */ true
	);
}

AActor* UGameplayLibrary::SpawnBloodPuddle(
	UObject* WorldContext,
	TSubclassOf<AActor> DecalClass,
	const FVector& Location,
	const FRotator& Rotator,
	const FVector& Scale
)
{
	UWorld* World = GEngine->GetWorldFromContextObject(
		WorldContext,
		EGetWorldErrorMode::ReturnNull
	);
	if ( World == nullptr )
	{
		UUtilityLibrary::PrintWarning(
			TEXT( "GameplayLibrary: Failed to SpawnBloodPuddle: World is invalid" )
		);
		return nullptr;
	}

	auto BloodActor = World->SpawnActor<AActor>( DecalClass, Location, Rotator );
	if ( !IsValid( BloodActor ) )
	{
		UUtilityLibrary::PrintWarning(
			TEXT( "GameplayLibrary: Failed to SpawnBloodPuddle: Spawned Actor is invalid" )
		);
		return nullptr;
	}

	// NOTE: For unknown reasons, passing a Transform with a scale in SpawnActor
	//		 doesn't apply the scale, so we're forcing it here.
	BloodActor->SetActorScale3D( Scale );

	return BloodActor;
}

AActor* UGameplayLibrary::SpawnBloodPuddleAtBone(
	UObject* WorldContext,
	TSubclassOf<AActor> DecalClass,
	USkeletalMeshComponent* MeshComponent,
	FName BoneName,
	const FVector& Scale
)
{
	FHitResult Hit {};
	bool bHasHit = UGameplayLibrary::LineTraceSingleByChannelFromBone(
		MeshComponent,
		BoneName,
		Hit,
		TArray<AActor*> {}
	);
	if ( !bHasHit )
	{
		UUtilityLibrary::PrintWarning(
			TEXT( "GameplayLibrary: Failed to spawn blood puddle due to a line trace from location %s" ),
			*Hit.TraceStart.ToCompactString()
		);
		return nullptr;
	}

	return SpawnBloodPuddle(
		WorldContext,
		DecalClass,
		Hit.ImpactPoint,
		Hit.ImpactNormal.ToOrientationRotator(),
		Scale
	);
}

APlayerController* UGameplayLibrary::GetPlayerControllerChecked( const AActor* Actor )
{
	const APawn* PlayerPawn = CastChecked<APawn>( Actor );
	return CastChecked<APlayerController>( PlayerPawn->GetController() );
}
