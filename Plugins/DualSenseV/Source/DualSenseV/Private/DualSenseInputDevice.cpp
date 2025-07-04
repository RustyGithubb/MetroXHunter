#include "DualSenseInputDevice.h"

#include "DualSensePlugin.h"
#include "DualSenseSettings.h"
#include "DualSenseFunctionLibrary.h"
#include "IDualSenseInputMessageReceiver.h"

#include "Internal/Helpers.h"
#include "Internal/IO.h"

#include "Internationalization/Regex.h"

#ifdef PLATFORM_WINDOWS
#include "Windows/WindowsApplication.h"
#include <windows.h>
#include <winuser.h>
#include <initguid.h>
#include <usbiodef.h>
#include <Dbt.h>
#endif

constexpr int MAX_CONTROLLERS = 4;

struct FDualSenseButtonDescriptor
{
	FDualSenseButtonDescriptor(
		EDS5Button NativeButton,
		const FKey& GamepadButtonKey,
		const FKey& NativeButtonKey
	)
		: NativeButton( NativeButton ),
		  GamepadButtonName( GamepadButtonKey.GetFName() ),
		  NativeButtonName( NativeButtonKey.GetFName() )
	{}

	EDS5Button NativeButton = EDS5Button::Square;
	FName GamepadButtonName {};
	FName NativeButtonName {};
};

static FDualSenseButtonDescriptor DualSenseButtonDescriptors[MAX_DUALSENSE_BUTTONS]
{
	{ EDS5Button::Square, EKeys::Gamepad_FaceButton_Left, FDualSenseKeys::Square },
	{ EDS5Button::Cross, EKeys::Gamepad_FaceButton_Bottom, FDualSenseKeys::Cross },
	{ EDS5Button::Circle, EKeys::Gamepad_FaceButton_Right, FDualSenseKeys::Circle },
	{ EDS5Button::Triangle, EKeys::Gamepad_FaceButton_Top, FDualSenseKeys::Triangle },

	{ EDS5Button::DPadUp, EKeys::Gamepad_DPad_Up, FDualSenseKeys::DPadUp },
	{ EDS5Button::DPadDown, EKeys::Gamepad_DPad_Down, FDualSenseKeys::DPadDown },
	{ EDS5Button::DPadLeft, EKeys::Gamepad_DPad_Left, FDualSenseKeys::DPadLeft },
	{ EDS5Button::DPadRight, EKeys::Gamepad_DPad_Right, FDualSenseKeys::DPadRight },

	{ EDS5Button::L1, EKeys::Gamepad_LeftShoulder, FDualSenseKeys::L1 },
	{ EDS5Button::L2, EKeys::Gamepad_LeftTrigger, FDualSenseKeys::L2 },
	{ EDS5Button::L3, EKeys::Gamepad_LeftThumbstick, FDualSenseKeys::L3 },
	{ EDS5Button::R1, EKeys::Gamepad_RightShoulder, FDualSenseKeys::R1 },
	{ EDS5Button::R2, EKeys::Gamepad_RightTrigger, FDualSenseKeys::R2 },
	{ EDS5Button::R3, EKeys::Gamepad_RightThumbstick, FDualSenseKeys::R3 },

	{ EDS5Button::Share, {}, FDualSenseKeys::Share },
	{ EDS5Button::Menu, EKeys::Gamepad_Special_Right, FDualSenseKeys::Menu },

	{ EDS5Button::PlayStation, {}, FDualSenseKeys::PlayStation },
	{ EDS5Button::Pad, EKeys::Gamepad_Special_Left, FDualSenseKeys::Pad },
	{ EDS5Button::Microphone, {}, FDualSenseKeys::Microphone },
};


FDualSenseInputDevice::FDualSenseInputDevice( const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler )
	: MessageHandler( MessageHandler )
{
	RegisteredControllers.Reserve( MAX_CONTROLLERS );

	GConfig->GetFloat( TEXT( "/Script/Engine.InputSettings" ), TEXT( "InitialButtonRepeatDelay" ), InitialButtonRepeatDelay, GInputIni );
	GConfig->GetFloat( TEXT( "/Script/Engine.InputSettings" ), TEXT( "ButtonRepeatDelay" ), ButtonRepeatDelay, GInputIni );

	UE_LOG(
		LogDualSensePlugin, Log,
		TEXT( "Retrieved values for InitialButtonRepeatDelay=%f and ButtonRepeatDelay=%f from config file." ),
		InitialButtonRepeatDelay, ButtonRepeatDelay
	);

#ifdef PLATFORM_WINDOWS
	if ( FSlateApplication::IsInitialized() )
	{
		// Since Unreal doesn't listen for device connection/disconnection events from Windows,
		// and we need those to know when to reconnect the gamepad, we have to do it ourselves.
		const GUID InterfaceClassGUID = {
			0x745A17A0,
			0x74D3,
			0x11D0,
			0xB6, 0xFE,
			0x00, 0xA0, 0xC9, 0x0F, 0x57, 0xDA
		};
		DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
		FMemory::Memzero( &NotificationFilter, sizeof( NotificationFilter ) );
		NotificationFilter.dbcc_size = sizeof( DEV_BROADCAST_DEVICEINTERFACE );
		NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
		NotificationFilter.dbcc_classguid = InterfaceClassGUID;
		FMemory::Memcpy( &NotificationFilter.dbcc_classguid, &GUID_DEVINTERFACE_USB_DEVICE, sizeof( GUID ) );

		const HWND WindowHandle = GetActiveWindow();
		const HDEVNOTIFY Notify = RegisterDeviceNotification( WindowHandle, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE );
		checkf( Notify != nullptr, TEXT( "DualSenseV: Windows: RegisterDeviceNotification failed." ) );

		// I personally don't like C-style cast for objects but looks like I don't have a choice
		// since using dynamic_cast makes my code crash and this seems to be the approved way by Unreal. 
		FWindowsApplication* WindowsApplication = (FWindowsApplication*)FSlateApplication::Get().GetPlatformApplication().Get();
		check( WindowsApplication );
		WindowsApplication->AddMessageHandler( *this );

		UE_LOG( LogDualSensePlugin, Log, TEXT( "Registered a new Windows notification filter to the engine." ) );
	}
	else
	{
		// TODO: Do not log this when packaging
		UE_LOG( LogDualSensePlugin, Warning, TEXT( "SlateApplication isn't initialized, auto-reconnection feature is not available." ) );
	}
#endif
}

FDualSenseInputDevice::~FDualSenseInputDevice()
{
	for ( int32 ControllerIndex = 0; ControllerIndex < RegisteredControllers.Num(); ControllerIndex++ )
	{
		// Disable unnecessary reset of inputs
		ReleaseController( ControllerIndex, /* bShouldResetInputs */ false );
	}

#ifdef PLATFORM_WINDOWS
	// We need to make sure the application is still valid and is not tearing down to avoid crashing.
	if ( FSlateApplication::IsInitialized() )
	{
		FWindowsApplication* WindowsApplication = (FWindowsApplication*)FSlateApplication::Get().GetPlatformApplication().Get();
		check( WindowsApplication );
		WindowsApplication->RemoveMessageHandler( *this );
	}
#endif
}

bool FDualSenseInputDevice::AddInputMessageReceiver( IDualSenseInputMessageReceiver* MessageReceiver )
{
	verify( MessageReceiver != nullptr );

	if ( MessageReceivers.Contains( MessageReceiver ) ) return false;

	// TODO: This would mean we have to trust the client for not changing its index midway,
	//		 shouldn't we ask for it here instead and store it in a map?
	if ( IsControllerAvailable( MessageReceiver->GetDualSenseControllerIndex() ) )
	{
		MessageReceiver->OnDualSenseConnected();
	}

	MessageReceivers.Add( MessageReceiver );
	return true;
}

void FDualSenseInputDevice::RemoveInputMessageReceiver( IDualSenseInputMessageReceiver* MessageReceiver )
{
	verify( MessageReceiver != nullptr );

	if ( !MessageReceivers.Contains( MessageReceiver ) ) return;

	MessageReceivers.RemoveSingle( MessageReceiver );
}

bool FDualSenseInputDevice::ReleaseController( int32 ControllerIndex, bool bShouldResetInputs )
{
	if ( !IsControllerAvailable( ControllerIndex ) ) return false;

	FDualSenseController& Controller = RegisteredControllers[ControllerIndex];

	// Reset inputs before releasing the controller, otherwise the player keep the inputs
	// he had right before and keep applying those.
	if ( bShouldResetInputs )
	{
		const UDualSenseSettings* Settings = UDualSenseSettings::Get();

		// Reset pressed buttons
		for ( int32 ButtonIndex = 0; ButtonIndex < MAX_DUALSENSE_BUTTONS; ButtonIndex++ )
		{
			const FDualSenseButtonDescriptor& Descriptor = DualSenseButtonDescriptors[ButtonIndex];

			if ( Controller.bWasPressed[ButtonIndex] )
			{
				if ( !Descriptor.NativeButtonName.IsNone() )
				{
					MessageHandler->OnControllerButtonReleased(
						Descriptor.NativeButtonName,
						Controller.UserId, Controller.DeviceId,
						/* bIsRepeat */ false
					);
				}

				// Emulate equivalent Gamepad inputs
				if ( Settings->bEmulateGamepadInputs && !Descriptor.GamepadButtonName.IsNone() )
				{
					MessageHandler->OnControllerButtonReleased(
						Descriptor.GamepadButtonName,
						Controller.UserId, Controller.DeviceId,
						/* bIsRepeat */ false
					);
				}
			}
		}

		// Reset axes
		SendLeftStickValues( Controller, FVector2D::ZeroVector );
		SendRightStickValues( Controller, FVector2D::ZeroVector );
		SendTriggerValues( Controller, 0.0f, 0.0f );
	}

	DS5W::freeDeviceContext( &Controller.DeviceContext );
	Controller.bIsConnected = false;

	// Send a disconnect event to the engine
	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
	DeviceMapper.Internal_MapInputDeviceToUser(
		Controller.DeviceId, Controller.UserId,
		EInputDeviceConnectionState::Disconnected
	);

	UE_LOG(
		LogDualSensePlugin, Log,
		TEXT( "Released controller %d" ),
		ControllerIndex
	);

	// Notify all message receivers of successful disconnection
	for ( IDualSenseInputMessageReceiver* MessageReceiver : MessageReceivers )
	{
		if ( ControllerIndex != MessageReceiver->GetDualSenseControllerIndex() ) continue;
		MessageReceiver->OnDualSenseDisconnected();
	}

	return true;
}

bool FDualSenseInputDevice::UpdateControllerOutputState( int32 ControllerIndex )
{
	if ( !IsControllerAvailable( ControllerIndex ) )
	{
		UE_LOG(
			LogDualSensePlugin, Error,
			TEXT( "Failed to update output state for controller %d: not initialized" ),
			ControllerIndex
		);
		return false;
	}

	FDualSenseController& Controller = RegisteredControllers[ControllerIndex];

	// Apply the output state
	const DS5W_RV ResultCode = DS5W::setDeviceOutputState( &Controller.DeviceContext, &Controller.OutputState );
	if ( DS5W_FAILED( ResultCode ) )
	{
		UE_LOG(
			LogDualSensePlugin, Error,
			TEXT( "Failed to update output state for controller %d: error code 0x%X" ),
			ControllerIndex,
			ResultCode
		);
		return false;
	}

	return true;
}

bool FDualSenseInputDevice::IsControllerAvailable( int32 ControllerIndex ) const
{
	if ( !RegisteredControllers.IsValidIndex( ControllerIndex ) ) return false;

	return RegisteredControllers[ControllerIndex].bIsConnected;
}

bool FDualSenseInputDevice::IsControllerButtonDown( int32 ControllerIndex, EDS5Button Button ) const
{
	if ( !IsControllerAvailable( ControllerIndex ) ) return false;

	const FDualSenseController& Controller = RegisteredControllers[ControllerIndex];

	switch ( Button )
	{
		case EDS5Button::Square:
			return Controller.InputState.buttonsAndDpad & DS5W_ISTATE_BTX_SQUARE;
		case EDS5Button::Cross:
			return Controller.InputState.buttonsAndDpad & DS5W_ISTATE_BTX_CROSS;
		case EDS5Button::Circle:
			return Controller.InputState.buttonsAndDpad & DS5W_ISTATE_BTX_CIRCLE;
		case EDS5Button::Triangle:
			return Controller.InputState.buttonsAndDpad & DS5W_ISTATE_BTX_TRIANGLE;

		case EDS5Button::DPadLeft:
			return Controller.InputState.buttonsAndDpad & DS5W_ISTATE_DPAD_LEFT;
		case EDS5Button::DPadRight:
			return Controller.InputState.buttonsAndDpad & DS5W_ISTATE_DPAD_RIGHT;
		case EDS5Button::DPadUp:
			return Controller.InputState.buttonsAndDpad & DS5W_ISTATE_DPAD_UP;
		case EDS5Button::DPadDown:
			return Controller.InputState.buttonsAndDpad & DS5W_ISTATE_DPAD_DOWN;

		case EDS5Button::L1:
			return Controller.InputState.buttonsA & DS5W_ISTATE_BTN_A_LEFT_BUMPER;
		case EDS5Button::L2:
			return Controller.InputState.buttonsA & DS5W_ISTATE_BTN_A_LEFT_TRIGGER;
		case EDS5Button::L3:
			return Controller.InputState.buttonsA & DS5W_ISTATE_BTN_A_LEFT_STICK;

		case EDS5Button::R1:
			return Controller.InputState.buttonsA & DS5W_ISTATE_BTN_A_RIGHT_BUMPER;
		case EDS5Button::R2:
			return Controller.InputState.buttonsA & DS5W_ISTATE_BTN_A_RIGHT_TRIGGER;
		case EDS5Button::R3:
			return Controller.InputState.buttonsA & DS5W_ISTATE_BTN_A_RIGHT_STICK;

		case EDS5Button::Share:
			return Controller.InputState.buttonsA & DS5W_ISTATE_BTN_A_SELECT;
		case EDS5Button::Menu:
			return Controller.InputState.buttonsA & DS5W_ISTATE_BTN_A_MENU;

		case EDS5Button::PlayStation:
			return Controller.InputState.buttonsB & DS5W_ISTATE_BTN_B_PLAYSTATION_LOGO;
		case EDS5Button::Pad:
			return Controller.InputState.buttonsB & DS5W_ISTATE_BTN_B_PAD_BUTTON;
		case EDS5Button::Microphone:
			return Controller.InputState.buttonsB & DS5W_ISTATE_BTN_B_MIC_BUTTON;

		case EDS5Button::MAX:
			break;
	}

	return false;
}

FDualSenseController* FDualSenseInputDevice::GetControllerData( int32 ControllerIndex )
{
	if ( !IsControllerAvailable( ControllerIndex ) ) return nullptr;

	return &RegisteredControllers[ControllerIndex];
}

const FDualSenseController* FDualSenseInputDevice::GetControllerData( int32 ControllerIndex ) const
{
	if ( !IsControllerAvailable( ControllerIndex ) ) return nullptr;

	return &RegisteredControllers[ControllerIndex];
}


FVector2D FDualSenseInputDevice::GetControllerLeftStickRawValue( int32 ControllerIndex ) const
{
	const FDualSenseController* Controller = GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return FVector2D::ZeroVector;

	return FVector2D {
		static_cast<double>( Controller->InputState.leftStick.x ),
		static_cast<double>( Controller->InputState.leftStick.y ),
	};
}

FVector2D FDualSenseInputDevice::GetControllerRightStickRawValue( int32 ControllerIndex ) const
{
	const FDualSenseController* Controller = GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return FVector2D::ZeroVector;

	return FVector2D {
		static_cast<double>( Controller->InputState.rightStick.x ),
		static_cast<double>( Controller->InputState.rightStick.y ),
	};
}

static FVector2D TranslateRawToUserValue( FVector2D Value, float Deadzone )
{
	constexpr float MAX_AXIS_VALUE = 128.0f;

	// Check for deadzone
	if ( Deadzone > 0.0f && Value.IsNearlyZero( Deadzone * MAX_AXIS_VALUE ) ) return FVector2D::ZeroVector;

	// Remap to a -1.0 : 1.0 range
	Value.X /= MAX_AXIS_VALUE;
	Value.Y /= MAX_AXIS_VALUE;

	// Clamp magnitude to 1.0 to still keep stick amplitude
	// while avoiding getting out-of-range
	const float SquaredLength = Value.SquaredLength();
	if ( SquaredLength > 1.0f )
	{
		const float Scale = FMath::InvSqrt( SquaredLength );
		Value.X *= Scale;
		Value.Y *= Scale;
	}

	return Value;
}

FVector2D FDualSenseInputDevice::GetControllerLeftStickUserValue( int32 ControllerIndex, float Deadzone ) const
{
	const FVector2D Value = GetControllerLeftStickRawValue( ControllerIndex );
	return TranslateRawToUserValue( Value, Deadzone );
}

FVector2D FDualSenseInputDevice::GetControllerRightStickUserValue( int32 ControllerIndex, float Deadzone ) const
{
	const FVector2D Value = GetControllerRightStickRawValue( ControllerIndex );
	return TranslateRawToUserValue( Value, Deadzone );
}

float FDualSenseInputDevice::GetControllerLeftTriggerRawValue( int32 ControllerIndex ) const
{
	if ( !IsControllerAvailable( ControllerIndex ) ) return 0.0f;

	const FDualSenseController& Controller = RegisteredControllers[ControllerIndex];
	return static_cast<float>( Controller.InputState.leftTrigger );
}

float FDualSenseInputDevice::GetControllerRightTriggerRawValue( int32 ControllerIndex ) const
{
	if ( !IsControllerAvailable( ControllerIndex ) ) return 0.0f;

	const FDualSenseController& Controller = RegisteredControllers[ControllerIndex];
	return static_cast<float>( Controller.InputState.rightTrigger );
}

float FDualSenseInputDevice::GetControllerLeftTriggerUserValue( int32 ControllerIndex ) const
{
	return GetControllerLeftTriggerRawValue( ControllerIndex ) / 255.0f;
}

float FDualSenseInputDevice::GetControllerRightTriggerUserValue( int32 ControllerIndex ) const
{
	return GetControllerRightTriggerRawValue( ControllerIndex ) / 255.0f;
}

void FDualSenseInputDevice::Tick( float DeltaTime )
{
	if ( UpdateControllersCooldown > 0.0f && ( UpdateControllersCooldown -= DeltaTime ) <= 0.0f )
	{
		QueryControllers();
	}
}

void FDualSenseInputDevice::SendControllerEvents()
{
	QUICK_SCOPE_CYCLE_COUNTER( FDualSenseInputDevice_SendControllerEvents );

	if ( MessageHandler == nullptr ) return;

	const UDualSenseSettings* Settings = UDualSenseSettings::Get();

	for ( int32 ControllerIndex = 0; ControllerIndex < RegisteredControllers.Num(); ControllerIndex++ )
	{
		FDualSenseController& Controller = RegisteredControllers[ControllerIndex];
		if ( !Controller.bIsConnected ) continue;

		// Update input state
		const DS5W_RV ResultCode = DS5W::getDeviceInputState(
			&Controller.DeviceContext,
			&Controller.InputState
		);

		if ( DS5W_FAILED( ResultCode ) )
		{
			ReleaseController( ControllerIndex );
			continue;
		}

		// Send all DualSense controllers inputs
		const double CurrentTime = FPlatformTime::Seconds();
		for ( int32 ButtonIndex = 0; ButtonIndex < MAX_DUALSENSE_BUTTONS; ButtonIndex++ )
		{
			const FDualSenseButtonDescriptor& Descriptor = DualSenseButtonDescriptors[ButtonIndex];
			
			const bool bIsPressed = IsControllerButtonDown( ControllerIndex, Descriptor.NativeButton );
			if ( bIsPressed && !Controller.bWasPressed[ButtonIndex] )
			{
				if ( !Descriptor.NativeButtonName.IsNone() )
				{
					MessageHandler->OnControllerButtonPressed(
						Descriptor.NativeButtonName,
						Controller.UserId, Controller.DeviceId,
						/* bIsRepeat */ false
					);
				}

				// Emulate equivalent Gamepad inputs
				if ( Settings->bEmulateGamepadInputs && !Descriptor.GamepadButtonName.IsNone() )
				{
					MessageHandler->OnControllerButtonPressed(
						Descriptor.GamepadButtonName,
						Controller.UserId, Controller.DeviceId,
						/* bIsRepeat */ false
					);
				}

				// Delay first repeat
				Controller.NextRepeatTime[ButtonIndex] = CurrentTime + InitialButtonRepeatDelay;
			}
			else if ( Controller.bWasPressed[ButtonIndex] && !bIsPressed )
			{
				if ( !Descriptor.NativeButtonName.IsNone() )
				{
					MessageHandler->OnControllerButtonReleased(
						Descriptor.NativeButtonName,
						Controller.UserId, Controller.DeviceId,
						/* bIsRepeat */ false
					);
				}

				// Emulate equivalent Gamepad inputs
				if ( Settings->bEmulateGamepadInputs && !Descriptor.GamepadButtonName.IsNone() )
				{
					MessageHandler->OnControllerButtonReleased(
						Descriptor.GamepadButtonName,
						Controller.UserId, Controller.DeviceId,
						/* bIsRepeat */ false
					);
				}
			}
			else if ( bIsPressed && Controller.NextRepeatTime[ButtonIndex] < CurrentTime )
			{
				if ( !Descriptor.NativeButtonName.IsNone() )
				{
					MessageHandler->OnControllerButtonPressed(
						Descriptor.NativeButtonName,
						Controller.UserId, Controller.DeviceId,
						/* bIsRepeat */ true
					);
				}

				// Emulate equivalent Gamepad inputs
				if ( Settings->bEmulateGamepadInputs && !Descriptor.GamepadButtonName.IsNone() )
				{
					MessageHandler->OnControllerButtonPressed(
						Descriptor.GamepadButtonName,
						Controller.UserId, Controller.DeviceId,
						/* bIsRepeat */ true
					);
				}

				// Delay next repeat
				Controller.NextRepeatTime[ButtonIndex] = CurrentTime + ButtonRepeatDelay;
			}

			Controller.bWasPressed[ButtonIndex] = bIsPressed;
		}

		// Left stick
		const FVector2D LeftStick = GetControllerLeftStickUserValue( ControllerIndex, 0.0f );
		SendLeftStickValues( Controller, LeftStick );

		// Right stick
		const FVector2D RightStick = GetControllerRightStickUserValue( ControllerIndex, 0.0f );
		SendRightStickValues( Controller, RightStick );

		// Trigger axes
		SendTriggerValues(
			Controller,
			GetControllerLeftTriggerUserValue( ControllerIndex ),
			GetControllerRightTriggerUserValue( ControllerIndex )
		);

		// Compute force feedback values
		const float LargeValue = FMath::Max(
			Controller.ForceFeedback.LeftLarge,
			Controller.ForceFeedback.RightLarge
		);
		const float SmallValue = FMath::Max(
			Controller.ForceFeedback.LeftSmall,
			Controller.ForceFeedback.RightSmall
		);

		// Only apply force feedback when there is a difference since the last time
		if ( !FMath::IsNearlyEqual( LargeValue, Controller.LastLargeValue )
		  || !FMath::IsNearlyEqual( SmallValue, Controller.LastSmallValue ) )
		{
			// Left side of the DualSense has a hard rumble motor and right side, a soft one.
			// So we map the largest value to the left side and smallest value to the right.
			Controller.OutputState.leftRumble  = static_cast<uint8>( LargeValue * 255.0f );
			Controller.OutputState.rightRumble = static_cast<uint8>( SmallValue * 255.0f );
			UpdateControllerOutputState( ControllerIndex );

			Controller.LastLargeValue = LargeValue;
			Controller.LastSmallValue = SmallValue;
		}
	}
}

void FDualSenseInputDevice::SetMessageHandler(
	const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler
)
{
	MessageHandler = InMessageHandler;
}

bool FDualSenseInputDevice::Exec( UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar )
{
	return false;
}

void FDualSenseInputDevice::SetChannelValue(
	int32 ControllerIndex,
	FForceFeedbackChannelType ChannelType,
	float Value
)
{
	FDualSenseController* Controller = GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return;
	if ( !Controller->bIsConnected ) return;

	switch ( ChannelType )
	{
		case FForceFeedbackChannelType::LEFT_LARGE:
			Controller->ForceFeedback.LeftLarge = Value;
			break;
		case FForceFeedbackChannelType::LEFT_SMALL:
			Controller->ForceFeedback.LeftSmall = Value;
			break;
		case FForceFeedbackChannelType::RIGHT_LARGE:
			Controller->ForceFeedback.RightLarge = Value;
			break;
		case FForceFeedbackChannelType::RIGHT_SMALL:
			Controller->ForceFeedback.RightSmall = Value;
			break;
	}

	//UE_LOG( LogDualSensePlugin, Log, TEXT( "SetChannelValue %d %d %.2f" ), ControllerIndex, (int)ChannelType, Value );
}

void FDualSenseInputDevice::SetChannelValues(
	int32 ControllerIndex,
	const FForceFeedbackValues& Values
)
{
	FDualSenseController* Controller = GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return;
	if ( !Controller->bIsConnected ) return;

	Controller->ForceFeedback = Values;

	/*UE_LOG(
		LogDualSensePlugin, Log,
		TEXT( "SetChannelValues %d %.2f %.2f %.2f %.2f" ),
		ControllerIndex, Values.LeftLarge, Values.LeftSmall, Values.RightLarge, Values.RightSmall
	);*/
}

bool FDualSenseInputDevice::SupportsForceFeedback( int32 ControllerIndex )
{
	return true;
}

void FDualSenseInputDevice::SetLightColor( int32 ControllerIndex, FColor Color )
{
	FDualSenseController* Controller = GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return;
	if ( !Controller->bIsConnected ) return;

	Controller->OutputState.lightbar = DS5W::color_R32G32B32A32_FLOAT(
		Color.R,
		Color.G,
		Color.B,
		Color.A
	);

	UpdateControllerOutputState( ControllerIndex );
}

void FDualSenseInputDevice::ResetLightColor( int32 ControllerIndex )
{
	FDualSenseController* Controller = GetControllerData( ControllerIndex );
	if ( Controller == nullptr ) return;
	if ( !Controller->bIsConnected ) return;

	Controller->OutputState.disableLeds = true;

	UpdateControllerOutputState( ControllerIndex );
}

void FDualSenseInputDevice::SetDeviceProperty( int32 ControllerIndex, const FInputDeviceProperty* Property )
{
}

IHapticDevice* FDualSenseInputDevice::GetHapticDevice()
{
	return nullptr;
}

bool FDualSenseInputDevice::IsGamepadAttached() const
{
	for ( const FDualSenseController& Controller : RegisteredControllers )
	{
		if ( Controller.bIsConnected )
		{
			return true;
		}
	}

	return false;
}

#ifdef PLATFORM_WINDOWS
static FString WindowsGetDevicePath( const LPARAM lParam )
{
	const PDEV_BROADCAST_HDR lpdb = reinterpret_cast<PDEV_BROADCAST_HDR>( lParam );
	if ( lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE )
	{
		const PDEV_BROADCAST_DEVICEINTERFACE lpdbv = reinterpret_cast<PDEV_BROADCAST_DEVICEINTERFACE>( lpdb );
		const FString Path = lpdbv->dbcc_name;
		return Path;
	}

	return TEXT( "UnknownWindowsDevice" );
}

static bool WindowsGetVendorAndProductIdentifiers( const FString& Path, FString& VendorIdentifier, FString& ProductIdentifier )
{
	const FRegexPattern VendorProductIdentifiersPattern( TEXT( "VID_(\\w+)&PID_(\\w+)" ) );

	FRegexMatcher VendorProductIdentifiersMatcher( VendorProductIdentifiersPattern, Path );
	if ( VendorProductIdentifiersMatcher.FindNext() )
	{
		VendorIdentifier  = VendorProductIdentifiersMatcher.GetCaptureGroup( 1 );
		ProductIdentifier = VendorProductIdentifiersMatcher.GetCaptureGroup( 2 );
		return true;
	}

	return false;
}

bool FDualSenseInputDevice::ProcessMessage( HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam, int32& OutResult )
{
	if ( msg == WM_DEVICECHANGE )
	{
		UE_LOG( LogDualSensePlugin, VeryVerbose, TEXT( "ProcessMessage %d %llu" ), msg, wParam );
		if ( wParam == DBT_DEVICEARRIVAL )
		{
			const FString Path = WindowsGetDevicePath( lParam );

			// TODO: Replace VID and PID with the constants so both internal code and this one are using the same values.
			FString VendorIdentifier, ProductIdentifier;
			if ( WindowsGetVendorAndProductIdentifiers( Path, VendorIdentifier, ProductIdentifier )
				&& VendorIdentifier == TEXT( "054C" ) && ProductIdentifier == TEXT( "0CE6" ) )
			{
				UE_LOG( LogDualSensePlugin, Verbose, TEXT( "Device %s has arrived." ), *Path );

				if ( RequestControllersUpdate() )
				{
					UE_LOG( LogDualSensePlugin, Log, TEXT( "A device has been plugged, an update is on-going in %fs." ), UpdateControllersCooldown );
				}
			}
			else
			{
				UE_LOG( LogDualSensePlugin, Verbose, TEXT( "Ignored device connection of %s" ), *Path );
			}
		}
		else if ( wParam == DBT_DEVICEREMOVECOMPLETE )
		{
			FString VendorIdentifier, ProductIdentifier;
			const FString Path = WindowsGetDevicePath( lParam );
			UE_LOG( LogDualSensePlugin, Verbose, TEXT( "Device %s has disconnected" ), *Path );
		}
	}

	return false;
}
#endif

void FDualSenseInputDevice::SendLeftStickValues( const FDualSenseController& Controller, FVector2D AnalogValues )
{
	const UDualSenseSettings* Settings = UDualSenseSettings::Get();
	if ( Settings->bEmulateGamepadInputs )
	{
		MessageHandler->OnControllerAnalog(
			EKeys::Gamepad_LeftX.GetFName(),
			Controller.UserId, Controller.DeviceId,
			AnalogValues.X
		);
		MessageHandler->OnControllerAnalog(
			EKeys::Gamepad_LeftY.GetFName(),
			Controller.UserId, Controller.DeviceId,
			AnalogValues.Y
		);
	}
}

void FDualSenseInputDevice::SendRightStickValues( const FDualSenseController& Controller, FVector2D AnalogValues )
{
	const UDualSenseSettings* Settings = UDualSenseSettings::Get();
	if ( Settings->bEmulateGamepadInputs )
	{
		MessageHandler->OnControllerAnalog(
			EKeys::Gamepad_RightX.GetFName(),
			Controller.UserId, Controller.DeviceId,
			AnalogValues.X
		);
		MessageHandler->OnControllerAnalog(
			EKeys::Gamepad_RightY.GetFName(),
			Controller.UserId, Controller.DeviceId,
			AnalogValues.Y
		);
	}
}

void FDualSenseInputDevice::SendTriggerValues( const FDualSenseController& Controller, float LeftTriggerValue, float RightTriggerValue )
{
	const UDualSenseSettings* Settings = UDualSenseSettings::Get();
	if ( Settings->bEmulateGamepadInputs )
	{
		MessageHandler->OnControllerAnalog(
			EKeys::Gamepad_LeftTriggerAxis.GetFName(),
			Controller.UserId, Controller.DeviceId,
			LeftTriggerValue
		);
		MessageHandler->OnControllerAnalog(
			EKeys::Gamepad_RightTriggerAxis.GetFName(),
			Controller.UserId, Controller.DeviceId,
			RightTriggerValue
		);
	}
}

bool FDualSenseInputDevice::QueryControllers()
{
	QUICK_SCOPE_CYCLE_COUNTER( FDualSenseInputDevice_QueryControllers );

	// Release all controllers and clear list
	for ( int32 ControllerIndex = 0; ControllerIndex < RegisteredControllers.Num(); ControllerIndex++ )
	{
		ReleaseController( ControllerIndex );
	}
	RegisteredControllers.Empty();

	DS5W::DeviceEnumInfo DeviceInfos[MAX_CONTROLLERS];
	uint32 ControllersCount = 0;

	// List all available controllers
	DS5W_ReturnValue ResultCode = DS5W::enumDevices( DeviceInfos, MAX_CONTROLLERS, &ControllersCount );
	if ( DS5W_FAILED( ResultCode ) )
	{
		UE_LOG(
			LogDualSensePlugin, Error,
			TEXT( "Failed to query the controllers while listing devices: error code 0x%04X" ),
			ResultCode
		);
		return false;
	}

	// Check number of available controllers
	if ( ControllersCount == 0 )
	{
		UE_LOG(
			LogDualSensePlugin, Log,
			TEXT( "Failed to query the controllers: no controller available" )
		);
		return false;
	}

	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();

	FDualSenseController Controller {};
	for ( int32 ControllerIndex = 0; ControllerIndex < static_cast<int32>( ControllersCount ); ControllerIndex++ )
	{
		// Initialize device context
		ResultCode = DS5W::initDeviceContext( &DeviceInfos[ControllerIndex], &Controller.DeviceContext );
		if ( DS5W_FAILED( ResultCode ) )
		{
			UE_LOG(
				LogDualSensePlugin, Error,
				TEXT( "Failed to initialize controller %d while initializing device context: error code 0x%04X" ),
				ControllerIndex,
				ResultCode
			);
			continue;
		}

		// Map controller to user and device
		DeviceMapper.RemapControllerIdToPlatformUserAndDevice(
			ControllerIndex,
			Controller.UserId,
			Controller.DeviceId
		);
		DeviceMapper.Internal_MapInputDeviceToUser(
			Controller.DeviceId, Controller.UserId,
			EInputDeviceConnectionState::Connected
		);

		// Initialize controller's structure
		FMemory::Memzero( &Controller.InputState, sizeof( DS5W::DS5InputState ) );
		FMemory::Memzero( &Controller.OutputState, sizeof( DS5W::DS5OutputState ) );
		Controller.bIsConnected = true;
		RegisteredControllers.Add( Controller );

		UE_LOG(
			LogDualSensePlugin, Log,
			TEXT( "Initialized controller %d via %s" ),
			ControllerIndex,
			Controller.DeviceContext._internal.connection == DS5W::_DeviceConnection::USB ? TEXT( "USB" ) : TEXT( "Bluetooth" )
		);
	}

	// Notify all message receivers of successful connection
	for ( IDualSenseInputMessageReceiver* MessageReceiver : MessageReceivers )
	{
		if ( !IsControllerAvailable( MessageReceiver->GetDualSenseControllerIndex() ) ) continue;
		MessageReceiver->OnDualSenseConnected();
	}

	UE_LOG(
		LogDualSensePlugin, Log,
		TEXT( "Registered %d controllers" ),
		RegisteredControllers.Num()
	);

	return true;
}

bool FDualSenseInputDevice::RequestControllersUpdate()
{
	// NOTE: We put an arbitrary cooldown because QueryControllers doesn't find instantly the gamepad
	//		 after a Windows connection event.
	UpdateControllersCooldown = 1.0f;
	return true;
}