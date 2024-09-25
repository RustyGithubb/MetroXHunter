/*
 * Implemented by Corentin Paya and Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerInputHandler.h"
#include "MetroPlayerController.generated.h"

class UEnhancedInputLocalPlayerSubsystem;

/*
 * Player Controller of the unique MetroXHunter game
 */
UCLASS()
class METROXHUNTER_API AMetroPlayerController : public APlayerController, public IPlayerInputHandler
{
	GENERATED_BODY()

public:
	void BeginPlay() override;

	void SetInputMappingContext_Implementation( UInputMappingContext* NewMappingContext ) override;
	void ResetInputMappingContext_Implementation() override;

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerController|Inputs" )
	TSoftObjectPtr<UInputMappingContext> DefaultMappingContext = nullptr;

	UInputMappingContext* LastMappingContext = nullptr;

	UEnhancedInputLocalPlayerSubsystem* InputSystem;
};
