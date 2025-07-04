/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#include "GameplayTagContainer.h"

#include "GameplayLibrary.generated.h"

class UAITargetComponent;

/*
 * Library sharing common gameplay-related code.
 */
UCLASS()
class METROXHUNTER_API UGameplayLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION( BlueprintCallable, Category = "GameplayLibrary" )
	static bool LineTraceSingleByChannelFromBone(
		USkeletalMeshComponent* MeshComponent,
		FName BoneName,
		FHitResult& Hit,
		TArray<AActor*> IgnoredActors,
		float Distance = 250.0f,
		ECollisionChannel TraceChannel = ECollisionChannel::ECC_Visibility,
		EDrawDebugTrace::Type DrawDebug = EDrawDebugTrace::None
	);

	UFUNCTION( BlueprintCallable, Category = "GameplayLibrary", meta = ( WorldContext = "WorldContext", DeterminesOutputType = "DecalClass" ) )
	static AActor* SpawnBloodPuddle(
		UObject* WorldContext,
		TSubclassOf<AActor> DecalClass,
		const FVector& Location,
		const FRotator& Rotator,
		const FVector& Scale
	);
	UFUNCTION( BlueprintCallable, Category = "GameplayLibrary", meta = ( WorldContext = "WorldContext", DeterminesOutputType = "DecalClass" ) )
	static AActor* SpawnBloodPuddleAtBone(
		UObject* WorldContext,
		TSubclassOf<AActor> DecalClass,
		USkeletalMeshComponent* MeshComponent,
		FName BoneName,
		const FVector& Scale
	);


	UFUNCTION( BlueprintCallable, Category = "GameplayLibrary" )
	static void SetBoneHidden(
		USkeletalMeshComponent* SkeletalMesh,
		const FName BoneName,
		bool bNewHidden = true,
		bool bShouldTerminate = false
	);

	UFUNCTION( BlueprintCallable, Category = "GameplayLibrary" )
	static void KillAllAttackersOfClass( UAITargetComponent* TargetComponent, const TSubclassOf<AActor> Class );

	UFUNCTION( BlueprintCallable, Category = "GameplayLibrary" )
	static void GetAllGameplayTags( TArray<FGameplayTag>& Tags );
	UFUNCTION( BlueprintCallable, Category = "GameplayLibrary" )
	static void GetAllChildrenGameplayTags( FGameplayTag ParentTag, TArray<FGameplayTag>& ChildrenTags );

	/*
	 * Returns the player controller out of an actor with an assertion.
	 */
	UFUNCTION( BlueprintPure, Category = "GameplayLibrary" )
	static APlayerController* GetPlayerControllerChecked( const AActor* Actor );
};
