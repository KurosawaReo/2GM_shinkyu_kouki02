/*
   - EnemyManager -
   共通のCharacterクラスから派生した敵クラス.

   [クラス構成]
   CharacterBase
   └PlayerManager
   └EnemyManager  ←ここ
*/
#include "EnemyManager.h"

//召喚した瞬間.
void AEnemyManager::BeginPlay() {

	ACharacterBase::BeginPlay(); //親クラスのBeginPlay()を呼び出す.

	//TODO.
}

//常に実行.
void AEnemyManager::Tick(float DeltaTime) {

	ACharacterBase::Tick(DeltaTime); //親クラスのTick()を呼び出す.

	//TODO.
}