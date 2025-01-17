/*
 * Implemented by Corentin Paya and Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerInputHandler.h"
#include "Debug/TickDebugger.h"
#include "MetroPlayerController.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class AMetroPlayerCharacter;
class UGunControllerComponent;
class UCharacterControllerComponent;

/*
 * Player Controller of the unique MetroXHunter game
 */
UCLASS()
class METROXHUNTER_API AMetroPlayerController :
	public APlayerController, public IPlayerInputHandler, public ITickDebugger
{
	GENERATED_BODY()

public:
	void BeginPlay() override;
	void LateBeginPlay();

	// Begin IPlayerInputHandler interface
	void SetInputMappingContext_Implementation( UInputMappingContext* NewMappingContext ) override;
	void ResetInputMappingContext_Implementation() override;
	void RevertInputMappingContext_Implementation( UInputMappingContext* MappingContext ) override;
	// End IPlayerInputHandler interface

	// Start ITickDebugger interface
	void TickDebug_Implementation( float DeltaTime, FString& OutDebugText ) override;
	// End ITickDebugger interface
	
public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerController|Inputs" )
	TArray<TSoftObjectPtr<UInputMappingContext>> DefaultMappingContexts {};

	UEnhancedInputLocalPlayerSubsystem* InputSystem;

protected:
	AMetroPlayerCharacter* PlayerCharacter = nullptr;
	UGunControllerComponent* GunController = nullptr;
	UCharacterControllerComponent* CharacterController = nullptr;

	UInputMappingContext* LastOverriddenMappingContext = nullptr;
	bool bAreDefaultMappingContextsActive = false;
};
