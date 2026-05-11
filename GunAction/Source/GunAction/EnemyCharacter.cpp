/*
   - EnemyCharacter -
   共通のCharacterクラスから派生した敵クラス.

   [クラス構成]
   CharacterBase
   └PlayerCharacter
   └EnemyCharacter  ←ここ
*/
#include "EnemyCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"

//他クラスのinclude.
#include "BulletBase.h"

#pragma region "基本処理"

//コンストラクタ.
AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	//キャラクター設定.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
}

//召喚した瞬間に実行.
void AEnemyCharacter::BeginPlay() {

	ACharacterBase::BeginPlay(); //親クラスのBeginPlay()を呼び出す.

	//エラー対策.
	if (HasAnyFlags(RF_ClassDefaultObject)) {
		return;
	}

	//初期state.
	CurrentState = ECharaState::Alive;
	//一定時間ごとに実行.
	GetWorldTimerManager().SetTimer(tmShot,     this, &AEnemyCharacter::OnFire, spanShot, true);
	GetWorldTimerManager().SetTimer(tmChangeAI, this, &AEnemyCharacter::ChangeAIState, spanChangeAI, true);
}

//常に実行.
void AEnemyCharacter::Tick(float DeltaTime) {

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
	double deg = rad * 180 / PI;
	FRotator rot(0, deg, 0);
	SetActorRotation(rot, ETeleportType::None);

	FVector forward = FVector(); //方向設定用.
	//AI行動別.
	switch (AIState)
	{
	case EAIState::Goto:
		forward = { cos(deg * PI / 180), sin(rad * PI / 180), 0 };
		break;
	case EAIState::StepL:
		forward = { cos((deg + 90) * PI / 180), sin((deg + 90) * PI / 180), 0 };
		break;
	case EAIState::StepR:
		forward = { cos((deg - 90) * PI / 180), sin((deg - 90) * PI / 180), 0 };
		break;
	}
	//進む.
	Move(forward, 1.0f);
}

#pragma endregion

#pragma region "Get"

//死亡状態かどうか.
bool AEnemyCharacter::IsDead() const
{
	return CurrentState == ECharaState::Dead;
}

#pragma endregion

#pragma region "射撃"

/// <summary>
/// 射撃処理.
/// </summary>
void AEnemyCharacter::OnFire()
{
	//プレイヤー取得.
	ACharacter* player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	//目標地点.
	FVector Target = player->GetActorLocation();
	//射撃開始.
	ShotStart(Target);
}

#pragma endregion

#pragma region "ダメージ処理"

//弾が当たったら実行される.
void AEnemyCharacter::OnBulletHit() 
{
//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("hit enemy"));

	Death(); //死亡処理.
}

//死亡処理.
void AEnemyCharacter::Death()
{
	//既に死亡している場合は処理しない.
	if (IsDead()) { return; }

	CurrentState = ECharaState::Dead;

	//死亡後の処理.
	PlayDeathEffect();   //死亡エフェクト再生.
	PlayDeathSound();    //死亡音再生.
	DisableComponents(); //コンポーネントを無効化.
	Destroy();			 //消滅.
}

#pragma endregion

#pragma region "AI"

/// <summary>
/// AI行動選択.
/// </summary>
void AEnemyCharacter::ChangeAIState() {

	//列挙の数.
	const int max = StaticEnum<EAIState>()->NumEnums();
	//抽選.
	const int rnd = FMath::RandRange(0, max - 1);
	//行動変更.
	AIState = static_cast<EAIState>(rnd);
}

#pragma endregion
