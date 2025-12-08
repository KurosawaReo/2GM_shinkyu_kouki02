/*
   こっちはおそらく仮で用意してた敵。
   本格的にはEnemyManagerを使う。
*/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	ES_Alive UMETA(DisplayName = "Alive"),
	ES_Dead UMETA(DisplayName = "Dead")
};

UCLASS()
class GUNACTION_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//弾との衝撃処理.
	UFUNCTION()
//	void OnBulletHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	void OnBulletHit();

	//死亡処理.
	virtual void Die();

	//死亡状態の取得.
	UFUNCTION(BlueprintCallable,Category = "Enemy")
	bool IsDead() const;


	// Called to bind functionality to input
	

protected:
	// エネミーのステート
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	EEnemyState CurrentState;

	// 死亡時のエフェクト
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Death")
	class UParticleSystem* DeathEffect;

	// 死亡時の音
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Death")
	class USoundBase* DeathSound;

	// 死亡アニメーション再生
	virtual void PlayDeathAnimation();

	// 死亡エフェクト再生
	virtual void PlayDeathEffect();

	// 死亡音再生
	virtual void PlayDeathSound();

	// コンポーネント無効化
	virtual void DisableComponents();
};
