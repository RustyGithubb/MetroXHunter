/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractableWidget.generated.h"

/**
 *	The widget of the interactables
 */
UCLASS()
class METROXHUNTER_API UInteractableWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnPickupTargeted( int Amount );

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnObjectTargeted();

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnUntargeted();

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void EditSprite( UTexture2D* Sprite);
};
