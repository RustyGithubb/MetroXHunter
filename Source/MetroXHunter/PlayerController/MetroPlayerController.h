/*
 * Implemented by Corentin Paya and Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerInputHandler.h"
#include "MetroPlayerController.generated.h"

class UEnhancedInputLocalPlayerSubsystem;

/**
 * Player Controller of the unique Metro X Hunter game
 */
UCLASS()
class METROXHUNTER_API AMetroPlayerController : public APlayerController, public IPlayerInputHandler
{
	GENERATED_BODY()

public:
	void BeginPlay() override;

	void SetInputMappingContext_Implementation( UInputMappingContext* MappingContext );
	void ResetInputMappingContext_Implementation();

public:
	/*
	* Input mapping context of the player pontroller
	*/
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerController|Inputs" )
	TSoftObjectPtr<UInputMappingContext> DefaultMappingContext = nullptr;

	UEnhancedInputLocalPlayerSubsystem* InputSystem;
};
