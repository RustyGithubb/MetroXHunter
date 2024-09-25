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

/*UENUM(BlueprintType)
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
};*/

UENUM( BlueprintType )
enum class EReloadState : uint8
{
	Start,
	Normal,
	Active,
	Perfect,
	Failed,
};

/*
 * Event called when the reload state change.
 */

UCLASS( BlueprintType, meta=(BlueprintSpawnableComponent) )
class METROXHUNTER_API UReloadComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UReloadComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
	void SetupPlayerInputComponent();

	//UFUNCTION( BlueprintCallable, Category = "Reload" )
	//EGunReloadState GetCurrentReloadState() const;

	UFUNCTION( BlueprintCallable, Category = "Reload" )
	void InitializeReloadData( UReloadData* NewReloadData );

	UFUNCTION( BlueprintCallable, Category = "Reload" )
	void UpdateAmmoCount( int NewCount );

	UFUNCTION( BlueprintCallable, Category = "Reload" )
	void ComputeReloadAmmoCount( int& NewMagazineAmmoCount, int& InventoryAmmoConsumed );

	UFUNCTION( BlueprintCallable, Category = "Reload" )
	void FinalizeReload( int NewAmmoCount, /*EGunReloadState ReloadType,*/ float FinalWaitingTime, int InventoryAmmoCountUsed );

	UFUNCTION( BlueprintCallable, Category = "Reload" )
	bool IsGunFireLocked() const;

	UFUNCTION( BlueprintCallable, Category = "Reload" )
	bool bIsAmmoFull() const;

	UFUNCTION( BlueprintCallable, Category = "Reload" )
	void RetrieveReferences();

	UFUNCTION( BlueprintCallable, Category = "Reload" )
	void GetAmmoData( int& IndexMagazine, int& MaxMagazineAmmo ) const;

	UFUNCTION( BlueprintCallable, Category = "Reload" )
	void GetNormalizedReloadTimings( float& PerfectReloadStartTime, float& ActiveReloadStartTime, float& ActiveReloadEndTime ) const;

	UFUNCTION( BlueprintCallable, Category = "Reload" )
	float GetNormalizedReloadElapsedTime() const;

public:

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|Inputs" )
	UInputAction* ReloadAction = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|Ammo" )
	int MaxMagazineAmmoCount = 6;

public:

	//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnReloadStateUpdated, EGunReloadState, NewState );
	//UPROPERTY( BlueprintAssignable, Category = "Reload|Events" )
	//FOnReloadStateUpdated OnReloadStateUpdated;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnAmmoCountUpdated );
	UPROPERTY( BlueprintAssignable, Category = "Reload|Events" )
	FOnAmmoCountUpdated OnAmmoCountUpdated;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnUpdateCursorPosition );
	UPROPERTY( BlueprintAssignable, Category = "Reload|Events" )
	FOnUpdateCursorPosition OnUpdateCursorPosition;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnReloadStateChanged, EReloadState, ReloadType );
	UPROPERTY( BlueprintAssignable, Category = "Reload|Events" )
	FOnReloadStateChanged OnReloadStateChanged;

private:

	//UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|Enum", meta = ( AllowPrivateAccess = "true" ) )
	//EGunReloadState CurrentReloadState = EGunReloadState::NormalStart;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|Enum", meta = ( AllowPrivateAccess = "true" ) )
	EGunState CurrentGunState = EGunState::Idle;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|Data Asset", meta = ( AllowPrivateAccess = "true" ) )
	UReloadData* ReloadDataAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = ( AllowPrivateAccess = "true" ))
	UInventoryComponent* PlayerInventory = nullptr;

	UPROPERTY( meta = ( AllowPrivateAccess = "true" ) )
	bool bIsReloadActive = false;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Reload|Debug", meta = ( AllowPrivateAccess = "true" ) )
	bool bUseInfiniteAmmo = false;

private:

	UFUNCTION( BlueprintCallable )
	void StartReloadSequence();

	UFUNCTION( BlueprintCallable )
	void OnReloadInput();

	UFUNCTION( BlueprintCallable )
	void UpdateReloadGauge();

	UFUNCTION( BlueprintCallable )
	void TriggerNormalReload();

	UFUNCTION( BlueprintCallable )
	void TriggerActiveReload();

	UFUNCTION( BlueprintCallable )
	void TriggerPerfectReload();

	UFUNCTION( BlueprintCallable )
	void TriggerFailedReload();

	//UFUNCTION(BlueprintCallable)
	//void UpdateCurrentReloadState(EGunReloadState NewState);

	UFUNCTION(BlueprintCallable)
	void UpdateCurrentGunState( EGunState NewState );

private :
	
	void GetPlayerInventory();
	void GetHUDFromPlayerController();
	void GetGunReference();

private:	

	APlayerController* PlayerController = nullptr; 
	AActor* CharacterGun = nullptr;
	AHUD* HUD = nullptr;

	FTimerHandle TimerHandle;
	FTimerHandle TimerHandleReloadPlayback;
};
