#include "Framework/LightningEmitter.h"
#include "Framework/LightningEffect.h"

#include "DrawDebugHelpers.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

#if WITH_EDITOR
#include "LevelEditor.h"
#include "IAssetViewport.h"
#include "PropertyEditorModule.h"
#endif // WITH_EDITOR

ALightningEmitter::ALightningEmitter()
{
	// Load assets for billboards and lightning curves
	static ConstructorHelpers::FObjectFinder<UTexture2D> MainBillboardSpriteAsset(
		TEXT( "'/Lightnings/Textures/T_Billboard.T_Billboard'" ) );
	static ConstructorHelpers::FObjectFinder<UTexture2D> SourcePointSpriteAsset(
		TEXT( "'/Lightnings/Textures/T_Billboard_Source.T_Billboard_Source'" ) );
	static ConstructorHelpers::FObjectFinder<UTexture2D> TargetPointSpriteAsset(
		TEXT( "'/Lightnings/Textures/T_Billboard_Target.T_Billboard_Target'" ) );

	static ConstructorHelpers::FObjectFinder<UCurveFloat> LightningWidthCurveAsset(
		TEXT( "'/Lightnings/Curves/Default/C_DefaultLightningOriginWidth.C_DefaultLightningOriginWidth'" ) );
	verify( LightningWidthCurveAsset.Succeeded() && "Failed to find the LightningWidthCurveAsset" );

	static ConstructorHelpers::FObjectFinder<UCurveFloat> LightningFlashingCurveAsset(
		TEXT( "'/Lightnings/Curves/Default/C_DefaultBoltFlashing.C_DefaultBoltFlashing'" ) );
	verify( LightningFlashingCurveAsset.Succeeded() && "Failed to find the LightningFlashingCurveAsset" );

	// Initialize billboard components for visualization
	MainBillboard = CreateDefaultSubobject<UBillboardComponent>( "MainBillboard" );
	SourcePoint = CreateDefaultSubobject<UBillboardComponent>( "SourcePoint" );
	TargetPoint = CreateDefaultSubobject<UBillboardComponent>( "TargetPoint" );

	// Set the main billboard as the root component
	SetRootComponent( MainBillboard );
	SourcePoint->SetupAttachment( MainBillboard );
	TargetPoint->SetupAttachment( MainBillboard );

	// Set sprites for the billboards
	MainBillboard->SetSprite( MainBillboardSpriteAsset.Object );
	SourcePoint->SetSprite( SourcePointSpriteAsset.Object );
	TargetPoint->SetSprite( TargetPointSpriteAsset.Object );

	// Configure billboard scaling and screen size
	MainBillboard->bIsScreenSizeScaled = true;
	SourcePoint->bIsScreenSizeScaled = true;
	TargetPoint->bIsScreenSizeScaled = true;

	MainBillboard->SetRelativeScale3D( FVector( 0.5f ) );
	SourcePoint->SetRelativeScale3D( FVector( 0.5f ) );
	TargetPoint->SetRelativeScale3D( FVector( 0.5f ) );

	MainBillboard->ScreenSize = 0.0015f;
	SourcePoint->ScreenSize = 0.0015f;
	TargetPoint->ScreenSize = 0.0015f;

#if WITH_EDITOR
	// Bind transform update events in editor mode
	SourcePoint->TransformUpdated.AddUObject( this, &ALightningEmitter::OnAttachmentTransform );
	TargetPoint->TransformUpdated.AddUObject( this, &ALightningEmitter::OnAttachmentTransform );
#endif // WITH_EDITOR

	// Enable ticking and set default tick behavior
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Set default lightning effect class
	LightningEffectClass = ALightningEffect::StaticClass();

	// Set default lightning parameters
	LightningParams.OriginParticlesWidth = LightningWidthCurveAsset.Object;
	LightningParams.BranchParticlesWidth = LightningWidthCurveAsset.Object;
	LightningParams.FlashingCurve = LightningFlashingCurveAsset.Object;

	// Configure collision responses to ignore unnecessary channels
	CollisionResponses =
	{
		{ECC_Destructible, ECR_Ignore},
		{ECC_Vehicle, ECR_Ignore},
		{ECC_PhysicsBody, ECR_Ignore},
		{ECC_Camera, ECR_Ignore},
		{ECC_Visibility, ECR_Ignore},
		{ECC_Pawn, ECR_Ignore},
		{ECC_WorldDynamic, ECR_Ignore},
		{ECC_WorldStatic, ECR_Ignore},
	};

	// Enable replication and always make the emitter relevant
	bAlwaysRelevant = true;
	bReplicates = false;
}

void ALightningEmitter::BeginPlay()
{
	Super::BeginPlay();

	// Ensure the emitter is inactive at the start
	DeactivateEmitter();

	// Initialize the random generator
	ResetGenerator();

	// Automatically activate the emitter if configured to do so
	if ( bAutoActivate )
	{
		ActivateEmitter();
	}
}

void ALightningEmitter::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
	// Cancel and clean up all pending pattern generation tasks
	for ( FAsyncTask<FPatternGenerator>* Task : PendingPatterns )
	{
		Task->Cancel();
	}

	for ( FAsyncTask<FPatternGenerator>* Task : PendingPatterns )
	{
		Task->EnsureCompletion( false );
		delete Task;
	}

	PendingPatterns.Empty();

	// Deactivate the emitter
	DeactivateEmitter();

	Super::EndPlay( EndPlayReason );
}

void ALightningEmitter::OnConstruction( const FTransform& Transform )
{
	Super::OnConstruction( Transform );

	// Reset the random generator upon construction
	ResetGenerator();
}

void ALightningEmitter::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
}

void ALightningEmitter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	// Check and clean up completed pattern generation tasks
	for ( int32 i = PendingPatterns.Num() - 1; i >= 0; --i )
	{
		FAsyncTask<FPatternGenerator>* Task = PendingPatterns[i];
		if ( Task && Task->IsDone() )
		{
			delete Task;
			PendingPatterns.RemoveAt( i, 1, true );
		}
	}
}

#if WITH_EDITOR
void ALightningEmitter::OnAttachmentTransform( USceneComponent* UpdatedComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport )
{
	// Update attachment parameters if the component's transform changes in the editor
	if ( !UpdatedComponent->IsSelectedInEditor() ) return;

	FLightningAttachment& Attachment = UpdatedComponent == SourcePoint ? SourceAttachment : TargetAttachment;
	const FVector Point = Attachment.GetAttachmentPoint( this, false );
	const FVector CompLocation = UpdatedComponent->GetComponentLocation();

	if ( CompLocation != Point )
	{
		Attachment.Type = EAttachType::AT_Location;
		Attachment.Location = CompLocation;

		// Invalidate the preview pattern to force regeneration
		InvalidatePreviewPattern();
	}
}

void ALightningEmitter::DrawDebugLightning( FLightningPattern Pattern )
{
	const float Thickness = bDrawCurveWidth ? LightningParams.ParticlesScalesWidth : 0.f;

	if ( Pattern.bIsValid )
	{
		// Draw the main lightning curve using the pattern points
		for ( int32 i = 0; i <= Pattern.Points.Num() - 2; ++i )
		{
			// Determine the color for the debug line
			FColor Color = bCurveCustomColor ? CurveColor : LightningParams.ColorParticle.ToFColor( false );

			// Retrieve current and next points in the pattern
			const FVector CurPoint = ULightningPatternLib::GetPoint( Pattern, i );
			const FVector NextPoint = ULightningPatternLib::GetPoint( Pattern, i + 1 );

			// Draw a debug line between the current and next points
			DrawDebugLine( GetWorld(), CurPoint, NextPoint, Color, false, -1.f, '\000', Thickness );
		}

		// Draw debug lines for branches if enabled
		if ( bDrawBranches )
		{
			for ( FBranchData Branch : Pattern.Branches )
			{
				DrawDebugLightning( Branch.Pattern );
			}
		}
	}
}

bool ALightningEmitter::ShouldTickIfViewportsOnly() const
{
	// Enable ticking even when only viewports are active
	return true;
}

void ALightningEmitter::PostEditChangeProperty( FPropertyChangedEvent& PropertyChangedEvent )
{
	Super::PostEditChangeProperty( PropertyChangedEvent );

	// Handle property changes in the editor
	if ( PropertyChangedEvent.GetPropertyName().IsEqual( "Type" ) ||
		PropertyChangedEvent.GetPropertyName().IsEqual( "AttachVolumeType" ) )
	{
		// Notify the details panel to update its UI
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>( "PropertyEditor" );
		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}

	// Invalidate the preview pattern to regenerate with new properties
	InvalidatePreviewPattern();
}
#endif // WITH_EDITOR

int32 ALightningEmitter::GetLightningSeed() const
{
	// Generate a random seed for lightning generation
	return EmitterRng.RandHelper( MAX_int32 );
}

void ALightningEmitter::ActivateEmitter_Implementation( bool bReset /*= false*/ )
{
	// Ensure activation happens only on the authority (server)
	if ( GetLocalRole() != ENetRole::ROLE_Authority )
	{
		return;
	}

	if ( bReset )
	{
		// Reset the random generator if requested
		ResetGenerator_Implementation();
	}

#if WITH_EDITOR
	// Handle activation differently in editor mode
	if ( GetWorld()->WorldType == EWorldType::Editor )
	{
		RequestSpawnLightning_Implementation( GetLightningSeed() );
	}
	else
#endif // WITH_EDITOR
	{
		RequestSpawnLightning( GetLightningSeed() );
	}

	// Setup looping if enabled
	if ( bLoop )
	{
		GetWorld()->GetTimerManager().SetTimer( LoopTimer, [this]()
		{
#if WITH_EDITOR
			if ( GetWorld()->WorldType == EWorldType::Editor )
			{
				ActivateEmitter_Implementation();
			}
			else
#endif // WITH_EDITOR
			{
				ActivateEmitter();
			}
		},
		LoopDuration / TimeMultiplier, false );
	}
}

void ALightningEmitter::DeactivateEmitter_Implementation()
{
	// Stop the looping timer
	GetWorld()->GetTimerManager().ClearTimer( LoopTimer );

	// Deactivate and remove all spawned lightning effects
	for ( int32 i = LightningsArray.Num() - 1; i >= 0; --i )
	{
		LightningsArray[i]->Deactivate();
	}

	LightningsArray.Empty();
}

void ALightningEmitter::ResetGenerator_Implementation()
{
	// Configure the random generator seed
	if ( EmitterSeed == -1 )
	{
		// Generate a random seed if none is specified
		EmitterRng.GenerateNewSeed();
	}
	else
	{
		// Initialize the random generator with the specified seed
		EmitterRng.Initialize( EmitterSeed );
	}
}

int32 ALightningEmitter::GetLightningsNum()
{
	// Return the current number of active lightning effects
	return LightningsArray.Num();
}

bool ALightningEmitter::HasAnyLightnings()
{
	// Check if there are any active lightning effects
	return LightningsArray.Num() > 0;
}

bool ALightningEmitter::IsEmitterActive()
{
	// Determine if the emitter is currently active
	return GetWorld()->GetTimerManager().IsTimerActive( LoopTimer ) || HasAnyLightnings();
}

#if WITH_EDITOR
void ALightningEmitter::RequestCollectGarbage()
{
	// Request garbage collection in the editor
	UWorld* World = GetWorld();
	if ( World && World->WorldType == EWorldType::Editor )
	{
		bNeedsGC = true;
	}
}
#endif // WITH_EDITOR

void ALightningEmitter::RequestSpawnLightning_Implementation( int32 Seed )
{
	// Get the source and target points for lightning
	const FVector Start = SourceAttachment.GetAttachmentPoint( this );
	const FVector End = TargetAttachment.GetAttachmentPoint( this );

	// Request to generate a new pattern and spawn the lightning
	FPatternGeneratedEvent OnPatternGenerated;
	OnPatternGenerated.BindLambda( [this]( FLightningPattern Pattern )
	{
		DoSpawnLightning( NULL, Pattern );
	} );

	RequestGeneratePattern( Start, End, OnPatternGenerated, Seed );
}

void ALightningEmitter::RequestGeneratePattern( FVector Start, FVector End, FPatternGeneratedEvent Callback, int32 Seed /*= -1*/ )
{
	// Create a new async task for pattern generation
	auto Task = new FAsyncTask<FPatternGenerator>(
		Callback,
		BranchingParams,
		Start, End,
		SourceAttachment.GetLightningDirection(),
		TargetAttachment.GetLightningDirection(),
		LightningParams.NoiseMultiplier,
		LightningParams.SparksStep,
		LightningParams.SegmentDivision,
		GetWorld()->WorldType == EWorldType::Editor,
		Seed );

	PendingPatterns.Add( Task );

	// Start the task either in the background or synchronously based on the configuration
	if ( bUseSeparateThread )
	{
		Task->StartBackgroundTask();
	}
	else
	{
		Task->StartSynchronousTask();
	}
}

void ALightningEmitter::InvalidatePreviewPattern()
{
	// Clear the preview pattern if not actively generating
	if ( !bGeneratingPreviewPattern )
	{
		PreviewPattern = FLightningPattern();
	}
}

void ALightningEmitter::DoSpawnLightning( ALightningEffect* Parent, FLightningPattern Pattern )
{
	// Set up parameters for spawning the lightning effect
	FActorSpawnParameters SpawnParams;
	SpawnParams.ObjectFlags |= RF_Transient; // Avoid saving the actor in the undo buffer/world
	SpawnParams.Owner = this;

	// Spawn the lightning effect actor
	auto LightningEffect = GetWorld()->SpawnActor<ALightningEffect>( LightningEffectClass, Pattern.Transform, SpawnParams );

	if ( LightningEffect )
	{
		// Configure the lightning effect with setup parameters
		FLightningSetupParams SetupParams;

		SetupParams.Root = Parent == NULL ? NULL : Parent->Root;
		SetupParams.Pattern = Pattern;
		SetupParams.Type = Parent == NULL ? ELightningType::Lightning_Origin : ELightningType::Lightning_Branch;
		SetupParams.Order = Parent == NULL ? 0 : ( Parent->GetOrder() + 1 );

		LightningEffect->bAlwaysRelevant = true;
		LightningEffect->Setup( this, SetupParams );
	}
}

