#include "DualSenseControllerComponent.h"

#include "Internal/Helpers.h"

#include "DualSensePlugin.h"
#include "DualSenseSettings.h"
#include "DualSenseInputDevice.h"

#include "Engine/Engine.h"
#include "CommonInputSubsystem.h"

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

	// NOTE: Wait for the next tick because in builds, the InputDevice is somehow not created before
	// the BeginPlay during builds...
	GetWorld()->GetTimerManager().SetTimerForNextTick(
		[&]()
		{
			// Retrieve plugin's input device which manages the controller
			InputDevice = FDualSensePlugin::GetChecked().GetInputDevice();
			verifyf( InputDevice.IsValid(), TEXT( "DualSenseV: Plugin's InputDevice is not valid" ) );

			if ( bAutoInitializeController )
			{
				InitializeController( ControllerIndex );
			}
		}
	);
}

void UDualSenseControllerComponent::EndPlay( EEndPlayReason::Type Reason )
{
	Super::EndPlay( Reason );

	ReleaseController();
}

bool UDualSenseControllerComponent::InitializeController( int32 InControllerIndex )
{
	// Check if controller is already initialized
	if ( bIsControllerBound )
	{
		UE_LOG(
			LogDualSensePlugin, Error,
			TEXT( "DualSenseControllerComponent: Failed to initialize controller %d: controller already bound" ),
			ControllerIndex
		);
		return false;
	}

	bIsControllerBound = false;

	// Bind to input device
	if ( !InputDevice->AddInputMessageReceiver( this ) )
	{
		UE_LOG(
			LogDualSensePlugin, Warning,
			TEXT( "DualSenseControllerComponent: Failed to connect controller %d of %s: already connected!" ),
			ControllerIndex,
			*GetOwner()->GetName()
		);
		return true;
	}

	bIsControllerBound = true;
	UE_LOG(
		LogDualSensePlugin, Log,
		TEXT( "DualSenseControllerComponent: Bound to controller %d of %s." ),
		ControllerIndex,
		*GetOwner()->GetName()
	);
	return true;
}

bool UDualSenseControllerComponent::ReleaseController()
{
	// TODO: Fix still receiving inputs when manually releasing
	InputDevice->RemoveInputMessageReceiver( this );
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

void UDualSenseControllerComponent::OnDualSenseConnected()
{
	// Set CommonUI gamepad input type to DualSense's
	if ( APawn* Pawn = GetOwner<APawn>() )
	{
		if ( APlayerController* PlayerController = Pawn->GetController<APlayerController>() )
		{
			LocalPlayer = PlayerController->GetLocalPlayer();

			if ( UCommonInputSubsystem* CommonInputSubsystem = LocalPlayer->GetSubsystem<UCommonInputSubsystem>() )
			{
				const UDualSenseSettings* Settings = UDualSenseSettings::Get();
				check( Settings );

				CommonInputSubsystem->SetGamepadInputType( Settings->CommonUIGamepadInputType );
			}
		}
	}

	bIsControllerBound = true;

	OnDualSenseConnect.Broadcast( this );

	UE_LOG(
		LogDualSensePlugin, Log,
		TEXT( "DualSenseControllerComponent: Controller %d of %s has been connected!" ),
		ControllerIndex,
		*GetOwner()->GetName()
	);
}

void UDualSenseControllerComponent::OnDualSenseDisconnected()
{
	// Reset CommonUI gamepad input type to default
	if ( IsValid( LocalPlayer ) )
	{
		const UCommonInputPlatformSettings* Settings = UPlatformSettingsManager::Get().GetSettingsForPlatform<UCommonInputPlatformSettings>();
		UCommonInputSubsystem* CommonInputSubsystem = LocalPlayer->GetSubsystem<UCommonInputSubsystem>();
		if ( Settings != nullptr && CommonInputSubsystem != nullptr )
		{
			CommonInputSubsystem->SetGamepadInputType( Settings->GetDefaultGamepadName() );
		}
	}

	bIsControllerBound = false;

	OnDualSenseDisconnect.Broadcast( this );

	UE_LOG(
		LogDualSensePlugin, Log,
		TEXT( "DualSenseControllerComponent: Controller %d of %s has been disconnected!" ),
		ControllerIndex,
		*GetOwner()->GetName()
	);
}

int32 UDualSenseControllerComponent::GetDualSenseControllerIndex()
{
	return ControllerIndex;
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
