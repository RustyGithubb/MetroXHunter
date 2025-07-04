#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "DualSenseTypes.h"
#include "IDualSenseInputMessageReceiver.h"
#include "DualSenseControllerComponent.generated.h"

class FDualSenseInputDevice;

/**
 *
 */
UCLASS( ClassGroup = ( "Input" ), meta = ( BlueprintSpawnableComponent ) )
class DUALSENSEV_API UDualSenseControllerComponent :
	public UActorComponent, public IDualSenseInputMessageReceiver
{
	GENERATED_BODY()

public:
	UDualSenseControllerComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay( EEndPlayReason::Type Reason ) override;

public:
	UFUNCTION( BlueprintCallable, Category = "DualSense" )
	bool InitializeController( int32 ControllerIndex = 0 );
	/*
	 * Disconnect from the controlled gamepad.
	 */
	UFUNCTION( BlueprintCallable, Category = "DualSense" )
	bool ReleaseController();

	UFUNCTION( BlueprintCallable, Category = "DualSense" )
	bool IsControllerInitialized() const;

	/*
	 * Apply a rumble on the left and hard motor of the controller.
	 * 
	 * @param Scale		Rumble scale from 0.0 to 1.0
	 */
	UFUNCTION( BlueprintCallable, Category = "DualSense" )
	bool SetLeftRumble( float Scale = 0.5f );
	/*
	 * Apply a rumble on the right and soft motor of the controller.
	 * 
	 * @param Scale		Rumble scale from 0.0 to 1.0
	 */
	UFUNCTION( BlueprintCallable, Category = "DualSense" )
	bool SetRightRumble( float Scale = 0.5f );

	UFUNCTION( BlueprintCallable, Category = "DualSense" )
	bool SetLeftTriggerEffect( const FDS5TriggerEffect& TriggerEffect );
	UFUNCTION( BlueprintCallable, Category = "DualSense" )
	bool SetRightTriggerEffect( const FDS5TriggerEffect& TriggerEffect );

	UFUNCTION( BlueprintCallable, Category = "DualSense" )
	bool SetMicrophoneLED( EDS5MicrophoneLED MicrophoneLED );
	UFUNCTION( BlueprintCallable, Category = "DualSense" )
	bool SetPlayerLEDs( const FDS5PlayerLEDs& PlayerLEDs );

	UFUNCTION( BlueprintCallable, Category = "DualSense" )
	bool SetLightbarColor( const FLinearColor NewColor );
	UFUNCTION( BlueprintCallable, Category = "DualSense" )
	bool SetCustomLightbarColorEnabled( bool bEnabled = true );

	UFUNCTION( BlueprintCallable, Category = "DualSense" )
	bool SetControllerOutput( const FDS5OutputState& OutputState );
	/*
	 * Get the current output state of the controller.
	 * This transpose the internal data to a usable data type.
	 * 
	 * TODO: Finish writing this function.
	 */
	UFUNCTION( BlueprintPure, Category = "DualSense" )
	void GetControllerOutput( FDS5OutputState& OutputState ) const;

	/*
	 * Get the raw value of the left analog stick.
	 * 
	 * The length of the returned vector is in range of 0.0 : 128.0.
	 * However, it can go greater than the maximum value.
	 * 
	 * Use GetControllerLeftStickUserValue for a game-ready input.
	 */
	UFUNCTION( BlueprintPure, Category = "DualSense" )
	FVector2D GetControllerLeftStickRawValue() const;
	/*
	 * Get the raw value of the right analog stick.
	 * 
	 * The length of the returned vector is in range of 0.0 : 128.0.
	 * However, it can go greater than the maximum value.
	 * 
	 * Use GetControllerRightStickUserValue for a game-ready input.
	 */
	UFUNCTION( BlueprintPure, Category = "DualSense" )
	FVector2D GetControllerRightStickRawValue() const;

	/*
	 * Get the value of the left analog stick with a deadzone applied.
	 * 
	 * The length of the returned vector is in range of 0.0 : 1.0.
	 * In contrary to GetControllerLeftStickRawValue, the length can not exceed 1.0.
	 * 
	 * The amplitude that the user gives is kept.
	 */
	UFUNCTION( BlueprintPure, Category = "DualSense" )
	FVector2D GetControllerLeftStickUserValue( float Deadzone = 0.1f ) const;
	/*
	 * Get the value of the right analog stick with a deadzone applied.
	 * 
	 * The length of the returned vector is in range of 0.0 : 1.0.
	 * In contrary to GetControllerRightStickRawValue, the length can not exceed 1.0.
	 * 
	 * The amplitude that the user gives is kept.
	 */
	UFUNCTION( BlueprintPure, Category = "DualSense" )
	FVector2D GetControllerRightStickUserValue( float Deadzone = 0.1f ) const;

	UFUNCTION( BlueprintPure, Category = "DualSense" )
	float GetControllerLeftTriggerRawValue() const;
	UFUNCTION( BlueprintPure, Category = "DualSense" )
	float GetControllerRightTriggerRawValue() const;

	UFUNCTION( BlueprintPure, Category = "DualSense" )
	float GetControllerLeftTriggerUserValue() const;
	UFUNCTION( BlueprintPure, Category = "DualSense" )
	float GetControllerRightTriggerUserValue() const;

	/*
	 * Get the raw accelerometer vector of the controller.
	 * Important: The returned value is very unstable.
	 */
	UFUNCTION( BlueprintPure, Category = "DualSense" )
	FVector GetControllerAccelerometer() const;
	/*
	 * Get the raw gyroscope vector of the controller.
	 * Important: The returned value is very unstable.
	 */
	UFUNCTION( BlueprintPure, Category = "DualSense" )
	FVector GetControllerOrientation() const;
	/*
	 * Get a specific touch data of the controller.
	 * Two fingers must hover the touch button to make it works.
	 */
	UFUNCTION( BlueprintPure, Category = "DualSense" )
	FDS5Touch GetControllerTouch( EDS5TouchIndex TouchIndex ) const;
	/*
	 * Get the battery of the controller.
	 * Important: The battery level may not be correctly reflected.
	 */
	UFUNCTION( BlueprintPure, Category = "DualSense" )
	FDS5Battery GetControllerBattery() const;

	/*
	 * Returns whenever the given button is hold down.
	 */
	UFUNCTION( BlueprintPure, Category = "DualSense" )
	bool IsControllerButtonDown( EDS5Button Button ) const;

public:
	// Begin IDualSenseInputMessageReceiver interface
	virtual void OnDualSenseConnected() override;
	virtual void OnDualSenseDisconnected() override;
	virtual int32 GetDualSenseControllerIndex() override;
	// End IDualSenseInputMessageReceiver interface

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FDualSenseUpdate, UDualSenseControllerComponent*, ControllerComponent );

	UPROPERTY( BlueprintAssignable, Category = "DualSense" )
	FDualSenseUpdate OnDualSenseConnect {};
	UPROPERTY( BlueprintAssignable, Category = "DualSense" )
	FDualSenseUpdate OnDualSenseDisconnect {};

public:
	/*
	 * Controller index that is currently controlled
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "DualSense", meta = ( ClampMin = "0", ClampMax = "16" ) )
	int32 ControllerIndex = 0;

	/*
	 * Whenever the component should automatically initialize the DualSense controller
	 * to the given controller index during BeginPlay.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "DualSense" )
	bool bAutoInitializeController = true;

private:
	bool UpdateOutputState();

private:
	bool bIsControllerBound = false;

	TSharedPtr<FDualSenseInputDevice> InputDevice = nullptr;

	UPROPERTY()
	ULocalPlayer* LocalPlayer = nullptr;
};
