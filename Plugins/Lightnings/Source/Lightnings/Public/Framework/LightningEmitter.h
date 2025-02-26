#pragma once

#include "CoreMinimal.h"
#include "Framework/LightningAttachment.h"
#include "Framework/LightningPattern.h"
#include "Helpers/PatternGenerator.h"
#include "Parameters/LightningType.h"
#include "Parameters/LightningParams.h"
#include "Parameters/BranchingParams.h"
#include "Parameters/EmissionParams.h"
#include "Parameters/LightningHitReaction.h"

#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "Components/BillboardComponent.h"
#include "Engine/CollisionProfile.h"
#include "LightningEmitter.generated.h"

DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam( FRootSpawnedSignature, ALightningEmitter, EventOnRootSpawned, ALightningEffect*, Root );

class ALightningEffect;

USTRUCT()
struct FLightningSetupParams
{
	GENERATED_USTRUCT_BODY()

	// Primary params
	ALightningEffect* Root = NULL;
	FLightningPattern Pattern;
	ELightningType Type;
	int32 Order;
};

UCLASS()
class LIGHTNINGS_API ALightningEmitter : public AActor
{
	GENERATED_BODY()

	friend FLightningAttachment;

public :
	ALightningEmitter();

	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;
	virtual void OnConstruction( const FTransform& Transform ) override;
	virtual void Tick( float DeltaTime ) override;
	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override;
#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override;
	virtual void PostEditChangeProperty( FPropertyChangedEvent& PropertyChangedEvent ) override;
#endif // WITH_EDITOR

	/*
	 * Runs lightnings emitter
	 * @param Reset - calls ResetGenerator if true
	 */

public :
	UFUNCTION( NetMulticast, Reliable, BlueprintCallable, Category = "LightningEmitter" )
	void ActivateEmitter( bool bReset = false );
	void ActivateEmitter_Implementation( bool bReset = false );

	// Removes all associated lightnings and stops spawning loop if it is running
	UFUNCTION( NetMulticast, Reliable, BlueprintCallable, Category = "LightningEmitter" )
	void DeactivateEmitter();

	// Resets a random number generator for lightning particles and switches back to RNG sequence begin
	UFUNCTION( NetMulticast, Reliable, BlueprintCallable, Category = "LightningEmitter" )
	void ResetGenerator();

	// Returns a number of currently active lightnings
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "LightningEmitter" )
	int32 GetLightningsNum();

	// Returns true if there are any lightnings spawned by the emitter
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "LightningEmitter" )
	bool HasAnyLightnings();

	/*
	 * Returns true if emitter's loop is active
	 * or there are any lightnings spawned by the emitter
	 */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "LightningEmitter" )
	bool IsEmitterActive();

	// Spawns a new lightning actor with the passed pattern
	void DoSpawnLightning( ALightningEffect* Root, FLightningPattern Pattern );

	UFUNCTION( BlueprintCallable, Category = "Branching" )
	void SetBranchesLimit( FBranchingParams& BranchingParam, int32 NewLimit );

#if WITH_EDITOR
	/*
	 * Forces garbage to be collected on the next tick (for edit-mode only)
	 * It is necessary because Garbage Collector doesn't work in edit-mode
	 */
	void RequestCollectGarbage();
#endif // WITH_EDITOR

	// RUNTIME BLUEPRINTS EVENTS

public :
	// Called when a new root has been spawned
	UFUNCTION( BlueprintNativeEvent )
	void OnRootSpawned( ALightningEffect* Root );

	virtual void OnRootSpawned_Implementation( ALightningEffect* Root ) {};

	UPROPERTY( EditAnywhere, Category = "Billboards" )
	UBillboardComponent* MainBillboard = NULL;

	UPROPERTY( EditAnywhere, Category = "Billboards" )
	UBillboardComponent* SourcePoint = NULL;

	UPROPERTY( EditAnywhere, Category = "Billboards" )
	UBillboardComponent* TargetPoint = NULL;

public :
	// Auto-activate on BeginPlay
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Activation" )
	bool bAutoActivate = false;
	// Whether the emitter is looped
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "Activation" )
	bool bLoop = false;
	// Loop duration value in seconds
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "Activation", meta = ( EditCondition = "bLoop == true" ) )
	float LoopDuration = 1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Attachment" )
	FLightningAttachment SourceAttachment;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Attachment" )
	FLightningAttachment TargetAttachment;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "GeometrySettings" )
	FLightningParams LightningParams;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "GeometrySettings" )
	FBranchingParams BranchingParams;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "VisualSettings" )
	FEmissionParams RootEmissionParams;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "VisualSettings" )
	FEmissionParams BranchEmissionParams;

	/*
	 * Particles are scaled with distance to camera increasing
	 * to avoid bloom fading out when using TemporalAA.
	 * This parameter sets scaling multiplier.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "VisualSettings" )
	float DisplacementMult = 0.0015f;

	// Permits to speed up or slow down lightning spread process
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Interp, Category = "SystemSettings" )
	float TimeMultiplier = 1.0f;

	/*
	 * Use asynchronous calculation of lightning's pattern
	 * (should reduce lags but create spawning delay)
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "SystemSettings" )
	bool bUseSeparateThread = true;

	// COLLISION SETTINGS

public :
	// Whether lightnings will generate hit events
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = LightningCollision )
	bool bGenerateHitEvents = false;

	// How lightning should react on a hit event.
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = LightningCollision, meta = ( EditCondition = "bGenerateHitEvents == true" ) )
	EHitReaction HitReaction = EHitReaction::HitReact_FadeOut;

	// Whether lightnings will generate overlap events
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = LightningCollision )
	bool bGenerateOverlapEvents = false;

	// The list of collision responses for specific channels
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = LightningCollision )
	TMap< TEnumAsByte<ECollisionChannel>, TEnumAsByte<ECollisionResponse> > CollisionResponses;

	// The list of actors to ignore while collision checks
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = LightningCollision )
	TArray<AActor*> IgnoredActors;

	// DEBUG SETTINGS

public :
	// Whether draw branches of the lightning
	UPROPERTY( EditAnywhere, Category = DebugSettings )
	bool bDrawBranches = true;

	// Whether draw a lightning curve with a width specified in lightning params
	UPROPERTY( EditAnywhere, Category = DebugSettings )
	bool bDrawCurveWidth = true;

	// Whether draw a lightning curve with a color specified below
	UPROPERTY( EditAnywhere, Category = DebugSettings )
	bool bCurveCustomColor = false;

	// Lightning curve visualization color
	UPROPERTY( EditAnywhere, Category = DebugSettings, meta = ( EditCondition = "bCurveCustomColor == true" ) )
	FColor CurveColor = FColor( 255, 0, 0 );

	// Attachment volumes visualization color
	UPROPERTY( EditAnywhere, Category = DebugSettings )
	FColor AttachmentVolumesColor = FColor( 255, 255, 0 );

	// BLUEPRINTS ASSIGNABLE EVENTS

	// Called when a new root has been spawned
	UPROPERTY( BlueprintAssignable, Category = LightningEmitterEvents )
	FRootSpawnedSignature EventOnRootSpawned;

	// A storage for root-type lightning actors
	TArray<ALightningEffect*> LightningsArray;

private :
#if WITH_EDITOR
	void OnAttachmentTransform( USceneComponent* UpdatedComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport );
	void DrawDebugLightning( FLightningPattern Pattern );
#endif // WITH_EDITOR

	// @return a seed for the next lightning RNG
	int32 GetLightningSeed() const;

	/*
	 * Request generate a new pattern with the given seed, between two specific points.
	 * Then spawn a new lightning actor with the generated pattern.
	 */
	UFUNCTION( NetMulticast, Reliable )
	void RequestSpawnLightning( int32 Seed );

	/*
	 * Request generate a lightning curve (pattern) between given points.
	 * Calls pattern generator syncronously or in a separate thread (if bUseSeparateThread == true)
	 * Returns a generated pattern via the passed callback.
	 */
	void RequestGeneratePattern( FVector Start, FVector End, FPatternGeneratedEvent Callback, int32 Seed = -1 );

	// Make the preview pattern invalid
	void InvalidatePreviewPattern();

private :
	UPROPERTY( EditAnywhere, Category = SystemSettings )
	TSubclassOf<ALightningEffect> LightningEffectClass;

	/*
	 * A seed for lightnings emitter RNG
	 * The same seed means the same sequence of exactly same lightnings.
	 * -1 means use random seed
	 */
	UPROPERTY( EditAnywhere, Category = SystemSettings )
	int32 EmitterSeed = -1;

	// The timer for looped execution
	FTimerHandle LoopTimer;

	// The pattern for debug curve line
	FLightningPattern PreviewPattern;

	// Is PreviewPattern currently being generated
	bool bGeneratingPreviewPattern = false;

	// An array of pending patterns
	// Needs to be cleared on EndPlay
	TArray<FAsyncTask<FPatternGenerator>*> PendingPatterns;

	// A generator for emitter random values (lightning random seeds, attachments, etc.)
	FRandomStream EmitterRng;

#if WITH_EDITOR
	// Whether the emitter need garbage collection (works in edit-mode only)
	bool bNeedsGC = false;

	// Garbage collections variables
	float GCTimeBuffer = 0.0f;
	float GCInterval = 1.0f;
#endif // WITH_EDITOR
};
