/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SubtitleComponent.generated.h"

 /*
  *
  */
UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API USubtitleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USubtitleComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;
};
