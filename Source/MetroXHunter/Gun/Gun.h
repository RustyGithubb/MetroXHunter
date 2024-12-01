/*
 * Implemented by Corentin Paya
 */
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gun.generated.h"

class UGunData;

UENUM( BlueprintType )
enum class EGunMode : uint8
{
	Bullet,
	Lightning,
};

UCLASS()
class METROXHUNTER_API AGun : public AActor
{
	GENERATED_BODY()
	
public:	
	AGun();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, Category = "GunMode" )
	EGunMode GunMode;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "GunData" )
	UGunData* GunData;
};
