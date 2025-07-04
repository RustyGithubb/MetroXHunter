/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZeroEnemySoundManagerComponent.generated.h"

UCLASS( Abstract, Blueprintable, ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UZeroEnemySoundManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UZeroEnemySoundManagerComponent();

	/*
	 * Called whenever the player shoots and destroys one of the destructible limbs.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnLimbDestroy( USkeletalMeshComponent* SkeletalMeshComponent, const FName& BoneName, const FVector& HitLocation );
	/*
	 * Called whenever the player shoots and destroys one of the four legs.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnLegDestroy( USkeletalMeshComponent* SkeletalMeshComponent, const FName& BoneName, const FVector& HitLocation );
	/*
	 * Called whenever the player shoots and destroys the bulb, the most vital part that one shots the enemy.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnBulbDestroy( const FHitResult& HitResult );

	/*
	 * Called whenever the enemy rush at the player.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnRushScream();
	/*
	 * Called whenever the enemy is knocked out.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnKnockedOut();
	/*
	 * Called whenever the enemy is getting up from ragdoll.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnRevival();
};
