/*
   - CharacterBase -

   プレイヤーと敵の基底クラス.
   元はなおと作のPlyerCharacterだったもの.
*/
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"
#include "Steam_Revolver.h"
#include "CharacterBase.generated.h"

/*
   [TODO] 2025/12/08
   CharacterBaseには、プレイヤーと敵の共通処理を入れたい.
   そのため、プレイヤーにしか必要のないカメラやUIなどは、ここではなくPlayerManagerに移動する.
*/

//前方宣言.
class ABulletBase;
class ASteam_Revolver;

// アニメーション状態の列挙型
UENUM(BlueprintType)
enum class EAnimationState : uint8
{
	Idle     UMETA(DisplayName = "Idle"),
	Move     UMETA(DisplayName = "Move"),
	Run      UMETA(DisplayName = "Run"),
	JumpUp   UMETA(DisplayName = "JumpUp"),
	JumpMid  UMETA(DisplayName = "JumpMid"),
	JumpDown UMETA(DisplayName = "JumpDown")
};

UCLASS()
class GUNACTION_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

//▼ ===== 変数 ===== ▼.
public:

#pragma region "Gun"
	//銃クラスの参照.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Gun")
	TSubclassOf<ASteam_Revolver> RevolverGunClass;

	UPROPERTY(BlueprintReadOnly, Category = "MyProperty|Base|Gun")
	ASteam_Revolver* RevolverGun;
#pragma endregion

#pragma region "Bullet"
	// 弾クラスの参照（Blueprintで設定可能）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Bullet")
	TSubclassOf<AActor> BulletClass;
	// 弾の発射距離（エディタで調整可能）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Bullet")
	float BulletTargetDistance = 10000.0f;
#pragma endregion

#pragma region "Animation"

	//腕のボーンインデックスをキャッシュ.
	UPROPERTY(VisibleAnywhere, Category = "MyProperty|Base|Animation")
	int32 RightArmBoneIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, Category = "MyProperty|Base|Animation")
	int32 RightForearmBoneIndex = INDEX_NONE;

	//アニメーション関連.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Animation")
	EAnimationState CurrentAnimationState; //現在のアニメーション状態.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Animation")
	class UAnimMontage* IdleAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Animation")
	class UAnimMontage* MoveAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Animation")
	class UAnimMontage* SprintAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Animation")
	class UAnimMontage* JumpUpAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Animation")
	class UAnimMontage* JumpMidAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Animation")
	class UAnimMontage* JumpDownAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Animation|Shooting")
	class UAnimMontage* PlayerFireAnimMontage;

#pragma endregion

#pragma region "Movement"
	// 移動パラメーター.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Movement")
	float BaseTurnRate = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Movement")
	float BaseLookUpRate = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Movement")
	float WalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Movement")
	float RunSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MyProperty|Base|Movement")
	bool bIsSprinting;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MyProperty|Base|Movement")
	bool bIsMoving;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MyProperty|Base|Movement")
	bool bIsJumping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MyProperty|Base|Movement")
	bool bAWasJumping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MyProperty|Base|Movement")
	double CurrentSpeed;
#pragma endregion

#pragma region "Ammunition"
	// 弾薬関連
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Ammunition")
	int32 MaxAmmoPerMagazine = 6;

	UPROPERTY(BlueprintReadOnly, Category = "MyProperty|Base|Ammunition")
	int32 CurrentAmmoCount = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Ammunition")
	float ReloadDuration = 2.5f; // リロード時間（秒）

	UPROPERTY(BlueprintReadOnly, Category = "MyProperty|Base|Ammunition")
	bool bIsReloading = false;

	UPROPERTY(BlueprintReadOnly, Category = "MyProperty|Base|Ammunition")
	float ReloadTimerElapsed = 0.0f;
#pragma endregion

//▼ ===== 関数 ===== ▼.
public:
#pragma region "コンストラクタ"
	ACharacterBase();
#pragma endregion

protected:
#pragma region "ライフサイクル"
	//召喚した瞬間.
	virtual void BeginPlay() override;
	//常に実行.
	virtual void Tick(float DeltaTime) override;
#pragma endregion

#pragma region "移動"
	//アニメーションを更新.
	void UpdateAnimationState();
	//攻撃アニメーション.
	void PlayFireAnimMontage();
	//アニメーションモンタージュを再生.
	void PlayAnimationMontage(EAnimationState AnimState);
#pragma endregion

#pragma region "射撃"

	//弾発射処理[仮想関数]
	virtual void ShotBullet(){} 
	//弾を発射する.
	bool ShotBulletExe(FVector loc, FRotator rot, FVector targetLoc, FActorSpawnParameters spawnParam);

	//ボーンインデックスを初期化する関数.
	void InitializeBoneIndices();

	//リロード開始.
	void StartReload();

	//リロード時間の更新.
	void UpdateReloadTimer(float DeltaTime);

	//銃を装備する..
	void EquipGun();

	//腕のボーンを回転させる関数.
	void RotateArmBones(const FRotator& TargetRotation);
#pragma endregion
};