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
#include "PlayerCharacter.h"
#include "EnemyCharacter.h"
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
	RevolverGun = nullptr;
	//ステータス初期値.
	Hp = MaxHp;
	AmmoCount = MaxAmmoCount;
}

/// <summary>
/// ゲーム開始時またはスポーン時に呼ばれる.
/// </summary>
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	//行動状態の初期化.
	CurrentActionState = ECharaActionState::Idle;

	//銃を装備.
	EquipGun();
	//デフォルトはダッシュ状態.
	OnWalkStop();
	//ボーンインデックスを初期化.
	InitBoneIndices();
}

/// <summary>
/// 毎フレーム呼ばれる.
/// </summary>
/// <param name="DeltaTime">前フレームからの経過時間（秒）</param>
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//向き補完.
	MoveRotating(DeltaTime);
	//リロード時間の更新.
	UpdateReloadTimer(DeltaTime);
	//ローリング更新.
	UpdateRoll(DeltaTime);
	//アニメーション状態を更新.
	UpdateAnim(DeltaTime);
}

#pragma endregion

#pragma region "Get"

//死亡状態かどうか.
bool ACharacterBase::IsDead() const
{
	return CurrentState == ECharaState::Dead;
}

#pragma endregion

#pragma region "動き"

/// <summary>
/// 移動処理.
/// </summary>
/// <param name="WorldDirection">方向</param>
/// <param name="ScaleValue">移動量</param>
/// <param name="bForce">?</param>
void ACharacterBase::Move(FVector WorldDirection, float ScaleValue, bool bForce) {
	AddMovementInput(WorldDirection, ScaleValue, bForce);
}

/// <summary>
/// 向き補完.
/// </summary>
void ACharacterBase::MoveRotating(float DeltaTime) {

	if (!bIsRotating) return;

	//時間経過.
	RotateElapsed += DeltaTime;
	//0→1に正規化.
	const float time = FMath::Clamp(RotateElapsed / RotateDuration, 0.0f, 1.0f);
	//角度補間.
	FQuat newQuat = FQuat::Slerp(
		StartRotation.Quaternion(),
		TargetRotation.Quaternion(),
		time
	);

	SetActorRotation(newQuat);

	//100%を超えたら終了.
	if (time >= 1.0f)
	{
		bIsRotating = false;
	}
}

/// <summary>
/// 向き補完の設定.
/// </summary>
/// <param name="Start">開始角度</param>
/// <param name="End">目標角度</param>
void ACharacterBase::MoveRotateSetting(FRotator Start, FRotator Target) {

	//開始角度を保存.
	StartRotation = Start;
	//目標角度を保存.
	TargetRotation = Target;
	//向き補間開始.
	RotateElapsed = 0.0f;
	bIsRotating = true;
}

/// <summary>
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

#pragma region "ジャンプ"

void ACharacterBase::OnJump()
{
	//ジャンプ中、ローリング中はジャンプ不可.
	if (bIsInAir || bIsRolling) { return; }

	Super::Jump();

	//ジャンプアニメーション再生.
	MyPlayAnim(ECharaActionState::Jump);
	//モンタージュの先頭セクション(JumpStart)から開始.
	GetMesh()->GetAnimInstance()->Montage_JumpToSection(
		FName("JumpStart"), JumpAnimMontage
	);
}

/// <summary>
/// ジャンプ終了（ボタンを離したとき）.
/// </summary>
void ACharacterBase::OnJumpStop()
{
	Super::StopJumping();
}

#pragma endregion

#pragma region "ローリング(回避)"

/// <summary>
/// ローリング開始.
/// キーを押したときに前転する.
/// </summary>
void ACharacterBase::OnRoll()
{
	//ローリング中、クールダウン中は再ローリング不可.
	if (!bCanRoll || bIsRolling) { return; }

	//ローリング中フラグを立てる.
	bIsRolling = true;
	bCanRoll = false;

	float MontageLength = MyPlayAnim(ECharaActionState::Roll);

	// モンタージュ終了後に EndRoll を呼ぶ.
	FTimerHandle RollEndTimer;
	GetWorld()->GetTimerManager().SetTimer(
		RollEndTimer,
		this,
		&APlayerCharacter::EndRoll,
		MontageLength * 0.8f, //ローリングが80%終わったら終了とする.
		false
	);

	// クールダウン開始.
	GetWorld()->GetTimerManager().SetTimer(
		RollCooldownTimer,
		[this]() { bCanRoll = true; },
		RollCooldown,
		false
	);
}

/// <summary>
/// ローリング更新.
/// </summary>
void ACharacterBase::UpdateRoll(float DeltaTime) {

	if (!bIsRolling) { return; }

	//前方向ベクトル取得.
	const FVector Forward = GetActorForwardVector();
	//移動量 = 方向 * 速度 * 時間.
	const FVector Move = Forward * RollSpeed * DeltaTime;

	//位置更新.
	SetActorLocation(GetActorLocation() + Move, true);
}

/// <summary>
/// ローリング終了.
/// モンタージュ再生が終わったら呼ばれる.
/// </summary>
void ACharacterBase::EndRoll()
{
	bIsRolling = false;
}

#pragma endregion

#pragma region "射撃"

/// <summary>
/// 発射チェック.
/// </summary>
/// <returns>問題ないならtrue</returns>
bool ACharacterBase::IsShotAble() {

	//nullチェック.
	if (GetWorld()  == nullptr ||
		BulletClass == nullptr
	){
		return false;
	}
	//弾薬がないならリロード開始.
	if (AmmoCount <= 0) {
		OnReload();
		return false;
	}
	//リロード中、ローリング中は射撃不可.
	if (bIsReloading || bIsRolling) {
		return false;
	}

	return true; //問題なし.
}

/// <summary>
/// 射撃開始
/// </summary>
/// <param name="ParamPos">射撃目標地点</param>
void ACharacterBase::ShotStart(FVector ParamPos) {

	//射撃可能なら.
	if (IsShotAble()) {

		TargetPosition = ParamPos;

		//目標角度を計算.
		FVector DirectionToTarget = ParamPos - GetActorLocation();
		DirectionToTarget.Normalize();
		//向き補完設定.
		MoveRotateSetting(
			GetActorRotation(),
			DirectionToTarget.Rotation()
		);

		//アニメーション再生.
		MyPlayAnim(ECharaActionState::Shot);

		//遅延実行.
		FTimerHandle handle;
		GetWorld()->GetTimerManager().SetTimer(
			handle,
			[this]()
			{
				//弾を召喚.
				bool ret = SpawnBullet(this, TargetPosition);
				if (ret) {
					//ショット時にクロスヘアのエフェクトを実行.
					CrosshairWidgetExe();
				}
			},
			RotateDuration, //向き終わる頃に実行.
			false
		);
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
		if (Cast<APlayerCharacter>(user)) {
			Bullet->SetTeam(ETeam::Player); //プレイヤーチームへ.
		}
		//敵が撃ったなら.
		if (Cast<AEnemyCharacter>(user)) {
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
			RevolverGun->PlayShotAnimation();
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
void ACharacterBase::RotateArmBones(const FRotator& Rotation)
{
	// キャラクター全体の回転で対応
	// 腕のボーン操作はアニメーションBP側で自動的に追従します
	UE_LOG(LogTemp, Warning, TEXT("Character rotation - Pitch: %f, Yaw: %f"), Rotation.Pitch, Rotation.Yaw);
}

#pragma endregion

#pragma region "アニメーション"

/// <summary>
/// アニメーション状態を更新.
/// 操作に応じてIdleやMoveなどのアニメーションを再生.
/// </summary>
void ACharacterBase::UpdateAnim(float DeltaTime)
{
	//射撃中の時.
	if (shotAnimTimer > 0) {
		shotAnimTimer -= DeltaTime;
	}
	//射撃してない時.
	else {
		//現在の速度を取得.
		CurrentSpeed = GetCharacterMovement()->Velocity.Length();
		//空中にいるか判定.
		const bool isInAir = GetCharacterMovement()->IsFalling();

		//ジャンプの更新.
		UpdateAnimJump(isInAir);

		//次のアニメーションを決める.
		ECharaActionState NewActionState;
		//地面にいる & ローリング中ではない.
		if (!isInAir && !bIsRolling) {
			//移動している.
			if (CurrentSpeed > 0.1f) {
				//ダッシュしている.
				if (bIsDash) {
					NewActionState = ECharaActionState::Run;
				}
				else {
					NewActionState = ECharaActionState::Move;
				}
			}
			else {
				NewActionState = ECharaActionState::Idle;
			}
			MyPlayAnim(NewActionState);
		}
	}
}

/// <summary>
/// アニメーション状態(ジャンプ関係)を更新.
/// </summary>
/// <param name="bIsInAir">空中にいるか</param>
void ACharacterBase::UpdateAnimJump(bool bIsInAirNow) {

	/*
	   [フロー]
	   踏み切り(JumpStart)
        → 空中に出たら JumpLoop へ切り替え（着地まで繰り返し）
        → 着地したら JumpLand を再生して終了
	*/
	
	if (JumpAnimMontage == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance == nullptr) return;

	//①地上→空中:JumpLoop に切り替え.
	if (bIsInAirNow && !bIsInAir)
	{
		if (AnimInstance->Montage_IsPlaying(JumpAnimMontage))
		{
			AnimInstance->Montage_JumpToSection(FName("JumpLoop"), JumpAnimMontage);
		}
	}

	// ② 空中 → 着地 : JumpLand を再生.
	if (!bIsInAirNow && bIsInAir)
	{
		if (AnimInstance->Montage_IsPlaying(JumpAnimMontage))
		{
			AnimInstance->Montage_JumpToSection(FName("JumpLand"), JumpAnimMontage);
		}
	}

	// 次フレームのために現在の状態を保存.
	bIsInAir = bIsInAirNow;
}

/// <summary>
/// 移動, ジャンプ, 射撃などのアニメーション切り替え.
/// </summary>
float ACharacterBase::MyPlayAnim(ECharaActionState ActionState)
{
	//アニメーションが同じなら.
	if (CurrentActionState == ActionState) {
		return 0; //処理しない.
	}
	CurrentActionState = ActionState; //状態更新.

	//状態別の処理.
	UAnimMontage* MontageToPlay = nullptr;
	switch (ActionState)
	{
		case ECharaActionState::Idle:
			MontageToPlay = IdleAnimMontage;
			break;
		case ECharaActionState::Move:
			MontageToPlay = MoveAnimMontage;
			break;
		case ECharaActionState::Run:
			MontageToPlay = SprintAnimMontage;
			break;
		case ECharaActionState::Jump:
			MontageToPlay = JumpAnimMontage;
			break;
		case ECharaActionState::Shot:
			MontageToPlay = ShotAnimMontage;
			shotAnimTimer = initShotAnimTime; //しばらくは射撃アニメーションを再生.
			break;
		case ECharaActionState::Roll:
			MontageToPlay = RollAnimMontage;
			break;

		default: break;
	}

	//nullチェック.
	if (MontageToPlay == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("MontageToPlay is null for animation state: %d"), (int32)ActionState);
		return 0;
	}
	if (GetMesh() == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Mesh is invalid!"));
		return 0;
	}

	//アニメーション再生.
	float duration = PlayAnimMontage(MontageToPlay, 1.0f);
	//再生時間を返す.
	return duration;
}
#pragma endregion

#pragma region "ダメージ"

/// <summary>
/// ダメージ処理.
/// </summary>
void ACharacterBase::Damage(int32 Value) 
{
	Hp -= Value; //体力減少.
	if (Hp <= 0) {
		Death(); //死亡処理.
	}
}

#pragma endregion

#pragma region "死亡"

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