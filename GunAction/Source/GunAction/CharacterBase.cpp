/*
   - CharacterBase -

   プレイヤーと敵の基底クラス.
   元はなおと作のPlyerCharacterだったもの.
*/
#include "CharacterBase.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

//他class.
#include "PlayerManager.h"
#include "EnemyManager.h"
#include "BulletBase.h"
#include "WeaponRevolver.h"

#pragma region "基本処理"

/// <summary>
/// コンストラクタ.
/// </summary>
ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	//カプセルコライダー設定.
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	//キャラクターの回転速度(カメラ方向に回転させる).
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	//キャラクター移動の設定.
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	//初期状態.
	bIsReloading = false;
	RevolverGun = nullptr;
	AmmoCount = MaxAmmoCount;
}

/// <summary>
/// ゲーム開始時またはスポーン時に呼ばれる.
/// </summary>
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("===== BeginPlay Start ====="));

	//銃を装備.
	EquipGun();
	//デフォルトはダッシュ状態.
	OnWalkStop();

	//ボーンインデックスを初期化.
	InitBoneIndices();

	// メッシュの確認
	if (GetMesh())
	{
		UE_LOG(LogTemp, Warning, TEXT("Mesh found: %s"), *GetMesh()->GetName());

		// スケルタルメッシュアセットの確認
		USkeletalMesh* SkeletalMesh = GetMesh()->GetSkeletalMeshAsset();
		if (SkeletalMesh)
		{
			UE_LOG(LogTemp, Warning, TEXT("Skeletal Mesh Asset: %s"), *SkeletalMesh->GetName());
		}

		// アニメーションBlueprintの確認
		if (GetMesh()->GetAnimClass())
		{
			UE_LOG(LogTemp, Warning, TEXT("Animation Class Set: %s"), *GetMesh()->GetAnimClass()->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ERROR: Animation Class is NOT set in Mesh!"));
		}

		// AnimInstanceの確認
		UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
		if (AnimInst)
		{
			UE_LOG(LogTemp, Warning, TEXT("AnimInstance found: %s"), *AnimInst->GetClass()->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ERROR: AnimInstance is NULL! Check Animation Blueprint assignment!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Mesh is NULL!"));
	}

	// モンタージュの確認
	UE_LOG(LogTemp, Warning, TEXT("IdleAnimMontage: %s"), IdleAnimMontage ? TEXT("Set") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("MoveAnimMontage: %s"), MoveAnimMontage ? TEXT("Set") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("SprintAnimMontage: %s"), SprintAnimMontage ? TEXT("Set") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("ShotAnimMontage: %s"), ShotAnimMontage ? TEXT("Set") : TEXT("NULL"));

	UE_LOG(LogTemp, Warning, TEXT("===== BeginPlay End ====="));
}

/// <summary>
/// 毎フレーム呼ばれる.
/// </summary>
/// <param name="DeltaTime">前フレームからの経過時間（秒）</param>
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//アニメーション状態を更新.
	UpdateAnimState(DeltaTime);
	//リロード時間の更新.
	UpdateReloadTimer(DeltaTime);
}

#pragma endregion

#pragma region "移動"

/// <summary>
/// 移動処理.
/// </summary>
/// <param name="WorldDirection">方向</param>
/// <param name="ScaleValue">移動量</param>
/// <param name="bForce">?</param>
void ACharacterBase::Move(FVector WorldDirection, float ScaleValue, bool bForce) {
	AddMovementInput(WorldDirection, ScaleValue, bForce);
}

/// 歩く.
/// </summary>
void ACharacterBase::OnWalkStart()
{
	bIsDash = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; //WalkSpeedに戻す.
}
/// <summary>
/// 歩くのをやめる.
/// </summary>
void ACharacterBase::OnWalkStop()
{
	bIsDash = true;
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;  //RunSpeedに変更.
}

#pragma endregion

#pragma region "射撃"

/// <summary>
/// 発射チェック.
/// </summary>
/// <returns>問題ないならtrue</returns>
bool ACharacterBase::IsShotAble() {

	//nullチェック.
	if (GetWorld() == nullptr) {
		return false;
	}
	if (BulletClass == nullptr) {
		return false;
	}
	//弾薬がないならリロード開始.
	if (AmmoCount <= 0) {
		OnReload();
		return false;
	}
	//リロード中は射撃不可.
	if (bIsReloading) {
		return false;
	}

	return true; //問題なし.
}

//射撃開始.
void ACharacterBase::ShotStart() {

	//射撃可能なら.
	if (IsShotAble()) {
		//アニメーション再生.
		//アニメーション内でNotifyを使って次の処理を行う.
		PlayAnim(EAnimationState::Shot);
	}
}

/// <summary>
/// 弾を召喚.
/// </summary>
/// <param name="targetPos">目標座標</param>
/// <returns>召喚に成功したか</returns>
bool ACharacterBase::SpawnBullet(TObjectPtr<ACharacterBase> user, FVector targetPos)
{
	//弾の設定 - ①スポーン位置.
	FVector SpawnLocation;
	{
		//銃を持ってる場合.
		if (IsHaveGun) {
			if (RevolverGun && RevolverGun->Muzzle) {
				SpawnLocation = RevolverGun->Muzzle->GetComponentLocation();
			}
		}
		//銃を持ってない場合.
		else {
			//ソケットの座標取得.
			FVector loc = GetMesh()->GetSocketLocation(MuzzleSocketName);
			SpawnLocation = loc;
		}
	}
	//弾の設定 - ②発射方向.
	FRotator BulletRotation;
	{
		FVector dir = targetPos - SpawnLocation;
		dir.Normalize();
		BulletRotation = dir.Rotation();
		//ランダムで少しずらす.
		BulletRotation += FRotator(
			FMath::FRandRange(-shotPosRandom, shotPosRandom),
			FMath::FRandRange(-shotPosRandom, shotPosRandom),
			FMath::FRandRange(-shotPosRandom, shotPosRandom)
		);
	}
	//弾の設定 - ③スポーンパラメーター.
	FActorSpawnParameters SpawnParams;
	{
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
	}

	//弾クラスを生成.
	ABulletBase* Bullet = GetWorld()->SpawnActor<ABulletBase>(BulletClass, SpawnLocation, BulletRotation, SpawnParams);

	//生成に成功したら.
	if (Bullet != nullptr)
	{
		//プレイヤーが撃ったなら.
		if (Cast<APlayerManager>(user)) {
			Bullet->SetTeam(ETeam::Player); //プレイヤーチームへ.
		}
		//敵が撃ったなら.
		if (Cast<AEnemyManager>(user)) {
			Bullet->SetTeam(ETeam::Enemy);  //敵チームへ.
		}

		//弾薬を消費.
		AmmoCount--;

		UE_LOG(LogTemp, Warning, TEXT("Shot! Remaining Ammo: %d"), AmmoCount);

		// マズルフラッシュエフェクトを再生
		if (RevolverGun && RevolverGun->PS_Muzzleflash_Revolver)
		{
			// 古いパーティクルを確実に終了させてから新規に開始
			RevolverGun->PS_Muzzleflash_Revolver->Deactivate();
			RevolverGun->PS_Muzzleflash_Revolver->Activate(true);
		}

		if (RevolverGun && RevolverGun->S_Revolver_Shot_01_Cue)
		{
			RevolverGun->S_Revolver_Shot_01_Cue->Play(0.0f);
			UE_LOG(LogTemp, Warning, TEXT("Shot Sound Played!"));
		}

		//銃の射撃アニメーションを再生
		if (RevolverGun)
		{
			RevolverGun->PlayFireAnimation();
		}

		return true; //発射成功.
	}
	return false; //発射失敗.
}

#pragma endregion

#pragma region "銃"

/// <summary>
/// リロード開始処理.
/// </summary>
void ACharacterBase::OnReload()
{
	if (bIsReloading) {
		return; //リロード中は処理しない.
	}
	if (AmmoCount >= MaxAmmoCount) {
		return; //弾を使ってないならリロード不要.
	}

	bIsReloading = true;
	ReloadTimerElapsed = 0.0f;

	UE_LOG(LogTemp, Warning, TEXT("Reload Started! Duration: %f seconds"), ReloadDuration);

	// 銃のシリンダーを開くアニメーション・音声を再生
	if (RevolverGun)
	{
		if (RevolverGun->S_Revolver_Cylinder_Chamber_Open_Cue)
		{
			RevolverGun->S_Revolver_Cylinder_Chamber_Open_Cue->Activate();
		}
		// 銃のリロードアニメーションを再生
		RevolverGun->PlayReloadAnimation();
	}
}

/// <summary>
/// ボーンインデックスを初期化する関数
/// </summary>
void ACharacterBase::InitBoneIndices()
{
	if (!GetMesh()) {
		UE_LOG(LogTemp, Error, TEXT("Mesh is NULL!"));
		return;
	}

	//右腕のボーン名.
	FName RightArmBoneName     = FName(TEXT("arm_r"));     //上腕.
	FName RightForearmBoneName = FName(TEXT("forearm_r")); //前腕.

	//ボーンインデックスを取得.
	RightArmBoneIndex = GetMesh()->GetBoneIndex(RightArmBoneName);
	RightForearmBoneIndex = GetMesh()->GetBoneIndex(RightForearmBoneName);

	if (RightArmBoneIndex != INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("Right Arm Bone found at index: %d"), RightArmBoneIndex);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Right Arm Bone '%s' not found!"), *RightArmBoneName.ToString());
	}

	if (RightForearmBoneIndex != INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("Right Forearm Bone found at index: %d"), RightForearmBoneIndex);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Right Forearm Bone '%s' not found!"), *RightForearmBoneName.ToString());
	}
}

/// <summary>
/// UpdateReloadTimer - リロード時間の更新.
/// リロード時間が経過したら弾薬を復旧し、リロード状態を解除する.
/// </summary>
/// <param name="DeltaTime">フレームの経過時間</param>
void ACharacterBase::UpdateReloadTimer(float DeltaTime)
{
	if (!bIsReloading) {
		return; //リロードしてないなら処理しない.
	}

	ReloadTimerElapsed += DeltaTime; //リロード時間経過.

	if (ReloadTimerElapsed >= ReloadDuration)
	{
		//リロード完了.
		bIsReloading = false;
		AmmoCount = MaxAmmoCount; //弾の数復活.

		UE_LOG(LogTemp, Warning, TEXT("Reload Complete! Ammo: %d"), AmmoCount);

		//銃のシリンダーを閉じるアニメーション・音声を再生.
		if (RevolverGun)
		{
			if (RevolverGun->S_Revolver_Cylinder_Chamber_Close_Cue)
			{
				RevolverGun->S_Revolver_Cylinder_Chamber_Close_Cue->Activate();
			}
		}
	}
}

/*
   TODO: IsHaveGunがfalseの時、銃装着(EquipGun)を行わないようにする。
*/

/// <summary>
/// EquipGun - 銃を装備する処理.
/// RevolverGunClassから銃をスポーンしてプレイヤーに装備させる.
/// </summary>
void ACharacterBase::EquipGun()
{
	//エラー対策.
	if (RevolverGun != nullptr) {
		return;
	}
	if (RevolverGunClass == nullptr) {
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	//銃をスポーン.
	RevolverGun = GetWorld()->SpawnActor<AWeaponRevolver>(RevolverGunClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (RevolverGun)
	{
		//銃を装備(ソケットにアタッチする)
		bool ret = RevolverGun->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, GunAttachSocketName);

		if (!ret) {
			UE_LOG(LogTemp, Error, TEXT("Failed to equip revolver gun!"));
		}
		//銃のコリジョンは不要なため無効化.
		if (RevolverGun->BoxCollision)
		{
			RevolverGun->BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		//スケルタルメッシュのコリジョンも無効化.
		if (RevolverGun->RevolverMain)
		{
			RevolverGun->RevolverMain->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		AmmoCount = MaxAmmoCount; //弾の残数を設定.
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn revolver gun!"));
	}
}

/// <summary>
/// 腕のボーンを回転させる関数
/// </summary>
void ACharacterBase::RotateArmBones(const FRotator& TargetRotation)
{
	// キャラクター全体の回転で対応
	// 腕のボーン操作はアニメーションBP側で自動的に追従します
	UE_LOG(LogTemp, Warning, TEXT("Character rotation - Pitch: %f, Yaw: %f"), TargetRotation.Pitch, TargetRotation.Yaw);
}

#pragma endregion

#pragma region "ダメージ・死亡"

/// <summary>
/// 死亡アニメーション再生.
/// </summary>
void ACharacterBase::PlayDeathAnimation()
{

}

/// <summary>
/// 死亡エフェクト再生.
/// </summary>
void ACharacterBase::PlayDeathEffect()
{
	if (DeathEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			DeathEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}
}

/// <summary>
/// 死亡音再生.
/// </summary>
void ACharacterBase::PlayDeathSound()
{
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			DeathSound,
			GetActorLocation()
		);
	}
}

/// <summary>
/// コンポーネント無効化.
/// </summary>
void ACharacterBase::DisableComponents()
{
	//移動を停止.
	if (auto cmp = GetCharacterMovement()) {
		cmp->StopMovementImmediately();
		cmp->DisableMovement();
	}
	//銃を消滅.
	if (RevolverGun) {
		RevolverGun->Destroy();
	}
	//Tickを停止.
	SetActorTickEnabled(false);
}

#pragma endregion

#pragma region "アニメーション"

/// <summary>
/// アニメーション状態を更新.
/// 操作に応じてIdleやMoveなどのアニメーションを再生.
/// </summary>
void ACharacterBase::UpdateAnimState(float DeltaTime)
{
	//射撃中.
	if (shotAnimTimer > 0) {
		shotAnimTimer -= DeltaTime;
	}
	//射撃してない.
	else {
		//次の状態に変える.
		EAnimationState NewAnimationState;
		{
			//現在の速度を取得.
			CurrentSpeed = GetCharacterMovement()->Velocity.Length();
			//ジャンプ状態の判定.
			bool bIsAirborne = !GetCharacterMovement()->IsMovingOnGround() && GetCharacterMovement()->Velocity.Z != 0.0f;

			if (bIsAirborne)
			{
				//空中にいる場合,垂直速度でジャンプ状態を判定.
				float VerticalVelocity = GetCharacterMovement()->Velocity.Z;

				if (VerticalVelocity > 100.0f)
				{
					//ジャンプアップ状態.
					NewAnimationState = EAnimationState::JumpUp;
				}
				else if (VerticalVelocity < -100.0f)
				{
					//ジャンプダウン状態.
					NewAnimationState = EAnimationState::JumpDown;
				}
				else
				{
					//ジャンプ中状態.
					NewAnimationState = EAnimationState::JumpMid;
				}
				bIsJumping = true;
			}
			else
			{
				//地面にいる場合.
				bIsJumping = false;

				if (CurrentSpeed > 0.1f)
				{
					bIsMoving = true;

					//スプリント中か判定.
					if (bIsDash)
					{
						NewAnimationState = EAnimationState::Run;
					}
					else
					{
						NewAnimationState = EAnimationState::Move;
					}
				}
				else
				{
					bIsMoving = false;
					NewAnimationState = EAnimationState::Idle;
				}
			}
		}
		//射撃以外のアニメーション.
		PlayAnim(NewAnimationState);
	}
}

/// <summary>
/// 移動, ジャンプ, 射撃などのアニメーション切り替え.
/// </summary>
void ACharacterBase::PlayAnim(EAnimationState AnimState)
{
	//アニメーションが同じなら.
	if (CurrentAnimationState == AnimState) {
		return; //処理しない.
	}
	CurrentAnimationState = AnimState; //状態更新.

	//状態別の処理.
	UAnimMontage* MontageToPlay = nullptr;
	switch (AnimState)
	{
	case EAnimationState::Idle:
		MontageToPlay = IdleAnimMontage;
		break;
	case EAnimationState::Move:
		MontageToPlay = MoveAnimMontage;
		break;
	case EAnimationState::Run:
		MontageToPlay = SprintAnimMontage;
		break;
	case EAnimationState::JumpUp:
		MontageToPlay = JumpUpAnimMontage;
		break;
	case EAnimationState::JumpMid:
		MontageToPlay = JumpMidAnimMontage;
		break;
	case EAnimationState::JumpDown:
		MontageToPlay = JumpDownAnimMontage;
		break;
	case EAnimationState::Shot:
		MontageToPlay = ShotAnimMontage;
		//しばらくは射撃アニメーションを再生.
		shotAnimTimer = initShotAnimTime;
		break;

	default: break;
	}

	//nullチェック.
	if (MontageToPlay == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("MontageToPlay is null for animation state: %d"), (int32)AnimState);
		return;
	}
	if (GetMesh() == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Mesh is invalid!"));
		return;
	}

	//アニメーションインスタンス取得.
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimInstance is null! Mesh: %s"), GetMesh() ? TEXT("Valid") : TEXT("Invalid"));
		return;
	}
	//アニメーション再生.
	UE_LOG(LogTemp, Warning, TEXT("Playing montage: %s for state: %d"), *MontageToPlay->GetName(), (int32)AnimState);
	AnimInstance->Montage_Play(MontageToPlay, 1.0f);
}
#pragma endregion