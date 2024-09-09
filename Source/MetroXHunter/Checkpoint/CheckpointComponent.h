#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CheckpointComponent.generated.h"

class UTelemetryComponent;

/*
 *
 */
UCLASS( BlueprintType, ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UCheckpointComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCheckpointComponent();

	virtual void BeginPlay() override;

	UFUNCTION( BlueprintCallable, Category = "Checkpoint" )
	void SaveCheckpoint(
		const FVector& NewCheckpointLocation,
		const FRotator& NewCheckpointRotation,
		const FString& NewCheckpointName
	);

public:
	UPROPERTY( EditAnywhere, Category = "Checkpoint" )
	TEnumAsByte<ETraceTypeQuery> RelocationTraceQuery;

	UPROPERTY( BlueprintReadOnly, Category = "Checkpoint" )
	FVector LastCheckpointLocation {};
	UPROPERTY( BlueprintReadOnly, Category = "Checkpoint" )
	FRotator LastCheckpointRotation {};
	UPROPERTY( BlueprintReadOnly, Category = "Checkpoint" )
	FString LastCheckpointName {};

private:
	float CharacterCapsuleHalfHeight = 0.0f;
	float CharacterCapsuleRadius = 0.0f;

	UTelemetryComponent* TelemetryComponent = nullptr;
};
