/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterControllerComponent.generated.h"

class UInputAction;
class AMetroPlayerCharacter;
class UPlayerMovementData;
struct FInputActionValue;

UENUM( BlueprintType )
enum class EMovementState : uint8
{
	Walk,
	Run,
};


UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UCharacterControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterControllerComponent();

	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	void SetupInputComponent( AMetroPlayerCharacter* InPlayer, TObjectPtr<class UInputComponent> InputComponent );

	UFUNCTION( BlueprintCallable, Category = "CharacterController|Movement" )
	void ToggleFreezeMovement( bool bShouldFreeze );
	UFUNCTION( BlueprintCallable, Category = "CharacterController|Movement" )
	void ToggleFreezeRotation( bool bShouldFreeze );
	UFUNCTION( BlueprintCallable, Category = "CharacterController|Movement" )
	void ToggleFreezeRun( bool bShouldFreeze );

	UFUNCTION( BlueprintCallable, Category = "CharacterController|Movement" )
	FVector2D GetInputDirection();

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "CharacterController|Movement" )
	UPlayerMovementData* PlayerMovementData = nullptr;
	UPROPERTY( BlueprintReadWrite, Category = "CharacterController|Movement" )
	float ScaleMovementSpeed = 1.0f;
	UPROPERTY( BlueprintReadWrite, Category = "CharacterController|Movement" )
	FVector2D MouseSensitivity { 1.0f };

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "CharacterController|Movement|Input" )
	UInputAction* MovementAction = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "CharacterController|Movement|Input" )
	UInputAction* RunAction = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "CharacterController|Movement|Input" )
	UInputAction* LookAction = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "CharacterController|Ability|Input" )
	UInputAction* AimAction = nullptr;

private:
	void SetupDefaultValues();

	void Move( const FInputActionValue& Value );
	void OnMoveInputReleased();
	void ToggleRun();
	void Look( const FInputActionValue& Value );
	void ToggleAim( const FInputActionValue& Value );

private:
	AMetroPlayerCharacter* Player = nullptr;

	EMovementState CurrentMovementState = EMovementState::Walk;

	bool bCanMove = true;
	bool bCanRotate = true;
	bool bCanRun = true;

	bool bWasRunning = false;
};
