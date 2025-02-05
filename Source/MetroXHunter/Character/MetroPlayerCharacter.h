/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "BaseMetroPlayerCharacter.h"
#include "MetroPlayerCharacter.generated.h"

class UCineCameraComponent;
class USpringArmComponent;
class UReloadComponent;
class UInventoryComponent;
class UInteractionComponent;
class UHealthComponent;
class USaveLoadComponent;
class UElectrocutableComponent;
class UAITargetComponent;
class UQuickTimeEventComponent;
class AGun;
class UDualSenseControllerComponent;
class UCameraAnimationSequence;
struct FCameraAnimationParams;

UCLASS()
class METROXHUNTER_API AMetroPlayerCharacter : public ABaseMetroPlayerCharacter
{
	GENERATED_BODY()

public:
	AMetroPlayerCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void StompKick();

	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable, Category = "PlayerCharacter|SpringArm")
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

	UFUNCTION( Blueprintcallable, BlueprintImplementableEvent, Category = "PlayerCharacter|CrossHair" )
	void UpdateCrossHairOnMovement( float MovementSpeed );
	UFUNCTION( Blueprintcallable, BlueprintImplementableEvent, Category = "PlayerCharacter|CrossHair" )
	void UpdateCrossHairOnLightning( float MovementSpeed );
	UFUNCTION( Blueprintcallable, BlueprintImplementableEvent, Category = "PlayerCharacter|CrossHair" )
	void UpdateCrossHairStopLightning( float MovementSpeed );

	UFUNCTION( Blueprintcallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Ability" )
	void UseSyringe();
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Ability" )
	void OnStompKick(float InitialVelocity, FVector Direction);

	UFUNCTION( BlueprintCallable, Category = "PlayerCharacter|Aim" )
	void StartAiming();
	virtual void StopAiming_Implementation() override;
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Aim" )
	void StartAimAssist();
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Aim" )
	void StopAimAssist();
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Aim" )
	FVector GetAdjustedImpactPoint();

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Camera" )
	USpringArmComponent* SpringArmComponent = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Camera" )
	USceneComponent* PivotCamera = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Camera" )
	UCineCameraComponent* CineCameraComponent = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|DualSens" )
	UDualSenseControllerComponent* DualSenseComponent;

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
	
	UPROPERTY( BlueprintReadWrite, Category = "PlayerCharacter|MovementData" )
	float RunMaxSpeed = 480.0f;

	UPROPERTY( BlueprintReadWrite, Category = "PlayerCharacter|Reference" )
	AActor* Bobine = nullptr;

protected:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Reload" )
	UReloadComponent* ReloadComponent = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Inventory" )
	UInventoryComponent* InventoryComponent = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Interact" )
	UInteractionComponent* InteractionComponent = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Save" )
	USaveLoadComponent* SaveLoadComponent = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Elec" )
	UElectrocutableComponent* ElectrocutableComponent = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Health" )
	UHealthComponent* HealthComponent = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|AI" )
	UAITargetComponent* AITargetComponent;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|QTE" )
	UQuickTimeEventComponent* QTEComponent;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|ForceFeedback" )
	UForceFeedbackEffect* KickForceFeedback;
};
