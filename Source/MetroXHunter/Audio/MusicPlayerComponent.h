/* 
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MusicPlayerComponent.generated.h"

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UMusicPlayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMusicPlayerComponent();

	// Begin ActorComponent interface
	virtual void BeginPlay() override;
	// End ActorComponent interface
};
