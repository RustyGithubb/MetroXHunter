/*
 * Implemented by BARRAU Benoit
 */

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GunData.generated.h"

/*
 * Gun Data Asset.
 */

UCLASS()
class METROXHUNTER_API UGunData : public UDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Gun Data|Damage" )
	int32 GunDamageAmmount = 50;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Gun Data|Ammo" )
	int32 CurrentAmmoInMagazine = 6;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Gun Data|Ammo" )
	int32 MaxMagazineAmmo = 6;

};
