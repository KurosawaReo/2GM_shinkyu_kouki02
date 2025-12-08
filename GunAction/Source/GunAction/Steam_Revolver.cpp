// Fill out your copyright notice in the Description page of Project Settings.

#include "Steam_Revolver.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

ASteam_Revolver::ASteam_Revolver()
{
	PrimaryActorTick.bCanEverTick = false;

	// ルートコンポーネント
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	// 発砲音声コンポーネント
	S_Revolver_Shot_03_Cue = CreateDefaultSubobject<UAudioComponent>(TEXT("S_Revolver_Shot_03_Cue"));
	S_Revolver_Shot_03_Cue->SetupAttachment(DefaultSceneRoot);
	S_Revolver_Shot_03_Cue->bAutoActivate = false;
	S_Revolver_Shot_03_Cue->SetVolumeMultiplier(1.0f); 

	S_Revolver_Shot_04_Cue = CreateDefaultSubobject<UAudioComponent>(TEXT("S_Revolver_Shot_04_Cue"));
	S_Revolver_Shot_04_Cue->SetupAttachment(DefaultSceneRoot);
	S_Revolver_Shot_04_Cue->bAutoActivate = false;
	S_Revolver_Shot_04_Cue->SetVolumeMultiplier(1.0f);

	S_Revolver_Shot_02_Cue = CreateDefaultSubobject<UAudioComponent>(TEXT("S_Revolver_Shot_02_Cue"));
	S_Revolver_Shot_02_Cue->SetupAttachment(DefaultSceneRoot);
	S_Revolver_Shot_02_Cue->bAutoActivate = false;
	S_Revolver_Shot_02_Cue->SetVolumeMultiplier(1.0f);

	S_Revolver_Shot_05_Cue = CreateDefaultSubobject<UAudioComponent>(TEXT("S_Revolver_Shot_05_Cue"));
	S_Revolver_Shot_05_Cue->SetupAttachment(DefaultSceneRoot);
	S_Revolver_Shot_05_Cue->bAutoActivate = false;
	S_Revolver_Shot_05_Cue->SetVolumeMultiplier(1.0f);

	// マズルフラッシュパーティクル
	PS_Muzzleflash_Revolver = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("PS_Muzzleflash_Revolver"));
	PS_Muzzleflash_Revolver->SetupAttachment(DefaultSceneRoot);
	PS_Muzzleflash_Revolver->bAutoActivate = false;

	// スケルタルメッシュ（リボルバーの本体）
	Steam_Revolver = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Steam_Revolver"));
	Steam_Revolver->SetupAttachment(DefaultSceneRoot);

	// 弾丸とシェルコンポーネント
	Bullet_4 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet_4"));
	Bullet_4->SetupAttachment(Steam_Revolver);

	Shell_4 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shell_4"));
	Shell_4->SetupAttachment(Bullet_4);

	Bullet_5 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet_5"));
	Bullet_5->SetupAttachment(Steam_Revolver);

	Shell_5 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shell_5"));
	Shell_5->SetupAttachment(Bullet_5);

	Bullet_1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet_1"));
	Bullet_1->SetupAttachment(Steam_Revolver);

	Shell_1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shell_1"));
	Shell_1->SetupAttachment(Bullet_1);

	Bullet_2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet_2"));
	Bullet_2->SetupAttachment(Steam_Revolver);

	Shell_2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shell_2"));
	Shell_2->SetupAttachment(Bullet_2);

	Bullet_3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet_3"));
	Bullet_3->SetupAttachment(Steam_Revolver);

	Shell_3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shell_3"));
	Shell_3->SetupAttachment(Bullet_3);

	// マズル
	Muzzle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(Steam_Revolver);

	Bullet_6 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet_6"));
	Bullet_6->SetupAttachment(Steam_Revolver);

	Shell_6 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shell_6"));
	Shell_6->SetupAttachment(Bullet_6);

	// ボックスコリジョン
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(DefaultSceneRoot);

	// シリンダーチャンバー音声
	S_Revolver_Cylinder_Chamber_Open_Cue = CreateDefaultSubobject<UAudioComponent>(TEXT("S_Revolver_Cylinder_Chamber_Open_Cue"));
	S_Revolver_Cylinder_Chamber_Open_Cue->SetupAttachment(DefaultSceneRoot);
	S_Revolver_Cylinder_Chamber_Open_Cue->bAutoActivate = false; 
	S_Revolver_Cylinder_Chamber_Open_Cue->SetVolumeMultiplier(1.0f);

	S_Revolver_Cylinder_Chamber_Close_Cue = CreateDefaultSubobject<UAudioComponent>(TEXT("S_Revolver_Cylinder_Chamber_Close_Cue"));
	S_Revolver_Cylinder_Chamber_Close_Cue->SetupAttachment(DefaultSceneRoot);
	S_Revolver_Cylinder_Chamber_Close_Cue->bAutoActivate = false;
	S_Revolver_Cylinder_Chamber_Close_Cue->SetVolumeMultiplier(1.0f);
	// ショット1発砲音声
	S_Revolver_Shot_01_Cue = CreateDefaultSubobject<UAudioComponent>(TEXT("S_Revolver_Shot_01_Cue"));
	S_Revolver_Shot_01_Cue->SetupAttachment(Muzzle);
	S_Revolver_Shot_01_Cue->bAutoActivate = false;
	S_Revolver_Shot_01_Cue->SetVolumeMultiplier(1.0f);
}

void ASteam_Revolver::BeginPlay()
{
	Super::BeginPlay();

	// 全てのコンポーネントのコリジョンを無効化.
	DisableAllCollisions();

	if (S_Revolver_Shot_01_Cue) S_Revolver_Shot_01_Cue->Stop();
	if (S_Revolver_Shot_02_Cue) S_Revolver_Shot_02_Cue->Stop();
	if (S_Revolver_Shot_03_Cue) S_Revolver_Shot_03_Cue->Stop();
	if (S_Revolver_Shot_04_Cue) S_Revolver_Shot_04_Cue->Stop();
	if (S_Revolver_Shot_05_Cue) S_Revolver_Shot_05_Cue->Stop();
	if (S_Revolver_Cylinder_Chamber_Open_Cue) S_Revolver_Cylinder_Chamber_Open_Cue->Stop();
	if (S_Revolver_Cylinder_Chamber_Close_Cue) S_Revolver_Cylinder_Chamber_Close_Cue->Stop();

	//パーティクルを停止.
	if (PS_Muzzleflash_Revolver)
	{
		PS_Muzzleflash_Revolver->Deactivate();
	}
}

void ASteam_Revolver::Tick(float DeltaTime)
{

}

void ASteam_Revolver::PlayFireAnimation()
{
	if (FireAnimMontage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("FireAnimMontage is not set!"));
		return;
	}

	if (Steam_Revolver == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Steam_Revolver mesh is invalid!"));
		return;
	}

	UAnimInstance* AnimInstance = Steam_Revolver->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimInstance is null!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Playing gun fire animation: %s"), *FireAnimMontage->GetName());
	AnimInstance->Montage_Play(FireAnimMontage, 1.0f);
}

void ASteam_Revolver::DisableAllCollisions()
{
	//コリジョンは無効か.
	if (RootComponent)
	{
		UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(RootComponent);
		if (PrimComp)
		{
			PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	// 全てのコンポーネントのコリジョン無効化
	TArray<UActorComponent*> Components;
	GetComponents(UActorComponent::StaticClass(), Components);

	for (UActorComponent* Component : Components)
	{
		UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Component);
		if (PrimComp)
		{
			PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("All gun collisions disabled!"));
}

void ASteam_Revolver::PlayReloadAnimation()
{
	if (ReloadAnimMontage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReloadAnimMontage is not set!"));
		return;
	}

	if (Steam_Revolver == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Steam_Revolver mesh is invalid!"));
		return;
	}

	UAnimInstance* AnimInstance = Steam_Revolver->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimInstance is null!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Playing gun reload animation: %s"), *ReloadAnimMontage->GetName());
	AnimInstance->Montage_Play(ReloadAnimMontage, 1.0f);
}