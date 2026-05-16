/*
   - EnemyCharacter -
   共通のCharacterクラスから派生した敵クラス.

   [クラス構成]
   CharacterBase
   └PlayerCharacter 
   └EnemyCharacter  ←今ここ
*/
#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h" //親クラス.
#include "EnemyCharacter.generated.h"

/*
   UEの列挙体は、最後に"MAX"という値が自動で追加される.

   StaticEnum<EAIState>()->NumEnums()
   で取得すると、値が6つでも"7"と返ってくるため注意.
*/

/// <summary>
/// AI行動のstate.
/// </summary>
UENUM(BlueprintType)
enum class EAIState : uint8
{
	Goto  UMETA(DisplayName = "Goto"),  //突っ込む.
	StepL UMETA(DisplayName = "StepL"), //左に移動.
	StepR UMETA(DisplayName = "StepR"), //右に移動.
	Jump  UMETA(DisplayName = "Jump"),  //ジャンプ.
	Shot  UMETA(DisplayName = "Shot"),  //撃つ.
	Roll  UMETA(DisplayName = "Roll"),  //ローリング.

	Count UMETA(Hidden)                 //state総数.

//  EAIState_MAX                        //[注意]自動追加される.
};

/// <summary>
/// 敵クラス.
/// </summary>
UCLASS()
class GUNACTION_API AEnemyCharacter : public ACharacterBase
{
	GENERATED_BODY()
	
//▼ ===== 変数 ===== ▼.
public:

#pragma region "AI"
	//AI行動状態.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Enemy|AI")
	EAIState AIState;
	//AI行動選択間隔.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Enemy|AI")
	float spanChangeAI = 1.0f;
#pragma region

	FVector  moveVec; //進行方向.
	FRotator moveRot; //進行角度.

	//タイマー.
	FTimerHandle tmChangeAI; //AI行動選択間隔.

//▼ ===== 関数 ===== ▼.
protected:
#pragma region "基本処理"
	AEnemyCharacter();						//コンストラクタ.
	void BeginPlay()           override;	//召喚した瞬間に実行.
	void Tick(float DeltaTime) override;	//常に実行.
#pragma endregion

public:
#pragma region "ダメージ・死亡"
	void Death() override;	//死亡処理.
#pragma endregion

#pragma region "AI・行動"
	void OnJump();							//ジャンプ.
	void OnShot();							//射撃開始.
	void OnRoll();							//ローリング.
	void ChangeAIState();					//AI行動選択.
	void AIAction();						//AI行動.
#pragma endregion
};