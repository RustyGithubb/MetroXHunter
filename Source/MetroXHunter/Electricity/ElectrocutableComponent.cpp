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
}

void UElectrocutableComponent::BeginPlay()
{
	Super::BeginPlay();
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

	for ( auto AttachmentComponent : FXAttachmentComponents )
	{
		if ( !IsValid( AttachmentComponent ) ) continue;

		UNiagaraComponent* FXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			LightningFX,
			AttachmentComponent,
			FName {},
			FVector::ZeroVector, FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			/* bAutoDestroy */ false,
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
	for ( auto FXComponent : FXPlayingComponents )
	{
		if ( !IsValid( FXComponent ) ) continue;
		FXComponent->DestroyComponent();
	}
	FXPlayingComponents.Empty();

	bIsFXPlaying = false;
}

