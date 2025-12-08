/*
   こっちはおそらく仮で用意してた敵。
   本格的にはEnemyManagerを使う。
*/

// 死亡後の消滅時間
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Death")
float DeathDestroyDelay = 2.0f;// Enemy.cpp
#include "Enemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//キャラクター設定.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;

	CurrentState = EEnemyState::ES_Alive;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	CurrentState = EEnemyState::ES_Alive;

	
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
//弾との衝突処理.
//void AEnemy::OnBulletHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
void AEnemy::OnBulletHit() //引数いらない.
{
#if false
	//OherActorがnullptrまたは既に死亡している場合は処理しない.
	if (!OtherActor || IsDead())
	{
		return;
	}

	//弾であるか確認(プロジェクトに応じてタグやクラスで判定).
	if (OtherActor->ActorHasTag(FName("Bullet")))
	{
		Die();
	}
#endif
	//死亡処理.
	Die();
}
// 死亡処理.
void AEnemy::Die()
{
	//既に死亡している場合は処理しない.
	if (IsDead()) { return; }

	CurrentState = EEnemyState::ES_Dead;

	// コンポーネントを無効化.
	DisableComponents();

	// 死亡エフェクト再生.
	PlayDeathEffect();

	// 死亡音再生.
	PlayDeathSound();

	// すぐに削除.
	Destroy();
}
// 死亡状態かどうか.
bool AEnemy::IsDead() const
{
	return CurrentState == EEnemyState::ES_Dead;
}

// 死亡アニメーション再生.
void AEnemy::PlayDeathAnimation()
{

}
// 死亡エフェクト再生.
void AEnemy::PlayDeathEffect()
{
	if (DeathEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			DeathEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}
}

// 死亡音再生.
void AEnemy::PlayDeathSound()
{
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			DeathSound,
			GetActorLocation()
		);
	}
}

// コンポーネント無効化.
void AEnemy::DisableComponents()
{
	// 移動を停止
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
	}

	// カプセルのコリジョンを無効化
	if (GetCapsuleComponent())
	{
		//GetCapsuleComponent()->SetCollisionEnabled(ECC_NoCollision);
	}

	// Tickを停止
	SetActorTickEnabled(false);
}

