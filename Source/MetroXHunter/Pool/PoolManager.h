#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PoolManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN( LogMetroPoolManager, Log, All );

USTRUCT()
struct FActorPool
{
    GENERATED_BODY()

    // All spawned actors for this pool
    UPROPERTY()
    TArray<AActor*> ActorArray;

    // Actors that are currently available (not in use)
    UPROPERTY()
    TArray<AActor*> AvailableActorArray;
};

UCLASS( Blueprintable )
class METROXHUNTER_API UPoolManager : public UObject
{
    GENERATED_BODY()

public:
    // Initializes the pool for a given actor class with a specified size
    UFUNCTION( BlueprintCallable, Category = "Pooling" )
    void InitializePool( TSubclassOf<AActor> ActorClass, int32 PoolSize );

    // Returns an available actor from the pool for the given class (O(1) lookup)
    UFUNCTION( BlueprintCallable, Category = "Pooling" )
    AActor* GetPooledActor( TSubclassOf<AActor> ActorClass );

    // Returns the actor to the pool (disables the actor and adds it back to available list)
    UFUNCTION( BlueprintCallable, Category = "Pooling" )
    void ReturnActorToPool( AActor* Actor );

    // Returns the number of available actors for the specified class
    UFUNCTION( BlueprintCallable, Category = "Pooling" )
    int32 GetAvailablePoolActor( TSubclassOf<AActor> ActorClass );

private:
    // Map associating each actor class with its pool of instances
    UPROPERTY()
    TMap<TSubclassOf<AActor>, FActorPool> ActorPools;
};
