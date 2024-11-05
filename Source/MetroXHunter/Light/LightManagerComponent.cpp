#include "LightManagerComponent.h"

#include "Curves/CurveVector.h"
#include "Components/LightComponent.h"
#include "Components/RectLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "DrawDebugHelpers.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "UtilityLibrary.h"

constexpr auto EMISSIVE_POWER_NAME = TEXT( "Emissive Power" );
constexpr auto FLICKERING_TIME_NAME = TEXT( "Flickering Time" );


bool FLightData::IsValid() const
{
	return ::IsValid( LightComponent )
		&& ::IsValid( CurrentLightFunction )
		&& ::IsValid( FlickeringLightCurve );
}


ULightManagerComponent::ULightManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULightManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	FindAllLightsInWorld();
}

void ULightManagerComponent::TickComponent( float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	if ( FlickeringLightsData.IsEmpty() ) return;

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	//for ( int32 Index = FlickeringLightsData.Num() - 1; Index >= 0; Index-- )
	for ( auto Itr = FlickeringLightsData.CreateIterator(); Itr; ++Itr )
	{
		FLightData& LightData = Itr.Value();

		if ( !LightData.IsValid() )
		{
			// Notify owner of light flickering end only if accessible
			if ( IsValid( LightData.LightComponent ) )
			{
				AActor* Owner = LightData.LightComponent->GetOwner();
				if ( IsValid( Owner ) )
				{
					IFlickableLight::Execute_OnFlickLightStop( Owner, LightData );
				}
			}

			Itr.RemoveCurrent();
			continue;
		}

		AActor* Owner = LightData.LightComponent->GetOwner();

		const float ElapsedTime = CurrentTime - LightData.StartWorldTime;
		if ( ElapsedTime >= LightData.TimeDuration + LightData.RestorationDelay )
		{
			// Restore light intensity and light function
			LightData.LightComponent->SetLightFunctionMaterial( LightData.OriginalLightFunction );
			LightData.LightComponent->SetIntensity( LightData.OriginalLightIntensity );

			UUtilityLibrary::LogMessage(
				TEXT( "Restored original light function for: %s" ),
				*LightData.LightComponent->GetName()
			);

			// Notify owner of light flickering en
			IFlickableLight::Execute_OnFlickLightStop( Owner, LightData );

			Itr.RemoveCurrent();
			continue;
		}
		else if ( ElapsedTime < LightData.TimeDuration )
		{
			// Update light's intensity and light function's emissive and flickering params
			const FVector CurveValue = LightData.FlickeringLightCurve->GetVectorValue( ElapsedTime );
			const float IntensityScale = CurveValue.X;
			const float FlickeringTime = CurveValue.Y;

			LightData.LightComponent->SetIntensity( LightData.OriginalLightIntensity * IntensityScale );
			LightData.CurrentLightFunction->SetScalarParameterValue(
				EMISSIVE_POWER_NAME,
				LightData.OriginalEmissivePower * IntensityScale
			);
			LightData.CurrentLightFunction->SetScalarParameterValue(
				FLICKERING_TIME_NAME,
				FlickeringTime
			);

			IFlickableLight::Execute_OnFlickLightUpdate( Owner, LightData, IntensityScale );
		}

		// Notify owner of light flickering tick
		IFlickableLight::Execute_OnFlickLightTick( Owner, DeltaTime, LightData );
	}
}

void ULightManagerComponent::TickDebug_Implementation( float DeltaTime, FString& OutDebugText )
{
	const float CurrentWorldTime = GetWorld()->GetTimeSeconds();

	FString FlickeringLightsString = "";
	for ( const auto& Pair : FlickeringLightsData )
	{
		const FLightData& LightData = Pair.Value;
		FlickeringLightsString += "- " + GetNameSafe( LightData.LightComponent ) + ": "
			+ "ElapsedTime=" + FString::SanitizeFloat( CurrentWorldTime - LightData.StartWorldTime ) + "s; "
			+ "TimeDuration=" + FString::SanitizeFloat( LightData.TimeDuration ) + "s; "
			+ "RestorationDelay=" + FString::SanitizeFloat( LightData.RestorationDelay ) + "s; "
			+ "OriginalIntensity=" + FString::SanitizeFloat( LightData.OriginalLightIntensity ) + "; "
			+ "\n";
	}

	// Constructs formating arguments
	FStringFormatNamedArguments Args {};
	Args.Add( "WorldLightsCount", DetectedLights.Num() );
	Args.Add( "LightsData", FlickeringLightsString );
	Args.Add( "LightsDataCount", FlickeringLightsData.Num() );

	// Formats debug string
	constexpr auto Format = TEXT(
		"[LightManagerComponent]\n"
		"WorldLights[{WorldLightsCount}]\n"
		"LightsData[{LightsDataCount}]:\n"
		"{LightsData}\n"
	);
	OutDebugText = FString::Format( Format, Args );
}

void ULightManagerComponent::FlickeringLights(
	float Radius,
	const FVector& Origin,
	UCurveVector* FlickeringLightCurve
)
{
	if ( LightFunctions.IsEmpty() || !IsValid( FlickeringLightCurve ) ) return;

	const float StartWorldTime = GetWorld()->GetTimeSeconds();

	float MinCurveTime = 0.0f, MaxCurveTime = 0.0f;
	FlickeringLightCurve->GetTimeRange( MinCurveTime, MaxCurveTime );

	if ( bShouldDebug )
	{
		DrawDebugSphere( GetWorld(), Origin, Radius, 32, FColor::Green, false, 5.0f, 0, 2.0f );
	}

	// Compute maximum distance out of all potential lights
	float MaxDistance = 0.0f;
	for ( auto Itr = DetectedLights.CreateIterator(); Itr; ++Itr )
	{
		const ULightComponent* LightComponent = *Itr;

		// Remove invalid light components
		if ( !IsValid( LightComponent ) )
		{
			Itr.RemoveCurrentSwap();
			continue;
		}

		float Distance = FVector::Dist( Origin, LightComponent->GetComponentLocation() );
		if ( Distance > Radius ) continue;

		MaxDistance = FMath::Max( MaxDistance, Distance );
	}

	// Flick all lights passing conditions
	for ( ULightComponent* LightComponent : DetectedLights )
	{
		float Distance = FVector::Dist( Origin, LightComponent->GetComponentLocation() );
		if ( Distance > Radius ) continue;

		// First try to find an existing LightData to the LightComponent
		FLightData* LightData = FlickeringLightsData.Find( LightComponent );
		if ( LightData != nullptr )
		{
			UUtilityLibrary::LogMessage(
				TEXT( "LightManager: Edit previous flickering light %s" ),
				*LightComponent->GetName()
			);
		}
		// Else create one
		else
		{
			FLightData NewLightData {};
			NewLightData.LightComponent = LightComponent;
			NewLightData.OriginalLightFunction = LightComponent->LightFunctionMaterial;
			NewLightData.OriginalLightIntensity = LightComponent->Intensity;

			// Create a dynamic material instance of the LightFunction for the flickering effect
			auto NewLightFunction = UMaterialInstanceDynamic::Create(
				UUtilityLibrary::PickRandomElement( LightFunctions ),
				nullptr
			);
			verifyf(
				IsValid( NewLightFunction ),
				TEXT( "Failed to create a MaterialInstanceDynamic for the Light Function of %s" ),
				*LightComponent->GetName()
			);

			NewLightData.CurrentLightFunction = NewLightFunction;
			NewLightFunction->GetScalarParameterValue(
				EMISSIVE_POWER_NAME,
				NewLightData.OriginalEmissivePower
			);
			LightComponent->SetLightFunctionMaterial( NewLightFunction );

			// Add light data to the map and use the pointer to the new inserted value
			// for later use.
			LightData = &FlickeringLightsData.Add( LightComponent, NewLightData );

			UUtilityLibrary::LogMessage(
				TEXT( "LightManager: New flickering light %s" ),
				*LightComponent->GetName()
			);
		}

		LightData->StartWorldTime = StartWorldTime;
		LightData->TimeDuration = MaxCurveTime;
		LightData->FlickeringLightCurve = FlickeringLightCurve;

		// Apply a delay before restoring lights based on distance from origin
		float RestorationFactor = Distance / MaxDistance;
		LightData->RestorationDelay = RestorationFactor * MaxCurveTime * RestorationTimeFactor;

		IFlickableLight::Execute_OnFlickLightStart( LightData->LightComponent->GetOwner(), *LightData );
	}
}

void ULightManagerComponent::RegisterLight( ULightComponent* LightComponent )
{
	// Ignore static lights
	if ( LightComponent->Mobility == EComponentMobility::Static )
	{
		UUtilityLibrary::LogWarning(
			TEXT( "LightManager: Failed to register %s: static lights are not supported!" ),
			*LightComponent->GetName()
		);
		return;
	}
	// Ignore rect light component as they are not supported for LightFunctions
	if ( LightComponent->IsA<URectLightComponent>() )
	{
		UUtilityLibrary::LogWarning(
			TEXT( "LightManager: Failed to register %s: RectLightComponent are not supported!" ),
			*LightComponent->GetName()
		);
		return;
	}
	// Ignore directional light component as we don't want to control the actual sun
	if ( LightComponent->IsA<UDirectionalLightComponent>() )
	{
		UUtilityLibrary::LogWarning(
			TEXT( "LightManager: Failed to register %s: DirectionalLightComponent are not supported!" ),
			*LightComponent->GetName()
		);
		return;
	}

	DetectedLights.Add( LightComponent );
}

void ULightManagerComponent::UnRegisterLight( ULightComponent* LightComponent )
{
	DetectedLights.Remove( LightComponent );
}

void ULightManagerComponent::FindAllLightsInWorld()
{
	DetectedLights.Empty();

	// Iterate over all actors in the world and filter by interface
	for ( TActorIterator<AActor> ActorItr( GetWorld() ); ActorItr; ++ActorItr )
	{
		AActor* Actor = *ActorItr;
		if ( !Actor->Implements<UFlickableLight>() ) continue;

		TArray<ULightComponent*> LightComponents = IFlickableLight::Execute_RetrieveFlickableLights( Actor );

		for ( auto LightComponent : LightComponents )
		{
			RegisterLight( LightComponent );
		}

		/*Actor->ForEachComponent<ULightComponent>(
			false,
			[this]( auto LightComponent )
			{
				RegisterLight( LightComponent );
			}
		);*/
	}

	if ( DetectedLights.IsEmpty() )
	{
		UUtilityLibrary::LogWarning( TEXT( "LightManager: No LightComponents have been found in the world!" ) );
	}
	else
	{
		UUtilityLibrary::LogMessage( TEXT( "LightManager: Registered %d LightComponents" ), DetectedLights.Num() );
	}
}
