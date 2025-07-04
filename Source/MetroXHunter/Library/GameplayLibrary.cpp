/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "Library/GameplayLibrary.h"
#include "Library/UtilityLibrary.h"

#include "AI/AITargetComponent.h"
#include "AI/AIAttackerComponent.h"
#include "Health/HealthComponent.h"

#include "GameplayTagsManager.h"

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

void UGameplayLibrary::SetBoneHidden(
	USkeletalMeshComponent* SkeletalMesh,
	const FName BoneName,
	bool bNewHidden, bool bShouldTerminate
)
{
	if ( bNewHidden )
	{
		SkeletalMesh->HideBoneByName(
			BoneName,
			bShouldTerminate ? EPhysBodyOp::PBO_Term : EPhysBodyOp::PBO_None
		);
	}
	else
	{
		SkeletalMesh->UnHideBoneByName( BoneName );
	}

	// NOTE: Using SetCollisionEnabled doesn't seem to work so we use this one instead.
	// NOTE: DON'T USE SetShapeCollisionEnabled OR IT MAY CRASH UPON SIMULATE PHYSICS. Use PBO_Term on HideBoneByName instead.
	/*int32 OperationCount = 0;
	SkeletalMesh->ForEachBodyBelow(
		BoneName, true, true,
		[&]( FBodyInstance* BodyInstance )
		{
			BodyInstance->SetShapeCollisionEnabled(
				0,
				bNewHidden ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics,
				false
			);
			OperationCount++;
		}
	);
	
	UUtilityLibrary::LogMessage( TEXT( "SetBoneHidden: %s on %d bones" ), *BoneName.ToString(), OperationCount );*/
}

void UGameplayLibrary::KillAllAttackersOfClass( UAITargetComponent* TargetComponent, const TSubclassOf<AActor> Class )
{
	TArray<UAIAttackerComponent*> Attackers {};
	TargetComponent->GetAttackers( Attackers );

	for ( UAIAttackerComponent* Attacker : Attackers )
	{
		AActor* Owner = Attacker->GetOwner();

		// As AIAttackerComponent should be put on AIController, we need to use the pawn instead.
		if ( AController* Controller = Cast<AController>( Owner ) )
		{
			Owner = Controller->GetPawn();
		}

		// Filter by class
		if ( !Owner->IsA( Class ) ) continue;

		UHealthComponent* HealthComponent = Owner->GetComponentByClass<UHealthComponent>();
		if ( !IsValid( HealthComponent ) ) continue;

		FDamageContext DamageContext {};
		DamageContext.DamageAmount = HealthComponent->CurrentHealth;
		DamageContext.DamageType = EDamageType::Generic;
		HealthComponent->CurrentHealth = 0;
		HealthComponent->OnDeath.Broadcast( DamageContext );
	}
}

void UGameplayLibrary::GetAllGameplayTags( TArray<FGameplayTag>& Tags )
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	FGameplayTagContainer Container {};
	Manager.RequestAllGameplayTags( Container, false );

	Container.GetGameplayTagArray( Tags );
}

void UGameplayLibrary::GetAllChildrenGameplayTags( FGameplayTag ParentTag, TArray<FGameplayTag>& ChildrenTags )
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	FGameplayTagContainer Container = Manager.RequestGameplayTagChildren( ParentTag );
	Container.GetGameplayTagArray( ChildrenTags );
}

APlayerController* UGameplayLibrary::GetPlayerControllerChecked( const AActor* Actor )
{
	const APawn* PlayerPawn = CastChecked<APawn>( Actor );
	return CastChecked<APlayerController>( PlayerPawn->GetController() );
}
