#include "DualSensePlugin.h"
#include "DualSenseFunctionLibrary.h"
#include "DualSenseInputDevice.h"

#include "InputCoreTypes.h"

DEFINE_LOG_CATEGORY( LogDualSensePlugin );

#define LOCTEXT_NAMESPACE "DualSensePlugin"

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
	// TODO: Remove this code
	//InputDevice = MakeShared<FDualSenseInputDevice>( nullptr );
	//UE_LOG( LogDualSensePlugin, Log, TEXT( "Created Input Device %p" ), InputDevice.Get() );

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
	// TODO: Ensure DualSenseComponent don't use the InputDevice before it is created
	if ( InputDevice == nullptr )
	{
		InputDevice = MakeShared<FDualSenseInputDevice>( MessageHandler );
		UE_LOG( LogDualSensePlugin, Log, TEXT( "Created Input Device %p" ), InputDevice.Get() );
	}

	return InputDevice;
}

TSharedPtr<FDualSenseInputDevice> FDualSensePlugin::GetInputDevice() const
{
	return InputDevice;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FDualSensePlugin, DualSenseV )
