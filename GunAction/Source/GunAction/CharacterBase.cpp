/*
   - CharacterBase -
   作成: 怜旺.

   プレイヤーと敵の基底クラス.
   元はなおと作のPlyerCharacterだったもの.
*/
#include "CharacterBase.h"

#include "CrosshairWidget.h" 
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"

#include "BulletBase.h"
#include "Steam_Revolver.h"
#include "EngineUtils.h"

#pragma region "コンストラクタ"
/// <summary>
/// コンストラクタ - ACharacterBase
/// プレイヤーキャラクターの初期設定を行う.
/// カプセルコライダー、カメラ、スプリングアームなどの設定を初期化する.
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
	bIsSprinting = false;
	CurrentAmmoCount = MaxAmmoPerMagazine;
	bIsReloading = false;
	RevolverGun = nullptr;
}
#pragma endregion

#pragma region "ライフサイクル"
/// <summary>
/// BeginPlay - ゲーム開始時またはアクタースポーン時に呼ばれる.
/// UI（クロスヘア）の初期化を行う.
/// </summary>
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("===== BeginPlay Start ====="));

	//銃を装備.
	EquipGun();

	//ボーンインデックスを初期化.
	InitializeBoneIndices();

	// メッシュの確認
	if (GetMesh())
	{
		UE_LOG(LogTemp, Warning, TEXT("Mesh found: %s"), *GetMesh()->GetName());

		UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
		if (AnimInst)
		{
			UE_LOG(LogTemp, Warning, TEXT("AnimInstance found: %s"), *AnimInst->GetClass()->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AnimInstance is NULL!"));
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

	UE_LOG(LogTemp, Warning, TEXT("===== BeginPlay End ====="));
}

/// <summary>
/// Tick - 毎フレーム呼ばれる処理.
/// アニメーション状態を更新する.
/// </summary>
/// <param name="DeltaTime">前フレームからの経過時間（秒）</param>
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//アニメーション状態を更新.
	UpdateAnimationState();

	//リロード時間の更新.
	UpdateReloadTimer(DeltaTime);
}
#pragma endregion

#pragma region "カメラ"
/// <summary>
/// GetCameraVector - カメラからの方向ベクトルを取得.
/// (弾の発射方向などの計算に使用)
/// </summary>
/// <param name="dir">"Forward", "Right", "up" のどれか</param>
/// <returns>ベクトル</returns>
FVector ACharacterBase::GetCameraVector(FString dir) const
{
	//カメラがない時はZeroVectorを返す.
	if (FollowCamera == nullptr) {
		return FVector::ZeroVector;
	}

	//前方向.
	if (dir == "Forward") {
		return FollowCamera->GetForwardVector();
	}
	//右方向.
	else if (dir == "Right") {
		return FollowCamera->GetRightVector();
	}
	//上方向.
	else if (dir == "Up") {
		return FollowCamera->GetUpVector();
	}
	//不正な指定.
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("GetCameraVectorに失敗"));
		return FVector::ZeroVector;
	}
}
/// <summary>
/// GetCameraLocation - カメラのワールド座標を取得.
/// 弾の発射位置などの計算に使用される.
/// </summary>
/// <returns>カメラのワールド座標。カメラがない場合はZeroVector</returns>
FVector ACharacterBase::GetCameraLocation() const
{
	if (FollowCamera == nullptr)
	{
		return FVector::ZeroVector;
	}
	return FollowCamera->GetComponentLocation();
}
/// <summary>
/// GetCameraRotation - カメラの回転を取得.
/// </summary>
/// <returns>カメラの回転。カメラがない場合はZeroRotator</returns>
FRotator ACharacterBase::GetCameraRotation() const
{
	if (FollowCamera == nullptr)
	{
		return FRotator::ZeroRotator;
	}
	return FollowCamera->GetComponentRotation();
}
#pragma endregion

#pragma region "移動"

/// <summary>
/// アニメーション状態を更新する関数.
/// キャラクターの移動状態に応じてIdleまたはMoveアニメーションを再生.
/// </summary>
void ACharacterBase::UpdateAnimationState()
{
	//現在の速度を取得.
	CurrentSpeed = GetCharacterMovement()->Velocity.Length();

	//ジャンプ状態の判定.
	bool bIsAirborne = !GetCharacterMovement()->IsMovingOnGround() && GetCharacterMovement()->Velocity.Z != 0.0f;

	EAnimationState NewAnimationState = CurrentAnimationState;
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
			if (bIsSprinting)
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

	//アニメーションが変わった場合のみ再生.
	if (NewAnimationState != CurrentAnimationState)
	{
		CurrentAnimationState = NewAnimationState;
		PlayAnimationMontage(NewAnimationState);
	}
}
void ACharacterBase::PlayAnimationMontage(EAnimationState AnimState)
{
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
	default:
		break;
	}
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

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimInstance is null! Mesh: %s"), GetMesh() ? TEXT("Valid") : TEXT("Invalid"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Playing montage: %s for state: %d"), *MontageToPlay->GetName(), (int32)AnimState);
	AnimInstance->Montage_Play(MontageToPlay, 1.0f);
}
#pragma endregion

#pragma region "射撃"
/// <summary>
/// EquipGun - 銃を装備する処理.
/// RevolverGunClassから銃をスポーンしてプレイヤーに装備させる.
/// </summary>
void ACharacterBase::EquipGun()
{
	if (RevolverGunClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("RevolverGunClass is not set! Please set it in Blueprint."));
		return;
	}

	// 銃をスポーン
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	RevolverGun = GetWorld()->SpawnActor<ASteam_Revolver>(RevolverGunClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (RevolverGun)
	{
		// 銃をメッシュにアタッチ（ソケット: "hand_r" 右手に装備）
		RevolverGun->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("hand_r"));

		// 銃のコリジョンを無効化（プレイヤーに装備されるため不要）
		if (RevolverGun->Box)
		{
			RevolverGun->Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		// スケルタルメッシュのコリジョンも無効化
		if (RevolverGun->Steam_Revolver)
		{
			RevolverGun->Steam_Revolver->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		UE_LOG(LogTemp, Warning, TEXT("Gun equipped successfully!"));
		CurrentAmmoCount = MaxAmmoPerMagazine;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn revolver gun!"));
	}
}

/// <summary>
/// CalculateAndShot - 弾の計算と発射処理.
/// カメラの位置と方向から弾をスポーンし、銃のマズルから発射する.
/// 弾薬が0の場合はリロードする.
/// </summary>
void ACharacterBase::CalculateAndShot()
{
	// リロード中は射撃不可
	if (bIsReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reloading... Cannot shoot!"));
		return;
	}

	// 弾薬がない場合はリロード開始
	if (CurrentAmmoCount <= 0)
	{
		StartReload();
		return;
	}

	if (FollowCamera == nullptr || GetWorld() == nullptr)
	{
		return;
	}

	// BulletClassが設定されているか確認.
	if (BulletClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("BulletClass is not set! Please set it in Blueprint."));
		return;
	}

	//クロスヘアの中心座標を画面座標で計算.
	FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	FVector2D CrosshairScreenLocation = ViewportSize / 2.0f; // 画面中央.

	// スクリーン座標をワールド座標に変換.
	FVector CrosshairWorldLocation = FVector::ZeroVector;
	FVector CrosshairWorldDirection = FVector::ZeroVector;


	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController)
	{
		PlayerController->DeprojectScreenPositionToWorld(
			CrosshairScreenLocation.X,
			CrosshairScreenLocation.Y,
			CrosshairWorldLocation,
			CrosshairWorldDirection
		);
	}

	// クロスヘアから遠くの目標地点を計算.
	FVector TargetPosition = CrosshairWorldLocation + (CrosshairWorldDirection * BulletTargetDistance);

	//カメラの位置を取得.
	FVector CameraLocation = FollowCamera->GetComponentLocation();
	FRotator CameraRotation = FollowCamera->GetComponentRotation();

	// 銃のマズルから弾を発射する場合
	FVector SpawnLocation = CameraLocation;

	if (RevolverGun && RevolverGun->Muzzle)
	{
		SpawnLocation = RevolverGun->Muzzle->GetComponentLocation();
	}
	else
	{
		// マズルがない場合はカメラの少し前方から発射
		SpawnLocation = CameraLocation + (GetCameraVector("Forward") * 100.0f) - (GetCameraVector("Right") * 20.0f);
	}

	//マズルから目標への方向を計算
	FVector BulletDirectionToTarget = TargetPosition - SpawnLocation;
	BulletDirectionToTarget.Normalize();
	FRotator BulletRotation = BulletDirectionToTarget.Rotation();

	// プレイヤーの回転をクロスヘア方向に向かせる
	FVector DirectionToTarget = TargetPosition - GetActorLocation();
	DirectionToTarget.Normalize();
	FRotator TargetRotation = DirectionToTarget.Rotation();

	// Y軸（Yaw）のみ回転させる（上下は変わらない）
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw = TargetRotation.Yaw;
	NewRotation.Pitch = TargetRotation.Pitch;
	SetActorRotation(NewRotation);



	//弾のスポーンパラメーター設定.
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	//弾クラスを生成.
	ABulletBase* Bullet = GetWorld()->SpawnActor<ABulletBase>(BulletClass, SpawnLocation, BulletRotation, SpawnParams);



	if (Bullet != nullptr)
	{
		//弾クラスのメンバ関数.
		Bullet->ShotPos(TargetPosition);

		//ショット時にクロスヘアのエフェクトを実行
		if (CrosshairWidget)
		{
			CrosshairWidget->OnShotEffect();
		}

		// 弾薬を消費
		CurrentAmmoCount--;

		UE_LOG(LogTemp, Warning, TEXT("Shot! Remaining Ammo: %d"), CurrentAmmoCount);

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

		// 銃の射撃アニメーションを再生
		if (RevolverGun)
		{
			RevolverGun->PlayFireAnimation();
		}
	}
	//プレイヤーアニメーション追加.
	PlayFireAnimMontage();
}
void ACharacterBase::PlayFireAnimMontage()
{
	if (PlayerFireAnimMontage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerFireAnimMontage is not set!"));
		return;
	}

	if (GetMesh() == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Mesh is invalid!"));
		return;
	}

	// アニメーションインスタンスを取得
	class UAnimInstance* FireAnimInstance = GetMesh()->GetAnimInstance();
	if (FireAnimInstance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimInstance is null!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Playing player fire montage: %s"), *PlayerFireAnimMontage->GetName());
	FireAnimInstance->Montage_Play(PlayerFireAnimMontage, 1.0f);
}

/// <summary>
/// ShotBullet - 弾発射コマンド.
/// CalculateAndShot関数を呼び出して弾を発射する.
/// 入力イベントから直接呼ばれる.
/// </summary>
void ACharacterBase::ShotBullet()
{
	CalculateAndShot();
}

/// <summary>
/// StartReload - リロード開始処理.
/// リロード時間をセットして、弾薬を満タンに戻す.
/// </summary>
void ACharacterBase::StartReload()
{
	if (bIsReloading)
	{
		return;
	}

	// 既に満タンの場合はリロード不要
	if (CurrentAmmoCount >= MaxAmmoPerMagazine)
	{
		return;
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
/// UpdateReloadTimer - リロード時間の更新.
/// リロード時間が経過したら弾薬を復旧し、リロード状態を解除する.
/// </summary>
/// <param name="DeltaTime">フレームの経過時間</param>
void ACharacterBase::UpdateReloadTimer(float DeltaTime)
{
	if (!bIsReloading)
	{
		return;
	}

	ReloadTimerElapsed += DeltaTime;

	if (ReloadTimerElapsed >= ReloadDuration)
	{
		// リロード完了
		bIsReloading = false;
		CurrentAmmoCount = MaxAmmoPerMagazine;

		UE_LOG(LogTemp, Warning, TEXT("Reload Complete! Ammo: %d"), CurrentAmmoCount);

		// 銃のシリンダーを閉じるアニメーション・音声を再生
		if (RevolverGun)
		{
			if (RevolverGun->S_Revolver_Cylinder_Chamber_Close_Cue)
			{
				RevolverGun->S_Revolver_Cylinder_Chamber_Close_Cue->Activate();
			}
		}
	}
}

/// <summary>
/// ターゲット位置を計算する関数.
/// カメラの位置から前方向へ指定距離だけ離れた地点を計算する.
/// 弾の発射目標地点を決定するために使用される.
/// </summary>
FVector ACharacterBase::CalculateTargetPosition(float Distance) const
{
	if (FollowCamera == nullptr)
	{
		return FVector::ZeroVector;
	}

	//カメラの位置を取得.
	FVector CameraLocation = GetCameraLocation();
	//カメラの方向を取得.
	FVector ForwardVector = GetCameraVector("Forward"); //前.
	//位置 + (前方向*距離)
	FVector TargetPosition = CameraLocation + (ForwardVector * Distance);

	return TargetPosition;
}

/// <summary>
/// ボーンインデックスを初期化する関数
/// </summary>
void ACharacterBase::InitializeBoneIndices()
{
	if (!GetMesh())
	{
		UE_LOG(LogTemp, Error, TEXT("Mesh is NULL!"));
		return;
	}

	// 右腕のボーン名（スケルトンに合わせて変更してください）
	FName RightArmBoneName = FName(TEXT("arm_r"));      // 上腕
	FName RightForearmBoneName = FName(TEXT("forearm_r")); // 前腕

	// ボーンインデックスを取得
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
/// 腕のボーンを回転させる関数
/// </summary>
void ACharacterBase::RotateArmBones(const FRotator& TargetRotation)
{
	// キャラクター全体の回転で対応
	// 腕のボーン操作はアニメーションBP側で自動的に追従します
	UE_LOG(LogTemp, Warning, TEXT("Character rotation - Pitch: %f, Yaw: %f"), TargetRotation.Pitch, TargetRotation.Yaw);
}

#pragma endregion
