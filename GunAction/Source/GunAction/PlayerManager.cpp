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

//他class.
//#include "Steam_Revolver.h"

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

	InitializeArmIK();

	//アニメーション状態の初期化.
	CurrentAnimationState = EAnimationState::Idle;
}

//常に実行.
void APlayerManager::Tick(float DeltaTime) {

	ACharacterBase::Tick(DeltaTime); //親クラスのTick()を呼び出す.

	// 腕のIKを毎フレーム更新
	UpdateArmIK();
	// 銃のアタッチメント位置を毎フレーム更新
	if (RevolverGun != nullptr)
	{
		RevolverGun->UpdateComponentTransforms();
	}
}

#pragma region "入力処理"
/// <summary>
/// SetupPlayerInputComponent - プレイヤー入力の設定.
/// </summary>
/// <param name="PlayerInputComponent">プレイヤー入力コンポーネント</param>
void APlayerManager::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Input(PlayerInputComponent);
}
/// <summary>
/// Input - 移動処理および入力バインド.
/// 移動、カメラ、ジャンプ、ダッシュ、射撃などの操作を登録.
/// </summary>
/// <param name="PlayerInputComponent">プレイヤー入力コンポーネント</param>
void APlayerManager::Input(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//移動.
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerManager::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerManager::MoveRight);

	//カメラ.
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &APlayerManager::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APlayerManager::LookUpAtRate);

	//ジャンプ.
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	//ダッシュ.
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayerManager::StartWalk);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerManager::StopWalk);

	//弾発射.
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APlayerManager::ShotBullet);

	//リロード.
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &APlayerManager::StartReload);
}
#pragma endregion

#pragma region "移動"
/// <summary>
/// MoveForward - 前後方向の移動.
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
		//前後に移動.
		AddMovementInput(Direction, Value);
	}
}
/// <summary>
/// MoveRight - 左右方向の移動.
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
		//左右に移動.
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
#pragma endregion


#pragma region "銃システム"

/*

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
*/

void APlayerManager::InitializeArmIK()
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

void APlayerManager::UpdateArmIK()
{
	if (!bEnbLeArmIK || !RevolverGun || !FollowCamera)
	{
		return;
	}
	// スクリーン座標をワールド座標に変換
	FVector CrosshairWorldLocation = FVector::ZeroVector;
	FVector CrosshairWorldDirection = FVector::ZeroVector;
	

	// 銃のマズルから目標位置への方向を計算
	FVector MuzzleLocation = FVector::ZeroVector;
	if (RevolverGun && RevolverGun->Muzzle)
	{
		MuzzleLocation = RevolverGun->Muzzle->GetComponentLocation();
	}
	else
	{
		MuzzleLocation = FollowCamera->GetComponentLocation() + (FollowCamera->GetForwardVector() * 100.0f);
	}
	const FVector TargetPosition = CrosshairWorldLocation + (CrosshairWorldDirection * BulletTargetDistance);

	// IKターゲット位置を設定（クロスヘアの方向に向ける）
	RightHandIKTarget = TargetPosition;
	RightHandIKAlpha = 1.0f;

}
#pragma endregion


#pragma region "カメラ"
/// <summary>
/// GetCameraVector - カメラからの方向ベクトルを取得.
/// (弾の発射方向などの計算に使用)
/// </summary>
/// <param name="dir">"Forward", "Right", "Up" のどれか</param>
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
	//発射していいかチェック.
	if (!ShotBulletCheck()) {
		return;
	}
	//nullチェック.
	if (FollowCamera == nullptr) {
		return;
	}

	//クロスヘアの中心座標を画面座標で計算.
	const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	const FVector2D CrosshairScreenLocation = ViewportSize / 2.0f; // 画面中央.

	//スクリーン座標をワールド座標に変換.
	FVector CrosshairWorldLocation  = FVector::ZeroVector;
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
	//弾を発射.
	bool ret = ShotBulletExe(this, TargetPosition);
	if (ret) {
		//ショット時にクロスヘアのエフェクトを実行.
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
}
#pragma endregion

#pragma region "ダメージ処理"
//弾が当たったら実行される.
void APlayerManager::OnBulletHit() {

//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("hit player"));
	//TODO
}
//死亡処理.
void APlayerManager::Die() {
	//TODO
}
#pragma endregion