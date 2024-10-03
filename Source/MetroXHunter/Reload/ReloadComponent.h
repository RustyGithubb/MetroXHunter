/*
 * Implemented by BARRAU Benoit
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReloadComponent.generated.h"

class UGunComponent;
class UReloadData;
class UInventoryComponent;
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
enum class EReloadState : uint8
{
	Start,
	Normal,
	Active,
	Perfect,
	Failed,
	Cancel,
};

 /*
  * The reload component manage the differents types of reload that the player can achieve.
  */

UCLASS( BlueprintType, meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UReloadComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UReloadComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
	void SetupPlayerInputComponent();

	UFUNCTION( BlueprintCallable, Category = "Reload" )
	void SetAmmoCount( int NewCount );
	UFUNCTION( BlueprintCallable, Category = "Reload" )
	void DecrementAmmo();
	UFUNCTION( BlueprintCallable, Category = "Reload" )
	void ComputeReloadAmmoCount( int& NewMagazineAmmoCount, int& InventoryAmmoConsumed );

	UFUNCTION( BlueprintCallable, Category = "Reload" )
	void CancelReload();
	UFUNCTION( BlueprintCallable, Category = "Reload" )
	void FinalizeReload( int NewAmmoCount, float FinalWaitingTime, int InventoryAmmoCountUsed );

	UFUNCTION( BlueprintCallable, Category = "Reload" )
	bool IsReloading() const;
	UFUNCTION( BlueprintCallable, Category = "Reload" )
	bool IsAmmoFull() const;
	UFUNCTION( BlueprintCallable, Category = "Reload" )
	bool IsGunEmpty() const;

	UFUNCTION( BlueprintCallable, Category = "Reload" )
	void GetCurrentAmmo( int& CurrentAmmo ) const;
	UFUNCTION( BlueprintCallable, Category = "Reload" )
	void GetMaxAmmo( int& MaxAmmo ) const;
	UFUNCTION( BlueprintCallable, Category = "Reload" )
	void GetNormalizedReloadTimings( 
		float& PerfectReloadStartTime, float& ActiveReloadStartTime, float& ActiveReloadEndTime
	) const;
	UFUNCTION( BlueprintCallable, Category = "Reload" )
	float GetNormalizedReloadElapsedTime() const;

public:
	/*
	 * Event called when ammo decrease / reload
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnAmmoCountUpdated );
	UPROPERTY( BlueprintAssignable, Category = "Reload|Events" )
	FOnAmmoCountUpdated OnAmmoCountUpdated;

	/*
	 * Event called when gauge appear to update the moving cursor
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnUpdateCursorPosition );
	UPROPERTY( BlueprintAssignable, Category = "Reload|Events" )
	FOnUpdateCursorPosition OnUpdateCursorPosition;

	/* 
	 * Event called when the reload state change
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnReloadStateChanged, EReloadState, ReloadType );
	UPROPERTY( BlueprintAssignable, Category = "Reload|Events" )
	FOnReloadStateChanged OnReloadStateChanged;

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|Inputs" )
	UInputAction* ReloadAction = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|Ammo" )
	int CurrentAmmoInMagazine = 6;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|Ammo" )
	int MaxAmmoInMagazine = 6;

	UPROPERTY( BlueprintReadWrite, Category = "Reload" )
	float ReloadElapsedTime = 0.0f;

private:
	void RetrieveReferences();
	void StartReloadSequence();
	void OnReloadInput();
	void TriggerReload( EReloadState ReloadState, float ReloadAnimTime, float FinalWaitingTime );

	void RetrievePlayerInventory();
	void RetrieveHUD();

private:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|Enum", meta = ( AllowPrivateAccess = "true" ) )
	EGunState CurrentGunState = EGunState::Idle;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|DataAsset", meta = ( AllowPrivateAccess = "true" ) )
	UReloadData* ReloadDataAsset = nullptr;

	UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = "true" ) )
	UInventoryComponent* PlayerInventory = nullptr;

	UPROPERTY( meta = ( AllowPrivateAccess = "true" ) )
	bool bIsReloadActive = false;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reload|Debug", meta = ( AllowPrivateAccess = "true" ) )
	bool bUseInfiniteAmmo = false;

	APlayerController* PlayerController = nullptr;
	AActor* CharacterGun = nullptr;
	AHUD* HUD = nullptr;

	FTimerHandle TimerHandleReloadFinalize {};
	FTimerHandle TimerHandleReloadPlayback {};
};
