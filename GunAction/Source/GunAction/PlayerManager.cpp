/*
   - PlayerManager -
   共通のCharacterクラスから派生したプレイヤークラス.

   [クラス構成]
   CharacterBase
   └PlayerManager ←ここ
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
}

//常に実行.
void APlayerManager::Tick(float DeltaTime) {

	ACharacterBase::Tick(DeltaTime); //親クラスのTick()を呼び出す.
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
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerManager::MoveRight);

	//カメラ入力.
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &APlayerManager::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APlayerManager::LookUpAtRate);

	//ジャンプ入力.
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	//スプリント入力.
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayerManager::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerManager::StopSprint);

	//弾発射入力.
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APlayerManager::ShotBullet);

	//リロード入力.
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &APlayerManager::StartReload);
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
/// スケルタルメッシュの"Muzzle"ソケットから現在の正確な位置を取得する.
/// </summary>
/// <returns>マズルのワールド座標</returns>
FVector APlayerManager::GetMuzzleLocation() const
{
	if (RevolverGun == nullptr)
	{
		//UE_LOG(LogTemp, Warning, TEXT("GunMeshComponent is not valid!"));
		return GetActorLocation(); // フォールバック
	}

	USkeletalMeshComponent* GunMesh = Cast<USkeletalMeshComponent>(RevolverGun->GetRootComponent());

	if (GunMesh == nullptr)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Gun mesh component is not valid!"));
		return RevolverGun->GetActorLocation();
	}

	//マズルソケットが存在するか確認.
	if (!GunMesh->DoesSocketExist(MuzzleSocketName))
	{
		return GetActorLocation();
	}

	// マズルソケットのワールド座標を取得
	return GunMesh->GetSocketLocation(MuzzleSocketName);
}

/// <summary>
/// GetMuzzleRotation - マズル方向の取得.
/// スケルタルメッシュの"Muzzle"ソケットから現在の向きを取得する.
/// </summary>
/// <returns>マズルの回転情報</returns>
FRotator APlayerManager::GetMuzzleRotation() const
{
	if (RevolverGun == nullptr)
	{
		return RevolverGun->GetActorRotation();
	}
	USkeletalMeshComponent* GunMesh = Cast<USkeletalMeshComponent>(RevolverGun->GetRootComponent());

	if (GunMesh == nullptr)
	{
		return RevolverGun->GetActorRotation();
	}

	if (GunMesh->DoesSocketExist(MuzzleSocketName))
	{
	
		return RevolverGun->GetActorRotation();
	}

	return GunMesh->GetSocketRotation(MuzzleSocketName);
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
	//↑を取得する.
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
	//カメラの位置を取得.
	const FVector  CameraLocation = FollowCamera->GetComponentLocation();
	const FRotator CameraRotation = FollowCamera->GetComponentRotation();

	//弾の設定 - ①スポーン位置.
	FVector SpawnLocation;
	{
		if (RevolverGun && RevolverGun->Muzzle) {
			SpawnLocation = RevolverGun->Muzzle->GetComponentLocation();
		}
		else {
			//マズルがない場合はカメラの少し前方から発射.
			SpawnLocation = CameraLocation + (GetCameraVector("Forward") * 100.0f) - (GetCameraVector("Right") * 20.0f);
		}
	}

	//弾の設定 - ②発射方向.
	FRotator BulletRotation;
	{
		FVector dir = TargetPosition - SpawnLocation;
		dir.Normalize();
		BulletRotation = dir.Rotation();
	}

	//弾の設定 - ③スポーンパラメーター.
	FActorSpawnParameters SpawnParams;
	{
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
	}

	//弾を発射.
	bool ret = ShotBulletExe(SpawnLocation, BulletRotation, TargetPosition, SpawnParams);
	//発射に成功したら.
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

		// Y軸（Yaw）のみ回転させる（上下は変わらない）
		FRotator NewRotation = GetActorRotation();
		NewRotation.Yaw = TargetRotation.Yaw;
		NewRotation.Pitch = TargetRotation.Pitch;
		SetActorRotation(NewRotation);
	}

	//プレイヤーアニメーション.
	PlayFireAnimMontage();
}
#pragma endregion