#include "DualSenseControllerComponent.h"

#include "Device.h"
#include "DS5_Input.h"
#include "DSW_Api.h"
#include "Helpers.h"
#include "IO.h"

#include "DualSensePlugin.h"
#include "Engine/Engine.h"

static void TransposeRumble( const float UserScale, uint8& InternalRumble )
{
	InternalRumble = static_cast<uint8>( UserScale * 255.0f );
}

static void TransposeColor( const FLinearColor& UserColor, DS5W::Color& InternalColor )
{
	InternalColor = DS5W::color_R8G8B8_UCHAR_A32_FLOAT(
		UserColor.R * 255,
		UserColor.G * 255,
		UserColor.B * 255,
		UserColor.A
	);
}

static void TransposeMicrophoneLED(
	const EDS5MicrophoneLED UserMicrophoneLED,
	DS5W::MicLed& InternalMicrophoneLED
)
{
	InternalMicrophoneLED = static_cast<DS5W::MicLed>( UserMicrophoneLED );
}

static void TransposePlayerLEDs(
	const FDS5PlayerLEDs& UserPlayerLEDs,
	DS5W::PlayerLeds& InternalPlayerLEDs
)
{
	InternalPlayerLEDs.bitmask = static_cast<uint8>( UserPlayerLEDs.BitMask );
	InternalPlayerLEDs.playerLedFade = UserPlayerLEDs.bShouldFade;
	InternalPlayerLEDs.brightness = static_cast<DS5W::LedBrightness>( UserPlayerLEDs.Brightness );
}

static void TransposeTriggerEffect(
	const FDS5TriggerEffect& UserTriggerEffect,
	DS5W::TriggerEffect& InternalTriggerEffect
)
{
	FMemory::Memcpy(
		&InternalTriggerEffect,
		UserTriggerEffect.Data,
		sizeof( DS5W::TriggerEffect )
	);
}


UDualSenseControllerComponent::UDualSenseControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDualSenseControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Retrieve plugin's input device which manages the controller
	InputDevice = FDualSensePlugin::GetChecked().GetInputDevice();
	verifyf( InputDevice.IsValid(), TEXT( "DualSenseV: Plugin's InputDevice is not valid" ) );

	if ( bAutoInitializeController )
	{
		InitializeController( ControllerIndex );
	}
}

void UDualSenseControllerComponent::EndPlay( EEndPlayReason::Type Reason )
{
	if ( bAutoReleaseController )
	{
		ReleaseController();
	}
}

bool UDualSenseControllerComponent::InitializeController( int32 InControllerIndex )
{
	// Check if controller is already initialized
	if ( bIsControllerBound )
	{
		UE_LOG(
			LogDualSensePlugin, Error,
			TEXT( "DualSenseControllerComponent: Failed to initialize controller %d: controller already binded" ),
			ControllerIndex
		);
		return false;
	}

	bIsControllerBound = false;

	// TODO: Sync with actual plugin's controllers limit
	constexpr int MAX_CONTROLLERS = 16;

	// Check if controller index is valid
	ControllerIndex = InControllerIndex;
	if ( ControllerIndex < 0 || ControllerIndex >= MAX_CONTROLLERS )
	{
		UE_LOG(
			LogDualSensePlugin, Error,
			TEXT( "DualSenseControllerComponent: Failed to initialize controller %d for %s: controller index must be in range of 0 to %d" ),
			ControllerIndex,
			*GetOwner()->GetName(),
			MAX_CONTROLLERS
		);
		return false;
	}

	// Check controller availability
	if ( !InputDevice->InitializeController( ControllerIndex ) )
	{
		UE_LOG(
			LogDualSensePlugin, Log,
			TEXT( "DualSenseControllerComponent: Failed to initialize controller %d for %s: controller not available" ),
			ControllerIndex,
			*GetOwner()->GetName()
		);
		return false;
	}

	bIsControllerBound = true;

	UE_LOG(
		LogDualSensePlugin, Log,
		TEXT( "DualSenseControllerComponent: Connected controller %d for %s" ),
		ControllerIndex,
		*GetOwner()->GetName()
	);

	return true;
}

bool UDualSenseControllerComponent::ReleaseController()
{
	if ( !bIsControllerBound ) return false;

	InputDevice->ReleaseController( ControllerIndex );
	bIsControllerBound = false;

	UE_LOG(
		LogDualSensePlugin, Log,
		TEXT( "DualSenseControllerComponent: Released controller %d for %s" ),
		ControllerIndex,
		*GetOwner()->GetName()
	);

	return true;
}

bool UDualSenseControllerComponent::IsControllerInitialized() const
{
	return bIsControllerBound;
}

bool UDualSenseControllerComponent::SetLeftRumble( const float Scale )
{
	FDualSenseController* Controller = InputDevice->GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return false;

	TransposeRumble( Scale, Controller->OutputState.leftRumble );

	return UpdateOutputState();
}

bool UDualSenseControllerComponent::SetRightRumble( const float Scale )
{
	FDualSenseController* Controller = InputDevice->GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return false;

	TransposeRumble( Scale, Controller->OutputState.rightRumble );

	return UpdateOutputState();
}

bool UDualSenseControllerComponent::SetLeftTriggerEffect( const FDS5TriggerEffect& TriggerEffect )
{
	FDualSenseController* Controller = InputDevice->GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return false;

	TransposeTriggerEffect( TriggerEffect, Controller->OutputState.leftTriggerEffect );

	return UpdateOutputState();
}

bool UDualSenseControllerComponent::SetRightTriggerEffect( const FDS5TriggerEffect& TriggerEffect )
{
	FDualSenseController* Controller = InputDevice->GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return false;

	TransposeTriggerEffect( TriggerEffect, Controller->OutputState.rightTriggerEffect );

	return UpdateOutputState();
}

bool UDualSenseControllerComponent::SetMicrophoneLED( EDS5MicrophoneLED MicrophoneLED )
{
	FDualSenseController* Controller = InputDevice->GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return false;

	TransposeMicrophoneLED( MicrophoneLED, Controller->OutputState.microphoneLed );

	return UpdateOutputState();
}

bool UDualSenseControllerComponent::SetPlayerLEDs( const FDS5PlayerLEDs& PlayerLEDs )
{
	FDualSenseController* Controller = InputDevice->GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return false;

	TransposePlayerLEDs( PlayerLEDs, Controller->OutputState.playerLeds );

	return UpdateOutputState();
}

bool UDualSenseControllerComponent::SetLightbarColor( const FLinearColor NewColor )
{
	FDualSenseController* Controller = InputDevice->GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return false;

	Controller->OutputState.lightbar = DS5W::color_R32G32B32A32_FLOAT(
		NewColor.R,
		NewColor.G,
		NewColor.B,
		NewColor.A
	);

	return UpdateOutputState();
}

bool UDualSenseControllerComponent::SetCustomLightbarColorEnabled( bool bEnabled )
{
	FDualSenseController* Controller = InputDevice->GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return false;

	Controller->OutputState.disableLeds = !bEnabled;

	return UpdateOutputState();
}

bool UDualSenseControllerComponent::SetControllerOutput( const FDS5OutputState& UserOutputState )
{
	FDualSenseController* Controller = InputDevice->GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return false;

	TransposeRumble( UserOutputState.LeftRumble, Controller->OutputState.leftRumble );
	TransposeRumble( UserOutputState.RightRumble, Controller->OutputState.rightRumble );

	TransposeTriggerEffect( UserOutputState.LeftTriggerEffect, Controller->OutputState.leftTriggerEffect );
	TransposeTriggerEffect( UserOutputState.RightTriggerEffect, Controller->OutputState.rightTriggerEffect );

	TransposeMicrophoneLED( UserOutputState.MicrophoneLED, Controller->OutputState.microphoneLed );
	TransposePlayerLEDs( UserOutputState.PlayerLEDs, Controller->OutputState.playerLeds );

	TransposeColor( UserOutputState.LightbarColor, Controller->OutputState.lightbar );
	Controller->OutputState.disableLeds = !UserOutputState.bIsLightbarEnabled;

	return UpdateOutputState();
}

void UDualSenseControllerComponent::GetControllerOutput( FDS5OutputState& UserOutputState ) const
{
	FDualSenseController* Controller = InputDevice->GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return;

	UserOutputState.LeftRumble = Controller->OutputState.leftRumble * 128.0f;
	UserOutputState.RightRumble = Controller->OutputState.rightRumble * 128.0f;

	UserOutputState.LightbarColor = FLinearColor {
		FColor {
			Controller->OutputState.lightbar.r,
			Controller->OutputState.lightbar.g,
			Controller->OutputState.lightbar.b,
			255
		}
	};
	UserOutputState.bIsLightbarEnabled = !Controller->OutputState.disableLeds;

	UserOutputState.MicrophoneLED = static_cast<EDS5MicrophoneLED>( Controller->OutputState.microphoneLed );

	// TODO: Finish writing this function
	//UserOutputState.PlayerLEDs.BitMask = CurrentOutputState.playerLeds.bitmask;
	//UserOutputState.PlayerLEDs.BitMask = CurrentOutputState.playerLeds.bitmask;
}

FVector2D UDualSenseControllerComponent::GetControllerLeftStickRawValue() const
{
	return InputDevice->GetControllerLeftStickRawValue( ControllerIndex );
}

FVector2D UDualSenseControllerComponent::GetControllerRightStickRawValue() const
{
	return InputDevice->GetControllerRightStickRawValue( ControllerIndex );
}

FVector2D UDualSenseControllerComponent::GetControllerLeftStickUserValue( const float Deadzone ) const
{
	return InputDevice->GetControllerLeftStickUserValue( ControllerIndex, Deadzone );
}

FVector2D UDualSenseControllerComponent::GetControllerRightStickUserValue( const float Deadzone ) const
{
	return InputDevice->GetControllerRightStickUserValue( ControllerIndex, Deadzone );
}

float UDualSenseControllerComponent::GetControllerLeftTriggerRawValue() const
{
	return InputDevice->GetControllerLeftTriggerRawValue( ControllerIndex );
}

float UDualSenseControllerComponent::GetControllerRightTriggerRawValue() const
{
	return InputDevice->GetControllerRightTriggerRawValue( ControllerIndex );
}

float UDualSenseControllerComponent::GetControllerLeftTriggerUserValue() const
{
	return InputDevice->GetControllerLeftTriggerUserValue( ControllerIndex );
}

float UDualSenseControllerComponent::GetControllerRightTriggerUserValue() const
{
	return InputDevice->GetControllerRightTriggerUserValue( ControllerIndex );
}

FVector UDualSenseControllerComponent::GetControllerAccelerometer() const
{
	const FDualSenseController* Controller = InputDevice->GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return FVector::ZeroVector;

	return FVector {
		static_cast<double>( Controller->InputState.accelerometer.x ),
		static_cast<double>( Controller->InputState.accelerometer.y ),
		static_cast<double>( Controller->InputState.accelerometer.z ),
	};
}

FVector UDualSenseControllerComponent::GetControllerOrientation() const
{
	const FDualSenseController* Controller = InputDevice->GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return FVector::ZeroVector;

	return FVector {
		static_cast<double>( Controller->InputState.gyroscope.x ),
		static_cast<double>( Controller->InputState.gyroscope.y ),
		static_cast<double>( Controller->InputState.gyroscope.z ),
	};
}

FDS5Touch UDualSenseControllerComponent::GetControllerTouch( const EDS5TouchIndex TouchIndex ) const
{
	FDS5Touch TouchData {};

	const FDualSenseController* Controller = InputDevice->GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return TouchData;

	const DS5W::Touch* InternalTouchData = nullptr;
	switch ( TouchIndex )
	{
		case EDS5TouchIndex::First:
			InternalTouchData = &Controller->InputState.touchPoint1;
			break;
		case EDS5TouchIndex::Second:
			InternalTouchData = &Controller->InputState.touchPoint2;
			break;
	} 

	if ( InternalTouchData == nullptr ) return TouchData;

	TouchData.ID = InternalTouchData->id;
	TouchData.bIsDown = InternalTouchData->down;
	TouchData.Position.X = InternalTouchData->x;
	TouchData.Position.Y = InternalTouchData->y;

	return TouchData;
}

FDS5Battery UDualSenseControllerComponent::GetControllerBattery() const
{
	FDS5Battery Battery {};

	const FDualSenseController* Controller = InputDevice->GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return Battery;

	Battery.bIsCharging = Controller->InputState.battery.chargin;
	Battery.bIsFullCharged = Controller->InputState.battery.fullyCharged;
	Battery.Level = Controller->InputState.battery.level / 255.0f;

	return Battery;
}

bool UDualSenseControllerComponent::IsControllerButtonDown( const EDS5Button Button ) const
{
	return InputDevice->IsControllerButtonDown( ControllerIndex, Button );
}

bool UDualSenseControllerComponent::UpdateOutputState()
{
	if ( !bIsControllerBound )
	{
		UE_LOG(
			LogDualSensePlugin, Error,
			TEXT( "DualSenseControllerComponent: Failed to update controller %d's output state for %s: not initialized" ),
			ControllerIndex,
			*GetOwner()->GetName()
		);
		return false;
	}

	return InputDevice->UpdateControllerOutputState( ControllerIndex );
}
