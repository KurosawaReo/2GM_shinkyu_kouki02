/*
   - PlayerManager -
   共通のCharacterクラスから派生したプレイヤークラス.

   [クラス構成]
   CharacterBase
   └PlayerManager ←今ここ
   └EnemyManager
*/
#include "PlayerManager.h"

#include "CrosshairWidget.h" 
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"

//コンストラクタ.
APlayerManager::APlayerManager() {

	//スプリングアーム(カメラブーム)の作成.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 50.0f);

	//カメラの作成.
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

//召喚した瞬間.
void APlayerManager::BeginPlay() {

	ACharacterBase::BeginPlay(); //親クラスのBeginPlay()を呼び出す.

	//クロスヘアUIを初期化.
	InitializeUI();

	//アニメーション状態の初期化.
	CurrentAnimationState = EAnimationState::Idle;

	//剣を装備.
	EquipSword();
}

//常に実行.
void APlayerManager::Tick(float DeltaTime) {

	ACharacterBase::Tick(DeltaTime); //親クラスのTick()を呼び出す.

	UpdateHandIK();

	UpdateMeleeCombo(DeltaTime);
}

#pragma region "入力処理"
/// <summary>
/// SetupPlayerInputComponent - プレイヤー入力の設定.
/// キーボード、マウスなどの入力を各アクションにバインドする.
/// </summary>
/// <param name="PlayerInputComponent">プレイヤー入力コンポーネント</param>
void APlayerManager::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Input(PlayerInputComponent);
}
/// <summary>
/// Input - 移動処理および入力バインド.
/// キャラクターの移動、カメラ操作、ジャンプ、スプリント、射撃などの入力をセットアップする.
/// </summary>
/// <param name="PlayerInputComponent">プレイヤー入力コンポーネント</param>
void APlayerManager::Input(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//移動入力.
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerManager::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",   this, &APlayerManager::MoveRight);

	//カメラ入力.
	PlayerInputComponent->BindAxis("Turn",       this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp",     this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate",   this, &APlayerManager::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APlayerManager::LookUpAtRate);

	//ジャンプ入力.
	PlayerInputComponent->BindAction("Jump", IE_Pressed,  this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	//スプリント入力.
	PlayerInputComponent->BindAction("Sprint", IE_Pressed,  this, &APlayerManager::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerManager::StopSprint);

	//弾発射入力.
	PlayerInputComponent->BindAction("Fire",   IE_Pressed, this, &APlayerManager::ShotBullet);

	//リロード入力.
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &APlayerManager::StartReload);

	//近接攻撃入力
	PlayerInputComponent->BindAction("MeleeAttack", IE_Pressed, this, &APlayerManager::MeleeAttack);
	UE_LOG(LogTemp, Warning, TEXT("MeleeAttack input binding setup!"));
}
#pragma endregion

#pragma region "近接攻撃"

/// <summary>
/// OnMeleeAttackHit - 近接攻撃がヒットした時の処理.
/// 敵にダメージを与える判定を行う.
/// </summary>
/// <param name="AttackType">攻撃のタイプ（剣/キック）</param>
void APlayerManager::OnMeleeAttackHit(EMeleeAttackType AttackType)
{
	if (GetWorld() == nullptr)
	{
		return;
	}

	// 攻撃の原点（プレイヤー位置）
	FVector AttackOrigin = GetActorLocation();
	FVector AttackEnd = AttackOrigin + GetActorForwardVector() * MeleeAttackRange;

	// スフィアトレースで敵を検索
	FHitResult HitResult;
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(100.0f); // 半径100cm

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		AttackOrigin,
		AttackEnd,
		FQuat::Identity,
		ECC_GameTraceChannel1, // 敵用チャンネル
		CollisionShape,
		QueryParams
	);

	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();
		float Damage = MeleeAttackDamage;

		// キックの場合はダメージ増加
		if (AttackType == EMeleeAttackType::Kick)
		{
			Damage *= 1.5f;
		}

		UE_LOG(LogTemp, Warning, TEXT("Melee Hit! Target: %s, Damage: %f, Type: %s"),
			*HitActor->GetName(), Damage, AttackType == EMeleeAttackType::SwordSlash ? TEXT("SwordSlash") : TEXT("Kick"));

		// ダメージを与える処理（EnemyManagerなどが実装すること）
		// 例: HitActor->TakeDamage(Damage, FDamageEvent(), GetController(), this);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Melee attack - No hits detected in range"));
	}
}

/// <summary>
/// UpdateMeleeCombo - 近接攻撃のコンボシステム更新.
/// コンボウィンドウのタイマーを管理し、クールダウンを更新する.
/// Tick内で毎フレーム呼び出される.
/// </summary>
/// <param name="DeltaTime">フレームの経過時間</param>
void APlayerManager::UpdateMeleeCombo(float DeltaTime)
{
	// クールダウンタイマーの更新
	if (!bCanMeleeAttack)
	{
		MeleeAttackCooldownTimer -= DeltaTime;
		if (MeleeAttackCooldownTimer <= 0.0f)
		{
			bCanMeleeAttack = true;
			MeleeAttackCooldownTimer = 0.0f;
		}
	}

	// コンボウィンドウの更新
	if (bIsInComboWindow)
	{
		ComboWindowTimer -= DeltaTime;
		if (ComboWindowTimer <= 0.0f)
		{
			bIsInComboWindow = false;
			CurrentComboCount = 0;
			UE_LOG(LogTemp, Warning, TEXT("Combo window expired. Resetting combo."));
		}
	}
}

/// <summary>
/// EquipSword - 剣を装備する処理.
/// SwordClassからアクターをスポーンしてプレイヤーに装備させる.
/// BeginPlayで呼び出される.
/// </summary>
void APlayerManager::EquipSword()
{
	if (SwordClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwordClass is not set! Please set it in Blueprint."));
		return;
	}

	// 剣をスポーン
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	EquippedSword = GetWorld()->SpawnActor<AActor>(SwordClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (EquippedSword)
	{
		// 剣を右手にアタッチ
		EquippedSword->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SwordSocketName);

		UE_LOG(LogTemp, Warning, TEXT("Sword equipped successfully at socket: %s"), *SwordSocketName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn sword!"));
	}
}

/// <summary>
/// MeleeAttack - 近接攻撃処理.
/// 剣での攻撃またはキックを実行する（コンボシステム対応）.
/// 入力の "MeleeAttack" アクションにバインドされる.
/// </summary>
// PlayerManager.cpp - MeleeAttack関数（修正版）

void APlayerManager::MeleeAttack()
{
	UE_LOG(LogTemp, Warning, TEXT("=== MeleeAttack Called ==="));
	UE_LOG(LogTemp, Warning, TEXT("bCanMeleeAttack: %s"), bCanMeleeAttack ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("CurrentComboCount: %d"), CurrentComboCount);

	// クールダウン中は攻撃不可
	if (!bCanMeleeAttack)
	{
		UE_LOG(LogTemp, Warning, TEXT("Still on cooldown! Timer: %f"), MeleeAttackCooldownTimer);
		return;
	}

	// コンボウィンドウ外の場合はカウントをリセット
	if (!bIsInComboWindow)
	{
		CurrentComboCount = 0;
	}

	UAnimMontage* MontageToPlay = nullptr;
	EMeleeAttackType CurrentAttack = EMeleeAttackType::SwordSlash;

	// 現在のコンボ数に応じて攻撃を実行
	if (CurrentComboCount == 0)
	{
		// 1段目：剣での攻撃
		UE_LOG(LogTemp, Warning, TEXT(">>> Combo 0: Sword Slash Attack <<<"));

		CurrentAttack = EMeleeAttackType::SwordSlash;
		MontageToPlay = SwordSlashAnimMontage;

		if (SwordSlashAnimMontage == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("ERROR: SwordSlashAnimMontage is not set!"));
			return;
		}

		CurrentComboCount = 1;  // 次は1段目
	}
	else if (CurrentComboCount == 1)
	{
		// 2段目：キック攻撃
		UE_LOG(LogTemp, Warning, TEXT(">>> Combo 1: SwordDoubleSlash <<<"));

		CurrentAttack = EMeleeAttackType::SwordDoubleSlash;
		MontageToPlay = SwordDoubleSlashAnimMontage;

		if (SwordDoubleSlashAnimMontage == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("ERROR: KickAnimMontage is not set!"));
			return;
		}

		CurrentComboCount = 2;  // コンボ終了
	}
	else if (CurrentComboCount == 2)
	{
		// 2段目：キック攻撃
		UE_LOG(LogTemp, Warning, TEXT(">>> Combo : Sword Wave Attack <<<"));

		CurrentAttack = EMeleeAttackType::SwordWave;
		MontageToPlay = SwordWaveAnimMontage;

		if (SwordWaveAnimMontage == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("ERROR: KickAnimMontage is not set!"));
			return;
		}

		CurrentComboCount = 3;  // コンボ終了
	}
	else if (CurrentComboCount == 3)
	{
		// 2段目：キック攻撃
		UE_LOG(LogTemp, Warning, TEXT(">>> SwordPowerAttack <<<"));

		CurrentAttack = EMeleeAttackType::SwordPowerAttack;
		MontageToPlay = SwordPowerAttackAnimMontage;

		if (SwordWaveAnimMontage == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("ERROR: KickAnimMontage is not set!"));
			return;
		}

		CurrentComboCount = 4;  // コンボ終了
	}
	else if (CurrentComboCount == 4)
	{
		// 5段目：最終秘奥義
		UE_LOG(LogTemp, Warning, TEXT(">>> Combo 4: Sword Final Strike - ULTIMATE ATTACK! <<<"));
		CurrentAttack = EMeleeAttackType::SwordFinalStrike;
		MontageToPlay = SwordFinalStrikeAnimMontage;
		if (SwordFinalStrikeAnimMontage == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("ERROR: SwordFinalStrikeAnimMontage is not set!"));
			return;
		}
		CurrentComboCount = 5;
	}
	else if (CurrentComboCount == 5)
	{
		// 6段目：キック
		UE_LOG(LogTemp, Warning, TEXT(">>> Combo 5: Kick Attack <<<"));
		CurrentAttack = EMeleeAttackType::Kick;
		MontageToPlay = KickAnimMontage;
		if (KickAnimMontage == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("ERROR: KickAnimMontage is not set!"));
			return;
		}
		CurrentComboCount = 6;
	}
	else
	{
		// コンボ終了、リセット
		UE_LOG(LogTemp, Warning, TEXT("Combo finished, resetting..."));
		CurrentComboCount = 0;
		bIsInComboWindow = false;

		// リセット後に再度呼び出し
		MeleeAttack();
		return;
	}

	// アニメーション再生
	if (GetMesh() == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: Mesh is null!"));
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: AnimInstance is null!"));
		return;
	}

	if (MontageToPlay == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: MontageToPlay is null!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Playing montage: %s"), *MontageToPlay->GetName());
	AnimInstance->Montage_Play(MontageToPlay, 1.0f);

	// 攻撃判定を実行
	OnMeleeAttackHit(CurrentAttack);

	// コンボウィンドウを開く
	bIsInComboWindow = true;
	ComboWindowTimer = ComboWindowDuration;
	UE_LOG(LogTemp, Warning, TEXT("Combo window opened: %f seconds"), ComboWindowTimer);

	// クールダウン設定：アニメーションの長さをクールダウン時間として設定
	bCanMeleeAttack = false;
	float AnimationLength = MontageToPlay->GetPlayLength();
	MeleeAttackCooldownTimer = AnimationLength;

	// コンボウィンドウを開く：アニメーション終了後も次の入力を受け付けるように時間を延長
	bIsInComboWindow = true;
	ComboWindowTimer = AnimationLength + ComboWindowDuration;

	UE_LOG(LogTemp, Warning, TEXT("Attack executed! Combo: %d, Cooldown: %f seconds"),
		CurrentComboCount, MeleeAttackCooldownTimer);
}

#pragma endregion

#pragma region "移動"
/// <summary>
/// MoveForward - 前後方向の移動処理.
/// コントローラーの入力に応じてキャラクターを前後に移動させる.
/// </summary>
/// <param name="Value">入力値（-1.0 ～ 1.0）</param>
void APlayerManager::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		//カメラの前方向を取得.
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}
/// <summary>
/// MoveRight - 左右方向の移動処理.
/// コントローラーの入力に応じてキャラクターを左右に移動させる.
/// </summary>
/// <param name="Value">入力値（-1.0 ～ 1.0）</param>
void APlayerManager::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		//カメラの右方向を取得.
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}
/// <summary>
/// TurnAtRate - 視点の水平回転処理（Rate入力）
/// マウスの水平移動またはコントローラーのスティック入力でカメラを左右に回転させる.
/// </summary>
/// <param name="Rate">回転速度（入力値）</param>
void APlayerManager::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
/// <summary>
/// LookUpAtRate - 視点の垂直回転処理（Rate入力）
/// マウスの垂直移動またはコントローラーのスティック入力でカメラを上下に回転させる.
/// </summary>
/// <param name="Rate">回転速度（入力値）</param>
void APlayerManager::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
/// <summary>
/// StartSprint - スプリント開始処理.
/// キャラクターの移動速度をWalkSpeedからRunSpeedに変更する.
/// </summary>
void APlayerManager::StartSprint()
{
	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}
/// <summary>
/// StopSprint - スプリント終了処理.
/// キャラクターの移動速度をRunSpeedからWalkSpeedに戻す.
/// </summary>
void APlayerManager::StopSprint()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}
#pragma endregion


#pragma region "銃システム"

/// <summary>
/// GetMuzzleLocation - マズル位置の取得.
/// 銃ActorのMuzzleコンポーネントから現在の正確な位置を取得する.
/// </summary>
/// <returns>マズルのワールド座標</returns>
FVector APlayerManager::GetMuzzleLocation() const
{
	if (RevolverGun == nullptr)
	{
		return GetActorLocation(); // フォールバック
	}

	// 銃ActorのMuzzleコンポーネントを直接取得
	if (RevolverGun->Muzzle != nullptr)
	{
		return RevolverGun->Muzzle->GetComponentLocation();
	}

	// Muzzleが見つからない場合は銃の位置を返す
	return RevolverGun->GetActorLocation();
}

/// <summary>
/// GetMuzzleRotation - マズル方向の取得.
/// 銃ActorのMuzzleコンポーネントから現在の向きを取得する.
/// </summary>
/// <returns>マズルの回転情報</returns>
FRotator APlayerManager::GetMuzzleRotation() const
{
	if (RevolverGun == nullptr)
	{
		return FRotator::ZeroRotator;
	}

	if (RevolverGun->Muzzle != nullptr)
	{
		return RevolverGun->Muzzle->GetComponentRotation();
	}

	return RevolverGun->GetActorRotation();
}

void APlayerManager::UpdateHandIK()
{
	if (RevolverGun == nullptr || FollowCamera == nullptr)
	{
		return;
	}

	// マズルの位置を取得.
	FVector MuzzleLocation = GetMuzzleLocation();

	// カメラの情報を取得.
	FVector CameraLocation = FollowCamera->GetComponentLocation();
	FVector ForwardDir = FollowCamera->GetForwardVector();
	FVector RightDir = FollowCamera->GetRightVector();
	FVector UpDir = FollowCamera->GetUpVector();

	// 右手のIK目標位置：マズルから前方へ一定距離.
	RightHandIKLocation = MuzzleLocation + (ForwardDir * IKDistance);

	// 肘のIK目標位置：マズルから少し右下へ（自然な腕の曲がりのため）.
	RightElbowIKLocation = MuzzleLocation + (RightDir * 30.0f) + (UpDir * -20.0f);
}
#pragma endregion


#pragma region "カメラ"
/// <summary>
/// GetCameraVector - カメラからの方向ベクトルを取得.
/// (弾の発射方向などの計算に使用)
/// </summary>
/// <param name="dir">"Forward", "Right", "up" のどれか</param>
/// <returns>ベクトル</returns>
FVector APlayerManager::GetCameraVector(FString dir) const
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
FVector APlayerManager::GetCameraLocation() const
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
FRotator APlayerManager::GetCameraRotation() const
{
	if (FollowCamera == nullptr)
	{
		return FRotator::ZeroRotator;
	}
	return FollowCamera->GetComponentRotation();
}
#pragma endregion

#pragma region UI
/// <summary>
/// InitializeUI - UI初期化処理.
/// クロスヘアウィジェットをビューポートに追加して表示する.
/// BeginPlay時に呼ばれる.
/// </summary>
void APlayerManager::InitializeUI()
{
	if (CrosshairWidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("CrosshairWidgetClass is not set!"));
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController is not valid!"));
		return;
	}

	CrosshairWidget = CreateWidget<UCrosshairWidget>(PlayerController, CrosshairWidgetClass);
	if (CrosshairWidget != nullptr)
	{
		CrosshairWidget->AddToViewport(0);
		UE_LOG(LogTemp, Warning, TEXT("CrosshairWidget created and added to viewport"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create CrosshairWidget!"));
	}
}
#pragma endregion

#pragma region "射撃"
/// <summary>
/// ShotBullet() - 発射操作をした時に実行する.
/// [プレイヤー専用]
/// </summary>
void APlayerManager::ShotBullet()
{
	//リロード中は射撃不可.
	if (bIsReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reloading... Cannot shoot!"));
		return;
	}
	//弾薬がない場合はリロード開始.
	if (CurrentAmmoCount <= 0)
	{
		StartReload();
		return;
	}
	//BulletClassのnullチェック.
	if (BulletClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("BulletClass is not set! Please set it in Blueprint."));
		return;
	}
	//nullチェック.
	if (FollowCamera == nullptr || GetWorld() == nullptr)
	{
		return;
	}

	//クロスヘアの中心座標を画面座標で計算.
	const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	const FVector2D CrosshairScreenLocation = ViewportSize / 2.0f; // 画面中央.

	//スクリーン座標をワールド座標に変換.
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

	//目標地点を計算.
	const FVector TargetPosition = CrosshairWorldLocation + (CrosshairWorldDirection * BulletTargetDistance);

	//弾を発射.
	bool ret = ShotBulletExe(this, TargetPosition);
	if (ret) {
		//ショット時にクロスヘアのエフェクトを実行
		if (CrosshairWidget)
		{
			CrosshairWidget->OnShotEffect();
		}
	}

	// プレイヤーの回転をクロスヘア方向に向かせる
	{
		FVector DirectionToTarget = TargetPosition - GetActorLocation();
		DirectionToTarget.Normalize();
		FRotator TargetRotation = DirectionToTarget.Rotation();

		// Y軸（Yaw）とPitch軸を回転させる
		FRotator NewRotation = GetActorRotation();
		NewRotation.Yaw = TargetRotation.Yaw;
		NewRotation.Pitch = TargetRotation.Pitch;
		SetActorRotation(NewRotation);
	}

	//プレイヤーアニメーション.
	PlayAnimMontage(EAnimationState::Shot);
}
#pragma endregion

#pragma region "ダメージ処理"
//弾が当たったら実行される.
void APlayerManager::OnBulletHit() {

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("hit player"));
	//TODO
}
//死亡処理.
void APlayerManager::Die() {
	//TODO
}
#pragma endregion