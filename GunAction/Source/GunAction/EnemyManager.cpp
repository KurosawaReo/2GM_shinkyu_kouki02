/*
   - EnemyManager -
   共通のCharacterクラスから派生した敵クラス.

   [クラス構成]
   CharacterBase
   └PlayerManager
   └EnemyManager  ←ここ
*/
#include "EnemyManager.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"

//他クラスのinclude.
#include "BulletBase.h"

#pragma region "get"
//死亡状態かどうか.
bool AEnemyManager::IsDead() const
{
	return CurrentState == EEnemyState::ES_Dead;
}
#pragma endregion

#pragma region "基本処理"
//コンストラクタ.
AEnemyManager::AEnemyManager()
{
	PrimaryActorTick.bCanEverTick = true;

	//キャラクター設定.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
}
//召喚した瞬間.
void AEnemyManager::BeginPlay() {

	ACharacterBase::BeginPlay(); //親クラスのBeginPlay()を呼び出す.

	//初期state.
	CurrentState = EEnemyState::ES_Alive;
	//一定時間ごとに弾を発射する.
	GetWorldTimerManager().SetTimer(tmShot, this, &AEnemyManager::ShotBullet, shotTime, true);
}
//常に実行.
void AEnemyManager::Tick(float DeltaTime) {

	ACharacterBase::Tick(DeltaTime); //親クラスのTick()を呼び出す.

	//プレイヤー取得.
	ACharacter* player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	//座標取得.
	FVector myPos = GetActorLocation();
	FVector plyPos = player->GetActorLocation();
	//座標差の計算.
	FVector dis = plyPos - myPos;

	//向きを設定.
	double ang = atan2(dis.Y, dis.X);
	FRotator rot(0, ang * 180 / PI, 0);
	SetActorRotation(rot, ETeleportType::None);

	//前方向に進ませる.
	//FVector forward(cos(ang), sin(ang), 0);
	//SetActorLocation(GetActorLocation() + forward * SPEED);
}
#pragma endregion

#pragma region "ダメージ処理"
//弾が当たったら実行される.
void AEnemyManager::OnBulletHit() 
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("hit enemy"));
	Die(); //死亡処理.
}
//死亡処理.
void AEnemyManager::Die()
{
	//既に死亡している場合は処理しない.
	if (IsDead()) { return; }

	CurrentState = EEnemyState::ES_Dead;

	//死亡後の処理.
	DisableComponents(); //コンポーネントを無効化.
	PlayDeathEffect();   //死亡エフェクト再生.
	PlayDeathSound();    //死亡音再生.
	Destroy();			 //消滅.
}

//死亡アニメーション再生.
void AEnemyManager::PlayDeathAnimation()
{

}
//死亡エフェクト再生.
void AEnemyManager::PlayDeathEffect()
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
//死亡音再生.
void AEnemyManager::PlayDeathSound()
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

//コンポーネント無効化.
void AEnemyManager::DisableComponents()
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
#pragma endregion

#pragma region "射撃"
/// <summary>
/// ShotBullet() - 発射操作をした時に実行する.
/// [敵専用]
/// </summary>
void AEnemyManager::ShotBullet()
{
	const FVector pos     = GetActorLocation();
	const FVector forward = GetActorForwardVector();

	//目標地点を計算(仮)
	const FVector TargetPosition = pos + forward * shotStartDist;
	//弾を発射.
	ShotBulletExe(this, TargetPosition);
}
#pragma endregion