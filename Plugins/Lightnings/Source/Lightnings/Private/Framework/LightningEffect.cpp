#include "Framework/LightningEffect.h"
#include "Helpers/GuardedExecution.h"

#include "Engine/Texture2D.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Runtime/Engine/Classes/Curves/CurveFloat.h"
#include "Runtime/Launch/Resources/Version.h"
#include "NiagaraDataInterfaceVectorCurve.h"
#include "NiagaraDataInterfaceCurve.h"
#if ENGINE_MAJOR_VERSION > 4
#include "NiagaraSystemInstanceController.h"
#endif
#include "UObject/ConstructorHelpers.h"

#include "Pool/PoolManager.h"

ALightningEffect::ALightningEffect()
{
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> LightningParticlesAsset(
		TEXT("'/Lightnings/FX/NS_Lightning.NS_Lightning'"));

	static ConstructorHelpers::FObjectFinder<UTexture2D> BillboardSpriteAsset(
		TEXT("'/Lightnings/Textures/T_EffectBillboard.T_EffectBillboard'"));

	PrimaryActorTick.bCanEverTick = true;

	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("NiagaraComponent");
	Billboard = CreateDefaultSubobject<UBillboardComponent>("Billboard");

	SetRootComponent(NiagaraComponent);

	Billboard->SetupAttachment(GetRootComponent());

	NiagaraComponent->SetAutoActivate(false);
	NiagaraComponent->SetAsset(LightningParticlesAsset.Object);

	Billboard->SetSprite(BillboardSpriteAsset.Object);
	Billboard->bIsScreenSizeScaled = true;
	Billboard->ScreenSize = 0.0005f;
	
	SetActorTickEnabled(false);
}

void ALightningEffect::BeginPlay()
{
	Super::BeginPlay();
}

void ALightningEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!NiagaraComponent)
	{
		return;
	}

#if ENGINE_MAJOR_VERSION > 4
	FNiagaraSystemInstanceControllerPtr NiagaraSystem = NiagaraComponent->GetSystemInstanceController();
#else
	FNiagaraSystemInstance* NiagaraSystem = NiagaraComponent->GetSystemInstance();
#endif

	if (!NiagaraSystem || NiagaraSystem->IsPendingSpawn())
	{
		return;
	}

	EXECUTE_GUARDED
	(
		FlashingTimeline.TickTimeline(DeltaTime);
	)

	if (bActive)
	{
		UpdateLifetime(DeltaTime);
	}

	if (bIsFadingOut)
	{
		UpdateFading(DeltaTime);
	}
}

#if WITH_EDITOR
bool ALightningEffect::ShouldTickIfViewportsOnly() const
{
	return true;
}
#endif // WITH_EDITOR

void ALightningEffect::OnBoltFlashing(float Alpha)
{
	IntensityCur.BoltIntensity = IntensityDef.BoltIntensity * Alpha;
	UpdateColorMultiplier();
}

void ALightningEffect::UpdateLifetime(float DeltaTime)
{
	if (!LightningPattern.bIsValid || LightningPattern.SparkData.SparkDelayTimer.IsValid())
	{
		// Do nothing if the pattern is invalid or the spark is delayed
		return;
	}

	FLightningSparkData& SparkData = LightningPattern.SparkData;

	if (SparkData.SparkDirection.Equals(FVector(0.f, 0.f, 0.f)))
	{
		SparkData.SparkVelocity = Rnd.FRandRange(LightningParams.SparksRangeVelocity.GetLowerBoundValue(), LightningParams.SparksRangeVelocity.GetUpperBoundValue());
		const float VelFading = Rnd.FRandRange(LightningParams.SparksFadingVelocity.GetLowerBoundValue(), LightningParams.SparksFadingVelocity.GetUpperBoundValue());
		SparkData.SparkVelocity *= FMath::Pow(VelFading, LightningOrder);
		OnTargetPosReached();
	}

	FVector PointA = ULightningPatternLib::GetPoint(LightningPattern, SparkData.SparkTargetPosIdx - 1);
	FVector PointB = ULightningPatternLib::GetPoint(LightningPattern, SparkData.SparkTargetPosIdx);

	SparkData.SparkDirection = (PointB - PointA);
	SparkData.SparkDirection.Normalize();

	const float Step = DeltaTime * SparkData.SparkVelocity * LightningEmitter->TimeMultiplier;
	float DistToB = (PointB - SparkData.SparkPosition).Size();

	if (Step >= DistToB)
	{
		/**
		* Jump over next points if step is too high
		*/
		while (true)
		{
			if (SparkData.SparkTargetPosIdx >= LightningPattern.Points.Num() - 1)
			{
				break;
			}

			const FVector CurTargetPoint =
				ULightningPatternLib::GetPoint(LightningPattern, SparkData.SparkTargetPosIdx);
			const FVector NextTargetPoint =
				ULightningPatternLib::GetPoint(LightningPattern, SparkData.SparkTargetPosIdx + 1);

			const float DistToBNew = DistToB + (CurTargetPoint - NextTargetPoint).Size();

			if (LightningParams.bIsInstant || Step >= DistToBNew)
			{
				if (!DoSparkMovement(SparkData.SparkPosition, CurTargetPoint))
				{
					// Prevent further growth if the spark hits something
					break;
				}

				DistToB = DistToBNew;
				OnTargetPosReached();
			}
			else
			{
				break;
			}
		}

		DoSparkMovement(SparkData.SparkPosition, ULightningPatternLib::GetPoint(LightningPattern, SparkData.SparkTargetPosIdx));
		SparkData.SparkDirection = FVector(0.f, 0.f, 0.f);

		const float TimerRate = LightningEmitter->TimeMultiplier *
			Rnd.FRandRange(LightningParams.SparksRangeDelay.GetLowerBoundValue(), LightningParams.SparksRangeDelay.GetUpperBoundValue());

		FTimerDelegate TimerDel;
		TimerDel.BindUObject( this, &ALightningEffect::InvalidateSparkDelayTimer );

		GetWorld()->GetTimerManager().SetTimer( SparkData.SparkDelayTimer, TimerDel, TimerRatee, false );

		if (SparkData.SparkTargetPosIdx >= LightningPattern.Points.Num() - 1)
		{
			// Lightning is fully formed - stop growth and run discharge
			StopSparkMovement(true);
		}
	}
	else
	{
		DoSparkMovement(SparkData.SparkPosition, SparkData.SparkPosition + SparkData.SparkDirection * Step);
	}

	// Update current path points after movement		
	PointA = ULightningPatternLib::GetPoint(LightningPattern, SparkData.SparkTargetPosIdx - 1);
	PointB = ULightningPatternLib::GetPoint(LightningPattern, SparkData.SparkTargetPosIdx);

	const float GrowthStep = 1.f / float(LightningPattern.Points.Num() - 1);
	Growth = GrowthStep * (SparkData.SparkTargetPosIdx - 1);
	const float CurSegmentAlpha = (SparkData.SparkPosition - PointA).Size() / (PointB - PointA).Size();
	Growth += GrowthStep * CurSegmentAlpha;

	UpdateColorMultiplier();
}

void ALightningEffect::UpdateFading(float DeltaTime)
{
	if (!Root || (FMath::IsNearlyZero(IntensityCur.SparkIntensity, 0.01f) && FMath::IsNearlyZero(IntensityCur.TrailIntensity, 0.01f)))
	{
		Deactivate();
	}
	else
	{
		float MinTime, MaxTime;
		LightningParams.FlashingCurve->GetTimeRange(MinTime, MaxTime);
		const float BoltFlashingTime = (MaxTime - MinTime) / Root->FlashingTimeline.GetPlayRate();
		const float SparkFadeOutSpeed = (IntensityDef.SparkIntensity / BoltFlashingTime) * LightningParams.SparksFadeOutMult;
		const float TrailFadeOutSpeed = (IntensityDef.TrailIntensity / BoltFlashingTime) * LightningParams.TrailFadeOutMult;

		IntensityCur.SparkIntensity = FMath::FInterpConstantTo(IntensityCur.SparkIntensity, 0.f, DeltaTime, SparkFadeOutSpeed);
		IntensityCur.TrailIntensity = FMath::FInterpConstantTo(IntensityCur.TrailIntensity, 0.f, DeltaTime, TrailFadeOutSpeed);

		UpdateColorMultiplier();
	}
}

void ALightningEffect::Discharge()
{
	if (!IsRoot())
	{
		// Branches cannot discharge
		return;
	}

	if (LightningParams.FlashingCurve)
	{
		// Start root flashing
		FlashingTimeline.PlayFromStart();

		// Fade out branches
		for (int32 i = Branches.Num() - 1; i >= 0; --i)
		{
			Branches[i]->FadeOut();
		}
	}
	else
	{
		UE_LOG(LogClass, Warning, TEXT("ALightningEffect %s: FlashingCurve is NULL"), *GetName());
		Deactivate();
	}

	bActive = false;
}

void ALightningEffect::OnTargetPosReached()
{
	FLightningSparkData& SparkData = LightningPattern.SparkData;

	FLightningPattern BranchPattern;
	if (LightningPattern.FindBranch(SparkData.SparkTargetPosIdx, BranchPattern))
	{
		/** Spawn a branch */
		LightningEmitter->DoSpawnLightning(this, BranchPattern);
	}

	if (SparkData.SparkTargetPosIdx >= (LightningPattern.Points.Num() - 1))
	{
		StopSparkMovement(true);
	}
	else
	{
		++SparkData.SparkTargetPosIdx;
	}
}

void ALightningEffect::UpdateColorMultiplier()
{
	FRichCurve ColorCurve = FRichCurve();

	const float SparkLength = 0.002;

	const float LeftBound = 1.f - SparkLength;

	/**
	* A helper param to switch between growth (trail) intensity and discharge (bolt) intensity
	*
	* 0.f - full growth intensity
	* 1.f - full discharge intensity
	*/
	float GrowthToDischargeAlpha = FMath::Clamp((Growth - LeftBound) / (1.f - LeftBound), 0.f, 1.f);

	if (FlashingTimeline.IsPlaying())
	{
		// Force set GrowthAlpha to 1.0 in case discharge has been started by hit (Growth < 1.0)
		GrowthToDischargeAlpha = 1.f;
	}

	const bool bIsRoot = IsRoot();

	const float TrailColor = bIsRoot ? FMath::Lerp(IntensityCur.TrailIntensity, IntensityCur.BoltIntensity, GrowthToDischargeAlpha) : IntensityCur.TrailIntensity;
	const float SparkColor = FMath::Lerp(IntensityCur.SparkIntensity, bIsRoot ? IntensityCur.BoltIntensity : IntensityCur.TrailIntensity, GrowthToDischargeAlpha);

	ColorCurve.SetKeyInterpMode(ColorCurve.AddKey(-SparkLength * 2.f, TrailColor), ERichCurveInterpMode::RCIM_Linear, false);
	ColorCurve.SetKeyInterpMode(ColorCurve.AddKey(Growth - SparkLength, TrailColor), ERichCurveInterpMode::RCIM_Cubic, true);
	ColorCurve.SetKeyInterpMode(ColorCurve.AddKey(Growth, SparkColor), ERichCurveInterpMode::RCIM_Cubic, true);
	ColorCurve.SetKeyInterpMode(ColorCurve.AddKey(Growth + SparkLength, 0.f), ERichCurveInterpMode::RCIM_Cubic, true);

	SetNiagaraVariableVector3Curve("User.ColorScale", ColorCurve, ColorCurve, ColorCurve);

	EXECUTE_GUARDED
	(
		OnIntensityChanged(TrailColor);
	)
}

void ALightningEffect::SetNiagaraDataInterface(const FString& Name, UNiagaraDataInterface* DataInterface)
{
	auto OverrideParameters = (FNiagaraParameterStore*)(&NiagaraComponent->GetOverrideParameters());

	const FNiagaraVariable Variable(FNiagaraTypeDefinition(DataInterface->GetClass()), *Name);
	const int32 Index = OverrideParameters->IndexOf(Variable);

	if (Index == INDEX_NONE)
	{
		return;
	}

	OverrideParameters->SetDataInterface(DataInterface, Index);
}

void ALightningEffect::SetNiagaraVariableFloatCurve(FName Name, FRichCurve Curve)
{
	const FNiagaraUserRedirectionParameterStore& OverrideParameters = NiagaraComponent->GetOverrideParameters();

	const FNiagaraVariable Variable(FNiagaraTypeDefinition(UNiagaraDataInterfaceCurve::StaticClass()), *Name.ToString());
	const int32 Index = OverrideParameters.IndexOf(Variable);

	if (Index == INDEX_NONE)
	{
		return;
	}

	UNiagaraDataInterfaceCurve* InterfaceCurve = Cast<UNiagaraDataInterfaceCurve>(OverrideParameters.GetDataInterface(Index));

	if (InterfaceCurve)
	{
		InterfaceCurve->Curve = Curve;
	}
}

void ALightningEffect::SetNiagaraVariableVector3Curve(FName Name, FRichCurve CurveX, FRichCurve CurveY, FRichCurve CurveZ)
{
	const FNiagaraUserRedirectionParameterStore& OverrideParameters = NiagaraComponent->GetOverrideParameters();

	const FNiagaraVariable Variable(FNiagaraTypeDefinition(UNiagaraDataInterfaceVectorCurve::StaticClass()), *Name.ToString());
	const int32 Index = OverrideParameters.IndexOf(Variable);

	if (Index == INDEX_NONE)
	{
		return;
	}

	UNiagaraDataInterfaceVectorCurve* InterfaceVectorCurve = Cast<UNiagaraDataInterfaceVectorCurve>(OverrideParameters.GetDataInterface(Index));
	
	if (InterfaceVectorCurve)
	{
		InterfaceVectorCurve->XCurve = CurveX;
		InterfaceVectorCurve->YCurve = CurveY;
		InterfaceVectorCurve->ZCurve = CurveZ;
	}
}

bool ALightningEffect::DoSparkMovement(const FVector& Start, const FVector& End)
{
	QUICK_SCOPE_CYCLE_COUNTER( LightningEffect_DoSparkMovement );

	if (LightningEmitter->bGenerateHitEvents)
	{
		// Do a hit check first
		TArray<FHitResult> HitsResult;
		const int32 MaxHits = LightningEmitter->HitReaction == EHitReaction::HitReact_Ignore ? -1 : 1;
		if (DoCollisionCheck(Start, End, ECollisionResponse::ECR_Block, MaxHits, HitsResult))
		{
			for (const FHitResult& Hit : HitsResult)
			{
				EXECUTE_GUARDED
				(
					OnLightningHit(Hit);
				)
			}

			if (LightningEmitter->HitReaction != EHitReaction::HitReact_Ignore)
			{
				// Stop lightning growth on hit if it is not ignored
				StopSparkMovement(LightningEmitter->HitReaction == EHitReaction::HitReact_Discharge);

				return false;
			}
		}
	}

	LightningPattern.SparkData.SparkPosition = End;

	if (LightningEmitter->bGenerateOverlapEvents)
	{
		//Do overlaps check
		TArray<FHitResult> OverlapsResult;
		if (DoCollisionCheck(Start, End, ECollisionResponse::ECR_Overlap, -1, OverlapsResult))
		{
			EXECUTE_GUARDED
			(
				OnLightningOverlap(OverlapsResult);
			)
		}
	}

	return true;
}

void ALightningEffect::StopSparkMovement(bool bDischarge)
{
	if (!LightningPattern.bIsValid)
	{
		// Spark movement has been stopped already
		return;
	}

	LightningPattern.bIsValid = false;

	EXECUTE_GUARDED
	(
		OnLightningSparkEndMove();
	)

	if (IsRoot())
	{
		// Discharge/fade out is only for root lightnings
		// Branches should be kept alive and removed later

		if (!bDischarge)
		{
			// Fade out lightning
			// That will fade out branches as well if this lightning is root
			FadeOut();
		}
		else
		{
			// Discharge lightning
			Discharge();
		}
	}

	bActive = false;
}

bool ALightningEffect::DoCollisionCheck(const FVector& Start, const FVector& End, ECollisionResponse Response, int32 MaxResults, TArray<FHitResult>& OutHits)
{
	QUICK_SCOPE_CYCLE_COUNTER( LightningEffect_DoCollisionCheck );

	if (MaxResults == 0)
	{
		return false;
	}

	TMap< TEnumAsByte<ECollisionChannel>, TEnumAsByte<ECollisionResponse> >::TIterator
		ChannelsIter = LightningEmitter->CollisionResponses.CreateIterator();

	FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
	QueryParams.AddIgnoredActors(LightningEmitter->IgnoredActors);

	int32 ResultsNum = 0;

	TArray<FHitResult> Hits;
	Hits.Reserve( MaxResults );

	for (; ChannelsIter; ++ChannelsIter)
	{
		const ECollisionChannel CurChannel = ChannelsIter->Key;
		const ECollisionResponse CurResponse = ChannelsIter->Value;

		if (CurResponse != Response)
		{
			continue;
		}

		if (GetWorld()->LineTraceMultiByChannel(Hits, Start, End, CurChannel, QueryParams))
		{
			OutHits.Append(Hits);
			Hits.Empty();
			++ResultsNum;

			if (MaxResults > 0 && ResultsNum >= MaxResults)
			{
				break;
			}
		}
	}

	return ResultsNum > 0;
}

void ALightningEffect::Setup(ALightningEmitter* Emitter, FLightningSetupParams SetupParams)
{
	QUICK_SCOPE_CYCLE_COUNTER( LightningEffect_Setup );

	LightningEmitter = Emitter;
	LightningPattern = SetupParams.Pattern;
	Root = SetupParams.Root == NULL ? this : SetupParams.Root;
	LightningParams = Emitter->LightningParams;
	LightningType = SetupParams.Type;
	LightningOrder = SetupParams.Order;

	// Override default interfaces (do not use LUT as it crashes the niagara sometimes)

	auto ColorScaleInterface = NewObject<UNiagaraDataInterfaceVectorCurve>();
	ColorScaleInterface->bUseLUT = false;

	auto PatternInterface = NewObject<UNiagaraDataInterfaceVectorCurve>();
	PatternInterface->bUseLUT = false;

	auto WidthCurveInterface = NewObject<UNiagaraDataInterfaceCurve>();
	WidthCurveInterface->bUseLUT = false;

	SetNiagaraDataInterface( "User.ColorScale", ColorScaleInterface );
	SetNiagaraDataInterface( "User.Pattern", PatternInterface );
	SetNiagaraDataInterface( "User.WidthCurve", WidthCurveInterface );

	if (LightningPattern.Seed == -1)
	{
		// Use a random seed
		Rnd.GenerateNewSeed();
	}
	else
	{
		// Use the specified seed
		Rnd.Initialize(LightningPattern.Seed);
	}

	/** Calculating intensity params */
	const float FadingLow = LightningParams.TrailFadingIntensity.GetLowerBoundValue();
	const float FadingHigh = LightningParams.TrailFadingIntensity.GetUpperBoundValue();
	const float TrailIntensityFading = Rnd.FRandRange(FadingLow, FadingHigh);
	const float FlashingForceMult = Rnd.FRandRange(LightningParams.FlashingForce.GetLowerBoundValue(), LightningParams.FlashingForce.GetUpperBoundValue());
	const float FlashingRate = LightningEmitter->TimeMultiplier 
		* Rnd.FRandRange(LightningParams.FlashingRate.GetLowerBoundValue(), LightningParams.FlashingRate.GetUpperBoundValue());

	/**
	* SparkIntensity
	*/
	IntensityDef.SparkIntensity = LightningParams.SparksIntensity;
	IntensityCur.SparkIntensity = IntensityDef.SparkIntensity;

	/**
	* TrailIntensity
	*/
	IntensityDef.TrailIntensity = LightningParams.TrailIntensity * FMath::Pow(TrailIntensityFading, LightningOrder);
	IntensityCur.TrailIntensity = IntensityDef.TrailIntensity;

	if (SetupParams.Type == ELightningType::Lightning_Origin)
	{
		/**
		* BoltIntensity
		*/
		IntensityDef.BoltIntensity = LightningParams.LightningBoltIntensity * FlashingForceMult;
		IntensityCur.BoltIntensity = IntensityDef.BoltIntensity;

		/**
		* Setup of flashing timeline
		*/
		FOnTimelineFloat OnBoltFlashingEvent;
		FOnTimelineEvent OnBoltFlashingFinishedEvent;
		OnBoltFlashingEvent.BindUFunction(this, "OnBoltFlashing");
		OnBoltFlashingFinishedEvent.BindUFunction(this, "Deactivate");

		FlashingTimeline = FTimeline();
		FlashingTimeline.AddInterpFloat(LightningParams.FlashingCurve, OnBoltFlashingEvent);
		FlashingTimeline.SetTimelineFinishedFunc(OnBoltFlashingFinishedEvent);
		FlashingTimeline.SetPlayRate(FlashingRate);
	}

	/**
	* Activate
	*/
	if (!LightningPattern.bIsValid || !NiagaraComponent)
	{
		UE_LOG(LogClass, Log, TEXT("Validation check failed: %d && %d"), LightningPattern.bIsValid, NiagaraComponent != NULL)
		return;
	}

	const FEmissionParams& EmissionParams = IsRoot() ? LightningEmitter->RootEmissionParams : LightningEmitter->BranchEmissionParams;
	if (EmissionParams.bUseLightRenderer)
	{
		NiagaraComponent->SetVariableFloat("User.LightMultiplier", EmissionParams.LightMultiplier);
		NiagaraComponent->SetVariableFloat("User.LightExponent", EmissionParams.LightExponent);
		NiagaraComponent->SetVariableFloat("User.LightRadius", EmissionParams.LightRadius);
		NiagaraComponent->SetVariableFloat("User.LightVolumetricScattering", EmissionParams.LightVolumetricScattering);
	}
	else
	{
		NiagaraComponent->SetVariableFloat("User.LightRadius", 0.f);
	}

	NiagaraComponent->SetVariableFloat("User.DisplacementMult", LightningEmitter->DisplacementMult);

	NiagaraComponent->SetVariableInt("User.PartsCount", LightningPattern.Points.Num());
	NiagaraComponent->SetVariableLinearColor("User.Color", LightningParams.ColorParticle);
	NiagaraComponent->SetVariableFloat("User.WidthScale", LightningParams.ParticlesScalesWidth);

	/**
	* Applying width curve
	*/
	UCurveFloat* WidthCurveObj = IsRoot() ? LightningParams.OriginParticlesWidth : LightningParams.BranchParticlesWidth;
	FRichCurve WidthCurve;
	if (WidthCurveObj != NULL)
	{
		WidthCurve = WidthCurveObj->FloatCurve;
	}
	else
	{
		/**
		* Generating default width curve
		*/
		WidthCurve.AddKey(0.f, 1.f);
		WidthCurve.AddKey(1.f, 1.f);
	}

	SetNiagaraVariableFloatCurve("User.WidthCurve", WidthCurve);
	SetNiagaraVariableVector3Curve("User.Pattern", LightningPattern.PointsX, LightningPattern.PointsY, LightningPattern.PointsZ);

	NiagaraComponent->Activate(true);

	EXECUTE_GUARDED
	(
		OnLightningSpawned();
	)

	if (IsRoot())
	{
		LightningEmitter->LightningsArray.Add(this);

		EXECUTE_GUARDED
		(
			LightningEmitter->OnRootSpawned(this);
		)
	}
	else
	{
		Root->Branches.Add(this);

		EXECUTE_GUARDED
		(
			Root->OnLightningBranched(this, LightningOrder);
		)
	}

	bActive = true;

	UpdateLifetime(GetWorld()->GetDeltaSeconds());

	SetActorTickEnabled(true);
}

FLightningPattern ALightningEffect::GetPattern() const
{
	return LightningPattern;
}

bool ALightningEffect::IsRoot() const
{
	return LightningType == ELightningType::Lightning_Origin;
}

int32 ALightningEffect::GetOrder() const
{
	return LightningOrder;
}

void ALightningEffect::FadeOut()
{
	if (IsRoot())
	{
		for (int32 i = Branches.Num() - 1; i >= 0; --i)
		{
			// No need replication - already replicated
			Branches[i]->FadeOut();
		}
	}

	bActive = false;
	bIsFadingOut = true;
}

void ALightningEffect::Deactivate()
{
	QUICK_SCOPE_CYCLE_COUNTER( LightningEffect_Deactivate );

	SetActorTickEnabled(false);

	if (IsRoot())
	{
		// Clear branches
		for (int32 i = Branches.Num() - 1; i >= 0; --i)
		{
			Branches[i]->Deactivate();
		}

		LightningEmitter->LightningsArray.Remove(this);

#if WITH_EDITOR
		// Collect garbage on next tick if in edit-mode
		LightningEmitter->RequestCollectGarbage();
#endif // WITH_EDITOR

	}
	else
	{
		Root->Branches.Remove(this);
	}

	NiagaraComponent->Deactivate();

	LightningEmitter->PoolManager->ReturnActorToPool( this );
}

void ALightningEffect::InvalidateSparkDelayTimer()
{
	SparkDelayTimer.Invalidate();
}
