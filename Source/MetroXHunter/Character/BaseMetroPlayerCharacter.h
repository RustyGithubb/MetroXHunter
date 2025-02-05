/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseMetroPlayerCharacter.generated.h"

class ABaseGun;

/*
 * Façade used to refactor the Main Chara
 */
UCLASS()
class METROXHUNTER_API ABaseMetroPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerCharacter" )
	AActor* GetBobine();

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Weapon" )
	void EquipWeapon( bool bShouldEquip );

	UFUNCTION( Blueprintcallable, BlueprintImplementableEvent, Category = "PlayerCharacter|Weapon" )
	void ToggleHiddenInGame( bool bShouldHide );

	UFUNCTION( Blueprintcallable, BlueprintNativeEvent, Category = "PlayerCharacter|Aim" )
	void StopAiming();
	virtual void StopAiming_Implementation() {};

public:
	UPROPERTY( BlueprintReadWrite, Category = "PlayerCharacter" )
	bool bIsUnderAction = false;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnPlayerMovementUpdate, float, MovementSpeed, float, Alpha );
	UPROPERTY( BlueprintCallable, BlueprintAssignable, Category = "PlayerCharacter|Event|CharacterMovement" )
	FOnPlayerMovementUpdate OnPlayerMovementUpdate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnWeaponActivated, bool, bIsActivated );
	UPROPERTY( BlueprintCallable, BlueprintAssignable, Category = "PlayerCharacter|Event|Weapon" )
	FOnWeaponActivated OnWeaponActivated;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnAim, bool, bIsAiming );
	UPROPERTY( BlueprintCallable, BlueprintAssignable, Category = "PlayerCharacter|Event|Aim" )
	FOnAim OnAim;

	UPROPERTY( BlueprintReadWrite, Category = "PlayerCharacter|Reference" )
	ABaseGun* BaseGun = nullptr;
};
