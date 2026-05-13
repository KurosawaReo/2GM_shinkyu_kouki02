/*
   - PlayerCharacter -
   共通のCharacterクラスから派生したプレイヤークラス.

   [クラス構成]
   CharacterBase
   └PlayerCharacter ←今ここ
   └EnemyCharacter
*/
#include "PlayerCharacter.h"

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
APlayerCharacter::APlayerCharacter() {

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
/// 召喚した瞬間に実行.
/// </summary>
void APlayerCharacter::BeginPlay() {

	ACharacterBase::BeginPlay(); //親クラスのBeginPlay()を呼び出す.

	//クロスヘアUIを初期化.
	InitUI();
}

/// <summary>
/// 常に実行.
/// </summary>
void APlayerCharacter::Tick(float DeltaTime) {

	ACharacterBase::Tick(DeltaTime); //親クラスのTick()を呼び出す.
}

#pragma endregion

#pragma region "入力処理"

/// <summary>
/// プレイヤー入力の設定.
/// </summary>
/// <param name="PlayerInputComponent">プレイヤー入力</param>
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	Input(PlayerInputComponent);
}

/// <summary>
/// 入力バインド.
/// 移動、カメラ、ジャンプ、ダッシュ、射撃などの操作を登録.
/// </summary>
/// <param name="PlayerInputComponent">プレイヤー入力コンポーネント</param>
void APlayerCharacter::Input(UInputComponent* PlayerInputComponent)
{
	//移動.
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::OnMoveForward);
	PlayerInputComponent->BindAxis("MoveRight",   this, &APlayerCharacter::OnMoveRight);

	//カメラ.
	PlayerInputComponent->BindAxis("Turn",       this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp",     this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate",   this, &APlayerCharacter::OnTurnRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APlayerCharacter::OnLookUpRate);

	//ジャンプ.
	PlayerInputComponent->BindAction("Jump", IE_Pressed,    this, &ACharacterBase::OnJump);
	PlayerInputComponent->BindAction("Jump", IE_Released,   this, &ACharacterBase::OnJumpStop);

	//ロール(回避)
	PlayerInputComponent->BindAction("Roll", IE_Pressed, this, &ACharacterBase::OnRoll);

	//ダッシュ.
	PlayerInputComponent->BindAction("Sprint", IE_Pressed,  this, &APlayerCharacter::OnWalkStart);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerCharacter::OnWalkStop);

	//射撃.
	PlayerInputComponent->BindAction("Shot",   IE_Pressed,  this, &APlayerCharacter::OnShot);

	//リロード.
	PlayerInputComponent->BindAction("Reload", IE_Pressed,  this, &APlayerCharacter::OnReload);
}

#pragma endregion

#pragma region "移動"

/// <summary>
/// 前後方向の移動.
/// </summary>
/// <param name="Value">入力値(-1.0 ～ 1.0)</param>
void APlayerCharacter::OnMoveForward(float Value)
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
/// <param name="Value">入力値(-1.0 ～ 1.0)</param>
void APlayerCharacter::OnMoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		//カメラの右方向を取得.
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector  Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		//左右に移動.
		Move(Direction, Value);
	}
}
/// <summary>
/// 視点の水平回転処理.
/// カメラを左右に回転させる.
/// </summary>
/// <param name="Rate">回転速度</param>
void APlayerCharacter::OnTurnRate(float Rate)
{
	AddControllerYawInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
/// <summary>
/// 視点の垂直回転処理.
/// カメラを上下に回転させる.
/// </summary>
/// <param name="Rate">回転速度</param>
void APlayerCharacter::OnLookUpRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

#pragma endregion

#pragma region "射撃"

/// <summary>
/// 射撃処理.
/// </summary>
void APlayerCharacter::OnShot()
{
	//nullチェック.
	if (FollowCamera == nullptr) {
		return;
	}

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
	FVector Target = CrosshairWorldLocation + (CrosshairWorldDirection * BulletTargetDistance);
	//射撃開始.
	ShotStart(Target);
}

/// <summary>
/// クロスヘアエフェクト実行.
/// </summary>
void APlayerCharacter::CrosshairWidgetExe() {

	if (CrosshairWidget) {
		CrosshairWidget->OnShotEffect();
	}
}

#pragma endregion

#pragma region "ダメージ処理"

/// <summary>
/// 弾が当たったら実行される.
/// </summary>
void APlayerCharacter::OnBulletHit() {

//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("hit player"));

	Death(); //死亡処理.
}

/// <summary>
/// 死亡処理.
/// </summary>
void APlayerCharacter::Death() {
	//TODO
}

#pragma endregion

#pragma region UI

/// <summary>
/// UIの初期化処理.
/// </summary>
void APlayerCharacter::InitUI()
{
	//非表示なら表示しない.
	if (!IsShowCrosshair) {
		return;
	}

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
