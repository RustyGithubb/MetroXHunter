/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MainHUD.generated.h"

class UMetroCheatManager;
class APlayerController;
enum class E_InteractionType : uint8;

UINTERFACE( Blueprintable )
class UMainHUD : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface of the Main HUD to update the viewport.
 */
class METROXHUNTER_API IMainHUD
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "HUD|Cheat Menu" )
	bool ShowHideCheatMenuVisibility( UMetroCheatManager* CheatManager, APlayerController* PlayerController, bool bNewVisibility, bool bShouldToggle );

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "HUD|Interaction|Prompts" )
	void UpdatePrompts( E_InteractionType InteractionType );

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "HUD" )
	void ShowNewTextIndication( const FText& TextToShow );

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "HUD" )
	void SetUIMode( bool bShouldHideUI );
};
