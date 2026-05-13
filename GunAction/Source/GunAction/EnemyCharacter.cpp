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
	GetWorldTimerManager().SetTimer(tmChangeAI, this, &AEnemyCharacter::ChangeAIState, spanChangeAI, true);
}

//常に実行.
void AEnemyCharacter::Tick(float DeltaTime) {

	ACharacterBase::Tick(DeltaTime); //親クラスのTick()を呼び出す.

	//移動.
	Move(moveVec, 1.0f);
	//向き補完.
	MoveRotateSetting(GetActorRotation(), moveRot);
}

#pragma endregion

#pragma region "Get"

//死亡状態かどうか.
bool AEnemyCharacter::IsDead() const
{
	return CurrentState == ECharaState::Dead;
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
/// 射撃処理.
/// </summary>
void AEnemyCharacter::OnShot()
{
	//プレイヤー取得.
	ACharacter* player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	//目標地点.
	FVector Target = player->GetActorLocation();
	//射撃開始.
	ShotStart(Target);
}

/// <summary>
/// ローリング.
/// </summary>
void AEnemyCharacter::OnRoll()
{
	ACharacterBase::OnRoll();
}

/// <summary>
/// ジャンプ.
/// </summary>
void AEnemyCharacter::OnJump()
{
	ACharacterBase::OnJump();
}

/// <summary>
/// AI行動選択.
/// </summary>
void AEnemyCharacter::ChangeAIState() {

	//列挙の数.
	const int max = static_cast<int>(EAIState::Count);
	//抽選.
	const int rnd = FMath::RandRange(0, max - 1);
	//行動変更.
	AIState = static_cast<EAIState>(rnd);

	AIAction();
}

/// <summary>
/// AI行動.
/// </summary>
void AEnemyCharacter::AIAction() {

	//プレイヤー取得.
	ACharacter* player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	//座標取得.
	FVector myPos = GetActorLocation();
	FVector plyPos = player->GetActorLocation();
	//座標差の計算.
	FVector dis = plyPos - myPos;
	//向きを計算.
	const double rad = atan2(dis.Y, dis.X);

	//AI行動別.
	switch (AIState)
	{
		case EAIState::Goto:
		{
			moveVec.X = cos(rad);
			moveVec.Y = sin(rad);
		}
		break;

		case EAIState::StepL:
		{
			moveVec.X = cos(rad + PI / 2);
			moveVec.Y = sin(rad + PI / 2);
		}
		break;

		case EAIState::StepR:
		{
			moveVec.X = cos(rad - PI / 2);
			moveVec.Y = sin(rad - PI / 2);
		}
		break;

		case EAIState::Jump:
		{
			OnJump();
		}
		break;

		case EAIState::Shot:
		{
			moveVec = FVector::Zero(); //ベクトルリセット.
//			moveRot = FRotator(0, rad * 180 / PI, 0); //プレイヤーの方へ向く.

			OnShot();
		}
		break;

		case EAIState::Roll:
		{
			moveVec = FVector::Zero(); //ベクトルリセット.

			OnRoll();
		}
		break;

		default:
			FString text = FString::Printf(TEXT("Invalid Enum(%d): AEnemyCharacter 'AIState'"), static_cast<int>(AIState));
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, text);
			break;
	}

	//移動方向へ向く.
	if (!moveVec.IsNearlyZero())
	{
		moveRot = moveVec.Rotation();
	}
}

#pragma endregion
