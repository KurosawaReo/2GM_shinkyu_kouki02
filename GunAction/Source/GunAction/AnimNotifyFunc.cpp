/*
   - AnimNotifyFunc -
*/
#include "AnimNotifyFunc.h"
#include "PlayerManager.h"

void UAnimNotifyFunc::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	//エラー対策.
	if (MeshComp == nullptr) { return; }

	AActor* Owner = MeshComp->GetOwner();
	if (Owner == nullptr) { return; }

	//プレイヤーに変換.
	APlayerManager* Player = Cast<APlayerManager>(Owner);
	if (Player)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("ugoita C1"));

		//射撃実行.
		Player->ShotBulletTiming();
	}
}
