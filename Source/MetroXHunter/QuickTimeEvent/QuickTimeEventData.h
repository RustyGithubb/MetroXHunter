/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "Engine/DataAsset.h"
#include "QuickTimeEventData.generated.h"

/*
 * Represent the type of quick time event to use.
 */
UENUM( BlueprintType )
enum class EQuickTimeEventType : uint8
{
	/*
	 * An input must be pressed repetitively to succeed.
	 */
	Spam,
	/*
	 * A series of inputs must be pressed on the right time in the right order to succeed.
	 */
	Sequence,
};

/*
 * Data asset representing all info a quick time event needs to run.
 */
UCLASS()
class METROXHUNTER_API UQuickTimeEventData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "QuickTimeEvent" )
	EQuickTimeEventType EventType = EQuickTimeEventType::Spam;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "QuickTimeEvent|Spam", meta = ( Units = "Percent", EditCondition = "EventType==EQuickTimeEventType::Spam" ) )
	float StartProgress = 50.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "QuickTimeEvent|Spam", meta = ( Units = "Percent", EditCondition = "EventType==EQuickTimeEventType::Spam" ) )
	float ProgressPerInput = 15.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "QuickTimeEvent|Spam", meta = ( Units = "Percent", EditCondition = "EventType==EQuickTimeEventType::Spam && ProgressDecreaseCurve==nullptr" ) )
	float ProgressDecreasePerSecond = 200.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "QuickTimeEvent|Spam", meta = ( EditCondition = "EventType==EQuickTimeEventType::Spam" ) )
	UCurveFloat* ProgressDecreaseCurve = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "QuickTimeEvent|Spam", meta = ( Units = "Percent", EditCondition = "EventType==EQuickTimeEventType::Spam" ) )
	float FailUnderProgress = -10.0f;
	/*
	 * Deprecated: now we automatically link the input action to the data table
	 * using the UInputAction asset name.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "QuickTimeEvent|Spam", meta = ( EditCondition = "EventType==EQuickTimeEventType::Spam && false" ) )
	FDataTableRowHandle InputUIRowHandle {};
	/*
	 * Input action to spam during the event, it must be present in the InputMappingContext 
	 * of the component.
	 * 
	 * Important: We automatically use the asset name of the input action to find the icon
	 * to show using CommonUI. Therefore, the input action name must match one of the row
	 * inside the CommonUI action data table (minus the 'IA_' prefix).
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "QuickTimeEvent|Spam", meta = ( EditCondition = "EventType==EQuickTimeEventType::Spam" ) )
	TSoftObjectPtr<UInputAction> InputAction;
};
