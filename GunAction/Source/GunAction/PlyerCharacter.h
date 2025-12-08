/*
   【注意】
   古いプレイヤークラス.
   現在はCharacterBaseへと移行済み.
*/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"
#include "PlyerCharacter.generated.h"

class UCrosshairWidget;
class USpringArmComponent;
class UCameraComponent;
class ABulletBase;
class ASteam_Revolver;

// アニメーション状態の列挙型
UENUM(BlueprintType)
enum class EAnimationStateTmp : uint8
{
	Idle     UMETA(DisplayName = "Idle"),
	Move     UMETA(DisplayName = "Move"),
	Run      UMETA(DisplayName = "Run"),
	JumpUp   UMETA(DisplayName = "JumpUp"),
	JumpMid  UMETA(DisplayName = "JumpMid"),
	JumpDown UMETA(DisplayName = "JumpDown")
};

UCLASS()
class GUNACTION_API APlyerCharacter : public ACharacter
{
	GENERATED_BODY()

	//▼変数.
public:
	//クロスヘア.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UCrosshairWidget> CrosshairWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UCrosshairWidget* CrosshairWidget;

	//カメラコンポーネント.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class UCameraComponent* FollowCamera;

	// 銃クラスの参照
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
	TSubclassOf<ASteam_Revolver> RevolverGunClass;

	UPROPERTY(BlueprintReadOnly, Category = "Gun")
	ASteam_Revolver* RevolverGun;

	// 弾クラスの参照（Blueprintで設定可能）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bullet)
	TSubclassOf<AActor> BulletClass;

	//腕のボーンインデックスをキャッシュ.
	UPROPERTY(VisibleAnywhere, Category = "Animation")
	int32 RightArmBoneIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, Category = "Animation")
	int32 RightForearmBoneIndex = INDEX_NONE;

	//アニメーション関連.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	class UAnimMontage* IdleAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	class UAnimMontage* MoveAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	class UAnimMontage* SprintAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	class UAnimMontage* JumpUpAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	class UAnimMontage* JumpMidAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	class UAnimMontage* JumpDownAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Shooting")
	class UAnimMontage* PlayerFireAnimMontage;

	//移動パラメーター.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float BaseTurnRate = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float BaseLookUpRate = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float WalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float RunSpeed = 800.0f;

	// 弾の発射距離（エディタで調整可能）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
	float BulletTargetDistance = 10000.0f; // 100メートル先

	// 弾薬関連
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammunition")
	int32 MaxAmmoPerMagazine = 6;

	UPROPERTY(BlueprintReadOnly, Category = "Ammunition")
	int32 CurrentAmmoCount = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammunition")
	float ReloadDuration = 2.5f; // リロード時間（秒）

	UPROPERTY(BlueprintReadOnly, Category = "Ammunition")
	bool bIsReloading = false;

	UPROPERTY(BlueprintReadOnly, Category = "Ammunition")
	float ReloadTimerElapsed = 0.0f;

	//インスペクターに表示する方法.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bIsSprinting;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bIsMoving;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bIsJumping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bAWasJumping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	double CurrentSpeed;

	//現在のアニメーション状態.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	EAnimationStateTmp CurrentAnimationState;

	//▼関数.
public:
#pragma region "コンストラクタ"
	APlyerCharacter();
#pragma endregion

protected:
#pragma region "ライフサイクル"
	//召喚した瞬間.
	virtual void BeginPlay() override;
	//常に実行.
	virtual void Tick(float DeltaTime) override;
#pragma endregion

#pragma region "入力処理"
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void Input(UInputComponent* PlayerInputComponent);
#pragma endregion

#pragma region "カメラ"
	// カメラの向きを取得する関数.
	UFUNCTION(BlueprintCallable, Category = "Camera")
	FVector GetCameraVector(FString dir) const;

	UFUNCTION(BlueprintCallable, Category = "Camera")
	FVector GetCameraLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Camera")
	FRotator GetCameraRotation() const;
#pragma endregion

#pragma region "移動"
	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void StartSprint();
	void StopSprint();
	//アニメーションを更新.
	void UpdateAnimationState();

	//プレイヤー攻撃アニメーション.
	void PlayFireAnimMontage();
	//アニメーションモンタージュを再生.
	void PlayAnimationMontage(EAnimationStateTmp AnimState);
#pragma endregion

#pragma region "UI"
	void InitializeUI();
#pragma endregion

#pragma region "射撃"
	// 銃を装備する.
	void EquipGun();

	// カメラの前方向 × 距離の位置を計算.
	void CalculateAndShot();

	//弾発射処理.
	void ShotBullet();

	// リロード開始
	void StartReload();

	// リロード時間の更新
	void UpdateReloadTimer(float DeltaTime);

	//ボーンインデックスを初期化する関数.
	void InitializeBoneIndices();

	//腕のボーンを回転させる関数.
	void RotateArmBones(const FRotator& TargetRotation);

	// カメラ前方の目標地点を計算する関数.
	UFUNCTION(BlueprintCallable, Category = "Bullet")
	FVector CalculateTargetPosition(float Distance) const;
#pragma endregion
};