/*
   - AnimNotifyFunc -
*/
#include "AnimNotifyFunc.h"

#include "PlayerManager.h"
#include "EnemyManager.h"

void UAnimNotifyFunc::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	//エラー対策.
	if (MeshComp == nullptr) { return; }
	//owner取得.
	AActor* Owner = MeshComp->GetOwner();
	if (Owner == nullptr) { return; }

	//ownerがプレイヤーActorなら.
	if (auto* Player = Cast<APlayerManager>(Owner)) {
		Player->ShotExe();
	}
	//ownerが敵Actorなら
	if (auto* Enemy = Cast<AEnemyManager>(Owner)) {
		Enemy->ShotExe();
	}
}

