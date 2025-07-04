#pragma once

#include "Engine/DeveloperSettings.h"
#include "DualSenseSettings.generated.h"

UCLASS( Config = Input, DefaultConfig, meta = ( DisplayName = "Dual Sense V" ) )
class DUALSENSEV_API UDualSenseSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Begin UDeveloperSettings Interface
	virtual FName GetCategoryName() const override;
	// End UDeveloperSettings Interface

	/**
	 * Return settings of the DualSenseV plugin.
	 */
	UFUNCTION( BlueprintPure, meta = ( DisplayName = "Get DualSenseSettings", CompactNodeTitle = "Dual Sense Settings" ) )
	static const UDualSenseSettings* Get();

public:
	/**
	 * Whether the plugin should also send DualSense inputs as native gamepad inputs.
	 * 
	 * If set to false, you will have to bind the specific DualSense inputs to your
	 * InputMappingContexts yourself.
	 */
	UPROPERTY( Config, EditAnywhere, BlueprintReadOnly, Category = "DualSense" )
	bool bEmulateGamepadInputs = true;

	/**
	 * The name of the gamepad input type to set while a DualSense gamepad is active.
	 * 
	 * It allows CommonUI to properly switch the input icons with specific PlayStation
	 * icons.
	 */
	UPROPERTY( Config, EditAnywhere, BlueprintReadOnly, Category = "DualSense|CommonUI", meta = ( DisplayName = "Gamepad Input Type" ) )
	FName CommonUIGamepadInputType = TEXT( "PlayStation5" );
};