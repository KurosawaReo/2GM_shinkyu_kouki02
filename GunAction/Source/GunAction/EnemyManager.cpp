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

#pragma region "Get"
//死亡状態かどうか.
bool AEnemyManager::IsDead() const
{
	return CurrentState == ECharaState::Dead;
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
	CurrentState = ECharaState::Alive;
	//一定時間ごと.
	GetWorldTimerManager().SetTimer(tmShot,     this, &AEnemyManager::ShotBullet,    spanShot,     true);
	GetWorldTimerManager().SetTimer(tmChangeAI, this, &AEnemyManager::ChangeAIState, spanChangeAI, true);
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
	double rad = atan2(dis.Y, dis.X);
	double deg = rad * 180/PI;
	FRotator rot(0, deg, 0);
	SetActorRotation(rot, ETeleportType::None);

	FVector forward; //方向設定用.
	//AI行動別.
	switch (AIState) 
	{
		case EAIState::Goto:
			forward = {cos(deg * PI/180), sin(rad * PI/180), 0};
			break;
		case EAIState::StepL:
			forward = {cos((deg+90) * PI/180), sin((deg+90) * PI/180), 0};
			break;
		case EAIState::StepR:
			forward = {cos((deg-90) * PI/180), sin((deg-90) * PI/180), 0};
			break;
	}
	//進む.
	AddMovementInput(forward);
}
#pragma endregion

#pragma region "AI"
/// <summary>
/// AI行動選択.
/// </summary>
void AEnemyManager::ChangeAIState() {
	//列挙の数.
	const int max = StaticEnum<EAIState>()->NumEnums();
	//抽選.
	const int rnd = FMath::RandRange(0, max-1);
	//行動変更.
	AIState = static_cast<EAIState>(rnd);
}
#pragma endregion

#pragma region "射撃"
/// <summary>
/// ShotBullet() - 発射操作をした時に実行する.
/// [敵専用]
/// </summary>
void AEnemyManager::ShotBullet()
{
	//発射していいかチェック.
	if (!ShotBulletCheck()) {
		return;
	}
	//弾が残っていれば.
	if (AmmoCount > 0) {
		//プレイヤー取得.
		ACharacter* player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
		//目標地点を計算.
		const FVector TargetPosition = player->GetActorLocation();
		//弾を発射.
		ShotBulletExe(this, TargetPosition);
	}
}
#pragma endregion

#pragma region "ダメージ処理"
//弾が当たったら実行される.
void AEnemyManager::OnBulletHit() 
{
//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("hit enemy"));
	Die(); //死亡処理.
}
//死亡処理.
void AEnemyManager::Die()
{
	//既に死亡している場合は処理しない.
	if (IsDead()) { return; }

	CurrentState = ECharaState::Dead;

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
	// カプセルのコリジョンを無効化
	//GetCapsuleComponent()->SetCollisionEnabled(ECC_NoCollision);

	//移動を停止.
	if (auto cmp = GetCharacterMovement()) {
		cmp->StopMovementImmediately();
		cmp->DisableMovement();
	}
	//銃を消滅.
	if (RevolverGun) {
		RevolverGun->Destroy();
	}
	//Tickを停止.
	SetActorTickEnabled(false);
}
#pragma endregion