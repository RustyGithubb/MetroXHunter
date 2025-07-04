#include "PoolManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY( LogMetroPoolManager );

/**
 * Initializes the pool for a given actor class with a specified size.
 * This function spawns a number of actors, deactivates them, and adds them
 * both to the full pool and to the available pool.
 *
 * Called from Blueprint to pre-populate the pool.
 */
void UPoolManager::InitializePool( TSubclassOf<AActor> ActorClass, int32 PoolSize )
{
    if ( !ActorClass || PoolSize <= 0 )
    {
        UE_LOG( LogTemp, Warning, TEXT( "InitializePool: Invalid parameters." ) );
        return;
    }

    // Retrieve or create the pool associated with the given actor class.
    FActorPool& PoolStruct = ActorPools.FindOrAdd( ActorClass );
    PoolStruct.ActorArray.Empty();
    PoolStruct.AvailableActorArray.Empty();

    UWorld* World = GetWorld();
    if ( !World )
    {
        UE_LOG( LogTemp, Warning, TEXT( "InitializePool: World is null!" ) );
        return;
    }

    // Spawn and initialize each actor in the pool.
    for ( int32 i = 0; i < PoolSize; i++ )
    {
        AActor* PooledActor = World->SpawnActor<AActor>( ActorClass, FVector::ZeroVector, FRotator::ZeroRotator );
        if ( PooledActor )
        {
            // Deactivate actor at the start.
            PooledActor->SetActorHiddenInGame( true );
            PooledActor->SetActorEnableCollision( false );
            PooledActor->SetActorTickEnabled( false );

            // Add actor to the full pool and mark as available.
            PoolStruct.ActorArray.Add( PooledActor );
            PoolStruct.AvailableActorArray.Add( PooledActor );
        }
    }
}

/**
 * Retrieves an available actor from the pool for the given actor class.
 * This function uses the AvailableActorArray to quickly retrieve (and remove)
 * an actor that is currently not in use.
 *
 * The actor is reactivated before being returned.
 *
 * Called from Blueprint to obtain a pooled actor instead of spawning a new one.
 */
AActor* UPoolManager::GetPooledActor( TSubclassOf<AActor> ActorClass )
{
    QUICK_SCOPE_CYCLE_COUNTER( PoolManager_GetPooledActor );

    if ( !ActorClass )
    {
        return nullptr;
    }

    FActorPool* PoolStruct = ActorPools.Find( ActorClass );
    if ( PoolStruct && PoolStruct->AvailableActorArray.Num() > 0 )
    {
        // Retrieve and remove the last available actor from the pool.
        AActor* Actor = PoolStruct->AvailableActorArray.Pop();
        if ( IsValid(Actor) )
        {
            // Reactivate the actor.
            Actor->SetActorHiddenInGame( false );
            Actor->SetActorEnableCollision( true );
            Actor->SetActorTickEnabled( true );
            return Actor;
        }
    }
    else
    {
        UE_LOG( LogMetroPoolManager, Warning,
               TEXT( "GetPooledActor: No available actor of class '%s' found in pool (total size: %d)" ),
               *ActorClass->GetName(), PoolStruct ? PoolStruct->ActorArray.Num() : 0 );
    }

    return nullptr;
}

/**
 * Returns an actor to the pool.
 * This function deactivates the actor and adds it back to the available pool,
 * making it ready for future reuse.
 *
 * Called from Blueprint when the actor is no longer needed.
 */
void UPoolManager::ReturnActorToPool( AActor* Actor )
{
    if ( !Actor )
    {
        return;
    }

    // Reset the actor's state by deactivating it.
    Actor->SetActorHiddenInGame( true );
    Actor->SetActorEnableCollision( false );
    Actor->SetActorTickEnabled( false );

    // Find the pool to which this actor belongs and add it to the available list if not already present.
    for ( auto& Pair : ActorPools )
    {
        FActorPool& PoolStruct = Pair.Value;
        if ( PoolStruct.ActorArray.Contains( Actor ) )
        {
            if ( !PoolStruct.AvailableActorArray.Contains( Actor ) )
            {
                PoolStruct.AvailableActorArray.Add( Actor );
            }
            break;
        }
    }
}

/**
 * Returns the number of available actors in the pool for the specified actor class.
 * This function logs the number of available actors as well as the total pool size.
 *
 * Called from Blueprint to monitor pool usage.
 */
int32 UPoolManager::GetAvailablePoolActor( TSubclassOf<AActor> ActorClass )
{
    if ( !ActorClass )
    {
        return 0;
    }

    FActorPool* PoolStruct = ActorPools.Find( ActorClass );
    if ( !PoolStruct )
    {
        UE_LOG( LogMetroPoolManager, Verbose, TEXT( "GetAvailablePoolActor: Class '%s' is not initialized for pooling." ), *ActorClass->GetName() );
        return 0;
    }

    int32 AvailableCount = 0;
    // Count only valid actors.
    for ( AActor* Actor : PoolStruct->AvailableActorArray )
    {
        if ( IsValid( Actor ) )
        {
            AvailableCount++;
        }
    }

    UE_LOG( LogMetroPoolManager, Verbose, TEXT( "Pool for class '%s': %d available out of %d" ),
           *ActorClass->GetName(), AvailableCount, PoolStruct->ActorArray.Num() );
    return AvailableCount;
}