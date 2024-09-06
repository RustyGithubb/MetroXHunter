#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MainHUD.generated.h"

class UMXHCheatManager;
class APlayerController;
enum class E_InteractionType : uint8;

UINTERFACE(MinimalAPI)
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
	bool ShowHideCheatMenuVisibility(UMXHCheatManager* CheatManager, APlayerController* PlayerController, bool bNewVisibility, bool bShouldToggle);

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "HUD|Interaction|Prompts" )
	void UpdatePrompts( E_InteractionType InteractionType);

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "HUD" )
	bool ShowNewTextIndication(const FText& TextToShow);

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "HUD" )
	bool UpdateAmmoCount();

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "HUD|Interaction|Locker" )
	bool ShowLockerSkillCheck(bool bNewLockerVisibility);

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "HUD|Interaction|Locker" )
	bool GiveInputToLockerSkillCheck(FVector2D MovementsInput);

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "HUD|Interaction|Locker" )
	bool PressingLockerNow();
};
