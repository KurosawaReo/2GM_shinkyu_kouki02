/*
   - EnemyManager -
   共通のCharacterクラスから派生した敵クラス.

   [クラス構成]
   CharacterBase
   └PlayerManager 
   └EnemyManager  ←ここ
*/
#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h" //親クラス.
#include "EnemyManager.generated.h"

/// <summary>
/// 敵のstate列挙体.
/// UEではこういう書き方をするっぽい.
/// </summary>
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	ES_Alive UMETA(DisplayName = "Alive"),
	ES_Dead  UMETA(DisplayName = "Dead")
};

/// <summary>
/// 敵クラス.
/// </summary>
UCLASS()
class GUNACTION_API AEnemyManager : public ACharacterBase
{
	GENERATED_BODY()
	
//▼ ===== 変数 ===== ▼.
public:
	//敵の今の状態.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	EEnemyState CurrentState;

	//死亡時のエフェクト.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Death")
	class UParticleSystem* DeathEffect;

	//死亡時のサウンド.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Death")
	class USoundBase* DeathSound;
// 
//▼ ===== 関数 ===== ▼.
public:
#pragma region "get"
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	bool IsDead() const;					//死亡状態の取得.
#pragma endregion

#pragma region "基本処理"
	AEnemyManager();						//コンストラクタ.
	void BeginPlay()           override;	//召喚した瞬間.
	void Tick(float DeltaTime) override;	//常に実行.
#pragma endregion

#pragma region "ダメージ処理"
	UFUNCTION()
	void OnBulletHit();						//弾が当たったら実行される.
	void Die();								//死亡処理.
	
	void PlayDeathAnimation();				//死亡アニメーション再生.
	void PlayDeathEffect();					//死亡エフェクト再生.
	void PlayDeathSound();					//死亡音再生.

	void DisableComponents();				//コンポーネント無効化.
#pragma endregion
};