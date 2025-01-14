/*
 * Implemented by Corentin Paya
 */

#include "Gun/Gun.h"
#include "Character/MetroPlayerCharacter.h"
#include "Health/HealthComponent.h"
#include "Reload/ReloadComponent.h"
#include "Gun/GunData.h"

#include "Camera/CameraShakeSourceComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Sound/SoundBase.h"
#include "Library/GameplayLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"

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

	GetReferences();
}

void AGun::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

bool AGun::CanFire()
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
}

void AGun::TriggerShootAbility( UPARAM( ref ) FVector& ImpactDirection )
{
	if ( !CanFire() ) return;

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

	// Retrieve Hit Actor & Impact point
	FVector ImpactPoint {};
	FHitResult HitResult {};
	CheckLineCollision( true, ImpactDirection, ImpactPoint, HitResult );

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
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		GunData->MuzzleFlashNiagaraSystem,
		ShootPoint->GetComponentTransform().GetLocation(),
		ShootPoint->GetComponentTransform().Rotator()
	);

	// TODO: ADD FORCE FEEDBACK (HAPTIC)
}

void AGun::OnLightningStart()
{
}

void AGun::OnLightningAbility()
{
}

void AGun::OnLightningEnd()
{
}

void AGun::GetReferences()
{
	PlayerCharacter = Cast<AMetroPlayerCharacter>( UGameplayStatics::GetPlayerCharacter( GetWorld(), 0 ) );
	ReloadComponent = PlayerCharacter->GetComponentByClass<UReloadComponent>();

	ActorsToIgnore.Add( PlayerCharacter );
}

