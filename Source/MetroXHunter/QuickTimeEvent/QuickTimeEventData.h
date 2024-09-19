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
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "QuickTimeEvent|Spam", meta = ( Units = "Percent", EditCondition = "EventType==EQuickTimeEventType::Spam" ) )
	float ProgressDecreasePerSecond = 200.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "QuickTimeEvent|Spam", meta = ( Units = "Percent", EditCondition = "EventType==EQuickTimeEventType::Spam" ) )
	float FailUnderProgress = -10.0f;
};
