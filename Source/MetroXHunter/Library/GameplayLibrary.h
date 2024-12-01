/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameplayLibrary.generated.h"

/**
 * 
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
};
