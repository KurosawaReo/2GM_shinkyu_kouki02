// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_FireBullet.h"
#include "PlayerManager.h"
void UAnimNotify_FireBullet::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp == nullptr)return;

	AActor* Owner = MeshComp->GetOwner();
	if (Owner == nullptr) return;

	APlayerManager* Player = Cast<APlayerManager>(Owner);
	if (Player)
	{
		Player->FireBullet();
	}
}
