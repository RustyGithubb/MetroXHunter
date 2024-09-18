/*
 * Implemented by BARRAU Benoit
 */

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GunComponent.generated.h"

 /*
  * The gun component manage the shooting part, with a laser to aim + VFX.
  */

class UReloadComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class METROXHUNTER_API UGunComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UGunComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

public:

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Gun" )
	ACharacter* PlayerCharacter = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Gun" )
	class UReloadComponent* ReloadSystemComponent;

public :

	UFUNCTION( BlueprintCallable, Category = "Gun" )
	void InitializeAmmo();

	UFUNCTION( BlueprintCallable, Category = "Gun" )
	void InitializeGunData( UGunData* NewGunData );

	UFUNCTION( BlueprintCallable, Category = "Gun" )
	bool IsGunFull() const;

	UFUNCTION( BlueprintCallable, Category = "Gun" )
	bool IsGunEmpty() const;

	UFUNCTION( BlueprintCallable, Category = "Gun" )
	bool CanFire();

	UFUNCTION( BlueprintCallable, Category = "Gun" )
	FVector CalculateWorldlocation( bool UseMovementImprecision );

	UFUNCTION( BlueprintCallable, Category = "Gun" )
	void SetNewAmmoCount(int32 NewCount);

	UFUNCTION( BlueprintCallable, Category = "Gun" )
	void CallOnFire();

private:	

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Gun|Data Asset", meta = ( AllowPrivateAccess = "true" ) )
	UGunData* GunDataAsset = nullptr;


private :

	FVector GetImprecisionAdjustedVector( bool UseMovementImprecision );

};
