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