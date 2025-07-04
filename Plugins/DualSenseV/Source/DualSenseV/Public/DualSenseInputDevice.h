#pragma once

#include "IInputDevice.h"

#include "Internal/Device.h"
#include "Internal/DS5State.h"
#include "DualSenseTypes.h"

#ifdef PLATFORM_WINDOWS
#include "Windows/WindowsApplication.h"
#endif

class IDualSenseInputMessageReceiver;

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
	double NextRepeatTime[MAX_DUALSENSE_BUTTONS] {};

	FPlatformUserId UserId = PLATFORMUSERID_NONE;
	FInputDeviceId DeviceId = INPUTDEVICEID_NONE;
};

class FDualSenseInputDevice :
	public IInputDevice
#ifdef PLATFORM_WINDOWS
	, public IWindowsMessageHandler
#endif
{
public:
	explicit FDualSenseInputDevice( const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler );
	virtual ~FDualSenseInputDevice() override;

public:
	bool AddInputMessageReceiver( IDualSenseInputMessageReceiver* MessageReceiver );
	void RemoveInputMessageReceiver( IDualSenseInputMessageReceiver* MessageReceiver );

	bool ReleaseController( int32 ControllerIndex, bool bShouldResetInputs = true );

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

#ifdef PLATFORM_WINDOWS
	// Begin IWindowsMessageHandler interface
	virtual bool ProcessMessage( HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam, int32& OutResult ) override;
	// End IWindowsMessageHandler interface
#endif

private:
	void SendLeftStickValues( const FDualSenseController& Controller, FVector2D AnalogValues );
	void SendRightStickValues( const FDualSenseController& Controller, FVector2D AnalogValues );
	void SendTriggerValues( const FDualSenseController& Controller, float LeftTriggerValue, float RightTriggerValue );

	bool QueryControllers();
	bool RequestControllersUpdate();

private:
	TArray<FDualSenseController> RegisteredControllers {};
	TArray<IDualSenseInputMessageReceiver*> MessageReceivers {};
	TSharedPtr<FGenericApplicationMessageHandler> MessageHandler {};

	// NOTE: Set it to a value greater than 0.0f to update it first when the game runs in builds
	float UpdateControllersCooldown = 1.0f;

	float InitialButtonRepeatDelay = 0.2f;
	float ButtonRepeatDelay = 0.1f;
};