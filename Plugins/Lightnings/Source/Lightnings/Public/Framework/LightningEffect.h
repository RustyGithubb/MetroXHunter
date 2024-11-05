#pragma once

#include "CoreMinimal.h"
#include "Framework/LightningEmitter.h"
#include "Framework/LightningPattern.h"

#include "Components/BillboardComponent.h"
#include "NiagaraComponent.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"
#include "LightningEffect.generated.h"

class ALightningEffect;
class UNiagaraDataInterface;

DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam( FLightningSpawnedSignature, ALightningEffect, EventOnLightningSpawned, ALightningEffect*, Lightning );
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_ThreeParams( FLightningSparkMoveSignature, ALightningEffect, EventOnLightningSparkMove, ALightningEffect*, Lightning, FVector, Start, FVector, End );
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam( FLightningSparkEndMoveSignature, ALightningEffect, EventOnLightningSparkEndMove, ALightningEffect*, Lightning );
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_ThreeParams( FLightningBranchedSignature, ALightningEffect, EventOnLightningBranched, ALightningEffect*, Lightning, ALightningEffect*, Branch, int32, BranchOrder );
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams( FIntensityChangedSignature, ALightningEffect, EventOnIntensityChanged, ALightningEffect*, Lightning, float, Intensity );
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams( FLightningOverlapSignature, ALightningEffect, EventOnLightningOverlap, ALightningEffect*, Lightning, const TArray<FHitResult>&, Overlaps );
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams( FLightningHitSignature, ALightningEffect, EventOnLightningHit, ALightningEffect*, Lightning, FHitResult, Hit );

USTRUCT( BlueprintType )
struct FLightningIntensityParams
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY( BlueprintReadOnly, Category = LightningIntensity )
	float SparkIntensity = 0.f;
	UPROPERTY( BlueprintReadOnly, Category = LightningIntensity )
	float TrailIntensity = 0.f;
	UPROPERTY( BlueprintReadOnly, Category = LightningIntensity )
	float BoltIntensity = 0.f;
};

UCLASS()
class LIGHTNINGS_API ALightningEffect : public AActor
{
	GENERATED_BODY()

public:
	ALightningEffect();

	// AActor interface

	virtual void Tick( float DeltaTime ) override;
#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override;
#endif // WITH_EDITOR

	// User blueprint callable functions

	// Returns an actual pattern object (a copy)
	UFUNCTION( BlueprintPure, Category = Lightning )
	FLightningPattern GetPattern() const;

	// Check whether this lightning is a root
	UFUNCTION( BlueprintPure, Category = Lightning )
	bool IsRoot() const;

	// Get lightning's order (0 if root, >0 for branches)
	UFUNCTION( BlueprintPure, Category = Lightning )
	int32 GetOrder() const;

	// Stop lightning growth and run discharge animation (works only if root)
	UFUNCTION( BlueprintCallable, Category = Lightning )
	void Discharge();

	// Stop lightning growth and fade it out
	UFUNCTION( BlueprintCallable, Category = Lightning )
	void FadeOut();

	// Fills lightning's params and activates the lightning
	void Setup( ALightningEmitter* Emitter, FLightningSetupParams SetupParams );

	// Flashing timeline function (Update)
	UFUNCTION()
	void OnBoltFlashing( float Alpha );

	// Flashing timeline function (Finish)
	UFUNCTION()
	void Deactivate();

	/*
	 * Check Start-End line to any collisions
	 *
	 * @param Start - The first point for the line
	 * @param End - The second point for the line
	 * @param Response - The required collision response
	 * @param MaxResults - A limit of collisions to check ((MaxResults < 0) == infinite)
	 * @param OutHits - A result array reference
	 *
	 * @return true if there were any collisions
	 */
	UFUNCTION( BlueprintCallable, Category = Lightning )
	bool DoCollisionCheck( const FVector& Start, const FVector& End, ECollisionResponse Response, int32 MaxResults, TArray<FHitResult>& OutHits );

	// RUNTIME BLUEPRINT EVENTS

public :
	// Called when lightning is setup with emitter params (analog for BeginPlay event)
	UFUNCTION( BlueprintNativeEvent )
	void OnLightningSpawned();

	// Called when lightning spark has moved between Start and End points
	UFUNCTION( BlueprintNativeEvent )
	void OnLightningSparkMove( FVector Start, FVector End );

	// Called when lightning spark has reached Target Attachment point (lightning is fully formed)
	UFUNCTION( BlueprintNativeEvent )
	void OnLightningSparkEndMove();

	/*
	 * Called when lightning is branched
	 * @param Branch - new spawned lightning effect actor of the branch;
	 * @param Order - branch order
	 */
	UFUNCTION( BlueprintNativeEvent )
	void OnLightningBranched( ALightningEffect* Branch, int32 BranchOrder );

	// Called when intensity of lightning is changed (when the lightning is flashing for example)
	UFUNCTION( BlueprintNativeEvent )
	void OnIntensityChanged( float Intensity );

	// Called when a lightning overlaps an actor (or several) in the world
	UFUNCTION( BlueprintNativeEvent )
	void OnLightningOverlap( const TArray<FHitResult>& Overlaps );

	// Called when a lightning hits an actor in the world
	UFUNCTION( BlueprintNativeEvent )
	void OnLightningHit( FHitResult Hit );

	virtual void OnLightningSpawned_Implementation() {};
	virtual void OnLightningGenerated_Implementation() {};
	virtual void OnLightningSparkMove_Implementation( FVector Start, FVector End ) {};
	virtual void OnLightningSparkEndMove_Implementation() {};
	virtual void OnLightningBranched_Implementation( ALightningEffect* Branch, int32 BranchOrder ) {};
	virtual void OnIntensityChanged_Implementation( float Intensity ) {};
	virtual void OnLightningOverlap_Implementation( const TArray<FHitResult>& Overlaps ) {};
	virtual void OnLightningHit_Implementation( FHitResult Hit ) {};

public :
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = Components )
	UNiagaraComponent* NiagaraComponent = NULL;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Components )
	UBillboardComponent* Billboard = NULL;

	UPROPERTY( BlueprintReadOnly, Category = LightningParams )
	ALightningEmitter* LightningEmitter = NULL;

	UPROPERTY( BlueprintReadOnly, Category = LightningParams )
	ALightningEffect* Root = NULL;

	UPROPERTY( BlueprintReadOnly, Category = LightningParams )
	float Growth = 0.f;

	UPROPERTY( BlueprintReadOnly, Category = LightningParams )
	bool bActive = false;

	UPROPERTY( BlueprintReadOnly, Category = LightningParams )
	bool bIsFadingOut = false;

	UPROPERTY( BlueprintReadOnly, Category = LightningParams )
	FLightningParams LightningParams;

	// Intensity params (default/current)
	UPROPERTY( BlueprintReadOnly, Category = LightningParams )
	FLightningIntensityParams IntensityDef;
	UPROPERTY( BlueprintReadOnly, Category = LightningParams )
	FLightningIntensityParams IntensityCur;

	// BLUEPRINT ASSIGNABLE EVENTS

public :
	// Called when lightning is setup with emitter params (analog for BeginPlay event)
	UPROPERTY( BlueprintAssignable, Category = LightningEvents )
	FLightningSpawnedSignature EventOnLightningSpawned;

	// Called when lightning spark has moved between Start and End points
	UPROPERTY( BlueprintAssignable, Category = LightningEvents )
	FLightningSparkMoveSignature EventOnLightningSparkMove;

	// Called when lightning spark has reached Target Attachment point (lightning is fully formed)
	UPROPERTY( BlueprintAssignable, Category = LightningEvents )
	FLightningSparkEndMoveSignature EventOnLightningSparkEndMove;

	// Called when lightning is branched
	UPROPERTY( BlueprintAssignable, Category = LightningEvents )
	FLightningBranchedSignature EventOnLightningBranched;

	// Called when intensity of lightning is changed (when the lightning is flashing for example)
	UPROPERTY( BlueprintAssignable, Category = LightningEvents )
	FIntensityChangedSignature EventOnIntensityChanged;

	// Called when a lightning overlaps an actor (or several) in the world
	UPROPERTY( BlueprintAssignable, Category = LightningEvents )
	FLightningOverlapSignature EventOnLightningOverlap;

	// Called when a lightning hits an actor in the world
	UPROPERTY( BlueprintAssignable, Category = LightningEvents )
	FLightningHitSignature EventOnLightningHit;

public :
	// The lightning pattern
	FLightningPattern LightningPattern;

	/*
	 * A list of initialized branches.
	 * Always empty for non-root lightning actors.
	 */
	TArray<ALightningEffect*> Branches;

private:
	void OnTargetPosReached();
	void UpdateColorMultiplier();

	/*
	 * Set a niagara data interface by name.
	 * A new interface replaces the old one in override parameters.
	 *
	 * @param Name - a target interface parameter name
	 * @param DataInterface - a new instance of the interface
	 */
	void SetNiagaraDataInterface( const FString& Name, UNiagaraDataInterface* DataInterface );

	void SetNiagaraVariableFloatCurve( FName Name, FRichCurve Curve );
	void SetNiagaraVariableVector3Curve( FName Name, FRichCurve CurveX, FRichCurve CurveY, FRichCurve CurveZ );

	// Update spark position and growth
	void UpdateLifetime( float DeltaTime );

	// Update intensity on fade
	void UpdateFading( float DeltaTime );

	/*
	 * Moves lightning spark from Start to End,
	 * checking hits and overlaps and calling blueprint events.
	 *
	 * @param Start - beginning position of the spark movement, in world space
	 * @param End - ending position of the spark movement, in world space
	 * @return true if there is no blocking hit between Start and End
	 */
	bool DoSparkMovement( const FVector& Start, const FVector& End );

	/*
	 * Invalidates pattern, stops lightning growth and calls a blueprint event.
	 *
	 * @param bDischarge - run discharge (flashing) if true; fade out otherwise
	 */
	void StopSparkMovement( bool bDischarge );

private :
	// Timeline that is used for discharge animation
	FTimeline FlashingTimeline;

	// Local pseudo-random numbers generator
	FRandomStream Rnd;

	ELightningType LightningType;
	int32 LightningOrder = 0;
};
