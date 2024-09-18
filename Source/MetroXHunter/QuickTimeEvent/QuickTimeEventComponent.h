/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "LatentActions.h"
#include "Components/ActorComponent.h"
#include "QuickTimeEventComponent.generated.h"

class UQuickTimeEventData;
class UInputAction;

UENUM( BlueprintType )
enum class EQuickTimeEventResult : uint8
{
	Succeed,
	Failed,
};

UENUM()
enum class EQuickTimeEventOutputPins : uint8
{
	OnSucceed,
	OnFailed,
};

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UQuickTimeEventComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuickTimeEventComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent" )
	void StartEvent( UQuickTimeEventData* NewDataAsset );
	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent" )
	void StopEvent( EQuickTimeEventResult EventResult );

	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent", meta = ( WorldContext = "WorldContext", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "OutputPins" ) )
	static void LatentQuickTimeEvent(
		UObject* WorldContext,
		FLatentActionInfo LatentInfo,
		EQuickTimeEventOutputPins& OutputPins,
		UQuickTimeEventComponent* Component,
		UQuickTimeEventData* DataAsset
	);

	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent" )
	bool IsEventRunning() const;
	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent" )
	EQuickTimeEventResult GetEventResult() const;
	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent" )
	float GetInputProgress() const;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnEventStarted );
	UPROPERTY( BlueprintAssignable, Category = "QuickTimeEvent" )
	FOnEventStarted OnEventStarted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnEventStopped, EQuickTimeEventResult, EventResult );
	UPROPERTY( BlueprintAssignable, Category = "QuickTimeEvent" )
	FOnEventStopped OnEventStopped;

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "QuickTimeEvent" )
	UInputAction* InputAction = nullptr;

private:
	void SetupPlayerInputComponent();

	void OnInput();

private:
	EQuickTimeEventResult Result = EQuickTimeEventResult::Succeed;
	APlayerController* PlayerController = nullptr;

	UQuickTimeEventData* DataAsset = nullptr;
	float InputProgress = 0.0f;
};

class FLatentQuickTimeEvent : public FPendingLatentAction
{
public:
	FLatentQuickTimeEvent(
		FLatentActionInfo& LatentInfo, EQuickTimeEventOutputPins& OutputPins,
		UQuickTimeEventComponent* Component,
		UQuickTimeEventData* DataAsset
	)
		: LatentInfo( LatentInfo ), OutputPins( OutputPins ),
		  Component( Component ), DataAsset( DataAsset )
	{}

	virtual void UpdateOperation( FLatentResponse& Response ) override;

public:
	bool bIsInitialized = false;

	FLatentActionInfo LatentInfo {};
	EQuickTimeEventOutputPins& OutputPins;

	UQuickTimeEventComponent* Component = nullptr;
	UQuickTimeEventData* DataAsset = nullptr;
};