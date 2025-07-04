/*
 * Implemented by BARRAU Benoit
 */

#pragma once

#include "Engine/DataAsset.h"
#include "ReloadData.generated.h"

 /**
  * Data asset for the reload component
  */
UCLASS()
class METROXHUNTER_API UReloadData : public UDataAsset
{
	GENERATED_BODY()

public:
	// Default reload time if the player do nothing 
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ReloadData|ReloadBase", meta = ( Units = "Seconds" ) )
	float NormalReloadDuration = 3.0f; 

	// Faster reload time
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ReloadData|ReloadPerfect", meta = ( Units = "Seconds" ) )
	float PerfectReloadDuration = 0.4f; 

	// A bit slower than perfect reload time 
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ReloadData|ReloadFailed", meta = ( Units = "Seconds" ) )
	float FailedReloadDuration = 0.5f; 

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ReloadData|ReloadCurve" )
	UCurveFloat* ReloadCurve = nullptr;
};

// To update the UI divide the cursor value with the maximum angle
// +20 to get the beginning + 110 to get the end