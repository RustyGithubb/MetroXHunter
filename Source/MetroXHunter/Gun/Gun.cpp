/*
 * Implemented by Corentin Paya
 */

#include "Gun/Gun.h"

AGun::AGun()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGun::BeginPlay()
{
	Super::BeginPlay();
}

void AGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

