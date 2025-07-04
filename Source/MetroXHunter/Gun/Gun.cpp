/*
 * Implemented by Corentin Paya
 */

#include "Gun/Gun.h"
#include "Character/MetroPlayerCharacter.h"
#include "Health/HealthComponent.h"
#include "Reload/ReloadComponent.h"
#include "Gun/GunData.h"

#include "Sounds/GunSoundManagerComponent.h"

#include <DualSenseFunctionLibrary.h>
#include "DualSenseControllerComponent.h"
#include "Camera/CameraShakeSourceComponent.h"
#include "CineCameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Library/GameplayLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "Animation/AnimationAsset.h"

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

	SoundManagerComponent = GetComponentByClass<UGunSoundManagerComponent>();
	checkf( IsValid( SoundManagerComponent ), TEXT( "Add a GunSoundManagerComponent on the Gun class!" ) );

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
	UpdateDualSense();
}

void AGun::UpdateDualSense()
{
	if ( !PlayerCharacter ) return;
	if ( !PlayerCharacter->bIsAiming ) return;
	if ( !bShouldUpdateDualSense ) return;

	if ( !HandleCanFire() )
	{
		PlayerCharacter->DualSenseComponent->SetRightTriggerEffect(
			UDualSenseFunctionLibrary::MakeContinuousResistance( 0.0f, 1.0f )
		);
	}
	else
	{
		PlayerCharacter->DualSenseComponent->SetRightTriggerEffect(
			UDualSenseFunctionLibrary::MakeWeapon( 0.0f, 1.0f, 0.70f )
		);
	}
}

bool AGun::HandleCanFire()
{
	switch ( GunMode )
	{
	case EGunMode::Bullet:
	{
		if ( ReloadComponent->IsReloading() ) return false;

		if ( ReloadComponent->IsGunEmpty() ) return false;

		float CurrentGameTime = UKismetSystemLibrary::GetGameTimeInSeconds( GetWorld() );
		if ( CurrentGameTime - ShootCurentCooldown >= GunData->ShootCooldown ) return true;

		return false;
	}
	case EGunMode::Lightning:
	{
		return true;
	}
	}

	return false;
}

void AGun::OnReload()
{
	PlayerCharacter->PlayAnimMontage( GunData->ReloadAnimationMontage );
	WeaponSkeletonMesh->GetAnimInstance()->Montage_Play( GunData->WeaponReloadAnimationMontage );
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
		AddForceAtTrace( HitResult, GunData->LightningImpulseForce );
	}
	else
	{
		FVector WorldLocation = PlayerCharacter->CineCameraComponent->GetComponentLocation();
		FVector ForwardVector = PlayerCharacter->CineCameraComponent->GetForwardVector();

		ImpactPoint = WorldLocation + ( ForwardVector * GunData->LightningDistance );
	}

	TriggerLightningBeam( ImpactPoint );
}

void AGun::StopLightning()
{
	if ( LightningState == ELightningState::Active )
	{
		OnLightningEnd();
	}
}

void AGun::SwitchWeapon()
{
	if ( PlayerCharacter->bIsUnderAction || !PlayerCharacter->bIsWeaponEquipped ) return;

	switch ( GunMode )
	{
	case EGunMode::Bullet:
	{
		GunMode = EGunMode::Lightning;
		SoundManagerComponent->OnLightningSwitch( /* bIsLightningMpdeActive */ true );

		if ( PlayerCharacter->bIsAiming )
		{
			PlayerCharacter->StopAimAssist();
		}

		break;
	}
	case EGunMode::Lightning:
	{
		GunMode = EGunMode::Bullet;
		SoundManagerComponent->OnLightningSwitch( /* bIsLightningModeActive */ false );

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
	if ( !HandleCanFire() )
	{
		if ( ReloadComponent->IsGunEmpty() )
		{
			SoundManagerComponent->OnRifleShootEmpty();
			OnGunEmpty.Broadcast();
		}

		return;
	}

	FVector ImpactPoint {};
	FHitResult HitResult {};
	CheckLineCollision( true, ImpactDirection, GunData->ShootingDistance, ImpactPoint, HitResult );

	OnBulletShoot.Broadcast();
	PlayerCharacter->PlayAnimMontage( GunData->ShootAnimationMontage );
	WeaponSkeletonMesh->GetAnimInstance()->Montage_Play( GunData->WeaponShootAnimationMontage );

	// Update shoot cooldown
	ShootCurentCooldown = UKismetSystemLibrary::GetGameTimeInSeconds( GetWorld() );

	// Update Reload component
	ReloadComponent->DecrementAmmo();

	// Alert nearby ennemies
	MakeNoise( 1.0f, PlayerCharacter );

	// Play Shooting Sound
	SoundManagerComponent->OnRifleShoot( HitResult );

	// Camera Feedback
	CameraShakeSourceComponent->Start();
	PlayerCharacter->CameraShakeFeedback();

	// Spawn system muzzle flash
	UNiagaraComponent* MuzzleFlash = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		GunData->MuzzleFlashNiagaraSystem,
		ShootPoint->GetComponentTransform().GetLocation(),
		ShootPoint->GetComponentTransform().Rotator()
	);

	if ( GunData->ShootForceFeedback )
	{
		PlayerController->ClientPlayForceFeedback( GunData->ShootForceFeedback );
	}

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

		const EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType( HitResult );
		if ( SurfaceType == GunData->GlassSurfaceType )
		{
			// Spawn system glass struct
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				GunData->GlassImpactNiagaraSystem,
				ImpactPoint,
				ImpactRotation
			);
		}
		else
		{
			// Spawn system impact struct
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				GunData->StructurImpactNiagaraSystem,
				ImpactPoint,
				ImpactRotation
			);
		}
	}
}

void AGun::OnLightningStart()
{
	if ( CurrentEnergyAmount <= 0 ) return;

	// REPLACE WITH DATA ASSETS
	PlayerCharacter->DualSenseComponent->SetRightTriggerEffect(
		UDualSenseFunctionLibrary::MakeVibration( 0.0f, 1.0f, 1.0f )
	);

	// Activate Preload Lightning Niagara Effect
	PreloadLightningNiagara->SetVisibility( true );
	PreloadLightningNiagara->Activate();

	LightningState = ELightningState::Charging;
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
	if ( LightningState != ELightningState::Active ) return;

	ChargingWeight = ActionValue;

	// REPLACE WITH DATA ASSETS
	PlayerCharacter->DualSenseComponent->SetRightTriggerEffect(
		UDualSenseFunctionLibrary::MakeVibration( 0.0f, 1.0f, 0.5f )
	);

	if ( CurrentEnergyAmount <= 0 )
	{
		SoundManagerComponent->OnLightningShootEmpty();

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
	SoundManagerComponent->OnLightningStopShoot();

	PlayerCharacter->DualSenseComponent->SetRightTriggerEffect( UDualSenseFunctionLibrary::MakeOff() );

	if ( GunData->LightningForceFeedback )
	{
		PlayerController->ClientStopForceFeedback(
			GunData->LightningForceFeedback,
			TEXT( "LightningForceFeedback" )
		);
	}

	if ( LightningState == ELightningState::Active )
	{
		DeactivateAllEmitters();

		UpdateCrossHairStopLightning();
		LightningOrbNiagara->SetVisibility( false );
		LightningOrbNiagara->Deactivate();

		StopCameraAnimation();
	}

	ReverseLightningChargeTimeline();
}

void AGun::OnChargeStop()
{
	SoundManagerComponent->OnLightningStopCharging();

	PlayerCharacter->DualSenseComponent->SetRightTriggerEffect( UDualSenseFunctionLibrary::MakeOff() );

	if ( GunData->LightningForceFeedback )
	{
		PlayerController->ClientStopForceFeedback(
			GunData->LightningForceFeedback,
			TEXT( "LightningForceFeedback" )
		);
	}

	ReverseLightningChargeTimeline();
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
		OnAimTargetChanged.Broadcast( HealthComponent, HitResult );
		LastAimTarget = HealthComponent;
	}
}

