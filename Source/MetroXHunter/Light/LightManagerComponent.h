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

	UPROPERTY( BlueprintReadWrite, Category = "LightData" )
	UMaterialInterface* OriginalLightFunction = nullptr;

	UPROPERTY( BlueprintReadWrite, Category = "LightData" )
	UMaterialInstanceDynamic* CurrentLightFunction = nullptr;

	UPROPERTY( BlueprintReadWrite, Category = "LightData" )
	float OriginalLightIntensity = 0.0f;

	UPROPERTY( BlueprintReadWrite, Category = "LightData" )
	float OriginalEmissivePower = 0.0f;
	
	UPROPERTY( BlueprintReadWrite, Category = "LightData" )
	ULightComponent* LightComponent = nullptr;

	UPROPERTY( BlueprintReadWrite, Category = "LightData" )
	float StartWorldTime = 0.0f;

	UPROPERTY( BlueprintReadWrite, Category = "LightData" )
	float TimeDuration = 0.0f;

	UPROPERTY( BlueprintReadWrite, Category = "LightData" )
	float RestorationDelay = 0.0f;

	UPROPERTY( BlueprintReadWrite, Category = "LightData" )
	UCurveVector* FlickeringLightCurve = nullptr;

	bool IsValid() const;
};

UINTERFACE( Blueprintable )
class UFlickableLight : public UInterface
{
	GENERATED_BODY()
};

/*
 * Interface for an actor deciding which one of his LightComponents can be flickered.
 * This should be implemented on the owner.
 */
class METROXHUNTER_API IFlickableLight
{
	GENERATED_BODY()

public:
	/*
	 * Called when the LightManager needs to get all flickable lights in the world.
	 * This is called once during LightManager's BeginPlay.
	 */
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "FlickableLight", meta = ( ReturnDisplayName = "LightComponents" ) )
	TArray<ULightComponent*> RetrieveFlickableLights();

	// NOTE: Forcing as function so Unreal doesn't yell at us
	//		 because it doesn't support parameters references in events

	/*
	 * Called when a light has started to flicker.
	 */
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "FlickableLight", meta = ( ForceAsFunction ) )
	void OnFlickLightStart( UPARAM( ref ) FLightData& LightData );
	/*
	 * Called when a light has stopped to flicker.
	 */
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "FlickableLight", meta = ( ForceAsFunction ) )
	void OnFlickLightStop( const FLightData& LightData );
	/*
	 * Called when the parameters of a flickering light has been updated.
	 */
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "FlickableLight", meta = ( ForceAsFunction ) )
	void OnFlickLightUpdate( UPARAM( ref ) FLightData& LightData, float IntensityScale );
	/*
	 * Called when a flickering light is ticking.
	 */
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "FlickableLight", meta = ( ForceAsFunction ) )
	void OnFlickLightTick( float DeltaTime, UPARAM( ref ) FLightData& LightData );
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
	void FlickerLightsInRadius(
		float Radius,
		const FVector& Origin,
		UCurveVector* FlickeringLightCurve
	);
	UFUNCTION( BlueprintCallable, Category = "Light" )
	void FlickerLightsInArray(
		const TArray<ULightComponent*>& Lights,
		const FVector& Origin,
		UCurveVector* FlickeringLightCurve
	);
	UFUNCTION( BlueprintCallable, Category = "Light" )
	void FlickerLights(
		const TArray<ULightComponent*>& Lights,
		float MaxDistance,
		const FVector& Origin,
		UCurveVector* FlickeringLightCurve
	);

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

	UFUNCTION( BlueprintCallable, Category = "Light" )
	void FindAllLightsInWorld();

private:
	UPROPERTY()
	TArray<ULightComponent*> DetectedLights {};

	UPROPERTY()
	TMap<ULightComponent*, FLightData> FlickeringLightsData {};
};
