/*
   - AWeaponRevolver -
   武器クラス「短銃」
*/
#include "WeaponRevolver.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

AWeaponRevolver::AWeaponRevolver()
{
	PrimaryActorTick.bCanEverTick = false;

	// ルートコンポーネント
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	// リボルバー本体(メッシュ)
	RevolverMain = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RevolverMain"));
	RevolverMain->SetupAttachment(DefaultSceneRoot);
	// マズル
	Muzzle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(RevolverMain);
	// 当たり判定
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	BoxCollision->SetupAttachment(DefaultSceneRoot);

	// 弾丸とシェルコンポーネント
	Bullet_1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet_1"));
	Bullet_1->SetupAttachment(RevolverMain);
	Shell_1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shell_1"));
	Shell_1->SetupAttachment(Bullet_1);

	Bullet_2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet_2"));
	Bullet_2->SetupAttachment(RevolverMain);
	Shell_2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shell_2"));
	Shell_2->SetupAttachment(Bullet_2);

	Bullet_3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet_3"));
	Bullet_3->SetupAttachment(RevolverMain);
	Shell_3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shell_3"));
	Shell_3->SetupAttachment(Bullet_3);

	Bullet_4 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet_4"));
	Bullet_4->SetupAttachment(RevolverMain);
	Shell_4 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shell_4"));
	Shell_4->SetupAttachment(Bullet_4);

	Bullet_5 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet_5"));
	Bullet_5->SetupAttachment(RevolverMain);
	Shell_5 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shell_5"));
	Shell_5->SetupAttachment(Bullet_5);

	Bullet_6 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet_6"));
	Bullet_6->SetupAttachment(RevolverMain);
	Shell_6 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shell_6"));
	Shell_6->SetupAttachment(Bullet_6);

	// シリンダーチャンバー音声
	S_Revolver_Cylinder_Chamber_Open_Cue = CreateDefaultSubobject<UAudioComponent>(TEXT("S_Revolver_Cylinder_Chamber_Open_Cue"));
	S_Revolver_Cylinder_Chamber_Open_Cue->SetupAttachment(DefaultSceneRoot);
	S_Revolver_Cylinder_Chamber_Open_Cue->bAutoActivate = false; 
	S_Revolver_Cylinder_Chamber_Open_Cue->SetVolumeMultiplier(1.0f);

	S_Revolver_Cylinder_Chamber_Close_Cue = CreateDefaultSubobject<UAudioComponent>(TEXT("S_Revolver_Cylinder_Chamber_Close_Cue"));
	S_Revolver_Cylinder_Chamber_Close_Cue->SetupAttachment(DefaultSceneRoot);
	S_Revolver_Cylinder_Chamber_Close_Cue->bAutoActivate = false;
	S_Revolver_Cylinder_Chamber_Close_Cue->SetVolumeMultiplier(1.0f);
	
	// ショット発砲音声
	S_Revolver_Shot_01_Cue = CreateDefaultSubobject<UAudioComponent>(TEXT("S_Revolver_Shot_01_Cue"));
	S_Revolver_Shot_01_Cue->SetupAttachment(Muzzle);
	S_Revolver_Shot_01_Cue->bAutoActivate = false;
	S_Revolver_Shot_01_Cue->SetVolumeMultiplier(1.0f);

	S_Revolver_Shot_02_Cue = CreateDefaultSubobject<UAudioComponent>(TEXT("S_Revolver_Shot_02_Cue"));
	S_Revolver_Shot_02_Cue->SetupAttachment(DefaultSceneRoot);
	S_Revolver_Shot_02_Cue->bAutoActivate = false;
	S_Revolver_Shot_02_Cue->SetVolumeMultiplier(1.0f);

	S_Revolver_Shot_03_Cue = CreateDefaultSubobject<UAudioComponent>(TEXT("S_Revolver_Shot_03_Cue"));
	S_Revolver_Shot_03_Cue->SetupAttachment(DefaultSceneRoot);
	S_Revolver_Shot_03_Cue->bAutoActivate = false;
	S_Revolver_Shot_03_Cue->SetVolumeMultiplier(1.0f);

	S_Revolver_Shot_04_Cue = CreateDefaultSubobject<UAudioComponent>(TEXT("S_Revolver_Shot_04_Cue"));
	S_Revolver_Shot_04_Cue->SetupAttachment(DefaultSceneRoot);
	S_Revolver_Shot_04_Cue->bAutoActivate = false;
	S_Revolver_Shot_04_Cue->SetVolumeMultiplier(1.0f);

	S_Revolver_Shot_05_Cue = CreateDefaultSubobject<UAudioComponent>(TEXT("S_Revolver_Shot_05_Cue"));
	S_Revolver_Shot_05_Cue->SetupAttachment(DefaultSceneRoot);
	S_Revolver_Shot_05_Cue->bAutoActivate = false;
	S_Revolver_Shot_05_Cue->SetVolumeMultiplier(1.0f);

	// マズルフラッシュパーティクル
	PS_Muzzleflash_Revolver = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("PS_Muzzleflash_Revolver"));
	PS_Muzzleflash_Revolver->SetupAttachment(DefaultSceneRoot);
	PS_Muzzleflash_Revolver->bAutoActivate = false;
}

void AWeaponRevolver::BeginPlay()
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

void AWeaponRevolver::Tick(float DeltaTime)
{

}

//全てのコンポーネントのコリジョンを無効化.
void AWeaponRevolver::DisableAllCollisions()
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

//射撃アニメーションを再生.
void AWeaponRevolver::PlayFireAnimation()
{
	if (FireAnimMontage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("FireAnimMontage is not set!"));
		return;
	}

	if (RevolverMain == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Revolver mesh is invalid!"));
		return;
	}

	UAnimInstance* AnimInstance = RevolverMain->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimInstance is null!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Playing gun fire animation: %s"), *FireAnimMontage->GetName());
	AnimInstance->Montage_Play(FireAnimMontage, 1.0f);
}
//リロードアニメーションを再生.
void AWeaponRevolver::PlayReloadAnimation()
{
	if (ReloadAnimMontage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReloadAnimMontage is not set!"));
		return;
	}

	if (RevolverMain == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Revolver mesh is invalid!"));
		return;
	}

	UAnimInstance* AnimInstance = RevolverMain->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimInstance is null!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Playing gun reload animation: %s"), *ReloadAnimMontage->GetName());
	AnimInstance->Montage_Play(ReloadAnimMontage, 1.0f);
}