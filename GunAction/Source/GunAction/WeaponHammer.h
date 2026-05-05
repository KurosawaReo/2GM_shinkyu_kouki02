/*
   - AWeaponHammer -
   武器クラス「ハンマー」
*/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponHammer.generated.h"

UCLASS()
class GUNACTION_API AWeaponHammer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponHammer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
