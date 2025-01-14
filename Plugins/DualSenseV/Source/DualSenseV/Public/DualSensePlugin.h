#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "IInputDevice.h"
#include "IInputDeviceModule.h"

#include "Device.h"
#include "DS5State.h"
#include "DualSenseTypes.h"
#include "DualSensePlugin.generated.h"

DECLARE_LOG_CATEGORY_EXTERN( LogDualSensePlugin, Log, All );

constexpr int32 MAX_DUALSENSE_BUTTONS = static_cast<int32>( EDS5Button::MAX );

struct FDualSenseController
{
	DS5W::DeviceContext DeviceContext {};

	DS5W::DS5InputState InputState {};
	
	DS5W::DS5OutputState OutputState {};

	FForceFeedbackValues ForceFeedback {};

	float LastLargeValue = 0.0f;
	float LastSmallValue = 0.0f;

	bool bIsConnected = false;

	bool bWasPressed[MAX_DUALSENSE_BUTTONS] {};

	FPlatformUserId UserId = PLATFORMUSERID_NONE;
	FInputDeviceId DeviceId = INPUTDEVICEID_NONE;
};

class FDualSenseInputDevice : public IInputDevice
{
public:
	FDualSenseInputDevice( const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler );
	virtual ~FDualSenseInputDevice();

public:
	bool InitializeController( int32 ControllerIndex );
	bool ReleaseController( int32 ControllerIndex );

	bool UpdateControllerOutputState( int32 ControllerIndex );

	bool IsControllerAvailable( int32 ControllerIndex ) const;
	bool IsControllerButtonDown( int32 ControllerIndex, EDS5Button Button ) const;

	FDualSenseController* GetControllerData( int32 ControllerIndex );
	const FDualSenseController* GetControllerData( int32 ControllerIndex ) const;

	FVector2D GetControllerLeftStickRawValue( int32 ControllerIndex ) const;
	FVector2D GetControllerRightStickRawValue( int32 ControllerIndex ) const;

	FVector2D GetControllerLeftStickUserValue( int32 ControllerIndex, float Deadzone = 0.1f ) const;
	FVector2D GetControllerRightStickUserValue( int32 ControllerIndex, float Deadzone = 0.1f ) const;

	float GetControllerLeftTriggerRawValue( int32 ControllerIndex ) const;
	float GetControllerRightTriggerRawValue( int32 ControllerIndex ) const;

	float GetControllerLeftTriggerUserValue( int32 ControllerIndex ) const;
	float GetControllerRightTriggerUserValue( int32 ControllerIndex ) const;

public:
	// Begin IInputDevice interface
	/** Tick the interface (e.g. check for new controllers) */
	virtual void Tick( float DeltaTime ) override;
	/** Poll for controller state and send events if needed */
	virtual void SendControllerEvents() override;
	/** Set which MessageHandler will get the events from SendControllerEvents. */
	virtual void SetMessageHandler( const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler ) override;
	/** Exec handler to allow console commands to be passed through for debugging */
	virtual bool Exec( UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar ) override;

	/**
	 * Force Feedback pass through functions
	 */
	virtual void SetChannelValue( int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value ) override;
	virtual void SetChannelValues( int32 ControllerId, const FForceFeedbackValues& values ) override;
	virtual bool SupportsForceFeedback( int32 ControllerId ) override;

	/**
	 * Pass though functions for light color
	 */
	virtual void SetLightColor( int32 ControllerId, FColor Color ) override;
	virtual void ResetLightColor( int32 ControllerId ) override;

	/**
	* Sets a property for a given controller id.
	 * Will be ignored for devices which don't support the property.
	 *
	 * @param ControllerId the id of the controller whose property is to be applied
	 * @param Property Base class pointer to property that will be applied
	 */
	virtual void SetDeviceProperty( int32 ControllerId, const FInputDeviceProperty* Property ) override;

	/** If this device supports a haptic interface, implement this, and inherit the IHapticDevice interface */
	virtual class IHapticDevice* GetHapticDevice() override;

	virtual bool IsGamepadAttached() const override;
	// End IInputDevice interface

private:
	bool QueryControllers();

private:
	TArray<FDualSenseController> RegisteredControllers {};
	TSharedPtr<FGenericApplicationMessageHandler> MessageHandler {};
};

UCLASS( Config = Input, DefaultConfig, meta = ( DisplayName = "Dual Sense V" ) )
class DUALSENSEV_API UDualSenseSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Begin UDeveloperSettings Interface
	virtual FName GetCategoryName() const override;
	// End UDeveloperSettings Interface
	
	UFUNCTION( BlueprintCallable, meta = ( DisplayName = "GetDualSensePluginSettings" ) )
	static const UDualSenseSettings* Get();

public:
	/*
	 * Whether the plugin should also send the DualSense's inputs as native engine's
	 * gamepad inputs.
	 * 
	 * If set to false, you will have to bind the DualSense inputs to your InputMappingContexts
	 * yourself.
	 */
	UPROPERTY( Config, EditAnywhere, BlueprintReadOnly, Category = "Inputs" )
	bool bEmulateGamepadInputs = true;
};

class DUALSENSEV_API FDualSensePlugin : public IInputDeviceModule
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual TSharedPtr<IInputDevice> CreateInputDevice(
		const TSharedRef<FGenericApplicationMessageHandler>& MessageHandler
	) override;

	TSharedPtr<FDualSenseInputDevice> GetInputDevice() const;

	static inline FDualSensePlugin& GetChecked()
	{
		return FModuleManager::LoadModuleChecked<FDualSensePlugin>( "DualSenseV" );
	}

private:
	TSharedPtr<FDualSenseInputDevice> InputDevice = nullptr;
};
