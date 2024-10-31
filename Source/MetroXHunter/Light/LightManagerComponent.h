/*
 * Implemented by BARRAU Benoit
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Debug/TickDebugger.h"
#include "LightManagerComponent.generated.h"

class ULightComponent;
class UCurveVector;

USTRUCT( BlueprintType )
struct FLightData
{
	GENERATED_BODY()

	UPROPERTY()
	UMaterialInterface* OriginalLightFunction = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* CurrentLightFunction = nullptr;

	UPROPERTY()
	float OriginalLightIntensity = 0.0f;

	UPROPERTY()
	float OriginalEmissivePower = 0.0f;
	
	UPROPERTY()
	ULightComponent* LightComponent = nullptr;

	UPROPERTY()
	float StartWorldTime = 0.0f;

	UPROPERTY()
	float TimeDuration = 0.0f;

	UPROPERTY()
	float RestorationDelay = 0.0f;

	UPROPERTY()
	UCurveVector* FlickeringLightCurve = nullptr;

	bool IsValid() const;
};

/*
 * The light manager component get all the lights in the game.
 * It will make a free party for you with it's amazing flickerings lights.
 * It will adapt a different radius for each interaction to flicker the game lights.
 */
UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API ULightManagerComponent : public UActorComponent, public ITickDebugger
{
	GENERATED_BODY()

public:
	ULightManagerComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	void TickDebug_Implementation( float DeltaTime, FString& OutDebugText ) override;
	
	UFUNCTION( BlueprintCallable, Category = "Light" )
	void FlickeringLights( float Radius, const FVector& Origin, UCurveVector* FlickeringLightCurve );

	UFUNCTION( BlueprintCallable, Category = "Light" )
	void RegisterLight( ULightComponent* LightComponent );
	UFUNCTION( BlueprintCallable, Category = "Light" )
	void UnRegisterLight( ULightComponent* LightComponent );

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Light" )
	TArray<UMaterialInterface*> LightFunctions;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Light" )
	bool bShouldDebug = false;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Light|Flickering" )
	float RestorationTimeFactor = 0.5f; 

private:
	void FindAllLightsInWorld();

private:
	UPROPERTY()
	TArray<ULightComponent*> DetectedLights {};

	UPROPERTY()
	TMap<ULightComponent*, FLightData> FlickeringLightsData {};
};
