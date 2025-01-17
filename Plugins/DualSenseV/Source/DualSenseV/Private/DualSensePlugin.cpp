#include "DualSensePlugin.h"
#include "DualSenseFunctionLibrary.h"

#include "Device.h"
#include "DS5_Input.h"
#include "DSW_Api.h"
#include "Helpers.h"
#include "IO.h"

#include "InputCoreTypes.h"

DEFINE_LOG_CATEGORY( LogDualSensePlugin );

#define LOCTEXT_NAMESPACE "DualSense5Plugin"

constexpr int MAX_CONTROLLERS = 16;

FName UDualSenseSettings::GetCategoryName() const
{
	return TEXT( "Plugins" );
}

const UDualSenseSettings* UDualSenseSettings::Get()
{
	return GetDefault<UDualSenseSettings>();
}

FDualSenseInputDevice::FDualSenseInputDevice( const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler )
	: MessageHandler( MessageHandler )
{
	QueryControllers();
}

FDualSenseInputDevice::~FDualSenseInputDevice()
{
	for ( int32 ControllerIndex = 0; ControllerIndex < RegisteredControllers.Num(); ControllerIndex++ )
	{
		ReleaseController( ControllerIndex );
	}
}

bool FDualSenseInputDevice::InitializeController( int32 ControllerIndex )
{
	// TODO: Don't query all controllers for each initialization, only once at game startup
	QueryControllers();

	if ( !IsControllerAvailable( ControllerIndex ) ) return false;

	return true;
}

bool FDualSenseInputDevice::ReleaseController( int32 ControllerIndex )
{
	if ( !IsControllerAvailable( ControllerIndex ) ) return false;

	FDualSenseController& Controller = RegisteredControllers[ControllerIndex];

	DS5W::freeDeviceContext( &Controller.DeviceContext );
	Controller.bIsConnected = false;

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

FVector2D FDualSenseInputDevice::GetControllerRightStickRawValue( int32 ControllerIndex  ) const
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

void FDualSenseInputDevice::Tick( float DeltaTime ) {}

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

void FDualSenseInputDevice::SendControllerEvents()
{
	if ( MessageHandler == nullptr ) return;

	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();

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
			Controller.bIsConnected = false;
			
			DeviceMapper.Internal_MapInputDeviceToUser(
				Controller.DeviceId, Controller.UserId,
				EInputDeviceConnectionState::Disconnected
			);

			UE_LOG(
				LogDualSensePlugin, Error,
				TEXT( "Failed to update output state for controller %d: error code 0x%X" ),
				ControllerIndex,
				ResultCode
			);
			continue;
		}

		// Send all DualSense controllers inputs
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

			Controller.bWasPressed[ButtonIndex] = bIsPressed;
		}

		// Left stick
		const FVector2D LeftStick = GetControllerLeftStickUserValue( ControllerIndex, 0.0f );
		if ( Settings->bEmulateGamepadInputs )
		{
			MessageHandler->OnControllerAnalog(
				EKeys::Gamepad_LeftX.GetFName(),
				Controller.UserId, Controller.DeviceId,
				LeftStick.X
			);
			MessageHandler->OnControllerAnalog(
				EKeys::Gamepad_LeftY.GetFName(),
				Controller.UserId, Controller.DeviceId,
				LeftStick.Y
			);
		}

		// Right stick
		const FVector2D RightStick = GetControllerRightStickUserValue( ControllerIndex, 0.0f );
		if ( Settings->bEmulateGamepadInputs )
		{
			MessageHandler->OnControllerAnalog(
				EKeys::Gamepad_RightX.GetFName(),
				Controller.UserId, Controller.DeviceId,
				RightStick.X
			);
			MessageHandler->OnControllerAnalog(
				EKeys::Gamepad_RightY.GetFName(),
				Controller.UserId, Controller.DeviceId,
				RightStick.Y 
			);
		}

		// Trigger axes
		if ( Settings->bEmulateGamepadInputs )
		{
			MessageHandler->OnControllerAnalog(
				EKeys::Gamepad_LeftTriggerAxis.GetFName(),
				Controller.UserId, Controller.DeviceId,
				GetControllerLeftTriggerUserValue( ControllerIndex )
			);
			MessageHandler->OnControllerAnalog(
				EKeys::Gamepad_RightTriggerAxis.GetFName(),
				Controller.UserId, Controller.DeviceId,
				GetControllerRightTriggerUserValue( ControllerIndex )
			);
		}

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
			Controller.OutputState.leftRumble = static_cast<uint8>( LargeValue * 255.0f );
			Controller.OutputState.rightRumble = static_cast<uint8>( SmallValue * 255.0f );
			UpdateControllerOutputState( ControllerIndex );

			Controller.LastLargeValue = LargeValue;
			Controller.LastSmallValue = SmallValue;

			/*UE_LOG(
				LogDualSensePlugin, Log,
				TEXT(
					"Updated force feedback values of controller %d: %d %d"
				),
				ControllerIndex,
				Controller.OutputState.leftRumble, Controller.OutputState.rightRumble
			);*/
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

bool FDualSenseInputDevice::QueryControllers()
{
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

	UE_LOG(
		LogDualSensePlugin, Log,
		TEXT( "Registered %d controllers" ),
		RegisteredControllers.Num()
	);

	return true;
}

static const FName NAME_DualSense5( TEXT( "DualSense5" ) );

void FDualSensePlugin::StartupModule()
{
	IInputDeviceModule::StartupModule();

	EKeys::AddMenuCategoryDisplayInfo(
		NAME_DualSense5,
		LOCTEXT( "DualSense5SubCategory", "Dual Sense 5" ),
		TEXT( "GraphEditor.PadEvent_16x" )
	);

	EKeys::AddKey( FKeyDetails( FDualSenseKeys::Square, LOCTEXT( "DualSense5_Square", "Dual Sense 5 Square" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	EKeys::AddKey( FKeyDetails( FDualSenseKeys::Cross, LOCTEXT( "DualSense5_Cross", "Dual Sense 5 Cross" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	EKeys::AddKey( FKeyDetails( FDualSenseKeys::Circle, LOCTEXT( "DualSense5_Circle", "Dual Sense 5 Circle" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	EKeys::AddKey( FKeyDetails( FDualSenseKeys::Triangle, LOCTEXT( "DualSense5_Triangle", "Dual Sense 5 Triangle" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	
	EKeys::AddKey( FKeyDetails( FDualSenseKeys::DPadLeft, LOCTEXT( "DualSense5_DPadLeft", "Dual Sense 5 D-pad Left" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	EKeys::AddKey( FKeyDetails( FDualSenseKeys::DPadRight, LOCTEXT( "DualSense5_DPadRight", "Dual Sense 5 D-pad Right" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	EKeys::AddKey( FKeyDetails( FDualSenseKeys::DPadUp, LOCTEXT( "DualSense5_DPadUp", "Dual Sense 5 D-pad Up" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	EKeys::AddKey( FKeyDetails( FDualSenseKeys::DPadDown, LOCTEXT( "DualSense5_DPadDown", "Dual Sense 5 D-pad Down" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	
	EKeys::AddKey( FKeyDetails( FDualSenseKeys::L1, LOCTEXT( "DualSense5_L1", "Dual Sense 5 L1" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	EKeys::AddKey( FKeyDetails( FDualSenseKeys::L2, LOCTEXT( "DualSense5_L2", "Dual Sense 5 L2" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	EKeys::AddKey( FKeyDetails( FDualSenseKeys::L3, LOCTEXT( "DualSense5_L3", "Dual Sense 5 L3" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	EKeys::AddKey( FKeyDetails( FDualSenseKeys::R1, LOCTEXT( "DualSense5_R1", "Dual Sense 5 R1" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	EKeys::AddKey( FKeyDetails( FDualSenseKeys::R2, LOCTEXT( "DualSense5_R2", "Dual Sense 5 R2" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	EKeys::AddKey( FKeyDetails( FDualSenseKeys::R3, LOCTEXT( "DualSense5_R3", "Dual Sense 5 R3" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	
	EKeys::AddKey( FKeyDetails( FDualSenseKeys::Share, LOCTEXT( "DualSense5_Share", "Dual Sense 5 Share" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	EKeys::AddKey( FKeyDetails( FDualSenseKeys::Menu, LOCTEXT( "DualSense5_Menu", "Dual Sense 5 Menu" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	EKeys::AddKey( FKeyDetails( FDualSenseKeys::PlayStation, LOCTEXT( "DualSense5_PlayStation", "Dual Sense 5 PlayStation" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	EKeys::AddKey( FKeyDetails( FDualSenseKeys::Pad, LOCTEXT( "DualSense5_Pad", "Dual Sense 5 Pad" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	EKeys::AddKey( FKeyDetails( FDualSenseKeys::Microphone, LOCTEXT( "DualSense5_Microphone", "Dual Sense 5 Microphone" ), FKeyDetails::GamepadKey, NAME_DualSense5 ) );
	
	// NOTE: Creating the input device during module start up because CreateInputDevice
	// is somehow called too late after components' BeginPlay and I don't know how to change that.
	// It would cause crashes with DualSenseControllerComponents trying to use the InputDevice
	// in Standalone and builds.
	InputDevice = MakeShared<FDualSenseInputDevice>( nullptr );
	UE_LOG( LogDualSensePlugin, Log, TEXT( "Created Input Device %p" ), InputDevice.Get() );

	UE_LOG( LogDualSensePlugin, Log, TEXT( "Started up module" ) );
}

void FDualSensePlugin::ShutdownModule()
{
	// TODO: Remove if the module can't be shutdown without having to restart the engine.
	EKeys::RemoveKeysWithCategory( NAME_DualSense5 );
}

TSharedPtr<IInputDevice> FDualSensePlugin::CreateInputDevice(
	const TSharedRef<FGenericApplicationMessageHandler>& MessageHandler
)
{
	InputDevice->SetMessageHandler( MessageHandler );
	return InputDevice;
}

TSharedPtr<FDualSenseInputDevice> FDualSensePlugin::GetInputDevice() const
{
	return InputDevice;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FDualSensePlugin, DualSenseV )