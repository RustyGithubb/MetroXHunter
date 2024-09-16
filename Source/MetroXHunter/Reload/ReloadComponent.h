#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReloadComponent.generated.h"

/*
* The reload component manage the differents types of reload that the player can achieve.
*/

class UReloadData;
class UInputAction;

UENUM( BlueprintType )
enum class EGunState : uint8
{
	Idle,
	Firing,
	Reloading,
};

UENUM( BlueprintType )
enum class EGunReloadState : uint8
{
	NormalStart,
	NormalFinished,
	ActiveStart,
	ActiveFinished,
	PerfectStart,
	PerfectFinished,
	FailedStart,
	FailedFinished,
};

UCLASS( BlueprintType, meta=(BlueprintSpawnableComponent) )
class METROXHUNTER_API UReloadComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UReloadComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
	void SetupPlayerInputComponent();

public:

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|Inputs", meta = ( AllowPrivateAccess = "true" ) )
	UInputAction* ReloadAction = nullptr;



private:

	UFUNCTION( BlueprintCallable )
	void StartReloadSequence();

	UFUNCTION( BlueprintCallable )
	void UpdateCurrentReloadState();
	UFUNCTION( BlueprintCallable )
	void UpdateAmmoCount();

	UFUNCTION( BlueprintCallable )
	void GetReferences();
	UFUNCTION( BlueprintCallable )
	void GetCurrentReloadState();
	UFUNCTION( BlueprintCallable )
	void GetAmmoData();
	UFUNCTION( BlueprintCallable )
	void GetNormalizedReloadTimings();
	UFUNCTION( BlueprintCallable )
	void GetNormalizedReloadElapsedTime();

	UFUNCTION( BlueprintCallable )
	void ComputeReloadAmmoCount();
	UFUNCTION( BlueprintCallable )
	void OnReloadInput();

private:	

	APlayerController* PlayerController = nullptr;

	bool bIsAmmoFull = false;
	bool bIsGunFireLocked = false;
	bool bIsInfiniteAmmo = false;
		
};
