/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "Components/ActorComponent.h"
#include "GunSoundManagerComponent.generated.h"

UCLASS( Abstract, Blueprintable, ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UGunSoundManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunSoundManagerComponent();

	/*
	 * Called when the player shoots its rifle.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnRifleShoot( const FHitResult& HitResult );
	/*
	 * Called when the player failed to shoot its rifle due to an empty magazine.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnRifleShootEmpty();

	/*
	 * Called when the player switch from a weapon to another.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnLightningSwitch( bool bIsLightningActive );
	/*
	 * Called when the player failed to shoot its tesla gun due to a lack of energy.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnLightningShootEmpty();
	/*
	 * Called when the player start shooting with the tesla gun.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnLightningStartShoot();
	/*
	 * Called when the player stop shooting with the tesla gun.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnLightningStopShoot();
	/*
	 * Called when the tesla gun starts reloading itself.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnLightningStartReload();
	/*
	* Called when the tesla gun stops its reloading.
	*/
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnLightningStopReload();
};
