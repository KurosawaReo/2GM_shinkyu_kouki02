/*----------------------------------------------------/
   - CharacterBase -
   プレイヤーと敵の親クラス.
/-----------------------------------------------------/
   [クラス構成]
   CharacterBase   ←今ここ
   └PlayerManager
   └EnemyManager

   CharacterBase: プレイヤーと敵 に使うものを入れる
   PlayerManager: プレイヤー     に使うものを入れる
   EnemyManager : 敵             に使うものを入れる
/-----------------------------------------------------/
   [仮想関数]
   ざっくり解説。(※あくまでも使い方の一例)

   virtualに対してoverride(=上書き)する。

   //親のクラス.
   class CharacterBase 
   {
       void BeginPlay(){
	       Test(); //①とりあえず親から関数を呼びたい!
	   }
       virtual void Test(){}; //②けど中身はまだ作らないよ(→virtual)
   }

   //子のクラス.
   class PlayerManager : public CharacterBase 
   {
       void Test() override; //③子で中身を作りますよ(→override)
   }

   //④その中身はこれですよ.
   void PlayerManager::Test(){
       ...
   }
/----------------------------------------------------*/
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"
#include "Steam_Revolver.h"
#include "CharacterBase.generated.h"

/// <summary>
/// 敵のstate列挙体.
/// UEではこういう書き方をするっぽい.
/// </summary>
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	ES_Alive UMETA(DisplayName = "Alive"),
	ES_Dead  UMETA(DisplayName = "Dead")
};

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
	JumpDown UMETA(DisplayName = "JumpDown"),
	Shot     UMETA(DisplayName = "Shot")
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


	//弾関連.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Gun|Ammunition")
	int32 MaxAmmoPerMagazine = 6;		//連続で弾を撃てる数.

	UPROPERTY(BlueprintReadOnly, Category = "MyProperty|Base|Gun|Ammunition")
	int32 CurrentAmmoCount = 6;			//弾の残り数.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Gun|Ammunition")
	float ReloadDuration = 2.5f;		//リロード時間(秒)

	UPROPERTY(BlueprintReadOnly, Category = "MyProperty|Base|Gun|Ammunition")
	bool  bIsReloading = false;			//リロードしているか.

	UPROPERTY(BlueprintReadOnly, Category = "MyProperty|Base|Gun|Ammunition")
	float ReloadTimerElapsed = 0.0f;	//リロード経過時間計測用.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Gun|Ammunition")
	float shotPosRandom = 1.0f;			//射撃の正確さ(どれだけずらすか)
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

	//アニメーション関係.
	float shotAnimTimer;
	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Animation")
	class UAnimMontage* ShotAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Animation|Time")
	float initShotAnimTime; //射撃アニメーション時間.
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
	//アニメーション更新.
	void UpdateAnimState(float DeltaTime);
	//アニメーション再生.
	void PlayAnimMontage(EAnimationState AnimState);
#pragma endregion

#pragma region "射撃"
	//[仮想関数] 弾発射処理.
	virtual void ShotBullet(){}
	//弾を発射する.
	bool ShotBulletExe(AActor* user, FVector targetPos);

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

#pragma region "ダメージ処理"
	UFUNCTION()
	virtual void OnBulletHit(){}	//[仮想関数] 弾が当たったら実行される.
	virtual void Die(){}			//[仮想関数] 死亡処理.
#pragma endregion
};