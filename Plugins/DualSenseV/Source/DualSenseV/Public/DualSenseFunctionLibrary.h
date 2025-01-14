// /* * Implemented by Firstname Lastname (Nickname) */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InputCoreTypes.h"

#include "DualSenseTypes.h"
#include "DualSenseFunctionLibrary.generated.h"

struct DUALSENSEV_API FDualSenseKeys
{
	static inline const FKey Square = TEXT( "DualSense5_Square" );
	static inline const FKey Cross = TEXT( "DualSense5_Cross" );
	static inline const FKey Circle = TEXT( "DualSense5_Circle" );
	static inline const FKey Triangle = TEXT( "DualSense5_Triangle" );

	static inline const FKey DPadLeft = TEXT( "DualSense5_DPadLeft" );
	static inline const FKey DPadRight = TEXT( "DualSense5_DPadRight" );
	static inline const FKey DPadUp = TEXT( "DualSense5_DPadUp" );
	static inline const FKey DPadDown = TEXT( "DualSense5_DPadDown" );

	static inline const FKey L1 = TEXT( "DualSense5_L1" );
	static inline const FKey L2 = TEXT( "DualSense5_L2" );
	static inline const FKey L3 = TEXT( "DualSense5_L3" );
	static inline const FKey R1 = TEXT( "DualSense5_R1" );
	static inline const FKey R2 = TEXT( "DualSense5_R2" );
	static inline const FKey R3 = TEXT( "DualSense5_R3" );

	static inline const FKey Share = TEXT( "DualSense5_Share" );
	static inline const FKey Menu = TEXT( "DualSense5_Menu" );
	static inline const FKey PlayStation = TEXT( "DualSense5_PlayStation" );
	static inline const FKey Pad = TEXT( "DualSense5_Pad" );
	static inline const FKey Microphone = TEXT( "DualSense5_Microphone" );
};

/**
 * 
 */
UCLASS()
class DUALSENSEV_API UDualSenseFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION( BlueprintPure, Category = "DualSense" )
	static FDS5TriggerEffect MakeOff();

	UFUNCTION( BlueprintPure, Category = "DualSense" )
	static FDS5TriggerEffect MakeFeedback( float Position, float Force );
	
	UFUNCTION( BlueprintPure, Category = "DualSense" )
	static FDS5TriggerEffect MakeWeapon( float StartPosition, float EndPosition, float Force );
	
	UFUNCTION( BlueprintPure, Category = "DualSense" )
	static FDS5TriggerEffect MakeVibration( float Position, float Amplitude, float Frequency );
	
	UFUNCTION( BlueprintPure, Category = "DualSense", meta = ( DeprecatedFunction, DeprecationMessage = "This effect is not official, please use MakeFeedback instead." ) )
	static FDS5TriggerEffect MakeContinuousResistance( float StartPosition, float Force );

	UFUNCTION( BlueprintPure, Category = "DualSense", meta = ( DeprecatedFunction, DeprecationMessage = "This effect is not official, please use MakeWeapon instead." ) )
	static FDS5TriggerEffect MakeSectionResistance(
		float StartPosition,
		float EndPosition,
		float Force
	);

	UFUNCTION( BlueprintPure, Category = "DualSense", meta = ( DeprecatedFunction, DeprecatedMessage = "This effect is not official, please use MakeVibration instead." ) )
	static FDS5TriggerEffect MakeEffectEx(
		float StartPosition,
		float BeginForce,
		float MiddleForce,
		float EndForce,
		float Frequency,
		bool bShouldKeepEffect = true
	);
};
