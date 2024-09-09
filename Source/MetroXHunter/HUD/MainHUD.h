#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MainHUD.generated.h"

class UMXHCheatManager;
class APlayerController;
enum class E_InteractionType : uint8;

UINTERFACE( Blueprintable )
class UMainHUD : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class METROXHUNTER_API IMainHUD
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "HUD|Cheat Menu" )
	bool ShowHideCheatMenuVisibility( UMXHCheatManager* CheatManager, APlayerController* PlayerController, bool bNewVisibility, bool bShouldToggle );

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "HUD|Interaction|Prompts" )
	void UpdatePrompts( E_InteractionType InteractionType );

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "HUD" )
	void ShowNewTextIndication( const FText& TextToShow );

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "HUD" )
	void UpdateAmmoCount();

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "HUD" )
	void SetUIMode(bool bShouldHideUI);

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "HUD|Interaction|Locker" )
	void ShowLockerSkillCheck( bool bNewLockerVisibility );

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "HUD|Interaction|Locker" )
	void GiveInputToLockerSkillCheck( FVector2D MovementsInput );

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "HUD|Interaction|Locker" )
	bool PressingLockerNow();
};
