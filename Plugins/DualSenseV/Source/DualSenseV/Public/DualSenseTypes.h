#pragma once

#include "CoreMinimal.h"
#include "DualSenseTypes.generated.h"

UENUM( BlueprintType, meta = ( DisplayName = "DualSense5 Touch Index" ) )
enum class EDS5TouchIndex : uint8
{
	First	= 0x00,
	Second	= 0x01,
};

USTRUCT( BlueprintType, meta = ( DisplayName = "DualSense5 Touch" ) )
struct FDS5Touch
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "DualSense" )
	uint8 ID = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "DualSense" )
	bool bIsDown = false;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "DualSense" )
	FVector2D Position = FVector2D::ZeroVector;
};

/*
 * Enum representing all different buttons that the DualSense5 handles.
 */
UENUM( BlueprintType, meta = ( DisplayName = "DualSense5 Button" ) )
enum class EDS5Button : uint8
{
	/*
	 * Emulated as Face Button Left.
	 */
	Square,
	/*
	 * Emulated as Face Button Bottom.
	 */
	Cross,
	/*
	 * Emulated as Face Button Right.
	 */
	Circle,
	/*
	 * Emulated as Face Button Up.
	 */
	Triangle,

	DPadLeft,
	DPadRight,
	DPadUp,
	DPadDown,

	/*
	 * Emulted as Left Shoulder.
	 */
	L1,
	/*
	 * Emulated as Left Trigger.
	 */
	L2,
	/*
	 * Emulated as Left Thumbstick.
	 */
	L3,

	/*
	 * Emulated as Right Shoulder.
	 */
	R1,
	/*
	 * Emulated as Right Trigger.
	 */
	R2,
	/*
	 * Emulated as Right Shumbstick.
	 */
	R3,

	Share,
	/*
	 * Emulated as Special Right button. 
	 */
	Menu,

	PlayStation,
	/*
	 * Emulated as Special Left button.
	 */
	Pad,
	Microphone,

	MAX				UMETA( Hidden ),
};

USTRUCT( BlueprintType, meta = ( DisplayName = "DualSense5 Battery" ) )
struct FDS5Battery
{
	GENERATED_BODY()

	/*
	 * Battery level from 0.0f to 1.0f.
	 */
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "DualSense" )
	float Level = 0.0f;

	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "DualSense" )
	bool bIsCharging = false;

	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "DualSense" )
	bool bIsFullCharged = false;
};

UENUM( BlueprintType, meta = ( DisplayName = "DualSense5 Microphone LED" ) )
enum class EDS5MicrophoneLED : uint8
{
	Off		= 0x00,
	On		= 0x01,
	Pulse	= 0x02,
};

UENUM( BlueprintType, meta = ( DisplayName = "DualSense5 Trigger Effect Type" ) )
enum class EDS5TriggerEffectType : uint8
{
	NoResitance			= 0x00,
	ContinuousResitance = 0x01,
	SectionResitance	= 0x02,
	EffectEx			= 0x26,
	Calibrate			= 0xFC,
};

USTRUCT( BlueprintType, meta = ( DisplayName = "DualSense5 Trigger Effect" ) )
struct FDS5TriggerEffect
{
	GENERATED_BODY()

	uint8 Data[11] {};

	uint8& operator[]( const int32 Index )
	{
		return Data[Index];
	}
};

UENUM( BlueprintType, meta = ( DisplayName = "DualSense5 LED Brightness" ) )
enum class EDS5LEDBrightness : uint8
{
	Low = 0x02,
	Medium = 0x01,
	High = 0x00,
};

UENUM( BlueprintType, meta = ( Bitflags, DisplayName = "DualSense5 Player LED" ) )
enum class EDS5PlayerLED : uint8
{
	Off = 0x00,
	Left = 0x01,
	MiddleLeft = 0x02,
	Middle = 0x04,
	MiddleRight = 0x08,
	Right = 0x10,
};
ENUM_CLASS_FLAGS( EDS5PlayerLED )

USTRUCT( BlueprintType, meta = ( DisplayName = "DualSense5 Player LEDs" ) )
struct FDS5PlayerLEDs
{
	GENERATED_BODY()

	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "DualSense" )
	EDS5PlayerLED BitMask = EDS5PlayerLED::Off;

	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "DualSense" )
	bool bShouldFade = false;

	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "DualSense" )
	EDS5LEDBrightness Brightness = EDS5LEDBrightness::High;
};

USTRUCT( BlueprintType, meta = ( DisplayName = "DualSense5 Output State" ) )
struct FDS5OutputState
{
	GENERATED_BODY()

	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "DualSense", meta = ( ClampMin = "0.0", ClampMax = "10.0" ) )
	float LeftRumble = 0.0f;

	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "DualSense", meta = ( ClampMin = "0.0", ClampMax = "10.0" ) )
	float RightRumble = 0.0f;

	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "DualSense" )
	EDS5MicrophoneLED MicrophoneLED = EDS5MicrophoneLED::Off;

	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "DualSense" )
	FDS5PlayerLEDs PlayerLEDs {};

	/*
	 * If set to false, the lightbar will be set to the default DualSense color,
	 * preventing to be customized.
	 */
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "DualSense" )
	bool bIsLightbarEnabled = true;

	/*
	 * Custom color of the lightbar.
	 */
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "DualSense" )
	FLinearColor LightbarColor = FLinearColor::White;

	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "DualSense" )
	FDS5TriggerEffect LeftTriggerEffect {};

	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "DualSense" )
	FDS5TriggerEffect RightTriggerEffect {};
};