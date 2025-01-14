/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MetroPlayerCharacter.generated.h"

class UCineCameraComponent;
class USpringArmComponent;
class UReloadComponent;
class UInventoryComponent;
class AGun;

UCLASS()
class METROXHUNTER_API AMetroPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMetroPlayerCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION( BlueprintImplementableEvent, Category = "PlayerCharacter|SpringArm")
	void UpdateTargetArmLength( float TargetArmLength );

	UFUNCTION( BlueprintImplementableEvent, Category = "PlayerCharacter|Camera" )
	void UpdateCameraPosition();
	UFUNCTION( BlueprintImplementableEvent, Category = "PlayerCharacter|Camera" )
	void ResetCameraPosition();
	UFUNCTION( BlueprintImplementableEvent, Category = "PlayerCharacter|Camera" )
	void UpdateCameraFocal();
	UFUNCTION( BlueprintImplementableEvent, Category = "PlayerCharacter|Camera" )
	void ResetCameraFocal();
	UFUNCTION( BlueprintImplementableEvent, Category = "PlayerCharacter|Camera" )
	void UpdateCameraFocusPoint();
	UFUNCTION( BlueprintImplementableEvent, Category = "PlayerCharacter|Camera" )
	void ResetCameraFocusPoint();
	UFUNCTION( BlueprintImplementableEvent, Category = "PlayerCharacter|Camera" )
	void CameraShakeFeedback();

	UFUNCTION( Blueprintcallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Weapon" )
	void EquipWeapon(bool bShouldEquip);

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Aim" )
	void StartAimAssist();
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Aim" )
	void StopAimAssist();
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Aim" )
	FVector GetAdjustedImpactPoint();

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnHealFailed );
	UPROPERTY( BlueprintAssignable, BlueprintCallable, Category = "PlayerCharacter|Heal" )
	FOnHealFailed OnHealFailed;

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Camera" )
	USpringArmComponent* SpringArmComponent = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Camera" )
	USceneComponent* PivotCamera = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Camera" )
	UCineCameraComponent* CineCameraComponent = nullptr;

	UPROPERTY( BlueprintReadOnly, Category = "CharacterController|Movement" )
	FVector2D InputDirection {};
	UPROPERTY( BlueprintReadOnly, Category = "CharacterController|Rotation" )
	FVector2D LookDirection {};

	UPROPERTY( BlueprintReadWrite, Category = "PlayerCharacter|Weapon" )
	bool bIsWeaponEquipped = false;

	UPROPERTY( BlueprintReadOnly, Category = "PlayerCharacter|Aim" )
	bool bIsAiming = false;

	UPROPERTY(BlueprintReadWrite, Category = "PlayerCharacter|Aim" )
	float ShootingImprecisionValue = 1.0f;

	UPROPERTY( BlueprintReadWrite, Category = "PlayerCharacter|Reference" )
	AGun* Springfield = nullptr;

protected:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Reload" )
	UReloadComponent* ReloadComponent = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Inventory" )
	UInventoryComponent* InventoryComponent = nullptr;
};
