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
	static ConstructorHelpers::FObjectFinder<UTexture2D> MainBillboardSpriteAsset(
		TEXT("'/Lightnings/Textures/T_Billboard.T_Billboard'"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> SourcePointSpriteAsset(
		TEXT("'/Lightnings/Textures/T_Billboard_Source.T_Billboard_Source'"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> TargetPointSpriteAsset(
		TEXT("'/Lightnings/Textures/T_Billboard_Target.T_Billboard_Target'"));

	static ConstructorHelpers::FObjectFinder<UCurveFloat> LightningWidthCurveAsset(
		TEXT("'/Lightnings/Curves/Curve_LightningDefaultWidth.Curve_LightningDefaultWidth'"));
	static ConstructorHelpers::FObjectFinder<UCurveFloat> LightningFlashingCurveAsset(
		TEXT("'/Lightnings/Curves/Curve_BoltFlashing.Curve_BoltFlashing'"));

	MainBillboard = CreateDefaultSubobject<UBillboardComponent>("MainBillboard");
	SourcePoint = CreateDefaultSubobject<UBillboardComponent>("SourcePoint");
	TargetPoint = CreateDefaultSubobject<UBillboardComponent>("TargetPoint");

	SetRootComponent(MainBillboard);
	SourcePoint->SetupAttachment(MainBillboard);
	TargetPoint->SetupAttachment(MainBillboard);

	MainBillboard->SetSprite(MainBillboardSpriteAsset.Object);
	SourcePoint->SetSprite(SourcePointSpriteAsset.Object);
	TargetPoint->SetSprite(TargetPointSpriteAsset.Object);

	MainBillboard->bIsScreenSizeScaled = true;
	SourcePoint->bIsScreenSizeScaled = true;
	TargetPoint->bIsScreenSizeScaled = true;

	MainBillboard->SetRelativeScale3D(FVector(0.5f));
	SourcePoint->SetRelativeScale3D(FVector(0.5f));
	TargetPoint->SetRelativeScale3D(FVector(0.5f));

	MainBillboard->ScreenSize = 0.0015f;
	SourcePoint->ScreenSize = 0.0015f;
	TargetPoint->ScreenSize = 0.0015f;

#if WITH_EDITOR
	SourcePoint->TransformUpdated.AddUObject(this, &ALightningEmitter::OnAttachmentTransform);
	TargetPoint->TransformUpdated.AddUObject(this, &ALightningEmitter::OnAttachmentTransform);
#endif // WITH_EDITOR

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	LightningEffectClass = ALightningEffect::StaticClass();

	LightningParams.OriginParticlesWidth = LightningWidthCurveAsset.Object;
	LightningParams.BranchParticlesWidth = LightningWidthCurveAsset.Object;
	LightningParams.FlashingCurve = LightningFlashingCurveAsset.Object;

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

	bAlwaysRelevant = true;
	bReplicates = true;
}

void ALightningEmitter::BeginPlay()
{
	Super::BeginPlay();

	// Just in case if user has activated the emitter in-editor
	DeactivateEmitter();

	// Setup RNG
	ResetGenerator();

	if (bAutoActivate)
	{
		ActivateEmitter();
	}
}

void ALightningEmitter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (FAsyncTask<FPatternGenerator>* Task : PendingPatterns)
	{
		Task->Cancel();
	}

	for (FAsyncTask<FPatternGenerator>* Task : PendingPatterns)
	{
		Task->EnsureCompletion(false);
		delete Task;
	}

	PendingPatterns.Empty();

	DeactivateEmitter();

	Super::EndPlay(EndPlayReason);
}

void ALightningEmitter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ResetGenerator();
}

void ALightningEmitter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALightningEmitter, bAutoActivate);
	DOREPLIFETIME(ALightningEmitter, bLoop);
	DOREPLIFETIME(ALightningEmitter, LoopDuration);

	DOREPLIFETIME(ALightningEmitter, SourceAttachment);
	DOREPLIFETIME(ALightningEmitter, TargetAttachment);

	DOREPLIFETIME(ALightningEmitter, LightningParams);
	DOREPLIFETIME(ALightningEmitter, BranchingParams);

	DOREPLIFETIME(ALightningEmitter, RootEmissionParams);
	DOREPLIFETIME(ALightningEmitter, BranchEmissionParams);

	DOREPLIFETIME(ALightningEmitter, DisplacementMult);
	DOREPLIFETIME(ALightningEmitter, TimeMultiplier);
	DOREPLIFETIME(ALightningEmitter, bUseSeparateThread);

	DOREPLIFETIME(ALightningEmitter, bGenerateHitEvents);
	DOREPLIFETIME(ALightningEmitter, HitReaction);
	DOREPLIFETIME(ALightningEmitter, bGenerateOverlapEvents);
	DOREPLIFETIME(ALightningEmitter, IgnoredActors);
}

void ALightningEmitter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (int32 i = PendingPatterns.Num() - 1; i >= 0; --i)
	{
		FAsyncTask<FPatternGenerator>* Task = PendingPatterns[i];

		if (Task && Task->IsDone())
		{
			delete Task;
			PendingPatterns.RemoveAt(i, 1, true);
		}
	}

#if WITH_EDITOR
	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		const FVector Start = SourceAttachment.GetAttachmentPoint(this, false);
		const FVector End = TargetAttachment.GetAttachmentPoint(this, false);

		SourcePoint->SetWorldLocation(Start);
		TargetPoint->SetWorldLocation(End);

		if (PreviewPattern.bIsValid)
		{
			if ((SourceAttachment.Type == EAttachType::AT_Actor && !Start.Equals(ULightningPatternLib::GetStartPoint(PreviewPattern))) ||
				(TargetAttachment.Type == EAttachType::AT_Actor && !End.Equals(ULightningPatternLib::GetEndPoint(PreviewPattern))))
			{
				// Reset preview pattern if source/target actor has changed its position
				InvalidatePreviewPattern();
			}
		}

		// Check the emitter to be in a valid world (not inside of a blueprint menu)
		if (!PreviewPattern.bIsValid && !bGeneratingPreviewPattern)
		{
			FPatternGeneratedEvent OnPatternGenerated;
			OnPatternGenerated.BindLambda([this](FLightningPattern Pattern)
			{
				PreviewPattern = Pattern;
			});

			RequestGeneratePattern(Start, End, OnPatternGenerated);

			bGeneratingPreviewPattern = true;
		}

		if (PreviewPattern.bIsValid && bGeneratingPreviewPattern)
		{
			bGeneratingPreviewPattern = false;
		}

		if (bNeedsGC)
		{
			GCTimeBuffer += DeltaTime;

			if (GCTimeBuffer >= GCInterval)
			{
				GEngine->PerformGarbageCollectionAndCleanupActors();
				bNeedsGC = false;
				GCTimeBuffer = 0.f;
			}
		}

		bool bGameView = false;

		if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
		{
			FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
			const TSharedPtr<IAssetViewport> ActiveLevelViewport = LevelEditorModule.GetFirstActiveViewport();
			if (ActiveLevelViewport.IsValid())
			{
				bGameView = ActiveLevelViewport->IsInGameView();
			}
		}

		if (!bGameView)
		{
			DrawDebugLightning(PreviewPattern);
			SourceAttachment.DebugDrawVolume(this);
			TargetAttachment.DebugDrawVolume(this);
		}
	}
#endif // WITH_EDITOR
}

#if WITH_EDITOR
void ALightningEmitter::OnAttachmentTransform(USceneComponent* UpdatedComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	if (!UpdatedComponent->IsSelectedInEditor()) return;

	FLightningAttachment& Attachment = UpdatedComponent == SourcePoint ? SourceAttachment : TargetAttachment;
	const FVector Point = Attachment.GetAttachmentPoint(this, false);
	const FVector CompLocation = UpdatedComponent->GetComponentLocation();
	if (CompLocation != Point)
	{
		Attachment.Type = EAttachType::AT_Location;
		Attachment.Location = CompLocation;

		InvalidatePreviewPattern();
	}
}

void ALightningEmitter::DrawDebugLightning(FLightningPattern Pattern)
{
	const float Thickness = bDrawCurveWidth ? LightningParams.ParticlesScalesWidth : 0.f;

	if (Pattern.bIsValid)
	{
		// Draw lightning curve
		for (int32 i = 0; i <= Pattern.Points.Num() - 2; ++i)
		{
			FColor Color = bCurveCustomColor ? CurveColor : LightningParams.ColorParticle.ToFColor(false);

			const FVector CurPoint = ULightningPatternLib::GetPoint(Pattern, i);
			const FVector NextPoint = ULightningPatternLib::GetPoint(Pattern, i+1);

			DrawDebugLine(GetWorld(), CurPoint, NextPoint, Color, false, -1.f, '\000', Thickness);
		}

		if (bDrawBranches)
		{
			for (FBranchData Branch : Pattern.Branches)
			{
				DrawDebugLightning(Branch.Pattern);
			}
		}
	}
}

bool ALightningEmitter::ShouldTickIfViewportsOnly() const
{
	return true;
}

void ALightningEmitter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName().IsEqual("Type") ||
		PropertyChangedEvent.GetPropertyName().IsEqual("AttachVolumeType"))
	{
		// Update details panel on properties change
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}
	InvalidatePreviewPattern();
}
#endif // WITH_EDITOR

int32 ALightningEmitter::GetLightningSeed() const
{
	return EmitterRng.RandHelper(MAX_int32);
}

void ALightningEmitter::ActivateEmitter_Implementation(bool bReset /*= false*/)
{
	if (GetLocalRole() != ENetRole::ROLE_Authority)
	{
		// Do nothing if particles are replicated
		return;
	}

	if (bReset)
	{
		ResetGenerator_Implementation();
	}

#if WITH_EDITOR
	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		RequestSpawnLightning_Implementation(GetLightningSeed());
	}
	else
#endif // WITH_EDITOR
	{
		RequestSpawnLightning(GetLightningSeed());
	}

	if (bLoop)
	{
		GetWorld()->GetTimerManager().SetTimer(LoopTimer, [this] ()
		{
#if WITH_EDITOR
			if (GetWorld()->WorldType == EWorldType::Editor)
			{
				ActivateEmitter_Implementation();
			}
			else
#endif // WITH_EDITOR
			{
				ActivateEmitter();
			}
		},
		LoopDuration / TimeMultiplier, false);
	}
}

void ALightningEmitter::DeactivateEmitter_Implementation()
{
	GetWorld()->GetTimerManager().ClearTimer(LoopTimer);

	for (int32 i = LightningsArray.Num() - 1; i >= 0; --i)
	{
		LightningsArray[i]->Deactivate();
	}

	LightningsArray.Empty();
}

void ALightningEmitter::ResetGenerator_Implementation()
{
	// Setup seed RNG
	if (EmitterSeed == -1)
	{
		// Use a random seed
		EmitterRng.GenerateNewSeed();
	}
	else
	{
		// Use the specified seed
		EmitterRng.Initialize(EmitterSeed);
	}
}

int32 ALightningEmitter::GetLightningsNum()
{
	return LightningsArray.Num();
}

bool ALightningEmitter::HasAnyLightnings()
{
	return LightningsArray.Num() > 0;
}

bool ALightningEmitter::IsEmitterActive()
{
	return GetWorld()->GetTimerManager().IsTimerActive(LoopTimer) || HasAnyLightnings();
}

#if WITH_EDITOR
void ALightningEmitter::RequestCollectGarbage()
{
	UWorld* World = GetWorld();
	if (World && World->WorldType == EWorldType::Editor)
	{
		bNeedsGC = true;
	}
}
#endif // WITH_EDITOR

void ALightningEmitter::RequestSpawnLightning_Implementation(int32 Seed)
{
	const FVector Start = SourceAttachment.GetAttachmentPoint(this);
	const FVector End = TargetAttachment.GetAttachmentPoint(this);

	/** Request generate a new pattern and spawn lightning */
	FPatternGeneratedEvent OnPatternGenerated;
	OnPatternGenerated.BindLambda([this](FLightningPattern Pattern)
	{
		DoSpawnLightning(NULL, Pattern);
	});

	RequestGeneratePattern(Start, End, OnPatternGenerated, Seed);
}

void ALightningEmitter::RequestGeneratePattern(FVector Start, FVector End, FPatternGeneratedEvent Callback, int32 Seed /*= -1*/)
{
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
		Seed);

	PendingPatterns.Add(Task);

	if (bUseSeparateThread)
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
	if (!bGeneratingPreviewPattern)
	{
		PreviewPattern = FLightningPattern();
	}
}

void ALightningEmitter::DoSpawnLightning(ALightningEffect* Parent, FLightningPattern Pattern)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.ObjectFlags |= RF_Transient; // Use that to avoid save an actor into undo buffer/ world 
	SpawnParams.Owner = this;
	auto LightningEffect = GetWorld()->SpawnActor<ALightningEffect>(LightningEffectClass, Pattern.Transform, SpawnParams);

	if (LightningEffect)
	{
		FLightningSetupParams SetupParams;

		SetupParams.Root = Parent == NULL ? NULL : Parent->Root;
		SetupParams.Pattern = Pattern;
		SetupParams.Type = Parent == NULL ? ELightningType::Lightning_Origin : ELightningType::Lightning_Branch;
		SetupParams.Order = Parent == NULL ? 0 : (Parent->GetOrder() + 1);

		LightningEffect->bAlwaysRelevant = true;
		LightningEffect->Setup(this, SetupParams);
	}
}
