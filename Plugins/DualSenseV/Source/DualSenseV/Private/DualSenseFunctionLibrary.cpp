// /* * Implemented by Firstname Lastname (Nickname) */

#include "DualSenseFunctionLibrary.h"

static uint8 NormalizedFloatToByte(
    const float Value,
    const uint8 MinOutput = 0, const uint8 MaxOutput = 255
)
{
    return FMath::Clamp( static_cast<uint8>( Value * 255.0f ), MinOutput, MaxOutput );
}

static uint8 RemapFloatToByteClamped(
    float Value,
    const uint8 MinOutput = 0, const uint8 MaxOutput = 255
)
{
    Value = FMath::Clamp( Value, 0.0f, 1.0f );

    //const float Ratio = ( Value - MinInput ) / ( MaxInput - MinInput );
    return FMath::Lerp( MinOutput, MaxOutput, Value );
}

/*
 * Most implementations come from this gist:
 * https://gist.github.com/Nielk1/6d54cc2c00d2201ccb8c2720ad7538db 
 */

FDS5TriggerEffect UDualSenseFunctionLibrary::MakeOff()
{
    FDS5TriggerEffect TriggerEffect {};
    TriggerEffect[0x00] = 0x05;
    return TriggerEffect;
}

FDS5TriggerEffect UDualSenseFunctionLibrary::MakeFeedback(
    const float Position,
    const float Force
)
{
    const uint8 ForceByte = RemapFloatToByteClamped( Force, 0, 8 );
    if ( ForceByte == 0 ) return MakeOff();
    
    const uint8 PositionByte = RemapFloatToByteClamped( Position, 0, 9 );
    
    const uint8 ForceValue = ( ForceByte - 1 ) & 0x07;
    uint32 ForceZones  = 0;
    uint16 ActiveZones = 0;
    for ( int32 Index = PositionByte; Index < 10; Index++ )
    {
        ForceZones  |= static_cast<uint32>( ForceValue << ( 3 * Index ) );
        ActiveZones |= static_cast<uint16>( 1 << Index );
    }
    
    FDS5TriggerEffect TriggerEffect {};
    TriggerEffect[0x00] = 0x21;
    TriggerEffect[0x01] = ActiveZones >> 0;
    TriggerEffect[0x02] = ActiveZones >> 8;
    TriggerEffect[0x03] = ForceZones  >> 0;
    TriggerEffect[0x04] = ForceZones  >> 8;
    TriggerEffect[0x05] = ForceZones  >> 16;
    TriggerEffect[0x06] = ForceZones  >> 24;
    return TriggerEffect;
}

FDS5TriggerEffect UDualSenseFunctionLibrary::MakeWeapon(
    const float StartPosition,
    const float EndPosition,
    const float Force
)
{
    const uint8 ForceByte = RemapFloatToByteClamped( Force, 0, 8 );
    if ( ForceByte == 0 ) return MakeOff();
    
    const uint8 StartPositionByte = FMath::Clamp(
        RemapFloatToByteClamped( StartPosition, 0, 8 ),
        2,
        7
    );
    const uint8 EndPositionByte = FMath::Max(
        RemapFloatToByteClamped( EndPosition, 0, 8 ),
        static_cast<uint8>( StartPositionByte + 1 )
    );

    const uint16 StartAndEndZones = ( 1 << StartPositionByte ) | ( 1 << EndPositionByte );
    
    FDS5TriggerEffect TriggerEffect {};
    TriggerEffect[0x00] = 0x25;
    TriggerEffect[0x01] = StartAndEndZones >> 0;
    TriggerEffect[0x02] = StartAndEndZones >> 8;
    TriggerEffect[0x03] = ForceByte - 1;
    return TriggerEffect;
}

FDS5TriggerEffect UDualSenseFunctionLibrary::MakeVibration(
    const float Position,
    const float Amplitude,
    const float Frequency
)
{
    const uint8 PositionByte = RemapFloatToByteClamped( Position, 0, 9 );
    const uint8 AmplitudeByte = RemapFloatToByteClamped( Amplitude, 0, 8 );
    const uint8 StrengthByte = ( AmplitudeByte - 1 ) & 0x07;

    // Compute amplitude and active zones
    uint32 AmplitudeZones = 0;
    uint16 ActiveZones = 0;
    for ( int32 Index = PositionByte; Index < 10; Index++ )
    {
        AmplitudeZones |= static_cast<uint32>( StrengthByte << ( 3 * Index ) );
        ActiveZones    |= static_cast<uint16>( 1 << Index );
    }

    // Transpose bytes to array
    FDS5TriggerEffect TriggerEffect {};
    TriggerEffect[0x00] = 0x26;
    TriggerEffect[0x01] = ActiveZones    >> 0;
    TriggerEffect[0x02] = ActiveZones    >> 8;
    TriggerEffect[0x03] = AmplitudeZones >> 0;
    TriggerEffect[0x04] = AmplitudeZones >> 8;
    TriggerEffect[0x05] = AmplitudeZones >> 16;
    TriggerEffect[0x06] = AmplitudeZones >> 24;
    TriggerEffect[0x09] = NormalizedFloatToByte( Frequency );
    return TriggerEffect;
}

FDS5TriggerEffect UDualSenseFunctionLibrary::MakeContinuousResistance(
    const float StartPosition,
    const float Force
)
{
    FDS5TriggerEffect TriggerEffect {};
    TriggerEffect[0x00] = 0x01;
    TriggerEffect[0x01] = NormalizedFloatToByte( StartPosition );
    TriggerEffect[0x02] = NormalizedFloatToByte( Force );
    return TriggerEffect;
}

FDS5TriggerEffect UDualSenseFunctionLibrary::MakeSectionResistance(
    const float StartPosition,
    const float EndPosition,
    const float Force
)
{
    FDS5TriggerEffect TriggerEffect {};
    TriggerEffect[0x00] = 0x02;
    TriggerEffect[0x01] = NormalizedFloatToByte( StartPosition );
    TriggerEffect[0x02] = NormalizedFloatToByte( EndPosition );
    TriggerEffect[0x03] = NormalizedFloatToByte( Force );
    return TriggerEffect;
}

FDS5TriggerEffect UDualSenseFunctionLibrary::MakeEffectEx(
    const float StartPosition,
    const float BeginForce,
    const float MiddleForce,
    const float EndForce,
    const float Frequency,
    const bool bShouldKeepEffect
)
{
    // This is an old implementation, use MakeVibration instead.
    
    FDS5TriggerEffect TriggerEffect {};
    TriggerEffect[0x00] = 0x26;
    TriggerEffect[0x01] = 0xFF - NormalizedFloatToByte( StartPosition );
    
    if ( bShouldKeepEffect )
    {
        TriggerEffect[0x02] = 0x02;
    }
    
    TriggerEffect[0x04] = NormalizedFloatToByte( BeginForce );
    TriggerEffect[0x05] = NormalizedFloatToByte( MiddleForce );
    TriggerEffect[0x06] = NormalizedFloatToByte( EndForce );
    
    TriggerEffect[0x09] = NormalizedFloatToByte( Frequency / 2.0f, 1 );
    return TriggerEffect;
}
