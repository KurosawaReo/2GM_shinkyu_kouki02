// PlayerManager.h

#pragma once
#include "CoreMinimal.h"
#include "CharacterBase.h" //親クラス.
#include "Steam_Revolver.h"
#include "PlayerManager.generated.h"
class UCrosshairWidget;
class USpringArmComponent;
class UCameraComponent;

// 近接攻撃のタイプ
UENUM(BlueprintType)
enum class EMeleeAttackType : uint8
{
	SwordSlash       = 0 UMETA(DisplayName = "Sword_Slash"),
	SwordDoubleSlash = 1 UMETA(DisplayName = "Sword_Double_Slash"),
	SwordWave        = 2 UMETA(DisplayName = "Swprd_Wave"),
	SwordPowerAttack = 3 UMETA(DisplayName = "Sword_Power_Attak"),
	SwordFinalStrike = 4 UMETA(DisplayName = "Sword_FinalStrike"),
	Kick = 5 UMETA(DisplayName = "Kick")
};

/// <summary>
/// プレイヤークラス.
/// </summary>
UCLASS()
class GUNACTION_API APlayerManager : public ACharacterBase
{
	GENERATED_BODY()
	//▼ ===== 変数 ===== ▼.
public:
	//クロスヘア.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|UI")
	TSubclassOf<UCrosshairWidget> CrosshairWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|UI")
	UCrosshairWidget* CrosshairWidget;
	//カメラコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|Camera")
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|Camera")
	class UCameraComponent* FollowCamera;
	// マズルソケットの名前
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|Gun|Muzzle")
	FName MuzzleSocketName = FName("Muzzle"); // スケルタルメッシュ内のソケット名

	//▼ ===== IKアニメーション用の変数 ===== ▼
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|IK")
	float IKAlpha = 1.0f; // IKの強度（0.0～1.0）

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|IK")
	FVector RightHandIKLocation = FVector::ZeroVector; // 右手のIK目標位置（ワールド座標）

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|IK")
	FVector RightElbowIKLocation = FVector::ZeroVector; // 肘のIK目標位置（オプション）

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|IK")
	FName RightHandSocketName = FName("upperarm_r");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|IK")
	float IKDistance = 100.0f;//まずるからの距離.

	//▼ ===== 近接攻撃用の変数 ===== ▼
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|MeleeAttack")
	TSubclassOf<AActor> SwordClass;

	UPROPERTY(BlueprintReadOnly, Category = "MyProperty|Player|MeleeAttack")
	AActor* EquippedSword;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|MeleeAttack")
	UAnimMontage* SwordSlashAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|MeleeAttack")
UAnimMontage* SwordDoubleSlashAnimMontage;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|MeleeAttack")
UAnimMontage* SwordWaveAnimMontage;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|MeleeAttack")
UAnimMontage* SwordPowerAttackAnimMontage;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|MeleeAttack")
UAnimMontage* SwordFinalStrikeAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|MeleeAttack")
	UAnimMontage* KickAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|MeleeAttack")
	float MeleeAttackCooldown = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|MeleeAttack")
	float ComboWindowDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|MeleeAttack")
	float MeleeAttackRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|MeleeAttack")
	float MeleeAttackDamage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|MeleeAttack")
	FName SwordSocketName = FName("hand_l");

	// 内部状態
	bool bCanMeleeAttack = true;
	float MeleeAttackCooldownTimer = 0.0f;

	// コンボシステム
	int32 CurrentComboCount = 0;
	float ComboWindowTimer = 0.0f;
	bool bIsInComboWindow = false;


	//▼ ===== 関数 ===== ▼.
protected:
	//コンストラクタ.
	APlayerManager();
#pragma region "ライフサイクル"
	//召喚した瞬間.
	void BeginPlay() override;
	//常に実行.
	void Tick(float DeltaTime) override;
#pragma endregion
#pragma region "入力処理"
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void Input(UInputComponent* PlayerInputComponent);
#pragma endregion
#pragma region "移動"
	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void StartSprint();
	void StopSprint();
#pragma endregion
#pragma region "銃システム"
private:
	// マズル位置を取得する関数
	FVector GetMuzzleLocation() const;
	// マズル方向を取得する関数
	FRotator GetMuzzleRotation() const;

	void UpdateHandIK();
#pragma endregion
#pragma region "カメラ"
	UFUNCTION(BlueprintCallable, Category = "Camera")
	FVector  GetCameraVector(FString dir) const;
	UFUNCTION(BlueprintCallable, Category = "Camera")
	FVector  GetCameraLocation() const;
	UFUNCTION(BlueprintCallable, Category = "Camera")
	FRotator GetCameraRotation() const;
#pragma endregion
#pragma region "UI"
	void InitializeUI();
#pragma endregion

#pragma region "射撃"
	void ShotBullet() override; //override
#pragma endregion

#pragma region "近接攻撃"
	//剣を装備.
	void EquipSword();
	//近接攻撃実行.
	void MeleeAttack();

	void UnequipSword();
	//近接攻撃がヒットした時の処理.
	void OnMeleeAttackHit(EMeleeAttackType AtaackType);
	//コンボシステムの更新.
	void UpdateMeleeCombo(float DeltaTime);

#pragma endregion

public:
#pragma region "ダメージ処理"
	void OnBulletHit() override; //弾が当たったら実行される.
	void Die()         override; //死亡処理.
#pragma endregion
};