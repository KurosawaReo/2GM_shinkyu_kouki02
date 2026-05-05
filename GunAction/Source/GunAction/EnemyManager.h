/*
   - EnemyManager -
   共通のCharacterクラスから派生した敵クラス.

   [クラス構成]
   CharacterBase
   └PlayerManager 
   └EnemyManager  ←今ここ
*/
#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h" //親クラス.
#include "EnemyManager.generated.h"

/// <summary>
/// AI行動のstate.
/// </summary>
UENUM(BlueprintType)
enum class EAIState : uint8
{
	Goto  UMETA(DisplayName = "Goto"),  //突っ込む.
	StepL UMETA(DisplayName = "StepL"), //左に避ける.
	StepR UMETA(DisplayName = "StepR"), //右に避ける.
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

#pragma region "基本"
	//キャラクター状態.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Enemy")
	ECharaState CurrentState;
	//AI行動状態.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Enemy")
	EAIState AIState;
#pragma region

#pragma region "射撃"
	//射撃間隔.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Enemy|Shot")
	float spanShot = 0.2f;
#pragma region

#pragma region "AI"
	//AI行動選択間隔.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Enemy|AI")
	float spanChangeAI = 1.0f;
#pragma region

	//タイマー.
	FTimerHandle tmShot;	 //射撃間隔.
	FTimerHandle tmChangeAI; //AI行動選択間隔.

//▼ ===== 関数 ===== ▼.
protected:
#pragma region "基本処理"
	AEnemyManager();						//コンストラクタ.
	void BeginPlay()           override;	//召喚した瞬間.
	void Tick(float DeltaTime) override;	//常に実行.
#pragma endregion

public:
#pragma region "Get"
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	bool IsDead() const;					//死亡状態の取得.
#pragma endregion

#pragma region "射撃"
	void OnFire();							//射撃開始.
#pragma endregion

#pragma region "ダメージ・死亡"
	void OnBulletHit() override;			//弾が当たったら実行される.
	void Death()       override;			//死亡処理.
#pragma endregion

#pragma region "AI"
	void ChangeAIState();
#pragma endregion
};