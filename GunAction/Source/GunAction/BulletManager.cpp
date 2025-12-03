

#include "BulletManager.h"

ABulletManager::ABulletManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABulletManager::BeginPlay()
{
	Super::BeginPlay();
}

void ABulletManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


