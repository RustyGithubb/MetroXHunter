/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "Components/ActorComponent.h"
#include "EnhancedInputSubsystems.h"
#include "QuickTimeEventComponent.generated.h"

class UQuickTimeEventData;
class UInputAction;

/*
 * Represent the result of the quick time event.
 */
UENUM( BlueprintType )
enum class EQuickTimeEventResult : uint8
{
	/*
	 * The player succeed the event.
	 */
	Succeed,
	/*
	 * The player failed the event.
	 */
	Failed,
};

/*
 * Component responsible for the quick time event system.
 * Designed to be added on the player character as it needs its controller to bind the inputs.
 */
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

	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent" )
	bool IsEventRunning() const;
	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent" )
	float GetEventTime() const;
	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent" )
	float GetProgressDecreasePerSecond() const;
	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent" )
	EQuickTimeEventResult GetEventResult() const;
	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent" )
	UQuickTimeEventData* GetEventDataAsset() const;
	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent" )
	float GetInputProgress() const;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnEventStarted, UQuickTimeEventComponent*, Component, UQuickTimeEventData*, DataAsset );
	UPROPERTY( BlueprintAssignable, Category = "QuickTimeEvent" )
	FOnEventStarted OnEventStarted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams( FOnEventStopped, UQuickTimeEventComponent*, Component, UQuickTimeEventData*, DataAsset, EQuickTimeEventResult, EventResult );
	UPROPERTY( BlueprintAssignable, Category = "QuickTimeEvent" )
	FOnEventStopped OnEventStopped;

public:
	/*
	 * Input mapping context to enable during the event.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "QuickTimeEvent" )
	TSoftObjectPtr<UInputMappingContext> InputMappingContext = nullptr;

private:
	void SetupPlayerInputComponent();

	void OnInput( const FInputActionInstance& InputInstance );

private:
	APlayerController* PlayerController = nullptr;
	UQuickTimeEventData* DataAsset = nullptr;

	float InputProgress = 0.0f;
	float EventStartTime = 0.0f;
	float DecreaseOffsetTime = 0.0f;

	EQuickTimeEventResult Result = EQuickTimeEventResult::Succeed;

	bool bIsInDeadZone = false;
};