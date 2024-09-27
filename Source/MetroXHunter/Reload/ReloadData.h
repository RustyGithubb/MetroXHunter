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
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ReloadData|ReloadBase" )
	float NormalReloadDuration = 2.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ReloadData|ReloadBase" )
	float NormalReloadAnimTime = 2.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ReloadData|ReloadActive" )
	float ActiveReloadStartTime = 0.55f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ReloadData|ReloadActive" )
	float ActiveReloadEndTime = 1.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ReloadData|ReloadActive" )
	float ActiveReloadAnimTime = 0.75f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ReloadData|ReloadPerfect" )
	float PerfectReloadStartTime = 0.4f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ReloadData|ReloadPerfect" )
	float PerfectReloadBonusTime = 2.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ReloadData|ReloadPerfect" )
	float PerfectReloadAnimTime = 0.5f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ReloadData|ReloadFailed" )
	float FailedReloadPenaltyTime = 2.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ReloadData|ReloadCurve" )
	UCurveFloat* ReloadCurve = nullptr;
};
