/*----------------------------------------------------/
   - CharacterBase -
   プレイヤーと敵の親クラス.

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

//他class.
#include "WeaponRevolver.h"

#include "CharacterBase.generated.h"

//前方宣言.
class ABulletBase;

/// <summary>
/// [enum] キャラクターの状態.
/// </summary>
UENUM(BlueprintType)
enum class ECharaState : uint8
{
	Alive UMETA(DisplayName = "Alive"),
	Dead  UMETA(DisplayName = "Dead")
};

/// <summary>
/// [enum] キャラクターのアニメーション状態. 
/// </summary>
UENUM(BlueprintType)
enum class ECharaAnimState : uint8
{
	Idle  UMETA(DisplayName = "Idle"), //停止.
	Move  UMETA(DisplayName = "Move"), //移動.
	Run   UMETA(DisplayName = "Run"),  //ダッシュ.
	Jump  UMETA(DisplayName = "Jump"), //ジャンプ.
	Shot  UMETA(DisplayName = "Shot"), //射撃.
	Roll  UMETA(DisplayName = "Roll")  //ローリング.
};

UCLASS()
class GUNACTION_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

//▼ ===== 変数 ===== ▼.
public:

#pragma region "移動"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Movement")
	float BaseLookUpRate = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Movement")
	float WalkSpeed = 200.0f; //歩速, 200くらいがちょうどいい.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Movement")
	float RunSpeed = 800.0f; //走速, 800くらい?

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MyProperty|Base|Movement")
	bool bIsDash;

	bool bWasInAir = false; //前フレームの空中フラグ.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MyProperty|Base|Movement")
	double CurrentSpeed;
#pragma endregion

#pragma region "ロール"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Roll")
	float RollCooldown = 1.0f;        // ロールのクールダウン時間（秒）.

	bool bIsRolling = false;          // ロール中フラグ.
	bool bCanRoll = true;			  // ロール可能フラグ（クールダウン用）.

	FTimerHandle RollCooldownTimer;   // クールダウン用タイマー.
#pragma endregion

#pragma region "弾"
	//弾クラスの参照.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Bullet")
	TSubclassOf<AActor> BulletClass;
	//弾の発射距離.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Bullet")
	float BulletTargetDistance = 10000.0f;

	FVector BulletTargetPosition; //弾の目標地点.
#pragma endregion

#pragma region "銃"
	//銃を持つか.
	//trueなら銃を召喚して持たせる, falseなら射撃のみ行う.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Gun")
	bool IsHaveGun;

	//[IsHaveGun = true]  銃をアタッチするソケット名.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Gun")
	FName GunAttachSocketName = TEXT("hand_r");
	//[IsHaveGun = false] 発射口となるソケット名.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Gun")
	FName MuzzleSocketName = TEXT("hand_r");

	//銃.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Gun")
	TSubclassOf<AWeaponRevolver> RevolverGunClass;
	//スポーンした銃を入れる用.
	UPROPERTY(BlueprintReadOnly, Category = "MyProperty|Base|Gun")
	AWeaponRevolver* RevolverGun;

	// ===== 射撃性能 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Gun|Status")
	int32 MaxAmmoCount = 6;				//連続で弾を撃てる数.
	UPROPERTY(BlueprintReadOnly, Category = "MyProperty|Base|Gun|Status")
	int32 AmmoCount = 6;				//弾の残数.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Gun|Status")
	float ReloadDuration = 2.5f;		//リロード時間(秒)
	UPROPERTY(BlueprintReadOnly, Category = "MyProperty|Base|Gun|Status")
	bool  bIsReloading = false;			//リロードしているか.
	UPROPERTY(BlueprintReadOnly, Category = "MyProperty|Base|Gun|Status")
	float ReloadTimerElapsed = 0.0f;	//リロード経過時間計測用.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Gun|Status")
	float shotPosRandom = 0.0f;			//射撃の正確さ(どれだけずらすか)
#pragma endregion

#pragma region "アニメーション"
	//腕のボーンインデックスをキャッシュ.
	UPROPERTY(VisibleAnywhere, Category = "MyProperty|Base|Animation")
	int32 RightArmBoneIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, Category = "MyProperty|Base|Animation")
	int32 RightForearmBoneIndex = INDEX_NONE;

	//アニメーション関係.
	float shotAnimTimer;
	
	UPROPERTY(VisibleAnywhere, Category = "MyProperty|Base|Animation")
	ECharaAnimState CurrentAnimationState; //現在のアニメーション状態.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Animation")
	UAnimMontage* IdleAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Animation")
	UAnimMontage* MoveAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Animation")
	UAnimMontage* SprintAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Animation")
	UAnimMontage* JumpAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Animation")
	UAnimMontage* ShotAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Animation")
	UAnimMontage* RollAnimMontage; //ローリングアニメーション.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Animation|Time")
	float initShotAnimTime; //射撃アニメーション時間.
#pragma endregion

#pragma region "死亡"
	//死亡時のエフェクト.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Death")
	class UParticleSystem* DeathEffect;
	//死亡時のサウンド.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Base|Death")
	class USoundBase* DeathSound;
#pragma endregion

//▼ ===== 関数 ===== ▼.
protected:
#pragma region "基本処理"
	ACharacterBase();								//コンストラクタ.
	virtual void BeginPlay() override;				//召喚した瞬間.
	virtual void Tick(float DeltaTime) override;	//常に実行.
#pragma endregion

public:
#pragma region "移動"
	//移動処理.
	void Move(FVector WorldDirection, float ScaleValue, bool bForce = false);
	//歩行操作.
	void OnWalkStart();
	void OnWalkStop();
#pragma endregion

#pragma region "ローリング(回避)"
	void OnRoll();                  // ロール入力.
	void RollEnd();                 // ロール終了処理.
#pragma endregion

#pragma region "銃"
	void OnReload();										//リロード開始処理.
	void InitBoneIndices();									//ボーンインデックスを初期化する関数.
	void UpdateReloadTimer(float DeltaTime);				//リロード時間の更新.
	void EquipGun();										//銃を装備する.
	void RotateArmBones(const FRotator& TargetRotation);	//腕のボーンを回転させる関数.
#pragma endregion

#pragma region "射撃"
	//発射チェック.
	bool IsShotAble();
	//射撃開始.
	void ShotStart();
	//射撃実行[仮想関数]
	virtual void ShotExe(){};
	//弾を召喚.
	bool SpawnBullet(TObjectPtr<ACharacterBase> user, FVector targetPos);
#pragma endregion

#pragma region "ダメージ・死亡"
	UFUNCTION()
	virtual void OnBulletHit(){}	//弾が当たったら実行される.[仮想関数]
	virtual void Death(){}			//死亡処理.                [仮想関数]

	void PlayDeathAnimation();		//死亡アニメーション再生.
	void PlayDeathEffect();			//死亡エフェクト再生.
	void PlayDeathSound();			//死亡音再生.

	void DisableComponents();		//コンポーネント無効化.
#pragma endregion

#pragma region "アニメーション"
	//アニメーション更新.
	void UpdateAnim(float DeltaTime);
	void UpdateAnimJump(bool bIsInAir);
	//アニメーション再生.
	float MyPlayAnim(ECharaAnimState AnimState);
#pragma endregion
};