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
/// 敵クラス.
/// </summary>
UCLASS()
class GUNACTION_API AEnemyManager : public ACharacterBase
{
	GENERATED_BODY()
	
//▼ ===== 変数 ===== ▼.
// 
//▼ ===== 関数 ===== ▼.
protected:
#pragma region "ライフサイクル"
	//召喚した瞬間.
	void BeginPlay() override;
	//常に実行.
	void Tick(float DeltaTime) override;
#pragma endregion
};
