/*
 * Implemented by Corentin Paya
 */

#include "Gun/Gun.h"
#include "Character/MetroPlayerCharacter.h"
#include "Health/HealthComponent.h"
#include "Reload/ReloadComponent.h"
#include "Gun/GunData.h"

#include <DualSenseFunctionLibrary.h>
#include "DualSenseControllerComponent.h"
#include "Camera/CameraShakeSourceComponent.h"
#include "CineCameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Sound/SoundBase.h"
#include "Library/GameplayLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "Library/UtilityLibrary.h"

AGun::AGun()
{
	PrimaryActorTick.bCanEverTick = true;

	/* Create all the components */
	SceneRoot = CreateDefaultSubobject<USceneComponent>( TEXT( "SceneRoot" ) );
	RootComponent = SceneRoot;

	WeaponSkeletonMesh = CreateDefaultSubobject<USkeletalMeshComponent>( TEXT( "WeaponSkeletonMesh" ) );
	WeaponSkeletonMesh->SetupAttachment( RootComponent );

	ShootPoint = CreateDefaultSubobject<USceneComponent>( TEXT( "ShootPoint" ) );
	ShootPoint->SetupAttachment( WeaponSkeletonMesh );

	CameraShakeSourceComponent = CreateDefaultSubobject<UCameraShakeSourceComponent>( TEXT( "CameraShakeSource" ) );
	CameraShakeSourceComponent->SetupAttachment( RootComponent );
}

void AGun::BeginPlay()
{
	Super::BeginPlay();

	// Late Begin Play
	GetWorld()->OnWorldBeginPlay.AddUObject( this, &AGun::LateBeginPlay );
}

void AGun::LateBeginPlay()
{
	GetReferences();
	GetNiagaraEffects();
}

void AGun::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	CheckCurrentTargetType();
}

bool AGun::HandleCanFire()
{
	switch ( GunMode )
	{
		case EGunMode::Bullet:
		{
			if ( ReloadComponent->IsGunEmpty() )
			{
				UGameplayStatics::PlaySound2D( GetWorld(), GunData->ShootFailedSound );
				return false;
			}

			if ( ReloadComponent->IsReloading() ) return false;

			float CurrentGameTime = UKismetSystemLibrary::GetGameTimeInSeconds( GetWorld() );
			if ( CurrentGameTime - ShootCurentCooldown >= GunData->ShootCooldown ) return true;

			return false;
		}
		case EGunMode::Lightning:
		{
			if ( bIsLightningCharged && CurrentEnergyAmount > 0 ) return true;
			return false;
		}
	}

	return false;
}

void AGun::OnReload()
{
	PlayerCharacter->PlayAnimMontage( GunData->ReloadAnimationMontage );
}

void AGun::RetrieveFirstBeamHit()
{
	EnemiesTargeted.Empty();

	FVector ImpactPoint {};
	FHitResult HitResult {};
	CheckSphereCollision( true, GunData->LightningDistance, ImpactPoint, HitResult, 20.0f );

	if ( HitResult.bBlockingHit )
	{
		EnemiesTargeted.Add( HitResult.GetActor() );
		RetrieveReflectedEnemies( ImpactPoint );
	}
	else
	{
		FVector WorldLocation = PlayerCharacter->CineCameraComponent->GetComponentLocation();
		FVector ForwardVector = PlayerCharacter->CineCameraComponent->GetForwardVector();

		ImpactPoint = WorldLocation + ( ForwardVector * GunData->LightningDistance );
	}

	AddForceAtTrace( HitResult, GunData->LightningImpulseForce );
	TriggerLightningBeam( ImpactPoint );
}

void AGun::SwitchWeapon()
{
	switch ( GunMode )
	{
		case EGunMode::Bullet:
		{
			GunMode = EGunMode::Lightning;
			UGameplayStatics::PlaySound2D( GetWorld(), GunData->LightningAbilityOn );

			if ( PlayerCharacter->bIsAiming )
			{
				PlayerCharacter->StopAimAssist();
			}

			break;
		}
		case EGunMode::Lightning:
		{
			GunMode = EGunMode::Bullet;
			UGameplayStatics::PlaySound2D( GetWorld(), GunData->LightningAbilityOff );

			if ( PlayerCharacter->bIsAiming )
			{
				PlayerCharacter->StartAimAssist();
			}

			break;
		}
	}

	OnToggleWeaponMode.Broadcast( GunMode );

	if ( !GunData->ShootForceFeedback ) return;
	PlayerController->ClientPlayForceFeedback( GunData->SwitchModeForceFeedback );
}

void AGun::TriggerShootAbility( UPARAM( ref ) FVector& ImpactDirection )
{
	if ( !HandleCanFire() ) return;

	FVector ImpactPoint {};
	FHitResult HitResult {};
	CheckLineCollision( true, ImpactDirection, GunData->ShootingDistance, ImpactPoint, HitResult );


	OnBulletShoot.Broadcast();
	PlayerCharacter->PlayAnimMontage( GunData->ShootAnimationMontage );

	// Update shoot cooldown
	ShootCurentCooldown = UKismetSystemLibrary::GetGameTimeInSeconds( GetWorld() );

	// Update Reload component
	ReloadComponent->DecrementAmmo();

	// Alert nearby ennemies
	MakeNoise( 1.0f, PlayerCharacter );

	// Play Shooting Sound
	UGameplayStatics::PlaySound2D( GetWorld(), GunData->ShootSound );

	// Camera Feedback
	CameraShakeSourceComponent->Start();
	PlayerCharacter->CameraShakeFeedback();

	if ( !HitResult.bBlockingHit ) return;
	FVector NormalHit = HitResult.Normal;

	AddImpulseAtTrace( HitResult, GunData->ShootImpulseForce );

	// Tansform for decals & FX
	FTransform HitTransform( NormalHit.ToOrientationQuat(), ImpactPoint, FVector( 1.0f ) );

	// Spawn Parameters for decals & FX
	FActorSpawnParameters SpawnParameter {};
	SpawnParameter.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParameter.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

	UHealthComponent* HealthComponent = HitResult.GetActor()->GetComponentByClass<UHealthComponent>();
	if ( IsValid( HealthComponent ) )
	{
		if ( HealthComponent->bShouldEmitBloodParticles )
		{
			// Spawn system blood impact
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				GunData->BloodImpactNiagaraSystem,
				ImpactPoint,
				NormalHit.ToOrientationRotator()
			);

			// Spawn blood bullet decal
			AActor* BloodBulletActor = GetWorld()->SpawnActor(
				GunData->BloodBulletDecal,
				&HitTransform,
				SpawnParameter
			);

			BloodBulletActor->AttachToComponent(
				HitResult.GetComponent(),
				FAttachmentTransformRules::KeepWorldTransform
			);
		}

		// Apply damage to healthcomponent
		FDamageContext DamageContext {};
		DamageContext.DamageAmount = GunData->ShootDamage;
		DamageContext.DamageType = EDamageType::Bullet;
		DamageContext.AttackerActor = PlayerCharacter;
		DamageContext.InflictorActor = this;
		DamageContext.HitResult = HitResult;

		HealthComponent->TakeDamage( DamageContext );
	}
	else
	{
		// Spawn bullet decal
		GetWorld()->SpawnActor(
			GunData->BulletDecal,
			&HitTransform,
			SpawnParameter
		);

		FRotator ImpactRotation = UKismetMathLibrary::FindLookAtRotation(
			ImpactPoint,
			PlayerCharacter->GetActorLocation()
		);

		// Spawn system impact struct
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			GunData->StructurImpactNiagaraSystem,
			ImpactPoint,
			ImpactRotation
		);
	}

	// Spawn system muzzle flash
	UNiagaraComponent* MuzzleFlash = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		GunData->MuzzleFlashNiagaraSystem,
		ShootPoint->GetComponentTransform().GetLocation(),
		ShootPoint->GetComponentTransform().Rotator()
	);

	if ( !GunData->ShootForceFeedback ) return;
	PlayerController->ClientPlayForceFeedback( GunData->ShootForceFeedback );
}

void AGun::OnLightningStart()
{
	if ( CurrentEnergyAmount <= 0 ) return;

	bIsLightningActive = true;

	// REPLACE WITH DATA ASSETS
	PlayerCharacter->DualSenseComponent->SetRightTriggerEffect(
		UDualSenseFunctionLibrary::MakeVibration( 0.0f, 1.0f, 1.0f )
	);

	// Activate Preload Lightning Niagara Effect
	PreloadLightningNiagara->SetVisibility( true );
	PreloadLightningNiagara->Activate();

	PlayLightningChargeTimeline();

	if ( !GunData->LightningForceFeedback ) return;

	FForceFeedbackParameters ForceParam {};
	ForceParam.Tag = TEXT( "LightningForceFeedback" );
	ForceParam.bLooping = true;

	PlayerController->ClientPlayForceFeedback(
		GunData->LightningForceFeedback,
		ForceParam
	);
}

void AGun::OnLightningAbility( float ActionValue )
{
	if ( !bIsLightningCharged ) return;

	ChargingWeight = ActionValue;

	// REPLACE WITH DATA ASSETS
	PlayerCharacter->DualSenseComponent->SetRightTriggerEffect(
		UDualSenseFunctionLibrary::MakeVibration( 0.0f, 1.0f, 0.5f )
	);

	if ( CurrentEnergyAmount <= 0 )
	{
		UpdateCrossHairStopLightning();
		OnLightningEnd();
		return;
	}

	UpdateCrossHairOnLightning();
	RetrieveFirstBeamHit();
	TriggerEnvironmentFlickering();
}

void AGun::OnLightningEnd()
{
	StopSound(); // Shouldn't we play an ending sound instead ?

	PlayerCharacter->DualSenseComponent->SetRightTriggerEffect( UDualSenseFunctionLibrary::MakeOff() );

	if ( GunData->LightningForceFeedback )
	{
		PlayerController->ClientStopForceFeedback(
			GunData->LightningForceFeedback,
			TEXT( "LightningForceFeedback" )
		);
	}

	if ( !bIsLightningActive )
	{
		ReverseLightningChargeTimeline();
		return;
	}

	DeactivateAllEmitters();
	UpdateCrossHairStopLightning();

	bIsLightningCharged = false;
	bIsLightningActive = false;
	LightningChargeValue = 0.0f;

	LightningOrbNiagara->SetVisibility( false );
	LightningOrbNiagara->Deactivate();

	StopCameraAnimation();
	StopLightningChargeTimeline();
}

void AGun::GetReferences()
{
	PlayerCharacter = Cast<AMetroPlayerCharacter>( UGameplayStatics::GetPlayerCharacter( GetWorld(), 0 ) );
	PlayerController = UGameplayStatics::GetPlayerController( GetWorld(), 0 );

	ReloadComponent = PlayerCharacter->GetComponentByClass<UReloadComponent>();
	ReloadComponent->OnComputeReload.AddDynamic( this, &AGun::OnReload );

	ActorsToIgnore.Add( PlayerCharacter );
}

void AGun::CheckCurrentTargetType()
{
	if ( !PlayerCharacter->bIsAiming ) return;

	UHealthComponent* HealthComponent = nullptr;
	FVector ImpactDirection = PlayerCharacter->CineCameraComponent->GetForwardVector();

	FVector ImpactPoint {};
	FHitResult HitResult {};

	switch ( GunMode )
	{
	case EGunMode::Bullet:
	{
		CheckLineCollision( false, ImpactDirection, GunData->ShootingDistance, ImpactPoint, HitResult );
		if ( !IsValid( HitResult.GetActor() ) ) break;

		HealthComponent = HitResult.GetActor()->GetComponentByClass<UHealthComponent>();
		break;
	}
	case EGunMode::Lightning:
	{
		CheckSphereCollision( false, GunData->LightningDistance, ImpactPoint, HitResult, 20.0f );
		if ( !IsValid( HitResult.GetActor() ) ) break;

		HealthComponent = HitResult.GetActor()->GetComponentByClass<UHealthComponent>();
		break;
	}
	}

	if ( LastAimTarget != HealthComponent )
	{
		OnAimTargetChanged.Broadcast( HealthComponent );
		LastAimTarget = HealthComponent;
	}
}

