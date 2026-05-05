/*
   - AWeaponHammer -
   武器クラス「ハンマー」
*/
#include "WeaponHammer.h"

// Sets default values
AWeaponHammer::AWeaponHammer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWeaponHammer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeaponHammer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

