/*
   - AnimNotifyFunc -
*/
#include "AnimNotifyFunc.h"
#include "PlayerManager.h"

void UAnimNotifyFunc::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	//ƒGƒ‰[‘Îô.
	if (MeshComp == nullptr) { return; }

	AActor* Owner = MeshComp->GetOwner();
	if (Owner == nullptr) { return; }

	APlayerManager* Player = Cast<APlayerManager>(Owner);
	if (Player)
	{
		Player->ShotBulletTiming();
	}
}
