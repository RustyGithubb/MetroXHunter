/*
 * Implemented by BARRAU Benoit
 */

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ReloadData.generated.h"

/**
 * Reload Data Asset.
 */
UCLASS()
class METROXHUNTER_API UReloadData : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reload Data|Reload Base" )
	float NormalReloadDuration = 2.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reload Data|Reload Base" )
	float ReloadElapsedTime = 0.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reload Data|Reload Base" )
	float ReloadRefreshRate = 0.016667f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reload Data|Reload Base" )
	float NormalReloadAnimTime = 2.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reload Data|Reload Base" )
	bool isActiveReload = false;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reload Data|Reload Active" )
	float ActiveReloadStartTime = 0.55f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reload Data|Reload Active" )
	float ActiveReloadEndTime = 1.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reload Data|Reload Active" )
	float ActiveReloadAnimTime = 0.75f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reload Data|Reload Perfect" )
	float PerfectReloadStartTime = 0.4f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reload Data|Reload Perfect" )
	float PerfectReloadBonusTime = 2.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reload Data|Reload Perfect" )
	float PerfectReloadAnimTime = 0.5f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reload Data|Reload Failed" )
	float FailedReloadPenaltyTime = 2.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reload Data|Reload Curve" )
	TObjectPtr<UCurveFloat> ReloadCurve;
};
