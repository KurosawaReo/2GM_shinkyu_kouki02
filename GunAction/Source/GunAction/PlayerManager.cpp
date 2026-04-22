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

#pragma region "基本処理"

/// <summary>
/// コンストラクタ.
/// </summary>
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

/// <summary>
/// 召喚した瞬間.
/// </summary>
void APlayerManager::BeginPlay() {

	ACharacterBase::BeginPlay(); //親クラスのBeginPlay()を呼び出す.

	//クロスヘアUIを初期化.
	InitUI();
	//アニメーション状態の初期化.
	CurrentAnimationState = EAnimationState::Idle;
}

/// <summary>
/// 常に実行.
/// </summary>
void APlayerManager::Tick(float DeltaTime) {

	ACharacterBase::Tick(DeltaTime); //親クラスのTick()を呼び出す.
}

#pragma endregion

#pragma region "入力処理"

/// <summary>
/// プレイヤー入力の設定.
/// </summary>
/// <param name="PlayerInputComponent">プレイヤー入力</param>
void APlayerManager::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	Input(PlayerInputComponent);
}

/// <summary>
/// 入力バインド.
/// 移動、カメラ、ジャンプ、ダッシュ、射撃などの操作を登録.
/// </summary>
/// <param name="PlayerInputComponent">プレイヤー入力コンポーネント</param>
void APlayerManager::Input(UInputComponent* PlayerInputComponent)
{
	//移動.
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerManager::OnMoveForward);
	PlayerInputComponent->BindAxis("MoveRight",   this, &APlayerManager::OnMoveRight);

	//カメラ.
	PlayerInputComponent->BindAxis("Turn",       this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp",     this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate",   this, &APlayerManager::OnTurnRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APlayerManager::OnLookUpRate);

	//ジャンプ.
	PlayerInputComponent->BindAction("Jump", IE_Pressed,    this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released,   this, &ACharacter::StopJumping);

	//ダッシュ.
	PlayerInputComponent->BindAction("Sprint", IE_Pressed,  this, &APlayerManager::OnWalkStart);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerManager::OnWalkStop);

	//射撃.
	PlayerInputComponent->BindAction("Fire",   IE_Pressed,  this, &APlayerManager::OnFire);

	//リロード.
	PlayerInputComponent->BindAction("Reload", IE_Pressed,  this, &APlayerManager::OnReload);
}

#pragma endregion

#pragma region "移動"

/// <summary>
/// 前後方向の移動.
/// </summary>
/// <param name="Value">入力値（-1.0 ～ 1.0）</param>
void APlayerManager::OnMoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		//カメラの前方向を取得.
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		//前後に移動.
		Move(Direction, Value);
	}
}
/// <summary>
/// 左右方向の移動.
/// </summary>
/// <param name="Value">入力値（-1.0 ～ 1.0）</param>
void APlayerManager::OnMoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		//カメラの右方向を取得.
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		//左右に移動.
		Move(Direction, Value);
	}
}
/// <summary>
/// 視点の水平回転処理.
/// カメラを左右に回転させる.
/// </summary>
/// <param name="Rate">回転速度</param>
void APlayerManager::OnTurnRate(float Rate)
{
	AddControllerYawInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
/// <summary>
/// 視点の垂直回転処理.
/// カメラを上下に回転させる.
/// </summary>
/// <param name="Rate">回転速度</param>
void APlayerManager::OnLookUpRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

#pragma endregion

#pragma region "射撃"

/// <summary>
/// 射撃処理.
/// </summary>
void APlayerManager::OnFire()
{
	//nullチェック.
	if (FollowCamera == nullptr) {
		return;
	}
	//射撃開始.
	ShotStart();
}

/// <summary>
/// 射撃実行.
/// </summary>
void APlayerManager::ShotExe()
{
	//クロスヘアの中心座標を画面座標で計算.
	const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	const FVector2D CrosshairScreenLocation = ViewportSize / 2.0f; //画面中央.

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

	//弾を召喚.
	bool ret = SpawnBullet(this, TargetPosition);
	if (ret) {
		//ショット時にクロスヘアのエフェクトを実行.
		if (CrosshairWidget)
		{
			CrosshairWidget->OnShotEffect();
		}
	}

	//プレイヤーをクロスヘアの方へ向かせる.
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
}

#pragma endregion

#pragma region "ダメージ処理"

/// <summary>
/// 弾が当たったら実行される.
/// </summary>
void APlayerManager::OnBulletHit() {

//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("hit player"));

	Death(); //死亡処理.
}

/// <summary>
/// 死亡処理.
/// </summary>
void APlayerManager::Death() {
	//TODO
}

#pragma endregion

#pragma region UI

/// <summary>
/// UIの初期化処理.
/// </summary>
void APlayerManager::InitUI()
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

	//クロスヘアウィジェットをビューポートに追加して表示する.
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
