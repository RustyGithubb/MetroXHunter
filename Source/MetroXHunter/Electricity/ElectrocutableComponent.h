/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ElectrocutableComponent.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

/*
 * Component responsible for handling electricity.
 * It provides easy-to-use features for giggling skeletal meshes and attaching FX
 * to scene components.
 */
UCLASS( Blueprintable, ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UElectrocutableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UElectrocutableComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay( EEndPlayReason::Type Reason ) override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	UFUNCTION( BlueprintCallable, Category = "Electrocutable" )
	void Electrocute( float Duration );
	UFUNCTION( BlueprintCallable, Category = "Electrocutable" )
	void StopElectrocution();

	UFUNCTION( BlueprintPure, Category = "Electrocutable" )
	bool IsElectrocuting() const;

	UFUNCTION( BlueprintCallable, Category = "Electrocutable" )
	void UpdateFX();
	UFUNCTION( BlueprintCallable, Category = "Electrocutable" )
	void DestroyFX();

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnElectricStart, float, Duration );
	UPROPERTY( BlueprintAssignable, Category = "Electrocutable" )
	FOnElectricStart OnElectricStart {};

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnElectricTick, float, DeltaTime );
	UPROPERTY( BlueprintAssignable, Category = "Electrocutable" )
	FOnElectricTick OnElectricTick {};

	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnElectricEnd );
	UPROPERTY( BlueprintAssignable, Category = "Electrocutable" )
	FOnElectricEnd OnElectricEnd {};

public:
	UPROPERTY( BlueprintReadWrite, Category = "Electrocutable" )
	TArray<USceneComponent*> FXAttachmentComponents {};

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Electrocutable" )
	UNiagaraSystem* LightningFX = nullptr;

	UPROPERTY( BlueprintReadWrite, Category = "Electrocutable" )
	TArray<USkeletalMeshComponent*> GigglingSkeletalComponents {};

	/*
	 * Angular velocity force for giggling.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Electrocutable", meta = ( Units = "DegreesPerSecond" ) )
	float GigglingForce = 250.0f;

	/*
	 * Set whenever the "giggling" angular velocity should accumulate over time or not.
	 * You better not set this to true or meshes will fly around.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Electrocutable" )
	bool bGigglingAccumulateForce = false;

	/*
	 * Set whether the component should automatically fill the lists with the skeletal meshes
	 * of its owner.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Electrocutable" )
	bool bAutoFindSkeletalMeshes = false;

private:
	UPROPERTY()
	TArray<UNiagaraComponent*> FXPlayingComponents {};

	bool bIsFXPlaying = false;

	float ElectrocutionStartWorldTime = 0.0f;
	float ElectrocutionDuration = 0.0f;
};
