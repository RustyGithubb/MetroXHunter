/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "Electricity/ElectrocutableComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "Library/UtilityLibrary.h"

UElectrocutableComponent::UElectrocutableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// Hard-coding assets finding because it's not possible to reference them otherwise
	// without creating a blueprint.
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> SkeletalLightningAsset(
		TEXT("'/Game/Art/VFX/NS_Electrics/NS_Lightning_SkeletalMesh.NS_Lightning_SkeletalMesh'")
	);
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> StaticLightningAsset(
		TEXT("'/Game/Art/VFX/NS_Electrics/NS_Lightning_Cable.NS_Lightning_Cable'")
	);
	SkeletalLightningFX = SkeletalLightningAsset.Object;
	StaticLightningFX = StaticLightningAsset.Object;
}

void UElectrocutableComponent::BeginPlay()
{
	Super::BeginPlay();

	if ( bAutoFindSkeletalMeshes )
	{
		TArray<USkeletalMeshComponent*> SkeletalMeshComponents {};
		GetOwner()->GetComponents( SkeletalMeshComponents );

		for ( USkeletalMeshComponent* SkeletalMeshComponent : SkeletalMeshComponents )
		{
			GigglingSkeletalComponents.Add( SkeletalMeshComponent );
			FXAttachmentComponents.Add( SkeletalMeshComponent );
		}
	}
}

void UElectrocutableComponent::EndPlay( EEndPlayReason::Type Reason )
{
	DestroyFX();
}

void UElectrocutableComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	if ( !IsElectrocuting() )
	{
		StopElectrocution();
		return;
	}

	// "Giggle" electrocuted skeletal mesh components
	for ( auto SkeletalComponent : GigglingSkeletalComponents )
	{
		if ( !IsValid( SkeletalComponent ) ) continue;

		// Apply angular velocity independtly on each bone
		for ( auto BodyInstance : SkeletalComponent->Bodies )
		{
			BodyInstance->SetAngularVelocityInRadians(
				FMath::DegreesToRadians( FMath::VRand() * GigglingForce ),
				bGigglingAccumulateForce
			);
		}
	}

	OnElectricTick.Broadcast( DeltaTime );
}

void UElectrocutableComponent::Electrocute( float Duration )
{
	SetComponentTickEnabled( true );

	ElectrocutionStartWorldTime = GetWorld()->GetTimeSeconds();
	ElectrocutionDuration = Duration;

	OnElectricStart.Broadcast( Duration );

	UpdateFX();
}

void UElectrocutableComponent::StopElectrocution()
{
	SetComponentTickEnabled( false );

	OnElectricEnd.Broadcast();

	DestroyFX();
}

bool UElectrocutableComponent::IsElectrocuting() const
{
	if ( ElectrocutionStartWorldTime == 0.0f ) return false;
	return GetWorld()->GetTimeSeconds() - ElectrocutionStartWorldTime <= ElectrocutionDuration;
}

void UElectrocutableComponent::UpdateFX()
{
	if ( bIsFXPlaying ) return;

	for ( USceneComponent* AttachmentComponent : FXAttachmentComponents )
	{
		if ( !IsValid( AttachmentComponent ) ) continue;

		// Automatically choose between skeletal and static FX
		UNiagaraSystem* LightningFX = StaticLightningFX;
		if ( AttachmentComponent->IsA<USkeletalMeshComponent>() )
		{
			LightningFX = SkeletalLightningFX;
		}

		UNiagaraComponent* FXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			LightningFX,
			AttachmentComponent,
			NAME_None,
			FVector::ZeroVector, FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			/* bAutoDestroy */ true,
			/* bAutoActivate */ true,
			ENCPoolMethod::None,
			/* bPreCullCheck */ true
		);
		FXPlayingComponents.Add( FXComponent );
	}

	bIsFXPlaying = true;
}

void UElectrocutableComponent::DestroyFX()
{
	for ( UNiagaraComponent* FXComponent : FXPlayingComponents )
	{
		if ( !IsValid( FXComponent ) ) continue;
		FXComponent->Deactivate();
	}
	FXPlayingComponents.Empty();

	bIsFXPlaying = false;
}

