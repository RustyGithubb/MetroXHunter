/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "BaseMetroPlayerCharacter.h"
#include "CommonInputTypeEnum.h"
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
class ABaseGun;
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

	void FellOutOfWorld( const UDamageType& DamageType ) override;

	void StompKick();

	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable, Category = "PlayerCharacter|SpringArm")
	void UpdateTargetArmLength( float TargetArmLength );
	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable, Category = "PlayerCharacter|SpringArm" )
	void UpdateTargetArmOffset( FVector TargetArmOffset );

	UFUNCTION( BlueprintImplementableEvent, Category = "PlayerCharacter|Camera" )
	void UpdateCameraPosition();
	UFUNCTION( BlueprintImplementableEvent, Category = "PlayerCharacter|Camera" )
	void ResetCameraPosition();
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Camera" )
	void UpdateCameraFocal();
	UFUNCTION( BlueprintImplementableEvent, Category = "PlayerCharacter|Camera" )
	void ResetCameraFocal();
	UFUNCTION( BlueprintImplementableEvent, Category = "PlayerCharacter|Camera" )
	void UpdateCameraFocusPoint();
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Camera" )
	void ResetCameraFocusPoint();
	UFUNCTION( BlueprintImplementableEvent, Category = "PlayerCharacter|Camera" )
	void CameraShakeFeedback();

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Camera" )
	void PlayMovementCameraAnimation( UCameraAnimationSequence* CameraAnimation);

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Camera" )
	void StopMovementCameraAnimation();

	UFUNCTION( Blueprintcallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Ability" )
	void UseSyringe();
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Ability" )
	void OnMeleeAttack(float InitialVelocity, float DotProduct);
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Ability" )
	void OnCrossKick( float InitialVelocity, FVector Direction );

	virtual void StopAiming_Implementation() override;
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Aim" )
	void StartAimAssist();
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Aim" )
	void StopAimAssist();
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Aim" )
	FVector GetAdjustedImpactPoint();

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnInputTypeUpdate, ECommonInputType, InputType );
	UPROPERTY( BlueprintCallable, BlueprintAssignable, Category = "PlayerCharacter|Event|Input" )
	FOnInputTypeUpdate OnInputTypeUpdate;

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Camera" )
	USpringArmComponent* SpringArmComponent = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Camera" )
	USceneComponent* PivotCamera = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Camera" )
	UCineCameraComponent* CineCameraComponent = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|DualSens" )
	UDualSenseControllerComponent* DualSenseComponent;

	UPROPERTY( BlueprintReadOnly, Category = "PlayerCharacter|Input" )
	ECommonInputType CurrentInputType = ECommonInputType::Gamepad;

	UPROPERTY( BlueprintReadWrite, Category = "CharacterController|Movement" )
	FVector2D InputDirection {};
	UPROPERTY( BlueprintReadOnly, Category = "CharacterController|Rotation" )
	FVector2D LookDirection {};

	UPROPERTY( BlueprintReadWrite, Category = "PlayerCharacter|Weapon" )
	bool bIsWeaponEquipped = false;

	UPROPERTY( BlueprintReadWrite, Category = "PlayerCharacter|Aim" )
	bool bIsAiming = false;
	/* This condition let know the Animation blueprint if it should start playing the Aim Animation */
	UPROPERTY( BlueprintReadWrite, Category = "PlayerCharacter|Aim" )
	bool bShouldStartAiming = false;
	UPROPERTY( BlueprintReadWrite, Category = "PlayerCharacter|Aim" )
	float ShootingImprecisionValue = 1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "PlayerCharacter|ForceFeedback" )
	bool bCanAim = true;

	UPROPERTY( BlueprintReadWrite, Category = "PlayerCharacter|MovementData" )
	float RunMaxSpeed = 480.0f;

	UPROPERTY( BlueprintReadWrite, Category = "PlayerCharacter|Reference" )
	AActor* Bobine = nullptr;

	UPROPERTY( BlueprintReadWrite, Category = "PlayerCharacter|CameraRotation" )
	float AimDecelerationRate = 1.0f;

	UPROPERTY( BlueprintReadWrite, Category = "PlayerCharacter|Input" )
	float TimeSinceLastAction = 0.3f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Input" )
	float CancelActionTimer = 0.3f;

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
