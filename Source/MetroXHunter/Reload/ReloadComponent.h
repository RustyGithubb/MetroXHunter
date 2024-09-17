/*
 * Implemented by BARRAU Benoit
 */

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReloadComponent.generated.h"

/*
 * The reload component manage the differents types of reload that the player can achieve.
 */

class UReloadData;
class UMainHUD;
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

/*
 * Event called when the reload state change.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnReloadStateUpdated, EGunReloadState, NewState );

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

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|Ammo", meta = ( AllowPrivateAccess = "true" ) )
	int32 MaxMagazineAmmoCount = 6;

	UPROPERTY(BlueprintAssignable, Category = "Reload|Events")
	FOnReloadStateUpdated OnReloadStateUpdated;

public:

	UFUNCTION( BlueprintCallable, Category = "Reload")
	EGunReloadState GetCurrentReloadState() const;

	UFUNCTION( BlueprintCallable, Category = "Reload" )
	void InitializeReloadData(UReloadData* NewReloadData);

	UFUNCTION( BlueprintCallable, Category = "Reload" )
	void GetNormalizedReloadTimings(float& PerfectReloadStartTime, float& ActiveReloadStartTime, float& ActiveReloadEndTime) const;

	UFUNCTION( BlueprintCallable, Category = "Reload" )
	float GetNormalizedReloadElapsedTime() const;

private:

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload", meta = ( AllowPrivateAccess = "true" ) )
	EGunReloadState CurrentReloadState;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|Data Asset", meta = ( AllowPrivateAccess = "true" ) )
	UReloadData* ReloadDataAsset = nullptr;

private:

	UFUNCTION( BlueprintCallable )
	void StartReloadSequence();

	UFUNCTION( BlueprintCallable )
	void UpdateCurrentReloadState(EGunReloadState NewState);
	UFUNCTION( BlueprintCallable )
	void UpdateAmmoCount(int32 NewAmmoCount);
	UFUNCTION( BlueprintCallable )
	void GetReferences();
	UFUNCTION( BlueprintCallable )
	void GetAmmoData();
	UFUNCTION( BlueprintCallable )
	void ComputeReloadAmmoCount();
	UFUNCTION( BlueprintCallable )
	void OnReloadInput();

private:	

	APlayerController* PlayerController = nullptr; 
	class AHUD* HUD = nullptr;

	bool bIsAmmoFull = false;
	bool bIsGunFireLocked = false;
	bool bIsInfiniteAmmo = false;
		
};
