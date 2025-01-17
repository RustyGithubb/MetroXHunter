/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GunControllerComponent.generated.h"

class UInputAction;
class UEnhancedInputComponent;
class AGun;
class AMetroPlayerCharacter;
struct FInputActionValue;

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UGunControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunControllerComponent();

	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	void SetupInputComponent( AMetroPlayerCharacter* InPlayer, TObjectPtr<class UInputComponent> InputComponent );

	void OnShootActionPressed();
	void OnShootActionTriggered( const FInputActionValue& Value );
	void OnShootActionCompleted();

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "GunController|Input" )
	UInputAction* ShootAction = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "GunController|Input" )
	UInputAction* SwitchGunModeAction = nullptr;

	UPROPERTY( BlueprintReadOnly, Category = "GunController|Reference" )
	AMetroPlayerCharacter* Player = nullptr;
	UPROPERTY( BlueprintReadOnly, Category = "GunController|Reference" )
	AGun* Springfield = nullptr;
};
