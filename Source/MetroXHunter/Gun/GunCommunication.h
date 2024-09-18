/*
 * Implemented by BARRAU Benoit
 */

#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GunCommunication.generated.h"

UINTERFACE(MinimalAPI)
class UGunCommunication : public UInterface
{
	GENERATED_BODY()
};

/*
 * This interface defines communications to the gun, such as retrieving the ammo count,
 * the shoot location...
 */
class METROXHUNTER_API IGunCommunication
{
	GENERATED_BODY()

public:

	// Get the maximum number of ammo in a magazine
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "Gun Communication" )
	int GetMaxMagazineInAmmoCount() const;

	// Get the current number of ammo in the magazine
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "Gun Communication" )
	int GetCurrentMagazineAmmoCount();

	// Called when firing with the gun
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "Gun Communication" )
	void CallOnFire();

	// Set a new ammo count in the magazine
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "Gun Communication" )
	int SetNewAmmoCount(int NewCount);

	// Get the shoot point location
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "Gun Communication" )
	FVector GetShootPointLocation() const;

};
