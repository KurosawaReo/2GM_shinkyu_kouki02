
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletManager.generated.h"

UCLASS()
class GUNACTION_API ABulletManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ABulletManager();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	//ˆê’èŠÔŠu‚²‚Æ‚É”­ŽË.
	void ShotInterval(); //TODO <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
};
