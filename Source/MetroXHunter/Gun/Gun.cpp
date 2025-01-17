/*
 * Implemented by Corentin Paya
 */

#include "Gun/Gun.h"
#include "Character/MetroPlayerCharacter.h"
#include "Health/HealthComponent.h"
#include "Reload/ReloadComponent.h"
#include "Gun/GunData.h"

#include "Camera/CameraShakeSourceComponent.h"
#include "CineCameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Sound/SoundBase.h"
#include "Library/GameplayLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

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

			// TODO: MAKE FEEDBACK (DUAL SENSE) CANT FIRE
			return false;
		}

		if ( ReloadComponent->IsReloading() )
		{
			// TODO: MAKE FEEDBACK (DUAL SENSE) CANT FIRE
			return false;
		}

		// Check if the shoot is on cooldown
		float CurrentGameTime = UKismetSystemLibrary::GetGameTimeInSeconds( GetWorld() );
		if ( CurrentGameTime - ShootCurentCooldown >= GunData->ShootCooldown )
		{
			// TODO: MAKE WEAPON (DUAL SENSE) CAN FIRE
			return true;
		}
		else
		{
			// TODO: MAKE FEEDBACK (DUAL SENSE) CANT FIRE
			return false;
		}
	}
	case EGunMode::Lightning:
	{
		if ( bIsLightningCharged && CurrentEnergyAmount > 0 ) return true;

		return false;
	}
	}

	return false;
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

	TriggerLightningBeam( ImpactPoint );
	bIsLightningActive = true;

	// TODO: ADD FORCE FEEDBACK (HAPTIC)
}

void AGun::SwitchWeapon()
{
	// TODO: ADD FORCE FEEDBACK (HAPTIC)

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
}

void AGun::TriggerShootAbility( UPARAM( ref ) FVector& ImpactDirection )
{
	if ( !HandleCanFire() ) return;

	OnBulletShoot.Broadcast();

	// Update shoot cooldown
	ShootCurentCooldown = UKismetSystemLibrary::GetGameTimeInSeconds( GetWorld() );

	// Update Reload component
	ReloadComponent->DecrementAmmo();

	// Camera Feedback
	CameraShakeSourceComponent->Start();
	PlayerCharacter->CameraShakeFeedback();

	// Alert nearby ennemies
	MakeNoise( 1.0f, PlayerCharacter );

	// Play Shooting Sound
	UGameplayStatics::PlaySound2D( GetWorld(), GunData->ShootSound );

	FVector ImpactPoint {};
	FHitResult HitResult {};
	CheckLineCollision( true, ImpactDirection, GunData->ShootingDistance ,ImpactPoint, HitResult );

	if ( !HitResult.bBlockingHit ) return;
	FVector NormalHit = HitResult.Normal;

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

	// TODO: ADD FORCE FEEDBACK (HAPTIC)
}

void AGun::OnLightningStart()
{
	if ( CurrentEnergyAmount <= 0 ) return;

	// TODO: SET RIGHT TRIGGER EFFECT (HAPTIC)

	// Activate Preload Lightning Niagara Effect
	PreloadLightningNiagara->SetVisibility( true );
	PreloadLightningNiagara->Activate();

	PlayLightningChargeTimeline();
}

void AGun::OnLightningAbility( float ActionValue )
{
	if ( !bIsLightningCharged ) return;

	ChargingWeight = ActionValue;

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

	// STOP VIBRATION
	StopCameraAnimation();
	StopLightningChargeTimeline();
}

void AGun::GetReferences()
{
	PlayerCharacter = Cast<AMetroPlayerCharacter>( UGameplayStatics::GetPlayerCharacter( GetWorld(), 0 ) );
	ReloadComponent = PlayerCharacter->GetComponentByClass<UReloadComponent>();

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
			CheckSphereCollision( false, GunData->LightningDistance ,ImpactPoint, HitResult, 20.0f );
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

