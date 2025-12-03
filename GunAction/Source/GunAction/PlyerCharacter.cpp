// Fill out your copyright notice in the Description page of Project Settings.


#include "PlyerCharacter.h"
#include "CrosshairWidget.h" 
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "BulletBase.h"

#pragma region "コンストラクタ".
/// <summary>
/// コンストラクタ - APlyerCharacter
/// プレイヤーキャラクターの初期設定を行う.
/// カプセルコライダー、カメラ、スプリングアームなどの設定を初期化する.
/// </summary>
APlyerCharacter::APlyerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//カプセルコライダー設定.
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	//キャラクターの回転速度(カメラ方向に回転させる).
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	//キャラクター移動の設定.
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed	;

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

	//初期状態.
	bIsSprinting = false;
}
#pragma endregion

#pragma region "ライフサイクル".
/// <summary>
/// BeginPlay - ゲーム開始時またはアクタースポーン時に呼ばれる.
/// UI（クロスヘア）の初期化を行う.
/// </summary>
void APlyerCharacter::BeginPlay()
{
	Super::BeginPlay();

	//クロスヘアUIを初期化.
	InitializeUI();
}

/// <summary>
/// Tick - 毎フレーム呼ばれる処理.
/// アニメーション状態を更新する.
/// </summary>
/// <param name="DeltaTime">前フレームからの経過時間（秒）</param>
void APlyerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//アニメーション状態を更新.
	UpdateAnimationState();

}
#pragma endregion

#pragma region "入力処理".
/// <summary>
/// SetupPlayerInputComponent - プレイヤー入力の設定.
/// キーボード、マウスなどの入力を各アクションにバインドする.
/// </summary>
/// <param name="PlayerInputComponent">プレイヤー入力コンポーネント</param>
void APlyerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Input(PlayerInputComponent);
}
// <summary>
/// Input - 移動処理および入力バインド.
/// キャラクターの移動、カメラ操作、ジャンプ、スプリント、射撃などの入力をセットアップする.
/// </summary>
/// <param name="PlayerInputComponent">プレイヤー入力コンポーネント</param>
void APlyerCharacter::Input(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//移動入力.
	PlayerInputComponent->BindAxis("MoveForward", this, &APlyerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlyerCharacter::MoveRight);

	//カメラ入力.
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &APlyerCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APlyerCharacter::LookUpAtRate);

	//ジャンプ入力.
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	//スプリント入力.
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlyerCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlyerCharacter::StopSprint);

	//弾発射入力.
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APlyerCharacter::ShotBullet);
}
#pragma endregion

#pragma region "カメラ".
/// <summary>
/// GetCameraVector - カメラからの方向ベクトルを取得.
/// (弾の発射方向などの計算に使用)
/// </summary>
/// <param name="dir">"Forward", "Right", "up" のどれか</param>
/// <returns>ベクトル</returns>
FVector APlyerCharacter::GetCameraVector(FString dir) const
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
FVector APlyerCharacter::GetCameraLocation() const
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
FRotator APlyerCharacter::GetCameraRotation() const
{
	if (FollowCamera == nullptr)
	{
		return FRotator::ZeroRotator;
	}
	return FollowCamera->GetComponentRotation();
}
#pragma endregion

#pragma region "移動".
/// <summary>
/// MoveForward - 前後方向の移動処理.
/// コントローラーの入力に応じてキャラクターを前後に移動させる.
/// </summary>
/// <param name="Value">入力値（-1.0 ～ 1.0）</param>
void APlyerCharacter::MoveForward(float Value)
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
void APlyerCharacter::MoveRight(float Value)
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
void APlyerCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());

}
/// <summary>
/// LookUpAtRate - 視点の垂直回転処理（Rate入力）
/// マウスの垂直移動またはコントローラーのスティック入力でカメラを上下に回転させる.
/// </summary>
/// <param name="Rate">回転速度（入力値）</param>
void APlyerCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());

}
/// <summary>
/// StartSprint - スプリント開始処理.
/// キャラクターの移動速度をWalkSpeedからRunSpeedに変更する.
/// </summary>
void APlyerCharacter::StartSprint()
{
	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;

}
/// <summary>
/// StopSprint - スプリント終了処理.
/// キャラクターの移動速度をRunSpeedからWalkSpeedに戻す.
/// </summary>
void APlyerCharacter::StopSprint()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

/// <summary>
/// アニメーション状態を更新する関数.
/// キャラクターの移動状態に応じてIdleまたはMoveアニメーションを再生.
/// </summary>
void APlyerCharacter::UpdateAnimationState()
{
	//現在の速度の判定(速度が一定値以上).
	//bool bWasMoving = bIsMoving;

	//bIsMoving = (CurrentSpeed >= 0.1f);

	// 現在の速度を取得.
	CurrentSpeed = GetCharacterMovement()->Velocity.Length();

	// 移動状態の判定.
	if (CurrentSpeed > 0.1)
	{
		bIsMoving = true;
	}
	else
	{
		bIsMoving = false;
	}
	// ジャンプ状態の判定（空中にいるか）
	if (!GetCharacterMovement()->IsMovingOnGround() && GetCharacterMovement()->Velocity.Z != 0.0f)
	{
		bIsJumping = true;
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("true"));
	}
	else
	{
		bIsJumping = false;
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("false"));
	}

}
#pragma endregion

#pragma region "射撃".
/// <summary>
/// CalculateAndShot - 弾の計算と発射処理.
/// カメラの位置と方向から弾をスポーンし、ターゲット位置に向かって発射する.
/// 発射時にクロスヘアのエフェクトも実行される.
/// </summary>
void APlyerCharacter::CalculateAndShot()
{
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

	//カメラの位置を取得.
	FVector  CameraLocation = FollowCamera->GetComponentLocation();
	FRotator CameraRotation = FollowCamera->GetComponentRotation();

	// TODO: ↓↓↓銃の先端の座標から弾を出す <<<<<<<<<<<<<<<<<<<<<<<<<<
	
	//弾の発射位置(カメラの少し前方)
	FVector SpawnLocation = CameraLocation + (GetCameraVector("Forward") * 100.0f) - (GetCameraVector("Right") * 20.0f);

//	FString msg = FString::Printf(TEXT("SpawnLocation: %f, %f, %f"), SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z); //変数組み込み.
//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, msg); //表示.

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	//弾のスポーンパラメーター設定.
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	//弾クラスを生成.
	//SpawnActor<生成するもの>(クラス, 座標, 角度, SpawnParam);
	ABulletBase* Bullet = GetWorld()->SpawnActor<ABulletBase>(BulletClass, SpawnLocation, CameraRotation, SpawnParams);

	//弾の目標地点: カメラの前方向 × 距離.
	FVector TargetPosition = CalculateTargetPosition(BulletTargetDistance);

	if (Bullet != nullptr)
	{
		//弾クラスのメンバ関数.
		Bullet->ShotPos(TargetPosition);
		//ショット時にクロスヘアのエフェクトを実行
		if (CrosshairWidget)
		{
			CrosshairWidget->OnShotEffect();
		}
	}
}

/// <summary>
/// ShotBullet - 弾発射コマンド.
/// CalculateAndShot関数を呼び出して弾を発射する.
/// 入力イベントから直接呼ばれる.
/// </summary>
void APlyerCharacter::ShotBullet()
{
	CalculateAndShot();
}
/// <summary>
/// ターゲット位置を計算する関数.
/// カメラの位置から前方向へ指定距離だけ離れた地点を計算する.
/// 弾の発射目標地点を決定するために使用される.
/// </summary>
FVector APlyerCharacter::CalculateTargetPosition(float Distance) const
{
	if (FollowCamera == nullptr)
	{
		return FVector::ZeroVector;
	}

	//カメラの位置を取得.
	FVector CameraLocation = GetCameraLocation();
	//カメラの方向を取得.
	FVector ForwardVector  = GetCameraVector("Forward"); //前.
	//位置 + (前方向*距離)
	FVector TargetPosition = CameraLocation + (ForwardVector * Distance);

	return TargetPosition;
}
#pragma endregion

#pragma region UI.
/// <summary>
/// InitializeUI - UI初期化処理.
/// クロスヘアウィジェットをビューポートに追加して表示する.
/// BeginPlay時に呼ばれる.
/// </summary>
void APlyerCharacter::InitializeUI()
{

	if (CrosshairWidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("set"));
		return;

	}

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController == nullptr)
	{
		return;
	}
	// クロスヘアウィジェットを作成.
	CrosshairWidget = CreateWidget<UCrosshairWidget>(PlayerController, CrosshairWidgetClass);
	if (CrosshairWidget != nullptr)
	{
		CrosshairWidget->AddToViewport(0);
	}
}
#pragma endregion